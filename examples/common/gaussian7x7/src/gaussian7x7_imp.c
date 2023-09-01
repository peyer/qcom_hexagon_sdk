/**=============================================================================

@file
   gaussian7x7_imp.cpp

@brief
   implementation file for gaussian7x7 RPC interface.

Copyright (c) 2014-2015 QUALCOMM Technologies Incorporated.
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

// gaussian7x7 includes
#include "gaussian7x7_asm.h"
#include "gaussian7x7.h"

#include "q6cache.h"

#include "dspCV_worker.h"
#include "dspCV_concurrency.h"
#include "dspCV_hvx.h"

#include "hexagon_protos.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

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
    unsigned int          jobCount;         // atomic counter shared by all workers
    unsigned char        *src;              // source image pointer
    unsigned int          srcWidth;         // source image width
    unsigned int          height;           // number of output rows
    unsigned int          srcStride;        // source image stride
    unsigned char        *dst;              // destination image pointer
    unsigned int          dstStride;        // destination image stride
    unsigned int          rowsPerJob;       // number of rows to process per multi-threaded job
    dspCV_hvx_config_t   *hvxInfo;          // HVX configuration information
} gaussian7x7_callback_t;


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void gaussian7x7_callback(void* data)
{
    gaussian7x7_callback_t    *dptr = (gaussian7x7_callback_t*)data;

    // lock HVX, 128B mode preferred. Main thread has already confirmed HVX reservation.
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
    // 64B mode is also acceptable
    if (0 > lockResult) lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_64B, 0);
    
    if (0 > lockResult)
    {
         // this example doesn't handle cases where HVX could not be locked
         FARF(ERROR,"Warning - HVX is reserved but could not be locked. Worker thread bailing!");
         return;
    }
    // loop until no more horizontal stripes to process
    while (1)
    {
        // atomically add 1 to the job count to claim a stripe.
        unsigned int jobCount = dspCV_atomic_inc_return(&(dptr->jobCount)) - 1;

        // if all horizontal stripes have been claimed for processing, break out and exit the callback
        if (jobCount * dptr->rowsPerJob >= dptr->height)
        {
            break;
        }

        // Set pointers to appropriate line of image for this stripe
        unsigned char *src = dptr->src + (dptr->srcStride * dptr->rowsPerJob * jobCount);
        unsigned char *dst = dptr->dst + (dptr->dstStride * dptr->rowsPerJob * jobCount);

        // initiate L2 prefetch (first 7 rows)
        long long L2FETCH_PARA = CreateL2pfParam(dptr->srcStride, dptr->srcWidth, 7, 0);
        L2fetch( (unsigned int)src, L2FETCH_PARA);
        // next prefetches will just add 1 row
        L2FETCH_PARA = CreateL2pfParam(dptr->srcStride, dptr->srcWidth, 1, 0);

        unsigned char *pSrc[7];
        int i;
        for (i=0; i<7; i++)
        {
            pSrc[i] = src + i * dptr->srcStride;
        }
        
        // find height of this stripe. Usually dptr->rowsPerJob, except possibly for the last stripe.
        unsigned int remainingRows = dptr->height - (dptr->rowsPerJob * jobCount);
        unsigned int height = (remainingRows < dptr->rowsPerJob) ? remainingRows : dptr->rowsPerJob;

        // HVX-optimized implementation
        for (i = 0; i < height; i++)
        {
            // update pointers
            if (i > 0)
            {
                int j;
                for (j = 0; j < 6; j++) pSrc[j] = pSrc[j+1];
                pSrc[6] = pSrc[5] + dptr->srcStride;
                dst += dptr->dstStride;
            }
            // fetch next row
            if (i + 1 < height)
            {
                L2fetch( (unsigned int)(pSrc[6] + dptr->srcStride), L2FETCH_PARA);
            }
            Gaussian7x7u8PerRow(pSrc, dptr->srcWidth, dst, lockResult);
        }
    }

    // If HVX was locked, unlock it.
    dspCV_hvx_unlock();
    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token); 
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult gaussian7x7_Gaussian7x7u8(const uint8* imgSrc, int srcLen, 
    uint32 srcWidth, uint32 srcHeight, uint32 srcStride, uint8* imgDst, int dstLen, uint32 dstStride) 
{
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
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcWidth | srcStride | dstStride) & 127) == 0)
            && (srcHeight >= 7)))
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
    job.fptr = gaussian7x7_callback;

    // init job data pointer. In this case, all the jobs can share the same copy of job data.
    gaussian7x7_callback_t dptr;
    dptr.token = &token;
    dptr.jobCount = 0;
    dptr.src = (unsigned char *)imgSrc;
    dptr.srcWidth = srcWidth;
    dptr.height = (srcHeight - 6);
    dptr.srcStride = srcStride;
    dptr.dst = imgDst + (3 * dstStride);
    dptr.dstStride = dstStride;
    // Stripe image to balance load across available threads, aiming for 3 stripes per 
    // worker, making sure rowsPerJob is multiple of 2
    dptr.rowsPerJob = (dptr.height + (3 * numWorkers - 1)) / (3 * numWorkers);  
    dptr.hvxInfo = &hvxInfo;
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
    FARF(HIGH,"gaussian7x7 profiling: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        (int)(endCycles - startCycles), (int)(endTime - startTime), (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


