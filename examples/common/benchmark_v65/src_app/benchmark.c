/**=============================================================================
Copyright (c) 2016-2019 QUALCOMM Technologies Incorporated.
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
#include "remote.h"
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
#ifndef PATCH_WID
#define PATCH_WID    (16)
#endif
#ifndef PATCH_HGT
#define PATCH_HGT    (8)
#endif


// Define ION heap ID (system heap) for use with HVX/Compute DSP's. 
// This is the recommended heap for use with any DSP with an SMMU, 
// which includes all cDSP's and aDSP's with HVX. (This may already 
// be defined in rpcmem.h, depending on version of that file).
#ifndef ION_HEAP_ID_SYSTEM
#define ION_HEAP_ID_SYSTEM 25
#endif

#pragma weak remote_session_control  

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
    FFT,
    GATHER,
    SCATTER,
    WARP,
    UNKNOWN = 0x7FFFFFFFUL
} testCase;

static const signed char convMask[12] =
{
    1, -4, 7, 0,
    2, -5, 8, 0,
    3, -6, 9, 0
};

struct XYPair {
       int32_t x, y;
};


static void print_usage()
{
    printf("usage:\n\n"
           "  benchmark -f function_name [-cdqsv] [-w width] [-h height] [-L rpc_loops]\n"
           "            [-l dsp_loops] [-u usec] [-P power_level] [-y latency] [-m mode]\n"
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
           "       fast9 - fast9 corner detection over source image (stops at 5000 corners).\n"
           "       sobel3x3 - 3x3 Sobel filter over source image.\n"
           "       fft - 2D 1024x1024 FFT (8-bit real input, 32-bit fixed-point Q29.3 complex\n"
           "         output; each row is 1024 reals followed by 1024 imaginary).\n"
           "       gather - simple example usage of VGATHER instruction.\n"
           "       scatter - simple example usage of VSCATTER instruction.\n"
           "       crash10 - a dummy memcpy that triggers a DSP PD crash every 10th invocation.\n\n"
           "       warp - warp transform with a homograph matrix.\n"
           "  -c: mark RPC buffers as uncached from the Apps processor.\n"
           "       May reduce RPC overhead.\n\n"
           "  -d: enable DSP DCVS algorithm for adjusting clocks to optimize power\n"
           "       (OFF by default).\n\n"
           "  -q: enable FastRPC QoS mode to disable CPU low-power states, improving\n"
           "       performance at some power cost. (OFF by default).\n\n"
           "  -z: run benchmark DSP library in an unsigned PD.\n\n"
           "  -s: Skip bit-exactness checks (this saves test execution time, should not\n"
           "       impact profiling results).\n\n"
           "  -r: Participate in cooperative serialization. Concurrent participants will each\n"
           "        be gated on a shared virtual resource so that each participant can make full\n"
           "        use of cDSP resources in turn. This helps overall performance of some concurrencies."
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
           "  -p power_level : <Deprecated, instead use -P> Desired power level. 0 is highest-performance, i.e.\n"
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
           "  -P power_level: Desired power level. 0 is Disable Voltage corner, 1 is lowest-performance, i.e.\n"
           "       SVS2. 2 is next higher performance than 1, etc. Specific values are target-dependent,\n"
           "       Please refer to the definition of HAP_dcvs_voltage_corner_t @ SDK_ROOT\\incs\\HAP_power.h \n"
           "       e.g. it could be that 1 = SVS2, 2 = SVS, 3 = SVSPLUS, 4=NOM, and\n"
           "       5 = NOMPLUS 6 = TURBO, 7 = TURBOPLUS.\n\n" 
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
    int POWER_LEVEL = 6;                        //Set the default value to TURBO which will work in all targets.
    int POWER_LEVEL_DEP = 0;                    //Set the default value of the deprecated to the highest performance.
    int hap_power_level = POWER_LEVEL;          //Set the default value to the highest performance.
    boolean use_power_level = TRUE;             //default: Use the new power level
    boolean power_level_dep_cmd = FALSE;        //default: Flag to indicate if deprecated power level option is sent as command arg
    boolean power_level_cmd = FALSE;            //default: Flag to indicate if new power level option is sent as command arg
    int DCVS_ENABLE = 0;
    int LATENCY = 100;
    int MODE = 0;
    int VERBOSE = 0;
    int SKIPVERIFY = 0;
    int CACHE_FLAGS = RPCMEM_DEFAULT_FLAGS;
    int FASTRPC_QOS = 0;
    int UNSIGNED_PD = 0;
    int COMPUTE_RES = 0;
    char* FUNCTION = NULL;
    char* OUTFILE = NULL;
    char *benchmark_URI_Domain;

    // parse command line options
    int option = 0;
    while ((option = getopt(argc, argv,"cdvqzsf:w:h:L:l:u:p:y:m:o:P:r")) != -1) {
        switch (option) {
             case 'd' : DCVS_ENABLE = 1;
                 break;
             case 'c' : CACHE_FLAGS = RPCMEM_FLAG_UNCACHED;
                 break;
             case 'q' : FASTRPC_QOS = 1;
                 break;
             case 'z' : UNSIGNED_PD = 1;
                 break;
             case 'v' : VERBOSE = 1;
                 break;
             case 's' : SKIPVERIFY = 1;
                 break;
             case 'r' : COMPUTE_RES = 1;                 
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
             case 'p' : POWER_LEVEL_DEP = atoi(optarg);
                 power_level_dep_cmd = TRUE;
                 break;
             case 'y' : LATENCY = atoi(optarg);
                 break;
             case 'm' : MODE = atoi(optarg);
                 break;
             case 'o' : OUTFILE = optarg;
                 break;
             case 'P' : POWER_LEVEL = atoi(optarg);
                 power_level_cmd = TRUE;
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

    /*
     * use deprecated power level iff the new power level is not set and deprecated power level is set.
     * Otherwise use new power level.
     */
    use_power_level = (power_level_dep_cmd == FALSE) ? TRUE : (power_level_cmd == TRUE) ? TRUE : FALSE;
    hap_power_level = (use_power_level == TRUE) ? POWER_LEVEL : POWER_LEVEL_DEP;

    uint8_t *src=NULL, *dst=NULL, *ref=NULL;
    uint32_t width = WIDTH;              
    uint32_t height = HEIGHT;
    int vBorder = 0, hBorder = 0;
    int tolerance = 0;
    uint32_t stride = (WIDTH+127)&-128;    // keep aligned to 128 bytes!
    int dstStride=stride;
    int dspUsec = 0, dspCyc = 0;
    unsigned int rpcOverhead = 0;
    int retVal = 0;
    int i,j;
    int nErr = 0;
    int numFast9Corners = 0, numFast9CornersRef = 0;
    int maxnumcorners    = (width * height < 5000) ? width * height : 5000;    
    uint8_t *gridarr=NULL;
    int gridarrSz=0;
    int meshH=0;
    int meshW=0;      

    //deduce the horizStep, vertStep and nPatches from the user specified width and height, or the following code block can be commented to manually assign the values
    //to these three variables directly.
    int horizStep=0, vertStep=0, nPatches=0;//init with zero to suppress compiler warnings
    if (!strncmp(FUNCTION,"gather",20) || !strncmp(FUNCTION,"scatter",20)) 
    {
        if(height<PATCH_HGT+2||width<128)
        {
            printf("For scatter/gather, minimum supported height=%d and width=128.\n", PATCH_HGT+2);
            return AEE_EBADPARM;
        }        

        for(int vertStepTmp=PATCH_HGT+2;vertStepTmp<=height; vertStepTmp+=2)
        {
            int nPatchesTmp=height/vertStepTmp;
            int horizStepTmp=width/nPatchesTmp;

            if(horizStepTmp>=2&&  horizStepTmp%2==0&&           //the horizontal step are always even number
               vertStepTmp*(nPatchesTmp-1)+PATCH_HGT-1<height&&//the patches never warps, i.e. cross the image boundary
               horizStepTmp*(nPatchesTmp-1)+PATCH_WID-1<width)
            {
                horizStep=horizStepTmp;
                vertStep=vertStepTmp;
                nPatches=nPatchesTmp;
                break;
            }
        }
    }

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
        // it is the convention to express the stride in byte unit, the output of integrate is uint32_t, so need to enlarge by 4.
        dstStride=sizeof(uint32_t)/sizeof(uint8_t)*stride;
    }
    else if (!strncmp(FUNCTION,"fft",20)) 
    {
        printf("FFT only supports 1024x1024. Overriding any other setting...\n");
        WIDTH = HEIGHT = width = height = stride = 1024;
        srcSize = stride * height;
        func = FFT;
        dstSize = 8 * srcSize;
    }
    else if (!strncmp(FUNCTION,"gather",20)) 
    {
        printf("gather parameters: width=%d, height=%d, nPatches=%d, horizStep=%d, vertStep=%d\n", (int)width, (int)height, nPatches, horizStep, vertStep);
        srcSize = stride * height;
        func = GATHER;
        dstSize = nPatches*128;//each patch is of size 8*16 bytes		
    }
    else if (!strncmp(FUNCTION,"scatter",20)) 
    {
        printf("scatter parameters: width=%d, height=%d, nPatches=%d, horizStep=%d, vertStep=%d\n", (int)width, (int)height, nPatches, horizStep, vertStep);
        srcSize = stride * height;
        func = SCATTER;
        dstSize = srcSize;	
    }
    else if (!strncmp(FUNCTION,"epsilon",20)) func = EPSILON;
    else if (!strncmp(FUNCTION,"bilateral",20)) func = BILATERAL;
    else if (!strncmp(FUNCTION,"fast9",20)) 
    {
        func = FAST9;
        dstSize = 4 * srcSize;
    }
    else if (!strncmp(FUNCTION,"crash10",20)) func = CRASH10;
    else if (!strncmp(FUNCTION,"warp",20))
    {
        func = WARP;

        meshW=(width+15)/16+1;
        meshH=(height+15)/16+1;        
        
        int outHLuma = height;
        int outHChroma = height/2;
        
        dstStride=stride;        

        outHChroma=((outHChroma+7)/8)*8;
        
        dstSize=dstStride*(outHLuma+outHChroma);

        // In warp, the source image is assumed to be YUV4:2:0 format, where U and V parts are interleaved.
        // So if the Luma is of size height*width, the size of interleaved U and V is (height/2)*(width)
        srcSize=stride*(height/2*3);
    }

    rpcmem_init();

    // Unsigned PD
    if (1 == UNSIGNED_PD)
    {
        if (remote_session_control)
        {
            struct remote_rpc_control_unsigned_module data;
            data.enable = 1;
            data.domain = CDSP_DOMAIN_ID;
            retVal = remote_session_control(DSPRPC_CONTROL_UNSIGNED_MODULE, (void*)&data, sizeof(data));
            printf("remote_session_control returned %d for configuring unsigned PD.\n", retVal);
        }
        else
        {
            printf("Unsigned PD not supported on this device.\n");
        }
    }

    // Open a handle on benchmark RPC interface. Choose default URI for compute DSP. 
    // See remote.idl for more options/information.
    remote_handle64 handle = -1;
	benchmark_URI_Domain = benchmark_URI "&_dom=cdsp";		// try opening handle on CDSP.
    retVal = benchmark_open(benchmark_URI_Domain, &handle);
    if (retVal)
    {
        nErr = retVal;
        printf("Error 0x%x: unable to create fastrpc session on CDSP\n", retVal);
		goto bail;
    }
    
    // FastRPC QoS mode
    if (1 == FASTRPC_QOS)
    {
        struct remote_rpc_control_latency data;
        data.enable = 1;
        remote_handle64_control(handle, DSPRPC_CONTROL_LATENCY, (void*)&data, sizeof(data));

    }
    
    printf("setting clocks to power level %d, Deprecated power level NOT used %d\n", hap_power_level, use_power_level);
    retVal = benchmark_setClocks(handle, hap_power_level, LATENCY, DCVS_ENABLE, use_power_level);
    VERIFY(0 == retVal);
    
    // allocate ion buffers
    VERIFY(0 != (src = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, CACHE_FLAGS, srcSize)));
    printf("src - allocated %d\n", (int)srcSize);
    VERIFY(0 != (dst = (uint8_t*)rpcmem_alloc(ION_HEAP_ID_SYSTEM, CACHE_FLAGS, dstSize)));
    printf("dst - allocated %d\n", (int)dstSize);

    VERIFY(0 != (ref = (uint8_t*)memalign(ALIGNMENT, dstSize)));
    printf("ref - allocated %d (via malloc)\n", (int)dstSize);

    // generate pseudo-random image
    uint32_t heightTotal;
    if (WARP==func)
    {
        heightTotal = height/2*3;// for warp, the src image should include both the Luma and Chroma part.
    }
    else
    {
        heightTotal = height;
    }
    
    uint32 m_z = 0xabcd4321;
    for (j = 0; j < heightTotal; j++)
    {
        uint8_t *ptr = &src[j * stride];
        for (i = 0; i < width; i++)
        {
            m_z = 36969 * (m_z & 65535) + (m_z >> 16);
            *ptr++ = m_z;
        }
    }

    if (WARP==func)
    {           
        // generate the grid array for both the HVX codes use and reference C++ codes use
        gridarrSz = meshH*meshW*2*sizeof(int32_t);   // of size int32_t [meshH][meshW][2]
        
        VERIFY(0 != (gridarr = (uint8_t *)rpcmem_alloc(ION_HEAP_ID_SYSTEM, CACHE_FLAGS, gridarrSz)));
                      
        // One method to generate a homograph matrix is firstly calculate the matrix H
        //                                                                                 [   A*cos(theta)       -A*sin(theta)       Xt ]
        //                                                                          H =    [   A* sin(theta)       A*cos(theta)       Yt ]
        //                                                                                 [  0                             0          1 ] 
        // Then secondly calculate the inverse matrix of H.        
        // a warp homograph matrix with displacement (1,1) and rotation theta=pi/2.
        // The following homograph matrix is by let A=1, theta=pi/6, Xt=1 and Yt=1
        static const float hmat[9] = {0.866026,     0.5, -1.36603,
                                          -0.5,0.866026,-0.366025,
                                             0,       0,         1};
                                             
        // for unit warp, i.e. the output is the same as the input(except at the boundaries), use the following matrix        
        //
        // static const float hmat[9] = { 1.0,   0,  0,
        //                                 0, 1.0,  0,
        //                                 0,   0,1.0};  
        
        int res=constructMeshFromHomography(hmat, 1.0f, 16, meshW, meshH, (int32_t *)gridarr, 0, 0);

        for(i = 0; i < meshH; i++)
        {   
            struct XYPair *wp = (struct XYPair *)(gridarr+meshW*2*sizeof(int32_t)*i);

            int32_t max_y=height*64.0f;
            int32_t max_x=width*64.0f;         
           
            for(j= 0; j < meshW; j++)
            {
                if(wp[j].y>=max_y)
                {
                    wp[j].y=max_y-1;
                }
                else if(wp[j].y<0)
                {
                    wp[j].y=0;
                }
                                            
                if(wp[j].x>=max_x)
                {
                    wp[j].x=max_x-1;
                } 
                else if(wp[j].x<0)
                {
                    wp[j].x=0;
                }                
            }            
        }                
        
        if(0!=res)
        {
            fprintf(stderr, "constructMeshFromHomography returned %d\n", res);
            nErr=res;
            
            goto bail;
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
    unsigned long long t0 = GetTime();
    unsigned long long nextFrame = t0 + USEC;
    unsigned long long sumRPCoverhead = 0, sumDSPexectime = 0;
    unsigned int avgRPCoverhead = 0, avgDSPexectime = 0;
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
                        DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case DILATE3X3:
                    retVal = benchmark_dilate3x3(handle, src, srcSize, stride, width, height, dst, dstSize, stride, 
                        DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case CONV3X3:
                    retVal = benchmark_conv3x3(handle, src, srcSize, stride, width, height, (const char*) convMask, 12, 4, 
                        dst, dstSize, stride, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case SOBEL3X3:
                    retVal = benchmark_sobel3x3(handle, src, srcSize, stride, width, height, dst, dstSize, stride, 
                        DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case GAUSSIAN7X7:
                    retVal = benchmark_gaussian7x7(handle, src, srcSize, width, height, stride, 
                        dst, dstSize, stride, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case INTEGRATE:
                    retVal = benchmark_integrate(handle, src, srcSize, stride, width, height, (unsigned int *)dst, 
                        dstSize/sizeof(unsigned int), dstStride, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case EPSILON:
                    retVal = benchmark_epsilon(handle, src, srcSize, stride, width, height, 16, dst, 
                        dstSize, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case BILATERAL:
                    retVal = benchmark_bilateral9x9(handle, src, srcSize, stride, width, height, dst, 
                        dstSize, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case FFT:
                    retVal = benchmark_fft1024(handle, src, srcSize,  
                        (int32_t*) dst, dstSize/4, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case GATHER:
                    retVal = benchmark_gather(handle, src, srcSize,  
                         dst, dstSize, width, height, nPatches, horizStep, vertStep, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case SCATTER:
                    memset(dst,0,height*width);
                    retVal = benchmark_scatter(handle, src, srcSize,  
                                               dst, dstSize, width, height, nPatches, horizStep, vertStep, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case FAST9:
                    {
                        unsigned int barrier = 50;
                        unsigned int border  = 3;
                        retVal = benchmark_fast9(handle, src, srcSize, stride, width, height, barrier, border, (int16*)dst, 
                            maxnumcorners*4, maxnumcorners, &numFast9Corners, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                        printf("Fast9 found %d corners (stop searching after 5000 found).\n",numFast9Corners);
                    }
                    break;
                case CRASH10:
                    retVal = benchmark_crash10(handle, src, srcSize, stride, width, height, dst, 
                        dstSize, DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);
                    break;
                case WARP:                     
                    retVal = benchmark_warp(handle, src, srcSize,  
                                            dst, dstSize, width, height, stride, dstStride, gridarr, gridarrSz, meshW,
                                            DSPLOOPS, MODE, COMPUTE_RES, (int32*)&dspUsec, (int32*)&dspCyc);                                                                       
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
        rpcOverhead = (unsigned int) (t2 - t1 - dspUsec);
        if (VERBOSE)
            printf("total time %d uSec, DSP-measured %d uSec and %d cyc (RPC overhead %d uSec), observed clock %d MHz\n",
                            (int) (t2-t1), dspUsec, dspCyc, rpcOverhead, (dspUsec == 0 ? 0 : dspCyc/dspUsec));
#endif        
        nextFrame += USEC;
        totalTime += (t2 - t1);
        sumRPCoverhead += (unsigned long long) rpcOverhead;
        sumDSPexectime += (unsigned long long) dspUsec;
    }

    if (AEE_SUCCESS != retVal) {
        printf("Error code %d returned from function.\n", retVal);
            
        // check if handle is lost (i.e. DSP process has crashed)
        if (AEE_ENOSUCH == retVal) {
            printf("Attempting to restart session and continue\n");
            // close handle
            if (handle != -1)
                (void) benchmark_close(handle);
            handle = -1;

            // reopen handle
            retVal = benchmark_open(benchmark_URI_Domain, &handle);
            VERIFY(-1 != handle && 0 == retVal);
            // set clocks
            printf("setting clocks\n");
            retVal = benchmark_setClocks(handle, hap_power_level, LATENCY, DCVS_ENABLE,use_power_level);
            VERIFY(0 == retVal);
        }
    } else {
        avgRPCoverhead = (unsigned int) sumRPCoverhead/RPCLOOPS;
        avgDSPexectime = (unsigned int) sumDSPexectime/RPCLOOPS;
	}
    
#ifdef __hexagon__
    printf("run time of %s: %llu PCycles for %d RPC iterations, each with %d iterations inside the DSP.\n"
         "Last iteration DSP-measured time (for %d iterations): %d pCycles, \n", FUNCTION, totalTime, RPCLOOPS, DSPLOOPS, DSPLOOPS, dspUsec);
#else
    printf("run time of %s: %llu microseconds for %d RPC iterations, each with %d iterations inside the DSP.\n"
         "Last iteration DSP-measured time (for %d iterations): %d uSec, %d cycles (RPC overhead %d uSec), apparent clock rate %d MHz\n", FUNCTION, totalTime, RPCLOOPS, DSPLOOPS, DSPLOOPS, dspUsec, dspCyc, rpcOverhead, (dspUsec == 0 ? 0 : dspCyc/dspUsec));
    printf("Average DSP execution time = %d uSec, Average RPC overhead = %d uSec\n", avgDSPexectime, avgRPCoverhead);
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
            "DSP Clock (MHz), DCVS Enabled, FastRPC QoS Enabled, RPC loops, DSP loops, total time per RPC call (uSec), DSP time per RPC call (uSec), "
            "DSP time per DSP loop iteration (uSec), RPC overhead per RPC call (uSec)\n");
        // log to file
        fprintf(outfile, "%s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %.2f, %d, %.2f, %.2f\n", FUNCTION, WIDTH,
            HEIGHT, USEC, hap_power_level, LATENCY, (dspUsec == 0 ? 0 : dspCyc/dspUsec), DCVS_ENABLE, FASTRPC_QOS, RPCLOOPS, DSPLOOPS, (float)totalTime/RPCLOOPS, dspUsec,
            (float)dspUsec/DSPLOOPS, ((float)totalTime/RPCLOOPS) - dspUsec);
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
            integrate_ref (src, stride, width, height, (unsigned int *)ref, dstStride);
            width = width * sizeof(unsigned int);     // for bit-exact checking
            stride=dstStride;
            break;
        case FFT:
            retVal = fft1024_ref(src, (int32_t*)ref);
            if (retVal) printf ("FFT C reference function returned error code %d!\n",retVal);
            width = width * 8;     // for bit-exact checking
            stride = stride * 8;     // for bit-exact checking
            break;
        case GATHER:        
            gather_ref(src, ref, width, height, nPatches, horizStep, vertStep);     
            width = stride = nPatches * (128);  // patch width = 16, height = 8. width*height = 128
            height = 1;
            break;
        case SCATTER:
            memset(ref,0,height*width);            
            scatter_ref(src, ref, width, height, nPatches, horizStep, vertStep);
            stride = width;
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
        case WARP:                                          
                warp_ref(src, stride, width, height, (int32_t *)gridarr, ref, dstStride);                 
                height = height / 2 *3; // set the height for bit-exact testing, since compare both the Luma and Chroma parts               
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
    if(gridarr) {
        rpcmem_free(gridarr);
    }    
    // free ion buffers
    rpcmem_deinit();

    // close handle
	if (handle != -1)
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

