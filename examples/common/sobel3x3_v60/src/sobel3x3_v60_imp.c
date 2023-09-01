/**=============================================================================

@file
   sobel3x3_v60_imp.cpp

@brief
   implementation file for sobel filter RPC interface.

Copyright (c) 2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"

// profile DSP execution time (without RPC overhead) via HAP_perf api's.
#include "HAP_perf.h"

#include "AEEStdErr.h"

// includes
#include "sobel3x3_v60.h"
#include "sobel3x3_asm.h"

#include "q6cache.h"

#include "dspCV_worker.h"
#include "dspCV_concurrency.h"
#include "dspCV_hvx.h"

#include "hexagon_protos.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

// (128-byte is only mode supported in this example)
#define VLEN 128 
#define AHEAD 1

/*===========================================================================
    DECLARATIONS
===========================================================================*/

/*===========================================================================
    TYPEDEF
===========================================================================*/

// multi-threading context structure. This structure contains everything 
// the multi-threaded callback needs to process the right portion of the 
// input image.
typedef struct
{
    dspCV_synctoken_t    *token;            // worker pool token
    unsigned int          rowsPerJob;       // number of rows to process per multi-threaded job
    dspCV_hvx_config_t   *hvxInfo;          // HVX configuration information
    unsigned int          threadCount;      // thread counter shared by all workers

    unsigned char        *src;              // source image pointer
    unsigned int          srcWidth;         // source image width
    unsigned int          height;           // number of output rows
    unsigned int          srcStride;        // source image stride
    unsigned char        *dst;              // destination image pointer
    unsigned int          dstStride;        // destination image stride
} sobel3x3_callback_t;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void sobel3x3_callback(
    void* data
)
{
    sobel3x3_callback_t    *dptr = (sobel3x3_callback_t*)data;
    int tid           = dspCV_atomic_inc_return(&(dptr->threadCount)) - 1;
    int rowsPerJob    = dptr->rowsPerJob;
    int numThreads    = dptr->hvxInfo->numThreads;
    int length        = dptr->rowsPerJob;

    if (tid == numThreads - 1)
    {
        length =  dptr->height - 2 - (numThreads - 1) * rowsPerJob;
    }

    // lock HVX. Main thread has already confirmed HVX reservation.
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
    if (0 > lockResult)
    {
         // this simple example doesn't handle cases where 128B mode could not be locked
         FARF(ERROR,"Warning - HVX is reserved but could not be locked. Worker thread bailing!");
         return;
    }

    // Set pointers to appropriate line of image for this stripe
    unsigned char *src = dptr->src + (dptr->srcStride * (rowsPerJob * tid + 1));
    unsigned char *dst = dptr->dst + (dptr->dstStride * (rowsPerJob * tid + 1));
    unsigned char *L2FETCH_ADDR =  src + (2+AHEAD) * dptr->srcStride;

    // next prefetches will just add 1 row
    long long L2FETCH_PARA = CreateL2pfParam(dptr->srcStride, dptr->srcWidth, 2, 1);

    int i;

    // HVX-optimized implementation
    for (i = 0; i < length; i+=2)
    {
        // fetch next row
        if (i + (2+AHEAD) < length)
        {
            L2fetch((unsigned int)(L2FETCH_ADDR), L2FETCH_PARA);
        }
        sobelPer2Row(src, dptr->srcStride, dptr->srcWidth, dst);
        src += 2 * dptr->srcStride;
        dst += 2 * dptr->dstStride;
        L2FETCH_ADDR += 2 * dptr->srcStride;
    }

    // If HVX was locked, unlock it.
    dspCV_hvx_unlock();
    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token);
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
#if 0
#include <math.h>
#include <stdlib.h>
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
#endif

AEEResult sobel3x3_v60_sobel(
    const uint8* imgSrc, 
    int inpLen, 
    int32 srcStride, 
    int32 srcWidth, 
    int32 srcHeight, 
    uint8* imgDst, 
    int outpLen, 
    int32* dspUsec    
)
{
    *dspUsec = 0;

    // only supporting HVX version in this example.
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif
    // Only supporting 128-byte aligned!!
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcWidth | srcStride) & 127) == 0)
            && (srcHeight >= 3)))
    {
        return AEE_EBADPARM;
    }

    // Determine if it is safe (from an audio/voice/camera concurrency perspective) to run a compute function now
    dspCV_ConcurrencyAttribute attrib[1] = 
    {
        {COMPUTE_RECOMMENDATION, 0},  // query for compute concurrency recommendation
    };
    dspCV_concurrency_query(attrib, 1);
    if (COMPUTE_RECOMMENDATION_NOT_OK == attrib[0].value)
    {
        // return error back to application
        return AEE_EBADSTATE;
    }

    // Determine if HVX is available and in what configuration
    dspCV_hvx_config_t hvxInfo = {0};
    
    // for sake of example, assume only 128B implementation is available (i.e. intrinsics)
    hvxInfo.mode = DSPCV_HVX_MODE_128B;
    // Call utility function to prepare for a multi-threaded HVX computation sequence.
    dspCV_hvx_prepare_mt_job(&hvxInfo);

    // Check results and react accordingly. Treat failure to acquire HVX as a failure
    if (hvxInfo.numUnits <= 0)
    {
        dspCV_hvx_cleanup_mt_job(&hvxInfo);
        return AEE_EFAILED;
    }
    
    int numWorkers = hvxInfo.numThreads;
    
    // split src image into horizontal stripes, for multi-threading.
    dspCV_worker_job_t   job;
    dspCV_synctoken_t    token;

    // init the synchronization token for this dispatch. 
    dspCV_worker_pool_synctoken_init(&token, numWorkers);

    // init job function pointer
    job.fptr = sobel3x3_callback;

    // init job data pointer. In this case, all the jobs can share the same copy of job data.
    sobel3x3_callback_t dptr;
    dptr.token = &token;
    dptr.threadCount = 0;
    dptr.src = (unsigned char *)imgSrc;
    dptr.srcWidth = srcWidth;
    dptr.height = srcHeight;
    dptr.srcStride = srcStride;
    dptr.dst = imgDst;
    dptr.dstStride = srcStride;
    dptr.hvxInfo = &hvxInfo;
    dptr.rowsPerJob = ((dptr.height - 2 + (numWorkers - 1)) / (numWorkers)) & -2;
    job.dptr = (void *)&dptr;

    unsigned int i;
    for (i = 0; i < numWorkers; i++)
    {
        // for multi-threaded impl, use this line.
       (void) dspCV_worker_pool_submit(job);

        // This line can be used instead of the above to directly invoke the 
        // callback function without dispatching to the worker pool. 
        //job.fptr(job.dptr);
    }
    dspCV_worker_pool_synctoken_wait(&token);

    // clean up hvx configuration - release temporary reservation (if any), turn off power, etc.
    dspCV_hvx_cleanup_mt_job(&hvxInfo);

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    FARF(HIGH,"sobel3x3_v60 profiling: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        (int)(endCycles - startCycles), (int)(endTime - startTime), (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


