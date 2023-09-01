/**=============================================================================
Copyright (c) 2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "sobel3x3_v60.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "verify.h"
#include "rpcmem.h" // helper API's for shared buffer allocation

#define USE_ION_MEMORY   // to demonstrate the performance difference between ION and HLOS memory for sharing with ADSP.
#define WORKAROUND_OOB_ACCESS // over-allocate in case implementation has bugs that read out of bounds

#ifndef USE_ION_MEMORY
#define rpcmem_init()
#define rpcmem_deinit()
#define rpcmem_alloc(a, b, c) memalign(4096, (c)) // simulate allocation from page boundary (4 KB)
#define rpcmem_free(a) free((a))
#endif

#ifdef __hexagon__     // some defs/stubs so app can build for Hexagon simulation
#include "hexagon_sim_timer.h"
#define GetTime hexagon_sim_read_pcycles        // For Hexagon sim, use PCycles for profiling
#include "hexagon_cache.h" // for removing buffers from cache during simulation/profiling
#include <assert.h>

#else
#include <assert.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

unsigned long long GetTime( void )
{
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

#endif

// Define ION heap ID (system heap) for use with HVX/Compute DSP's. 
// This is the recommended heap for use with any DSP with an SMMU, 
// which includes all cDSP's and aDSP's with HVX. (This may already 
// be defined in rpcmem.h, depending on version of that file).
#ifndef ION_HEAP_ID_SYSTEM
#define ION_HEAP_ID_SYSTEM 25
#endif

#define VLEN            128

static void sobel_ref(const unsigned char* src,
                      unsigned int width,
                      unsigned int height,
                      unsigned int stride,
                      unsigned char* dst)

{
    unsigned char p00, p01, p02,p10,p12,p20,p21,p22;
    unsigned int SUM;
    unsigned int i, j, idx, H, V;

    for (i = 1; i < height-1 ; i++)
    {
        for (j = 1; j < width-1; j++)
        {
           idx = i*stride + j;
           p00 = src[idx - stride -1];
           p01 = src[idx - stride   ];
           p02 = src[idx - stride +1];
           p10 = src[idx             -1];
           p12 = src[idx             +1];
           p20 = src[idx + stride -1];
           p21 = src[idx + stride   ];
           p22 = src[idx + stride +1];
           H =   p00 + 2*p01 + p02
               - p20 - 2*p21 - p22;
           V =   p00 + 2*p10 + p20
               - p02 - 2*p12 - p22;
           SUM = abs((int)H) + abs((int)V);
           dst[i*stride + j] = (SUM > 255) ? 255 : (unsigned char)SUM;
        }
    }
}


int test_main_start(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}


int test_main_start(int argc, char* argv[])
{
    int WIDTH = 1920;
    int HEIGHT = 1080;
#ifdef __hexagon__
    WIDTH = 256;        // smaller size on simulator for faster run time
    HEIGHT = 64;
#endif
    int LOOPS = 1;
    int USEC = 33333;
    int MIPS = 1000;
    int MBPS = 12000;
#ifndef __hexagon__
    int VERBOSE = 0;
#endif

#ifndef __hexagon__
    if (argc == 1) 
    {
        printf("Usage: sobel3x3_v60 <width (must be multiple of 128)> <height> <number of frames> <uSec per frame> <DSP MIPS vote> <BUS MBPS vote> <verbose (0/1)>\n");
        printf("No arguments given, using defaults: %d %d %d %d %d %d %d.\n",WIDTH,HEIGHT,LOOPS,USEC,MIPS,MBPS,VERBOSE);
    }
    else if (argc != 8)
    {
        printf("Usage: sobel3x3_v60 <width (must be multiple of 128)> <height> <number of frames> <uSec per frame> <DSP MIPS vote> <BUS MBPS vote> <verbose (0/1)>\n");
        return -1;
    }
    else
    {
        WIDTH = atoi(argv[1]);
        HEIGHT = atoi(argv[2]);
        LOOPS = atoi(argv[3]);
        USEC = atoi(argv[4]);
        MIPS = atoi(argv[5]);
        MBPS = atoi(argv[6]);
        VERBOSE = atoi(argv[7]);
    }
#endif
    
	uint8_t *srcRaw=NULL, *src=NULL, *dst=NULL, *ref=NULL;
	uint32_t width = WIDTH;              
	uint32_t height = HEIGHT;
	uint32_t stride = WIDTH;    // keep aligned to 128 bytes!
    int dspUsec;
	int retVal;
	int i,j;
    int nErr = 0;

	int size = stride * height;

	rpcmem_init();

    // call dspCV_initQ6_with_attributes() to bump up Q6 clock frequency
    // Since this app is not real-time, and can fully load the DSP clock & bus resources 
    // throughout its lifetime, vote for the maximum available MIPS & BW.
    dspCV_Attribute attrib[] =
    {
        {DSP_TOTAL_MCPS, MIPS},                
        {DSP_MCPS_PER_THREAD, MIPS/2},       
        {PEAK_BUS_BANDWIDTH_MBPS, MBPS},     
        {BUS_USAGE_PERCENT, 50},            
    };

	retVal = dspCV_initQ6_with_attributes(attrib, sizeof(attrib)/sizeof(attrib[0]));
	printf("return value from dspCV_initQ6() : %d \n", retVal);
    VERIFY(0 == retVal);

	// allocate ion buffers
#ifdef WORKAROUND_OOB_ACCESS
    VERIFY(0 != (srcRaw = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size + 1024)));
#else
    VERIFY(0 != (srcRaw = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)));
#endif
    printf("src - allocated %d\n", (int)size);
    VERIFY(0 != (dst = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)));
    printf("dst - allocated %d\n", (int)size);

    VERIFY(0 != (ref = (uint8_t*)memalign(VLEN, size)));
    printf("ref - allocated %d (via malloc)\n", (int)size);

    src = srcRaw;
#ifdef WORKAROUND_OOB_ACCESS
    src += 512;
#endif
   
    // generate pseudo-random image
    uint32 m_w = 0x12349876;
    uint32 m_z = 0xabcd4321;
	for (j = 0; j < height; j++)
	{
		uint8_t *ptr = &src[j * stride];
		for (i = 0; i < width; i++)
		{
            m_z = 36969 * (m_z & 65535) + (m_z >> 16);
            m_w = 18000 * (m_w & 65535) + (m_w >> 16);
			*ptr++ = m_w;				
        }
	}

    // call API
	printf( "calling sobel3x3_v60 on a %dx%d image...\n", (int)width, (int)height);

#ifdef __hexagon__
    // simulate condition where src and dst are not already in cache (for profiling purposes)
    hexagon_buffer_cleaninv (src, size);
    hexagon_buffer_cleaninv (dst, size);
#endif

	unsigned long long totalTime = 0;
    unsigned long long t0 = GetTime();
    unsigned long long nextFrame = t0 + USEC;
    for (i = 0; i < LOOPS; i++)
    {
        unsigned long long t1 = GetTime();

        // For HVX case, note that src, stride, dst all must be multiples of 128 bytes.
        // The HVX code for this example function does not handle unaligned inputs.
        retVal = sobel3x3_v60_sobel(src, size, stride, width, height, dst, size, (int32*)&dspUsec);
        unsigned long long t2 = GetTime();
#ifndef __hexagon__
        // unless within a msec of next frame, sleep to emulate desired frame rate.
        if (t2 < nextFrame - 1000)
        {
            usleep(nextFrame - t2);
        }
        else printf ("Warning - unable to keep real time at %d uSec per frame!!\n",USEC);
        if (VERBOSE) printf("total time %d uSec, DSP-measured %d uSec\n", (int)(t2-t1), dspUsec);
#endif        
        nextFrame += USEC;
        totalTime += (t2 - t1);
    }

    VERIFY(0 == retVal);
#ifdef __hexagon__
    printf("run time of sobel3x3_v60: %llu PCycles for %d iterations\n", totalTime, LOOPS);
#else
	printf("run time of sobel3x3_v60: %llu microseconds for %d iterations. Last iteration DSP-measured time: %d uSec\n", totalTime, LOOPS, dspUsec);
#endif
	printf("return value from sobel3x3_v60: %d \n", retVal);

    // validate results
    sobel_ref (src, width, height, stride, ref);
    
    int bitexactErrors = 0;
	printf( "Checking for bit-exact errors... \n");
	for (j = 1; j < height-1; j++)
	{
		for (i = 1; i < width-1; i++)
		{
            if (ref[j * stride + i] != dst[j * stride + i])
            {
                bitexactErrors++;
                if (bitexactErrors < 10 )printf("Bit exact error: j=%d, i=%d, refval=%d, dst=%d\n",j,i,ref[j * stride + i], dst[j * stride + i]);
            }
        }
	}
	printf( "Number of bit-exact errors: %d \n", bitexactErrors);
    VERIFY(0 == bitexactErrors);
    
 bail:
    if(nErr) {
        printf("error: %d\n", nErr);
    }
    if(srcRaw) { 
        rpcmem_free(srcRaw);
    }
    if(dst) { 
        rpcmem_free(dst);
    }
    // free ion buffers
    rpcmem_deinit();

    if(ref) free(ref);

	printf("calling dspCV_deinitQ6()... \n");
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

