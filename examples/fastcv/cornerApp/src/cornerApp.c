#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "verify.h"
#include <stdio.h>
#include "cornerApp.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include "rpcmem.h"

#ifdef __hexagon__     // some defs so app can build for Android or Hexagon simulation
#include "hexagon_sim_timer.h"
#define GetTime hexagon_sim_read_pcycles        // For Hexagon sim, use PCycles for profiling
#define LOOPS 1

#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#define LOOPS 100          // reduce the profiling impact of dynamic loading. Load once, and run FastRPC+processing many times.

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

#define MAX_NUM_CORNERS 100
#define XY_SIZE	2*MAX_NUM_CORNERS



static uint8_t *dataBuf = NULL;
static uint8_t *blurredImgBuf = NULL;
static uint32_t *corners = NULL;
static uint32_t *renderBuf = NULL;

void deinitQ6(void)
{
	int nErr = 0;
	if (dataBuf) rpcmem_free(dataBuf);
	if (corners) rpcmem_free(corners);
	if (blurredImgBuf) rpcmem_free(blurredImgBuf);
	if (renderBuf) rpcmem_free(renderBuf);

	VERIFY(0 == dspCV_deinitQ6());
	return;

	bail:
	if(nErr) {
		printf("error in deinitQ6: %d\n", nErr);
	}
}
int initQ6(uint32_t srcHeight, uint32_t srcWidth)
{
	int nErr = 0;

    dspCV_Attribute attrib[] =
    {
        {DSP_TOTAL_MCPS, 1000},                 // Slightly more MCPS than are available on current targets
        {DSP_MCPS_PER_THREAD, 500},             // drive the clock to MAX on known targets
        {PEAK_BUS_BANDWIDTH_MBPS, 12000},       // 12 GB/sec is slightly higher than the max realistic max BW on existing targets.
        {BUS_USAGE_PERCENT, 100},               // This app is non-real time, and constantly reading/writing memory
    };

	nErr = dspCV_initQ6_with_attributes(attrib, sizeof(attrib)/sizeof(attrib[0]));
    if (nErr){
		printf("error in dspCV_initQ6: %d\n", nErr);
        return(1);
	}

	// allocate ion buffers
	VERIFY(0 != (dataBuf = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, srcWidth*srcHeight*3/2)));
	VERIFY(0 != (blurredImgBuf = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, srcWidth*srcHeight)));
	VERIFY(0 != (renderBuf = (uint32_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, srcWidth*srcHeight * 3 / 2 *sizeof(uint32_t))));
	VERIFY(0 != (corners = (uint32_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, XY_SIZE*sizeof(uint32_t))));

	return 0;

	bail:
	printf("out of memory in initQ6: %d\n", nErr);
    deinitQ6();
	return 2;
}

void compareResults(int arg1, int arg2, const char* compareStr)
{
	printf("%s\t Expected: %d\tGot: %d\n", compareStr, arg2, arg1);
	return;
}

int test_main_start(int argc, char* argv[]) {
	uint32_t srcWidth;
	uint32_t srcHeight;
	int retVal, i;

	uint32_t numCornersDetected = 0;        // DSP handle
	uint32_t cornerThreshold = 20;

	rpcmem_init();

	srcWidth = 800;
	srcHeight = 480;

    // call dspCV_initQ6_with_attributes() to bump up Q6 clock frequency
    // Since this app is not real-time, and can fully load the DSP clock & bus resources 
    // throughout its lifetime, vote for the maximum available MIPS & BW.
	retVal = initQ6(srcHeight, srcWidth);
    if (retVal)
    {
        printf("init failed!! status %d\n",retVal);
        rpcmem_deinit();
        return retVal;
    }
    printf("initq6 done...\n");
    if (retVal)
    {
        printf("init failed!! status %d\n",retVal);
        rpcmem_deinit();
        return retVal;
    }

	memcpy(dataBuf, tmpcorner_test2, srcWidth * srcHeight);

	unsigned long long t1 = GetTime();
    int anyFailures = 0;
    for (i = 0; i < LOOPS; i++)
    {
	    retVal = cornerApp_filterGaussianAndCornerFastQ(
			dataBuf, srcWidth*srcHeight * 3/2,
			srcWidth,
			srcHeight,
			blurredImgBuf, srcWidth * srcHeight,
			0,	// blurBorder (fcvFilterGaussian)
			srcWidth,  // srcStride (fcvCornerFast)
			cornerThreshold,
			7,
			(uint32*)corners, XY_SIZE,
			MAX_NUM_CORNERS,
			(uint32*)&numCornersDetected,
			(uint32*)renderBuf, srcWidth * srcHeight / 2);

            anyFailures |= retVal;
    }
    unsigned long long t2 = GetTime();

    if (AEE_SUCCESS != retVal)
    {
        printf("corner detection returned error code %d\n", retVal);
    }
    else
    {
#if 0
        uint32_t *cornerPtr = corners;
        for (i = 0; i < numCornersDetected; i++)
        {
            uint32_t x = *(cornerPtr++);
            uint32_t y = *(cornerPtr++);
            printf("corner #:%d \t (x,y): %d, %d\n", i, (int)x, (int)y);
        }
#endif
        printf("Num corners detected: %d. Expected: 60 \n", (int)numCornersDetected);
        anyFailures |= (numCornersDetected != 60);
    }
    
#ifdef __hexagon__
    printf("run time of corner detection: %llu PCycles (from %llu-%llu) for %d iterations\n", t2-t1, t1, t2, LOOPS);
    printf("To apply timefilter to profiling results, add this to simulation cmd line: --dsp_clock 600 --timefilter_ns %d-%d\n", (int)(t1/0.6), (int)(t2/0.6));
#else
	printf("run time of corner detection: %llu microseconds for %d iterations\n", t2-t1, LOOPS);
#endif

	// free ion buffers
	deinitQ6();
	rpcmem_deinit();
    printf("deinit done...\n");

    if (anyFailures != 0) printf ("cornerApp FAILED!!!\n");
    else printf ("cornerApp SUCCESS\n");
    return anyFailures;
}