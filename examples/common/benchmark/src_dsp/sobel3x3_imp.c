/**=============================================================================

@file
   sobel3x3_imp.cpp

@brief
   implementation file for sobel filter RPC interface.

Copyright (c) 2017 QUALCOMM Technologies Incorporated.
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
#include "benchmark.h"
#include "benchmark_asm.h"

#include "q6cache.h"

#include "dspCV_worker.h"
#include "dspCV_hvx.h"
#include "qurt_hvx.h"

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
    unsigned int          threadCount;      // thread counter shared by all workers
    unsigned int          numThreads;       // number of workers
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
static void sobel3x3_callback(
    void* data
)
{
    sobel3x3_callback_t    *dptr = (sobel3x3_callback_t*)data;
    int tid           = dspCV_atomic_inc_return(&(dptr->threadCount)) - 1;
    int rowsPerJob    = dptr->rowsPerJob;
    int numThreads    = dptr->numThreads;
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
         dspCV_worker_pool_synctoken_jobdone(dptr->token);
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

AEEResult benchmark_sobel3x3(
    remote_handle64 handle,
    const uint8* imgSrc, 
    int inpLen, 
    int32 srcStride, 
    int32 srcWidth, 
    int32 srcHeight, 
    uint8* imgDst, 
    int outpLen, 
    int32 dstStride, 
    int32 LOOPS,
    int32 wakeupOnly,
    int32* dspUsec,
    int32* dspCyc
)
{
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

    // Only supporting 128-byte aligned!!
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcStride | dstStride) & 127) == 0)
            && (srcHeight >= 3)))
    {
        return AEE_EBADPARM;
    }

// record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif
    
    for (int loops = 0; loops < LOOPS; loops++)
    {
        // query number of 128-byte HVX contexts
        const int numWorkers = (qurt_hvx_get_units() >> 8) & 0xFF;
        if (0 == numWorkers) return AEE_EUNSUPPORTED;
        
        // split src image into horizontal stripes, for multi-threading.
        dspCV_worker_job_t   job;
        dspCV_synctoken_t    token;

        // init the synchronization token for this dispatch. 
        dspCV_worker_pool_synctoken_init(&token, numWorkers);

        job.fptr = sobel3x3_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        sobel3x3_callback_t dptr;
        dptr.token = &token;
        dptr.threadCount = 0;
        dptr.numThreads = numWorkers;
        dptr.src = (unsigned char *)imgSrc;
        dptr.srcWidth = srcWidth;
        dptr.height = srcHeight;
        dptr.srcStride = srcStride;
        dptr.dst = imgDst;
        dptr.dstStride = srcStride;
        dptr.rowsPerJob = ((dptr.height - 2 + (numWorkers - 1)) / (numWorkers)) & -2;
        job.dptr = (void *)&dptr;
        unsigned int i;
        for (i = 0; i < numWorkers; i++)
        {
            // for multi-threaded impl, use this line.
           (void) dspCV_worker_pool_submit(job);

            // This line can be used instead of the above to directly invoke the 
            // callback function without dispatching to the worker pool. 
           // job.fptr(job.dptr);
        }
        dspCV_worker_pool_synctoken_wait(&token);
    }

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"dilate5x5_v60 profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}

