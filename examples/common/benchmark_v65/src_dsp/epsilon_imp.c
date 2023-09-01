/**=============================================================================

@file
   epsilon_imp.cpp

@brief
   implementation file for epsilon filter RPC interface.

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

// epsilon includes
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
#define   max_t(a, b)       ((a) > (b) ? a : b)
#define   roundup_t(a, m)   (((a)+(m)-1)&(-m))

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
    unsigned int         workerCount;       // atomic counter shared by all workers
    unsigned char        *src;              // source image pointer
    int                  width;             // source image width
    int                  height;            // number of output rows
    int                  stride;            // source image stride
    int                  threshold;         // threshold
    unsigned char        *dst;              // destination image pointer
    int                  nThreads;          // number of threads
} epsilon_callback_t;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void epsilon_callback(void* data)
{
    epsilon_callback_t    *dptr = (epsilon_callback_t*)data;

    unsigned char *src, *dst;
    int id, width, stride, height, threshold, NUM_THREADS;
    int y, ystart, yend, rows;

    epsilon_callback_t *pTD = (epsilon_callback_t *)dptr;

    width     = pTD->width;
    stride    = pTD->stride;

    int l2fsize= roundup_t(width, VLEN)*sizeof(unsigned char);
    long long L2FETCH_PARA = CreateL2pfParam(stride*sizeof(unsigned char), l2fsize, 1, 0);

    id        = dspCV_atomic_inc_return(&(pTD->workerCount)) - 1;
    src       = pTD->src;
    dst       = pTD->dst;
    height    = pTD->height;
    threshold = pTD->threshold;
    NUM_THREADS = pTD->nThreads;

    rows = (height-8)/NUM_THREADS;

    ystart = 4 + id*rows;

    if (id==(NUM_THREADS-1))
    {
       rows = (height-8) - (NUM_THREADS-1)*rows;
    }

    yend = ystart + rows;

    src += ystart*stride;
    dst += ystart*stride;

    for (y = ystart; y < yend; y++)
    {
        if ( (y+5)<height )
        {
            L2fetch( (unsigned int)src + 5*stride, L2FETCH_PARA);
        }

        epsilonFiltPerRow(src, stride, width, threshold, dst);

        src += stride;
        dst += stride;
    }

    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token); 
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult benchmark_epsilon(
    remote_handle64 handle,
    const uint8* src, 
    int srcLen, 
    int32 stride, 
    int32 width, 
    int32 height, 
    int32 threshold, 
    uint8* dst, 
    int dstLen, 
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

    // Only supporting 128-byte aligned!!
    if (!(src && dst && ((((uint32)src | (uint32)dst | width | stride | stride) & 127) == 0)
            && (height >= 7) && (srcLen == dstLen)))
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
        job.fptr = epsilon_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        epsilon_callback_t dptr;
        dptr.token = &token;
        dptr.workerCount = 0;
        dptr.src = (unsigned char *)src;
        dptr.width = width;
        dptr.height = height;
        dptr.stride = stride;
        dptr.dst = dst;
        dptr.stride = stride;
        dptr.threshold = threshold;
        dptr.nThreads = numWorkers;  
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
    FARF(HIGH,"epsilon profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


