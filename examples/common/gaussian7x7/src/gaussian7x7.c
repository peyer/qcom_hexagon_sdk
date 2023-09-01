#include "gaussian7x7.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "verify.h"
#include "rpcmem.h" // helper API's for shared buffer allocation

#define USE_ION_MEMORY   // to demonstrate the performance difference between ION and HLOS memory for sharing with ADSP.

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

#define LOOPS 1

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
#define LOOPS 1000          // reduce the profiling impact of dynamic loading. Load once, and run FastRPC+processing many times.

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

/* ======================================================================== */
/*  Reference C version of gaussian7x7u8()                                  */
/* ======================================================================== */
static const int GAUSS_7x7[7*7] = {
   1,   6,  15,  20,  15,   6,  1,
   6,  36,  90, 120,  90,  36,  6,
  15,  90, 225, 300, 225,  90, 15,
  20, 120, 300, 400, 300, 120, 20,
  15,  90, 225, 300, 225,  90, 15,
   6,  36,  90, 120,  90,  36,  6,
   1,   6,  15,  20,  15,   6,  1
};


static void Gaussian7x7u8_ref(
    unsigned char *src,
    int            width,
    int            height,
    int            stride,
    unsigned char *dst,
    int            dstStride
    )
{

    int x, y, s, t;
    int sum, out;

    for (y = 3; y < height - 3; y++)
    {
        for (x = 3; x < width - 3; x++)
        {
            sum = 0;

            for (t = -3; t <= 3; t++)
            {
                for (s = -3; s <= 3; s++)
                {
                    sum += src[(y+t)*stride + x+s] * GAUSS_7x7[((t+3)*7)+(s+3)];
                }
            }

            out  = sum >> 12;

            out = out < 0 ? 0 : out > 255 ? 255 : out;

            dst[y*dstStride + x] = (unsigned char)out;
        }
    }
}

int test_main_start(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}

#define WIDTH					1920
#define HEIGHT 					1080

int test_main_start(int argc, char* argv[])
{
	uint8_t *src=NULL, *dst=NULL, *ref=NULL;
	uint32_t srcWidth = WIDTH;              
	uint32_t srcHeight = HEIGHT;
#ifdef __hexagon__ 
    srcWidth = 256;        // smaller size on simulator for faster run time
    srcHeight = 64;
#endif
	uint32_t srcStride = srcWidth;    // keep aligned to 128 bytes!
	uint32_t dstWidth = srcWidth;   // keep aligned to 128 bytes!
	uint32_t dstHeight = srcHeight;
	uint32_t dstStride = srcStride;
	int retVal;
	int i,j;
    int nErr = 0;

	int srcSize = srcStride * srcHeight;
	int dstSize = dstStride * dstHeight;

	rpcmem_init();

    // call dspCV_initQ6_with_attributes() to bump up Q6 clock frequency
    // Since this app is not real-time, and can fully load the DSP clock & bus resources 
    // throughout its lifetime, vote for the maximum available MIPS & BW.
    dspCV_Attribute attrib[] =
    {
        {DSP_TOTAL_MCPS, 1000},                 // Slightly more MCPS than are available on current targets
        {DSP_MCPS_PER_THREAD, 500},             // drive the clock to MAX on known targets
        {PEAK_BUS_BANDWIDTH_MBPS, 12000},       // 12 GB/sec is slightly higher than the max realistic max BW on existing targets.
        {BUS_USAGE_PERCENT, 100},               // This app is non-real time, and constantly reading/writing memory
    };

	retVal = dspCV_initQ6_with_attributes(attrib, sizeof(attrib)/sizeof(attrib[0]));
	printf("return value from dspCV_initQ6() : %d \n", retVal);
    VERIFY(0 == retVal);

	// allocate ion buffers
    VERIFY(0 != (src = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, srcSize)));
    printf("src - allocated %d\n", (int)srcSize);
    VERIFY(0 != (dst = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, dstSize)));
    printf("dst - allocated %d\n", (int)dstSize);

    VERIFY(0 != (ref = (uint8_t*)malloc(dstSize)));
    printf("ref - allocated %d (via malloc)\n", (int)dstSize);

	// populate src buffer (with a simple pattern)
	for (j = 0; j < srcHeight; j++)
	{
		uint8_t *ptr = &src[j * srcStride];
		for (i = 0; i < srcWidth; i++)
		{
			*ptr++ = i + j;				// some incrementing pattern fill
        }
	}

    // call API
	printf( "calling gaussian7x7_Gaussian7x7u8 on a %dx%d image...\n", (int)srcWidth, (int)srcHeight);

#ifdef __hexagon__
    // simulate condition where src and dst are not already in cache (for profiling purposes)
    hexagon_buffer_cleaninv (src, srcSize);
    hexagon_buffer_cleaninv (dst, dstSize);
#endif

	unsigned long long t1 = GetTime();
    for (i = 0; i < LOOPS; i++)
    {
        // For HVX case, note that src, srcStride, dst, dstStride all must be multiples of 128 bytes.
        // The HVX code for this example function does not handle unaligned inputs.
        retVal = gaussian7x7_Gaussian7x7u8(src, srcSize, srcWidth, srcHeight, srcStride, dst, dstSize, dstStride);
    }

    unsigned long long t2 = GetTime();
    VERIFY(0 == retVal);
#ifdef __hexagon__
    printf("run time of gaussian7x7_Gaussian7x7u8: %llu PCycles (from %llu-%llu) for %d iterations\n", t2-t1, t1, t2, LOOPS);
    printf("To apply timefilter to profiling results, add this to simulation cmd line: --dsp_clock 800 --timefilter_ns %d-%d\n", (int)(t1/0.8), (int)(t2/0.8));
#else
	printf("run time of gaussian7x7_Gaussian7x7u8: %llu microseconds for %d iterations\n", t2-t1, LOOPS);
#endif
	printf("return value from gaussian7x7_Gaussian7x7u8: %d \n", retVal);

    // validate results
    Gaussian7x7u8_ref(src, srcWidth, srcHeight, srcStride, ref, dstStride);
    int bitexactErrors = 0;
	printf( "Checking for bit-exact errors... \n");
	for (j = 3; j < dstHeight-3; j++)
	{
		for (i = 3; i < dstWidth-3; i++)
		{
            if (ref[j * dstStride + i] != dst[j * dstStride + i])
            {
                bitexactErrors++;
//                printf("Bit exact error: j=%d, i=%d, refval=%d, dst=%d\n",j,i,ref[j * dstStride + i], dst[j * dstStride + i]);
            }
        }
	}
	printf( "Number of bit-exact errors: %d \n", bitexactErrors);
    VERIFY(0 == bitexactErrors);
    
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

