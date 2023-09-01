/**=============================================================================

@file
   gaussian7x7_imp.cpp

@brief
   implementation file for gaussian7x7 RPC interface.

Copyright (c) 2014-2017, 2019 QUALCOMM Technologies Incorporated.
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
#include "benchmark_asm.h"
#include "benchmark.h"

#include "q6cache.h"

#include "dspCV_worker.h"

#include "hexagon_protos.h"

#include "HAP_compute_res.h"

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
    unsigned int          numThreads;       // number of worker threads being used
} gaussian7x7_callback_t;


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void gaussian7x7_callback(void* data)
{
    gaussian7x7_callback_t    *dptr = (gaussian7x7_callback_t*)data;

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
            Gaussian7x7u8PerRow(pSrc, dptr->srcWidth, dst, 128);
        }
    }

    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token); 
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult benchmark_gaussian7x7(remote_handle64 handle, const uint8* imgSrc, int srcLen, 
    uint32 srcWidth, uint32 srcHeight, uint32 srcStride, uint8* imgDst, int dstLen, uint32 dstStride,
    int32 LOOPS, int32 wakeupOnly, int32 useComputRes, int32* dspUsec, int32* dspCyc) 
{
// only supporting HVX version in this example.
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

    // Only supporting 128-byte aligned!!
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcWidth | srcStride | dstStride) & 127) == 0)
            && (srcHeight >= 7)))
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
        compute_res_attr_t compute_res;
        unsigned int context_id = 0;
        
        if(useComputRes)
        {
           if(compute_resource_attr_init)
           {
               compute_resource_attr_init(&compute_res); 
               compute_resource_attr_set_serialize(&compute_res, 1);            
               context_id=compute_resource_acquire(&compute_res, 100000); // wait till 100ms
                
               if(context_id==0)
               {
                   return AEE_ERESOURCENOTFOUND;
               }                        
           }
           else
           {   
                FARF(HIGH, "Compute resource APIs not supported. Use legacy methods instead.");           
           }
        }
        
        int numWorkers = dspCV_num_hvx128_contexts;
        
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
        dptr.numThreads = numWorkers;
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
        
        if(useComputRes&&compute_resource_attr_init)
        {
            compute_resource_release(context_id);               
        }         
    }
    
    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"gaussian7x7 profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


