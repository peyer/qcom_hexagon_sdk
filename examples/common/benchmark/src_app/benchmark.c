/**=============================================================================
Copyright (c) 2016, 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "benchmark.h"
#include "benchmark_ref.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "verify.h"
#include "rpcmem.h" // helper API's for shared buffer allocation

#ifdef __hexagon__     // some defs/stubs so app can build for Hexagon simulation
#include "hexagon_sim_timer.h"
#include "hexagon_cache.h" // for removing buffers from cache during simulation/profiling
#define GetTime hexagon_sim_read_pcycles        // For Hexagon sim, use PCycles for profiling

#else
#include <malloc.h>
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

#define ALIGNMENT      128
#define MAX(a,b)       ((a) > (b) ? (a) : (b))

// Define ION heap ID (system heap) for use with HVX/Compute DSP's.
// This is the recommended heap for use with any DSP with an SMMU,
// which includes all cDSP's and aDSP's with HVX. (This may already
// be defined in rpcmem.h, depending on version of that file).
#ifndef ION_HEAP_ID_SYSTEM
#define ION_HEAP_ID_SYSTEM 25
#endif

typedef enum
{
    CONV3X3,
    DILATE3X3,
    DILATE5X5,
    GAUSSIAN7X7,
    INTEGRATE,
    EPSILON,
    BILATERAL,
    FAST9,
    CRASH10,
    SOBEL3X3,
    UNKNOWN = 0x7FFFFFFFUL
} testCase;

static const signed char convMask[12] =
{
    1, -4, 7, 0,
    2, -5, 8, 0,
    3, -6, 9, 0
};

static void print_usage()
{
    printf("usage:\n\n"
           "  benchmark -f function_name [-cdsv] [-w width] [-h height] [-L rpc_loops]\n"
           "            [-l dsp_loops] [-u usec] [-p power_level] [-y latency] [-m mode]\n"
           "            [-o file_name]\n"
           "options:\n\n"
           "  -f function_name: Which function to benchmark. Options follow.\n"
           "       conv3x3 - 3x3 convolution over source image.\n"
           "       dilate3x3 - 3x3 dilate filter over source image.\n"
           "       dilate5x5 - 5x5 dilate filter over source image.\n"
           "       gaussian7x7 - 7x7 Gaussian filter over source image.\n"
           "       integrate - 32-bit integral image over source image.\n"
           "       epsilon - 9x9 epsilon filter over source image.\n"
           "       bilateral - 9x9 bilateral filter over source image.\n"
           "       fast9 - fast9 corner detection over source image (stops at 5000 corners).\n\n"
           "       sobel3x3 - 3x3 Sobel filter over source image.\n\n"
           "       crash10 - a dummy memcpy that triggers a DSP PD crash every 10th invocation.\n\n"
           "  -d: enable DSP DCVS algorithm for adjusting clocks to optimize power\n"
           "       (OFF by default).\n\n"
           "  -c: mark RPC buffers as uncached from the Apps processor.\n"
           "       May reduce RPC overhead.\n\n"
           "  -s: Skip bit-exactness checks (this saves test execution time, should not\n"
           "       impact profiling results).\n\n"
           "  -v: verbose mode, displays extra profiling information for each RPC\n"
           "       loop (OFF by default).\n\n"
           "  -w width: Width of input image to use for benchmarking. Some functions.\n"
           "       may require a multiple of 128.\n\n"
           "  -h height: Height of input image to use for benchmarking.\n\n"
           "  -L rpc_loops: Number of round-trip RPC invocations of the desired\n"
           "       benchmarking function.\n\n"
           "  -l dsp_loops: Number of iterations of function within the DSP per RPC\n"
           "       invocation.\n\n"
           "  -u usec: Number of micro-seconds between each RPC invocation (default\n"
           "       33333, i.e. 30 fps).\n\n"
           "  -p power_level: Desired power level. 0 is highest-performance, i.e.\n"
           "       Turbo. 1 is next-highest, etc. Specific values are target-dependent,\n"
           "       e.g. it could be that 0 = Turbo, 1 = Nominal, 2 = SVS1, 3=SVS2, and\n"
           "       4 or higher is VDD_min.\n\n"
           "  -y latency: The sleep latency tolerance threshold (in micro-seconds) in\n"
           "       the DSP. The higher the value, the deeper level of sleep will be\n"
           "       allowed when idle. Usually this need not be modified (1000 is\n"
           "       default).\n\n"
           "  -m mode: Specifies some options for establishing baseline benchmarks as\n"
           "       follows (0 is default):\n"
           "       0 - Run benchmarked function as specified by the command line.\n"
           "       1 - Make RPC invocations for the function specified in command line,\n"
           "           but skip the actual processing (to help measure baseline\n"
           "           power/performance).\n"
           "       2 - Wake up the CPU at the specified usec interval, but skip the RPC\n"
           "           invocation and related processing (to help measure baseline\n"
           "           power/performance).\n\n"
           "  -o file_name: Specifies a path/file to append profiling results to. This\n"
           "       saves to a comma-separated text file, containing info about the\n"
           "       test case and profiling result. If file does not exist, it will be\n"
           "       created.\n\n"
           );
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
    int RPCLOOPS = 1;
    int DSPLOOPS = 1;
    int USEC = 33333;
    int POWER_LEVEL = 0;
    int DCVS_ENABLE = 0;
    int LATENCY = 100;
    int MODE = 0;
    int VERBOSE = 0;
    int SKIPVERIFY = 0;
    int CACHE_FLAGS = RPCMEM_DEFAULT_FLAGS;
    char* FUNCTION = NULL;
    char* OUTFILE = NULL;
    char *benchmark_URI_Domain;

    // parse command line options
    int option = 0;
    while ((option = getopt(argc, argv,"cdvsf:w:h:L:l:u:p:y:m:o:")) != -1) {
        switch (option) {
             case 'd' : DCVS_ENABLE = 1;
                 break;
             case 'c' : CACHE_FLAGS = RPCMEM_FLAG_UNCACHED;
                 break;
             case 'v' : VERBOSE = 1;
                 break;
             case 's' : SKIPVERIFY = 1;
                 break;
             case 'f' : FUNCTION = optarg;
                 break;
             case 'w' : WIDTH = atoi(optarg);
                 break;
             case 'h' : HEIGHT = atoi(optarg);
                 break;
             case 'L' : RPCLOOPS = atoi(optarg);
                 break;
             case 'l' : DSPLOOPS = atoi(optarg);
                 break;
             case 'u' : USEC = atoi(optarg);
                 break;
             case 'p' : POWER_LEVEL = atoi(optarg);
                 break;
             case 'y' : LATENCY = atoi(optarg);
                 break;
             case 'm' : MODE = atoi(optarg);
                 break;
             case 'o' : OUTFILE = optarg;
                 break;
             default: print_usage();
                 exit(EXIT_FAILURE);
        }
    }
    if (NULL == FUNCTION) {
        print_usage();
        printf ("Exiting...\n");
        return -1;
    }
    uint8_t *src=NULL, *dst=NULL, *ref=NULL;
	uint32_t width = WIDTH;
	uint32_t height = HEIGHT;
    int vBorder = 0, hBorder = 0;
    int tolerance = 0;
	uint32_t stride = (WIDTH+127)&-128;    // keep aligned to 128 bytes!
    int dspUsec = 0, dspCyc = 0;
	int retVal = 0;
	int i,j;
    int nErr = 0;
    int numFast9Corners = 0, numFast9CornersRef = 0;
    int maxnumcorners    = (width * height < 5000) ? width * height : 5000;

	int srcSize = stride * height;
    int dstSize = srcSize;

    testCase func = UNKNOWN;
    if (!strncmp(FUNCTION,"dilate5x5",20)) func = DILATE5X5;
    else if (!strncmp(FUNCTION,"dilate3x3",20)) func = DILATE3X3;
    else if (!strncmp(FUNCTION,"conv3x3",20)) func = CONV3X3;
    else if (!strncmp(FUNCTION,"sobel3x3",20)) func = SOBEL3X3;
    else if (!strncmp(FUNCTION,"gaussian7x7",20)) func = GAUSSIAN7X7;
    else if (!strncmp(FUNCTION,"integrate",20))
    {
        func = INTEGRATE;
        dstSize = 4 * srcSize;
    }
    else if (!strncmp(FUNCTION,"epsilon",20)) func = EPSILON;
    else if (!strncmp(FUNCTION,"bilateral",20)) func = BILATERAL;
    else if (!strncmp(FUNCTION,"fast9",20))
    {
        func = FAST9;
        dstSize = 4 * srcSize;
    }
    else if (!strncmp(FUNCTION,"crash10",20)) func = CRASH10;

	rpcmem_init();

    // Open a handle on benchmark RPC interface. Choose default URI for compute DSP.
    // See remote.idl for more options/information.
    remote_handle64 handle = -1;
	benchmark_URI_Domain = benchmark_URI "&_dom=cdsp";
    // first try opening handle on CDSP.
    retVal = benchmark_open(benchmark_URI_Domain, &handle);

    // if CDSP is not present, try ADSP.
    if (retVal != 0)
    {
        printf("cDSP not detected on this target (error code %d), attempting to use aDSP\n", retVal);
		benchmark_URI_Domain = benchmark_URI "&_dom=adsp";
        retVal = benchmark_open(benchmark_URI_Domain, &handle);
    }
    VERIFY(-1 != handle && 0 == retVal);

    printf("setting clocks to power level %d\n", POWER_LEVEL);
    retVal = benchmark_setClocks(handle, POWER_LEVEL, LATENCY, DCVS_ENABLE);
    VERIFY(0 == retVal);

	// allocate ion buffers
    VERIFY(0 != (src = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, CACHE_FLAGS, srcSize)));
    printf("src - allocated %d\n", (int)srcSize);
    VERIFY(0 != (dst = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, CACHE_FLAGS, dstSize)));
    printf("dst - allocated %d\n", (int)dstSize);

    VERIFY(0 != (ref = (uint8_t*)memalign(ALIGNMENT, dstSize)));
    printf("ref - allocated %d (via malloc)\n", (int)dstSize);

    // generate pseudo-random image
    uint32 m_z = 0xabcd4321;
	for (j = 0; j < height; j++)
	{
		uint8_t *ptr = &src[j * stride];
		for (i = 0; i < width; i++)
		{
            m_z = 36969 * (m_z & 65535) + (m_z >> 16);
			*ptr++ = m_z;
        }
	}
    // call API
	printf( "calling %s on a %dx%d image...\n", FUNCTION, (int)width, (int)height);

#ifdef __hexagon__
    // simulate condition where src and dst are not already in cache (for profiling purposes)
    hexagon_buffer_cleaninv (src, srcSize);
    hexagon_buffer_cleaninv (dst, dstSize);
#endif

	unsigned long long totalTime = 0;
    unsigned long long totalDSPTime = 0;
    unsigned long long totalDSPCycles = 0;
    unsigned long long t0 = GetTime();
    unsigned long long nextFrame = t0 + USEC;
    for (i = 0; i < RPCLOOPS; i++)
    {
        unsigned long long t1 = GetTime();

        // For HVX case, note that src, stride, dst all must be multiples of 128 bytes.
        // The HVX code for this example function does not handle unaligned inputs.
        if (MODE < 2)
        {
            switch (func)
            {
                case DILATE5X5:
                    retVal = benchmark_dilate5x5(handle, src, srcSize, stride, width, height, dst, dstSize, stride,
                        DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case DILATE3X3:
                    retVal = benchmark_dilate3x3(handle, src, srcSize, stride, width, height, dst, dstSize, stride,
                        DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case CONV3X3:
                    retVal = benchmark_conv3x3(handle, src, srcSize, stride, width, height, (const char*) convMask, 12, 4,
                        dst, dstSize, stride, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case SOBEL3X3:
                    retVal = benchmark_sobel3x3(handle, src, srcSize, stride, width, height, dst, dstSize, stride,
                        DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case GAUSSIAN7X7:
                    retVal = benchmark_gaussian7x7(handle, src, srcSize, width, height, stride,
                        dst, dstSize, stride, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case INTEGRATE:
                    retVal = benchmark_integrate(handle, src, srcSize, stride, width, height, (unsigned int *)dst,
                        dstSize/sizeof(unsigned int), stride, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case EPSILON:
                    retVal = benchmark_epsilon(handle, src, srcSize, stride, width, height, 16, dst,
                        dstSize, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case BILATERAL:
                    retVal = benchmark_bilateral9x9(handle, src, srcSize, stride, width, height, dst,
                        dstSize, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case FAST9:
                    {
                        unsigned int barrier = 50;
                        unsigned int border  = 3;
                        retVal = benchmark_fast9(handle, src, srcSize, stride, width, height, barrier, border, (int16*)dst,
                            maxnumcorners*4, maxnumcorners, &numFast9Corners, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                        printf("Fast9 found %d corners (stop searching after 5000 found).\n",numFast9Corners);
                    }
                    break;
                case CRASH10:
                    retVal = benchmark_crash10(handle, src, srcSize, stride, width, height, dst,
                        dstSize, DSPLOOPS, MODE, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                default:
                    printf("Unknown function option %s\n",FUNCTION);
                    VERIFY(0);
                    break;
            }
        }
        unsigned long long t2 = GetTime();
#ifndef __hexagon__
        // unless within a msec of next frame, sleep to emulate desired frame rate.
        if (t2 < nextFrame - 1000)
        {
            usleep(nextFrame - t2);
        }
        else printf ("Warning - unable to keep real time at %d uSec per frame!!\n",USEC);
        if (VERBOSE) printf("total time %d uSec, DSP-measured %d uSec and %d cyc, observed clock %d MHz\n", (int)(t2-t1), dspUsec, dspCyc, (dspUsec == 0 ? 0 : dspCyc/dspUsec));
#endif
        nextFrame += USEC;
        totalTime += (t2 - t1);
        totalDSPTime += dspUsec;
        totalDSPCycles += dspCyc;
    }

    if (AEE_SUCCESS != retVal)
    {
        printf("Error code %d returned from function.\n", retVal);

        // check if handle is lost (i.e. DSP process has crashed)
        if (AEE_ENOSUCH == retVal)
        {
            printf("Attempting to restart session and continue\n");
            // close handle
            (void) benchmark_close(handle);
            handle = -1;

            // reopen handle
            retVal = benchmark_open(benchmark_URI_Domain, &handle);
            VERIFY(-1 != handle && 0 == retVal);
            // set clocks
            printf("setting clocks\n");
            retVal = benchmark_setClocks(handle, POWER_LEVEL, LATENCY, DCVS_ENABLE);
            VERIFY(0 == retVal);
        }
    }


#ifdef __hexagon__
    printf("run time of %s: %llu PCycles for %d RPC iterations, each with %d iterations inside the DSP.\n"
         "Average DSP-measured time (for %d iterations): %.2f pCycles, \n", FUNCTION, totalTime, RPCLOOPS, DSPLOOPS, DSPLOOPS, (float)totalDSPTime/RPCLOOPS);
#else
	printf("run time of %s: %llu microseconds for %d RPC iterations, each with %d iterations inside the DSP.\n"
         "Average DSP-measured time (for %d iterations): %.2f uSec, %.2f cycles, apparent clock rate %d MHz\n", FUNCTION, totalTime, RPCLOOPS, DSPLOOPS, DSPLOOPS, (float)totalDSPTime/RPCLOOPS, (float)totalDSPCycles/RPCLOOPS, (totalDSPTime == 0 ? 0 : (int)(totalDSPCycles/totalDSPTime)));
#endif
	printf("return value from %s: %d \n", FUNCTION, retVal);

    // log result to file
    if (OUTFILE)
    {
        // check if file exists
        FILE *outfile = fopen(OUTFILE, "r");
        int exists = (NULL != outfile);
        if (outfile) fclose(outfile);
        outfile = fopen(OUTFILE, "a");
        if (!outfile) printf("Could not open file %s\n",OUTFILE);
        VERIFY(outfile);
        if (!exists) fprintf(outfile, "Kernel, Width, Height, Frame period (uSec), Power level, Latency Vote (uSec), "
            "DSP Clock (MHz), DCVS Enabled, RPC loops, DSP loops, total time per RPC call (uSec), DSP time per RPC call (uSec), "
            "DSP time per DSP loop iteration (uSec), RPC overhead per RPC call (uSec)\n");
        // log to file
        fprintf(outfile, "%s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %.2f, %d, %.2f, %.2f\n", FUNCTION, WIDTH,
            HEIGHT, USEC, POWER_LEVEL, LATENCY, dspCyc/dspUsec, DCVS_ENABLE, RPCLOOPS, DSPLOOPS, (float)totalTime/RPCLOOPS, (int)(totalDSPTime/RPCLOOPS),
            (float)totalDSPTime/RPCLOOPS/DSPLOOPS, (float)(totalTime - totalDSPTime)/RPCLOOPS);
        fclose(outfile);
    }
    // validate results
    if (!SKIPVERIFY) switch (func)
    {
        case DILATE5X5:
            dilate5x5_ref (src, stride, width, height, ref, stride);
            vBorder = hBorder = 2;
            break;
        case DILATE3X3:
            dilate3x3_ref (src, stride, width, height, ref, stride);
            vBorder = hBorder = 1;
            break;
        case CONV3X3:
            conv3x3_ref (src, stride, width, height, convMask, 4, ref, stride);
            vBorder = hBorder = 1;
            break;
        case SOBEL3X3:
            sobel3x3_ref (src, stride, width, height, ref, stride);
            vBorder = hBorder = 1;
            break;
        case GAUSSIAN7X7:
            gaussian7x7_ref (src, width, height, stride, ref, stride);
            vBorder = hBorder = 3;
            break;
        case INTEGRATE:
            integrate_ref (src, stride, width, height, (unsigned int *)ref, stride);
            width = width * sizeof(unsigned int);     // for bit-exact checking
            break;
        case EPSILON:
            epsilon_ref (src, stride, width, height, 16, ref);
            vBorder = hBorder = 4;
            break;
        case BILATERAL:
            bilateral_ref (src, stride, width, height, ref);
            vBorder = hBorder = 4;
            break;
        case FAST9:
            {
                unsigned int barrier = 50;
                unsigned int border  = 3;
                fast9_ref (src, stride, width, height, barrier, border, (int16*) ref, maxnumcorners, &numFast9CornersRef);
                if (numFast9Corners != numFast9CornersRef)
                {
                    printf("Number of corners doesn't match, %d vs. %d!\n", numFast9Corners, numFast9CornersRef);
                    VERIFY(0);
                }
                width = 4 * numFast9Corners; height = 1; // for bit-exact testing.
            }
            break;
        case CRASH10:
            crash10_ref (src, stride, width, height, ref);
            break;
        default:
            printf("Unknown function option %s\n",FUNCTION);
            VERIFY(0);
            break;
    }

    if (!SKIPVERIFY)
    {
        int bitexactErrors = 0;
        printf( "Checking for bit-exact errors... \n");
        for (j = vBorder; j < height - vBorder; j++)
        {
            for (i = hBorder; i < width - hBorder; i++)
            {
                if (abs(ref[j * stride + i] - dst[j * stride + i]) > tolerance)
                {
                    bitexactErrors++;
                    if (bitexactErrors < 10 )printf("Bit exact error: j=%d, i=%d, refval=%d, dst=%d\n",j,i,ref[j * stride + i], dst[j * stride + i]);
                }
            }
        }
        printf( "Number of bit-exact errors: %d \n", bitexactErrors);
        VERIFY(0 == bitexactErrors);
    }

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

    // close handle
    (void) benchmark_close(handle);

    if(ref) free(ref);

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

