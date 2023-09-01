#include "downscaleBy2.h"
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

int test_main_start(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}

#define WIDTH					1920
#define HEIGHT 					1080

int test_main_start(int argc, char* argv[])
{
	uint8_t *src=NULL, *dst=NULL;
	uint32_t srcWidth = WIDTH;              
	uint32_t srcHeight = HEIGHT;
	uint32_t srcStride = WIDTH;    // keep aligned to 128 bytes!
	uint32_t dstWidth = srcWidth / 2;   // keep aligned to 128 bytes!
	uint32_t dstHeight = srcHeight / 2;
	uint32_t dstStride = srcStride;
	int retVal;
	int i,j;
    int nErr = 0;

	int srcSize = srcStride * srcHeight;
	int dstSize = dstStride * dstHeight;

	// allocate ion buffers
	rpcmem_init();
    VERIFY(0 != (src = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, srcSize)));
    printf("src - allocated %d\n", (int)srcSize);
    VERIFY(0 != (dst = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, dstSize)));
    printf("dst - allocated %d\n", (int)dstSize);

 // call dspCV_initQ6_with_attributes() to define Q6 clock and bus frequencies.
 // Since this app is not real-time, and can fully load the DSP clock & bus resources 
 // throughout its lifetime, vote for the maximum available MIPS & BW. The selection of values
 // in this initialization is crucial in defining the desired power vs. performance trade-off. 
 dspCV_Attribute attrib[] =
 {
 // The below values will result in the maximum aDSP performance, at Turbo voltage.
     {DSP_TOTAL_MCPS, 1000},                 // Slightly more MCPS than are available on current targets
     {DSP_MCPS_PER_THREAD, 500},             // drive the clock to MAX on known targets
     {PEAK_BUS_BANDWIDTH_MBPS, 12000},       // 12 GB/sec is slightly higher than the max realistic max BW on existing targets.
     {BUS_USAGE_PERCENT, 100},               // This app is non-real time, and constantly reading/writing memory
 // The below values will result in performance at nominal voltage initially, 
 // and allow DCVS to move up and down as it deems optimal.
 //    {DSP_TOTAL_MCPS, 600},                 // Within nominal range (for 8996) for initial performance setting
 //    {DSP_MCPS_PER_THREAD, 50},             // A low setting here allows DCVS to drop clock as low as it deems possible over time
 //    {PEAK_BUS_BANDWIDTH_MBPS, 6000},       // A BW within nominal range (for 8996).
 //    {BUS_USAGE_PERCENT, 50},               // This app is non-real time, and constantly reading/writing memory
 };

	retVal = dspCV_initQ6_with_attributes(attrib, sizeof(attrib)/sizeof(attrib[0]));
	printf("return value from dspCV_initQ6() : %d \n", retVal);
    VERIFY(0 == retVal);

   // generate pseudo-random image
   uint32 m_w = 0x12349876;
   uint32 m_z = 0xabcd4321;
	for (j = 0; j < srcHeight; j++)
	{
		uint8_t *ptr = &src[j * srcStride];
		for (i = 0; i < srcWidth; i++)
		{
            m_z = 36969 * (m_z & 65535) + (m_z >> 16);
            m_w = 18000 * (m_w & 65535) + (m_w >> 16);
			*ptr++ = m_w;				
        }
	}

    // call API
	printf( "calling downscaleBy2_scaleDownBy2 on a %dx%d image...\n", (int)srcWidth, (int)srcHeight);

#ifdef __hexagon__
    // simulate condition where src and dst are not already in cache (for profiling purposes)
    hexagon_buffer_cleaninv (src, srcSize);
    hexagon_buffer_cleaninv (dst, dstSize);
#endif

    unsigned long profResult = 0;
	unsigned long long t1 = GetTime();
    for (i = 0; i < LOOPS; i++)
    {
        // For HVX case, note that src, srcStride, dst, dstStride all must be multiples of 128 bytes.
        // The HVX code for this example function does not handle unaligned inputs.
        retVal = downscaleBy2_scaleDownBy2(src, srcSize, srcWidth, srcHeight, srcStride, dst, dstSize, dstStride, (uint32*)(&profResult));
    }

    unsigned long long t2 = GetTime();
    VERIFY(0 == retVal);
#ifdef __hexagon__
    printf("run time of downscaleBy2_scaleDownBy2: %llu PCycles (from %llu-%llu) for %d iterations\n", t2-t1, t1, t2, LOOPS);
    printf("To apply timefilter to profiling results, add this to simulation cmd line: --dsp_clock 800 --timefilter_ns %d-%d\n", (int)(t1/0.8), (int)(t2/0.8));
#else
	printf("run time of downscaleBy2_scaleDownBy2: %llu microseconds for %d iterations\n", t2-t1, LOOPS);
	printf("DSP-measured duration (for single, last invocation): %u microseconds \n", (unsigned int)profResult);
#endif
	printf("return value from downscaleBy2_scaleDownBy2: %d \n", retVal);

    // validate results
    int bitexactErrors = 0;
	for (j = 0; j < dstHeight; j++)
	{
        uint8_t *ptr = &dst[j * dstStride];
		for (i = 0; i < dstWidth; i++)
		{
			uint8_t refval = (src[j * 2 * srcStride + i * 2] + src[j * 2 * srcStride + i * 2 + 1] 
                + src[(j * 2 + 1) * srcStride + i * 2] + src[(j * 2 + 1) * srcStride + i * 2 + 1]) / 4;
            if (refval != *ptr)
            {
            if (bitexactErrors < 10) printf("bit exact error: j=%d, i=%d, refval=%d, dst=%d\n",j,i,refval,*ptr);
            }
            bitexactErrors += (refval != *ptr++);
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

