/**=============================================================================
Copyright (c) 2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#include "fast9.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "verify.h"
#include "rpcmem.h"

#if defined(__hexagon__)
#include "hexagon_sim_timer.h"
#include "hexagon_cache.h"
#else
#include <unistd.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#endif

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define USE_ION_MEMORY           // to demonstrate the performance difference between ION and HLOS memory for sharing with ADSP.

#ifndef USE_ION_MEMORY
#define rpcmem_init()
#define rpcmem_deinit()
#define rpcmem_alloc(a, b, c) memalign(4096, (c)) // simulate allocation from page boundary (4 KB)
#define rpcmem_free(a) free((a))
#endif

#if defined(__hexagon__)
#define GetTime hexagon_sim_read_pcycles          // For Hexagon sim, use PCycles for profiling
#endif

#define LOOPS 1                  // reduce the profiling impact of dynamic loading. Load once, and run FastRPC+processing many times.
#define WIDTH 1920               // input image width
#define HEIGHT 1080              // input image height
#if defined(__hexagon__)
#define OUTF "out.raw"
#define INF \
    "football1920x1080.bin"
#else
#define OUTF "/data/local/out.raw"
#define INF \
    "/data/local/football1920x1080.bin"
#endif

// Define ION heap ID (system heap) for use with HVX/Compute DSP's. 
// This is the recommended heap for use with any DSP with an SMMU, 
// which includes all cDSP's and aDSP's with HVX. (This may already 
// be defined in rpcmem.h, depending on version of that file).
#ifndef ION_HEAP_ID_SYSTEM
#define ION_HEAP_ID_SYSTEM 25
#endif

/*===========================================================================
    GLOBAL VARIABLES
===========================================================================*/

/*===========================================================================
    DECLARATIONS
===========================================================================*/
int test_main_start(int argc, char* argv[]);

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
#ifdef __cplusplus
extern "C" {
#endif
#if defined(__hexagon__)
/* ======================================================================== */
int apps_mem_heap_init(int id, int flags, int rflags, int size)
{
   return 0;
}

/* ======================================================================== */
void* apps_mem_heap_malloc(int size)
{
    return malloc(size);
}

/* ======================================================================== */
void apps_mem_heap_free(void* po)
{
    return free(po);
}

/* ======================================================================== */
int adsp_pls_add_lookup(uint32 type, uint32 key, int size, int (*ctor)(void* ctx, void* data), void* ctx, void (*dtor)(void*), void** ppo)
{
   return 0;
}

#else
/* ======================================================================== */
unsigned long long GetTime( void )
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}
#endif
#ifdef __cplusplus
}
#endif

/* ======================================================================== */
int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}

