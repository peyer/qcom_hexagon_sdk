/**=============================================================================
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

// histogram includes
#include "histogram_asm.h"
#include "histogram.h"

#include "q6cache.h"

#include "dspCV_worker.h"
#include "dspCV_hvx.h"

#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================
    DEFINITIONS
===========================================================================*/
//#define PROFILING_ON
#define AHEAD 1
#define VLEN 128    // only supported VLEN.

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
    unsigned int          rowsPerJob;       // number of rows to process per multi-threaded job
    dspCV_hvx_config_t   *hvxInfo;          // HVX configuration information
    unsigned int          threadCount;      // thread counter shared by all workers

    unsigned char        *src;              // source image pointer
    unsigned int          srcWidth;         // source image width
    unsigned int          height;           // number of output rows
    unsigned int          srcStride;        // source image stride
    int                  *dst;              // destination image pointer
    unsigned int          dstStride;        // destination image stride
} histogram_callback_t;


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void histogram_callback(
    void* data
)
{
    histogram_callback_t    *dptr = (histogram_callback_t*)data;
    int tid           = dspCV_atomic_inc_return(&(dptr->threadCount)) - 1;
    int rowsPerJob    = dptr->rowsPerJob;
    int numThreads    = dptr->hvxInfo->numThreads;
    int length        = dptr->rowsPerJob;
    int srcWidth      = dptr->srcWidth;
    int srcStride     = dptr->srcStride;

    if (tid == numThreads - 1)
    {
        length =  dptr->height - (numThreads - 1) * rowsPerJob;
    }

    // lock HVX. This example only supports 128B mode.
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
    if (0 > lockResult)
    {
         FARF(HIGH,"Warning - HVX is reserved but could not be locked. Worker thread bailing!");
         return;
    }

    // atomically add 1 to the job count to claim a stripe.
    unsigned int jobCount = dspCV_atomic_inc_return(&(dptr->jobCount)) - 1;


    // Set pointers to appropriate line of image for this stripe
    unsigned char *src = dptr->src + (srcStride * (rowsPerJob * jobCount));
    int *dst = dptr->dst + tid * 256;

    int i, k, n;

    clearHistogram(dst);

    //  Consideration on n
    // - Reduce overhead of histogramPernRow
    // - MUST have width*n < 2^15
    // - able to data prefetch

    n = 8192 / srcWidth;
    unsigned char *L2FETCH_ADDR =  src + n * srcStride;
    long long L2FETCH_PARA = CreateL2pfParam(srcStride, srcWidth, n, 1);

    // HVX-optimized implementation
    for (i = 0; i < length; i+=n)
    {
        k = (length - i) > n ? n : (length-i);
        // fetch next row
        if (i + n < length)
        {
            if (i + 2*n >= length)
            {
                L2FETCH_PARA = CreateL2pfParam(srcStride, srcWidth, length - i - n, 1);
            }
            L2fetch((unsigned int)(L2FETCH_ADDR), L2FETCH_PARA);
        }
        histogramPernRow(src, srcStride, srcWidth, k, dst);
        src += n * srcStride;
        L2FETCH_ADDR += n * srcStride;
    }

    // If HVX was locked, unlock it.
    dspCV_hvx_unlock();
    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token);
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
AEEResult histogram_histogram(
    const unsigned char* imgSrc,
    int inpLen,
    int srcStride,
    int srcWidth,
    int srcHeight,
    int* imgDst,
    int outpLen
)
{
#if defined(__HEXAGON_ARCH__) && (__HEXAGON_ARCH__ < 60)
    // only supporting HVX version in this example.
    return AEE_EUNSUPPORTED;
#endif

#ifdef PROFILING_ON
    // record start time (in both microseconds and pcycles) for profiling
    uint64 startTime = HAP_perf_get_time_us();
#endif

    /* -----------------------------------------------------*/
    /*  Initialization                                      */
    /* -----------------------------------------------------*/
    // Only supporting 128-byte aligned!!
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcWidth | srcStride) & 127) == 0)
            && (srcHeight >= 3)))
    {
        return AEE_EBADPARM;
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

    int *output = (int *)memalign(VLEN, 256*sizeof(output[0])*hvxInfo.numThreads);   // gxx/gyy/gxy
    if (!output)
    {
        dspCV_hvx_cleanup_mt_job(&hvxInfo);
        return AEE_EFAILED;
    }

    int numWorkers = hvxInfo.numThreads;

    /* -----------------------------------------------------*/
    /*  Multi-threading setup                               */
    /* -----------------------------------------------------*/
    // split src image into horizontal stripes, for multi-threading.
    dspCV_worker_job_t   job;
    dspCV_synctoken_t    token;

    // init the synchronization token for this dispatch.
    dspCV_worker_pool_synctoken_init(&token, numWorkers);

    // init job function pointer
    job.fptr = histogram_callback;

    // init job data pointer. In this case, all the jobs can share the same copy of job data.
    histogram_callback_t dptr;
    dptr.token = &token;
    dptr.jobCount = 0;
    dptr.threadCount = 0;
    dptr.src = (unsigned char *)imgSrc;
    dptr.srcWidth = srcWidth;
    dptr.height = srcHeight;
    dptr.srcStride = srcStride;
    dptr.dst = output;
    dptr.dstStride = srcStride;
    dptr.rowsPerJob = ((dptr.height + (numWorkers - 1)) / (numWorkers));
    dptr.hvxInfo = &hvxInfo;
    job.dptr = (void *)&dptr;

    /* -----------------------------------------------------*/
    /*  Multi-threading dispatch                            */
    /* -----------------------------------------------------*/
    unsigned int i, j;
    for (i = 0; i < numWorkers; i++)
    {
#if 1
        // for multi-threaded impl, use this line.
       (void) dspCV_worker_pool_submit(job);
#else
        // This line can be used instead of the above to directly invoke the
        // callback function without dispatching to the worker pool.
        job.fptr(job.dptr);
#endif
    }
    dspCV_worker_pool_synctoken_wait(&token);

    // clean up hvx configuration - release temporary reservation (if any), turn off power, etc.
    dspCV_hvx_cleanup_mt_job(&hvxInfo);

    // combine the output
    for (i = 0; i < 256; i++)
    {
        unsigned sum = 0;
        for (j = 0; j < hvxInfo.numThreads; j++)
        {
            sum += output[j*256 + i];
        }
        imgDst[i] = sum;
    }

    free(output);

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endTime = HAP_perf_get_time_us();
    FARF(HIGH,"histogram profiling:  %d microseconds. ", (int)(endTime - startTime));
#endif
    return AEE_SUCCESS;
}

#ifdef __cplusplus
}
#endif
