/**=============================================================================

@file
   downscaleBy2_imp.cpp

@brief
   implementation file for downscaleBy2 RPC interface.

Copyright (c) 2013-2015 QUALCOMM Technologies Incorporated.
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

// downscaleBy2 includes
#include "downscaleBy2_asm.h"
#include "downscaleBy2.h"

#include "dspCV_worker.h"
#include "dspCV_hvx.h"
#include "dspCV_concurrency.h"

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
    unsigned int          srcHeight;        // source image height
    unsigned int          srcStride;        // source image stride
    unsigned char        *dst;              // destination image pointer
    unsigned int          dstStride;        // destination image stride
    unsigned int          rowsPerJob;       // number of rows to process per multi-threaded job
    dspCV_hvx_config_t   *hvxInfo;          // HVX configuration information
} downby2_callback_t;


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void downby2_callback(void* data)
{
    downby2_callback_t    *dptr = (downby2_callback_t*)data;

    // If HVX contexts reserved, lock one for this thread. 
    // (Even though HVX contexts have been reserved for this process, each thread still needs to 
    // lock a context before using any HVX instructions.)
    int hvxReserved = (dptr->hvxInfo->numUnits > 0);
    int lockResult = 0;
    
    if (hvxReserved)
    {
        // 128B mode preferred
        lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
        
        // 128B mode failure likely indicates a concurrent HVX user with mode already locked to 64B. 
        // This function is compatible with both 128B and 64B modes, so try falling back to 64B.
        if (lockResult != 128) lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_64B, 0);
        if (0 > lockResult)
        {
             // This should never happen, but fall back to scalar with a warning message. 
             FARF(HIGH,"Warning - HVX is reserved but could not be locked. Using scalar version");
             lockResult = 0;
        }
    }
    
    // loop until no more horizontal stripes to process
    while (lockResult >= 0)
    {
        // atomically add 1 to the job count to claim a stripe.
        unsigned int jobCount = dspCV_atomic_inc_return(&(dptr->jobCount)) - 1;

        // if all horizontal stripes have been claimed for processing, break out and exit the callback
        if (jobCount * dptr->rowsPerJob >= dptr->srcHeight)
        {
            break;
        }

        // Set pointers to appropriate line of image for this stripe
        unsigned char *src = dptr->src + (dptr->srcStride * dptr->rowsPerJob * jobCount);
        unsigned char *dst = dptr->dst + (dptr->dstStride * (dptr->rowsPerJob / 2) * jobCount);
        
        // find height of this stripe. Usually dptr->rowsPerJob, except possibly for the last stripe.
        unsigned int remainingRows = dptr->srcHeight - (dptr->rowsPerJob * jobCount);
        unsigned int srcHeight = (remainingRows < dptr->rowsPerJob) ? remainingRows : dptr->rowsPerJob;

        // call optimized assembly
        if (!hvxReserved || !lockResult)
        {
            // legacy implementation for non-HVX targets, or for HVX targets in case HVX is not available.
            down2(src, dptr->srcWidth, srcHeight, dptr->srcStride, dst, dptr->dstStride);
        }
        else
        {
            // HVX-optimized implementation
            down2_hvx(src, dptr->srcWidth, srcHeight, dptr->srcStride, dst, dptr->dstStride, lockResult);
        }
    }

    // If HVX was locked, unlock it.
    if (lockResult > 0) dspCV_hvx_unlock();
    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token); 
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult downscaleBy2_scaleDownBy2(const uint8* imgSrc, int srcLen, 
    uint32 srcWidth, uint32 srcHeight, uint32 srcStride, uint8* imgDst, int dstLen, uint32 dstStride, uint32 *profResult) 
{
    *profResult = 0;

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
   // uint64 startCycles = HAP_perf_get_pcycles();
#endif

    // parameter checks for sensible image size/alignment
    if (!(imgSrc && imgDst && (((uint32)imgSrc & 3) == 0) && (((uint32)imgDst & 3) == 0)
            && (srcWidth >= 16) && (srcHeight >= 2) 
            && ((srcWidth & 1) == 0) && ((srcHeight & 1) == 0)
            && (srcStride >= srcWidth) && (dstStride >= srcWidth/2)
			&& ((srcStride & 7) == 0) && ((dstStride & 3) == 0)))
    {
        return AEE_EBADPARM;
    }

    // Determine if it is safe (from an audio/voice/camera concurrency perspective) to run a compute function now
    dspCV_ConcurrencyAttribute attrib[] = 
    {
        {COMPUTE_RECOMMENDATION, 0},  // query for compute concurrency recommendation
    };
    dspCV_concurrency_query(attrib, sizeof(attrib)/sizeof(attrib[0]));
    if (COMPUTE_RECOMMENDATION_NOT_OK == attrib[0].value)
    {
        // return error back to application
        return AEE_EBADSTATE;
    }

    // Determine if HVX is available and in what configuration
    dspCV_hvx_config_t hvxInfo = {0};
    int numWorkers = 0;
    // This HVX implementation assumes 128-byte aligned buffers (and strides)
    if (0 == (127 & ((uint32)imgSrc | (uint32)imgDst | srcStride | dstStride)))
    {
        // Call utility function to prepare for a multi-threaded HVX computation sequence.
        dspCV_hvx_prepare_mt_job(&hvxInfo);
        
        // Check results and react accordingly (extended if/else used here for clarity of example)
        if (hvxInfo.numUnits < 0)
        {
            // hvxInfo.numUnits < 0 indicates the target does not have HVX HW. In this example,
            // we fall back to the non-HVX implementation (and use all available worker threads). 
            // In other functions without a fallback, it might be appropriate to return an error 
            // code indicating HVX is not present on the target.
            numWorkers = dspCV_num_workers;
        }
        else if (hvxInfo.numUnits == 0)
        {
            // hvxInfo.numUnits == 0 indicates the target has HVX HW, but currently there are no
            // contexts available for reservation. In this example,
            // we fall back to the non-HVX implementation (and use all available worker threads). 
            // In other functions without a fallback, it might be appropriate to either return an error 
            // code indicating HVX is not currently available.
            numWorkers = dspCV_num_workers;
        }
        else
        {
            // Reservation of HVX units was successful. Prepare to multi-thread across however many 
            // threads HVX was reserved for.
            numWorkers = hvxInfo.numThreads;
        }
    }
    else 
    {   
        // Boundary conditions for using this HVX function are not met. Perform corrective action.
        // In this example, we will fall back to non-HVX implementation. In other functions, it 
        // may be appropriate to return an error response.
        if (dspCV_hvx_num_reserved() > 0) 
        { 
            FARF(HIGH,"Warning - HVX is reserved but not used. src, dst, srcStride, and/or dstStride are not aligned to 128 bytes as required");
        }
        // multi-thread the non-HVX implementation according to how many worker threads are available.
        numWorkers = dspCV_num_workers;
    }
    
    // split src image into horizontal stripes, for multi-threading.
    dspCV_worker_job_t   job;
    dspCV_synctoken_t    token;

    // init the synchronization token for this dispatch. 
    dspCV_worker_pool_synctoken_init(&token, numWorkers);

    // init job function pointer
    job.fptr = downby2_callback;

    // init job data pointer. In this case, all the jobs can share the same copy of job data.
    downby2_callback_t dptr;
    dptr.token = &token;
    dptr.jobCount = 0;
    dptr.src = (unsigned char *)imgSrc;
    dptr.srcWidth = srcWidth;
    dptr.srcHeight = srcHeight;
    dptr.srcStride = srcStride;
    dptr.dst = imgDst;
    dptr.dstStride = dstStride;
    // Stripe image to balance load across available threads, aiming for 3 stripes per 
    // worker, making sure rowsPerJob is multiple of 2
    dptr.rowsPerJob = ((srcHeight + (3 * numWorkers - 1)) / (3 * numWorkers)) & ~1;  
    dptr.hvxInfo = &hvxInfo;
    job.dptr = (void *)&dptr;

    unsigned int i;
    for (i = 0; i < numWorkers; i++)
    {
        // for multi-threaded impl, use this line.
       (void) dspCV_worker_pool_submit(job);

        // This line can be used instead of the above to directly invoke the 
        // callback function without dispatching to the worker pool. Useful
        // to avoid multi-threading in debug scenarios to narrow down problems.
//        job.fptr(job.dptr);
    }
    dspCV_worker_pool_synctoken_wait(&token);
    
    // clean up hvx configuration - release temporary reservation (if any), turn off power, etc.
    dspCV_hvx_cleanup_mt_job(&hvxInfo);

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
 //   uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *profResult = (uint32)(endTime - startTime);
//    FARF(HIGH,"downscaleBy2 profiling: %d PCycles, %d microseconds. Observed clock rate %d MHz",
  //      (int)(endCycles - startCycles), (int)(endTime - startTime), (int)((endCycles - startCycles) / (endTime - startTime)));
#endif
	return 0;
}