/* ======================================================================== */
int test_main_start(int argc, char* argv[])
{
    uint8_t *src=NULL;
    short *dst=NULL;
    uint32_t srcWidth = WIDTH;
    uint32_t srcHeight = HEIGHT;
    uint32_t srcStride = WIDTH;    // keep aligned to 128 bytes!
    uint32_t dstHeight = srcHeight;
    uint32_t dstStride = srcStride;
    int retVal;
    int i;
    int nErr = 0;
    int maxnumcorners    = 3000;
    unsigned int barrier = 50;
    unsigned int border  = 3;
    int numcorners = 0;

    int srcSize = srcStride * srcHeight;
    int dstSize = dstStride * dstHeight;

    /* -----------------------------------------------------*/
    /*  Initialization                                      */
    /* -----------------------------------------------------*/
    // Initialize ION memory utility
    rpcmem_init();

    /* -----------------------------------------------------*/
    /*  HW Setup                                            */
    /* -----------------------------------------------------*/
    // These clock votes will configure for Turbo voltage/clocks
    // Lower these if power is more important than latency.
    dspCV_Attribute attrib[] =
    {
        {DSP_TOTAL_MCPS, 1000},                
        {DSP_MCPS_PER_THREAD, 1000/2},       
        {PEAK_BUS_BANDWIDTH_MBPS, 12000},     
        {BUS_USAGE_PERCENT, 50},            
    };

	retVal = dspCV_initQ6_with_attributes(attrib, sizeof(attrib)/sizeof(attrib[0]));
	printf("return value from dspCV_initQ6() : %d \n", retVal);
    VERIFY(0 == retVal);

    /* -----------------------------------------------------*/
    /*  Allocate memory                                     */
    /* -----------------------------------------------------*/
    // allocate ion buffers
    VERIFY(0 != (src = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, srcSize + 128)));
    printf("src - allocated ION %d\n", (int)srcSize + 128);
    VERIFY(0 != (dst = (short*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, dstSize)));
    printf("dst - allocated ION %d\n", (int)dstSize);

    /* -----------------------------------------------------*/
    /*  Read image input from file                          */
    /* -----------------------------------------------------*/
    FILE *fp;
    fp = fopen(INF, "rb");
    if (!fp) {
        printf("Error: Cannot open for input\n");
        VERIFY(0);
    }
    int size;

    size = fread(src, WIDTH*HEIGHT, 1, fp);

    if (size == 0) //NULL)
    {
        printf("Error, Unable to read - size %d \n", size);
        fclose(fp);
        VERIFY(0);
    }

    fclose(fp);

#ifdef __hexagon__
    // simulate condition where src and dst are not already in cache (for profiling purposes)
    hexagon_buffer_cleaninv (src, srcSize);
    hexagon_buffer_cleaninv ((unsigned char*)dst, dstSize);
#endif

    /* -----------------------------------------------------*/
    /*  Call function                                       */
    /* -----------------------------------------------------*/
    unsigned long long t1 = GetTime();
    for (i = 0; i < LOOPS; i++)
    {
        // For HVX case, note that src, srcStride, dst, dstStride all must be multiples of 128 bytes.
        // The HVX code for this example function does not handle unaligned inputs.
        retVal = fast9_fast9(src, srcSize, srcStride, srcWidth, srcHeight, barrier, border, dst, dstSize/sizeof(short), maxnumcorners, &numcorners);
    }
    unsigned long long t2 = GetTime();
    VERIFY(0 == retVal);

    printf("%d features have been detected.\n",numcorners);
#ifdef __hexagon__
    printf("run time of fast9_fast9: %llu PCycles (from %llu-%llu) for %d iterations\n", t2-t1, t1, t2, LOOPS);
    printf("To apply timefilter to profiling results, add this to simulation cmd line: --dsp_clock 800 --timefilter_ns %d-%d\n", (int)(t1/0.8), (int)(t2/0.8));
#else
    printf("run time of fast9_fast9: %llu microseconds for %d iterations\n", t2-t1, LOOPS);
#endif

    /* -----------------------------------------------------*/
    /*  Write image out                                     */
    /* -----------------------------------------------------*/
    fp = fopen(OUTF, "wb");
    assert(fp);

    size = fwrite(dst, sizeof(dst[0]), 2*numcorners, fp);
    if (size != 2*numcorners)
    {
        printf("Error, Unable to write - size %d \n", size);
        fclose(fp);
        VERIFY(0);
    }

    fclose(fp);
    printf("File write done \n");

    /* -----------------------------------------------------*/
    /*  Cleanup                                             */
    /* -----------------------------------------------------*/
 bail:
    if(nErr) {
        printf("error: %d\n", nErr);
    }
    if(src) {
        rpcmem_free(src);
    }
    if(dst) {
        rpcmem_free(dst);
    }
    // free ion buffers
    rpcmem_deinit();

    retVal = dspCV_deinitQ6();
    printf("return value from dspCV_deinitQ6(): %d \n", retVal);

    if (0 == (retVal | nErr))
    {
        printf("- success\n");
        return 0;
    }
    else
    {
        printf("- failure\n");
        return -1;
    }
}

/* ======================================================================== */
