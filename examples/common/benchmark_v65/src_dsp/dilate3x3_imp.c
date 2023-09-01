/**=============================================================================

@file
   dilate3x3_imp.cpp

@brief
   implementation file for dilate filter RPC interface.

Copyright (c) 2016, 2017, 2019 QUALCOMM Technologies Incorporated.
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

#include "hexagon_protos.h"

#include "HAP_compute_res.h"

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
    unsigned int          numThreads;       // HVX configuration information
    unsigned int          threadCount;      // thread counter shared by all workers

    unsigned char        *src;              // source image pointer
    unsigned int          srcWidth;         // source image width
    unsigned int          height;           // number of output rows
    unsigned int          srcStride;        // source image stride
    unsigned char        *dst;              // destination image pointer
    unsigned int          dstStride;        // destination image stride
} dilate3x3_callback_t;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void dilate3x3_callback(
    void* data
)
{
    dilate3x3_callback_t    *dptr = (dilate3x3_callback_t*)data;
    int tid           = dspCV_atomic_inc_return(&(dptr->threadCount)) - 1;
    int rowsPerJob    = dptr->rowsPerJob;
    int numThreads    = dptr->numThreads;
    int length        = dptr->rowsPerJob;

    if (tid == numThreads - 1)
    {
        length =  dptr->height - 2 - (numThreads - 1) * rowsPerJob;
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
        dilate3x3Per2Row(src, dptr->srcStride, dptr->srcWidth, dst, dptr->dstStride);
        src += 2 * dptr->srcStride;
        dst += 2 * dptr->dstStride;
        L2FETCH_ADDR += 2 * dptr->srcStride;
    }

    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token);
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult benchmark_dilate3x3(
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
    int32 useComputRes,
    int32* dspUsec,
    int32* dspCyc
)
{
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

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
            && (srcHeight >= 3)))
    {
        return AEE_EBADPARM;
    }

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
        job.fptr = dilate3x3_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        dilate3x3_callback_t dptr;
        dptr.token = &token;
        dptr.threadCount = 0;
        dptr.src = (unsigned char *)imgSrc;
        dptr.srcWidth = srcWidth;
        dptr.height = srcHeight;
        dptr.srcStride = srcStride;
        dptr.dst = imgDst;
        dptr.dstStride = dstStride;
        dptr.numThreads = numWorkers;
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
    FARF(HIGH,"dilate3x3 profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


