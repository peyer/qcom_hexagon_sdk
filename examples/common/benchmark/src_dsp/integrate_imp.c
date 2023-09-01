/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
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

// integrate includes
#include "benchmark.h"
#include "benchmark_asm.h"

#include "q6cache.h"

#include "qurt_sem.h"
#include "dspCV_worker.h"
#include "dspCV_hvx.h"

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define VLEN 128
#define roundup(x,m)            (((x)+(m)-1)&(-(m)))

#define PROFILING_ON
#define AHEAD 3
#define ROW 2

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
	qurt_sem_t           *sem0;             // synch the threads
	qurt_sem_t           *sem1;             // synch the threads
    dspCV_synctoken_t    *token;            // worker pool token
    unsigned int          rowsPerJob;       // number of rows to process per multi-threaded job
    dspCV_hvx_config_t   *hvxInfo;          // HVX configuration information
    unsigned int          threadCount;      // thread counter shared by all workers

    unsigned char        *src;              // source image pointer
    unsigned int          srcWidth;         // source image width
    unsigned int          height;           // number of output rows
    unsigned int          srcStride;        // source image stride
    unsigned int         *dst;              // destination image pointer
    unsigned int          dstStride;        // destination image stride
    unsigned int         *preint;
} integrate_callback_t;


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void integrate_callback(
    void* data
)
{
    integrate_callback_t    *dptr = (integrate_callback_t*)data;
    int tid           = dspCV_atomic_inc_return(&(dptr->threadCount)) - 1;
    int numThreads    = dptr->hvxInfo->numThreads;
    int length        = dptr->rowsPerJob;
    int srcStride     = dptr->srcStride;
    int dstStride     = dptr->dstStride;
    int srcWidth      = dptr->srcWidth;

    // lock HVX. Main thread has already confirmed HVX reservation.
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
    if (0 > lockResult)
    {
        // this example only works in 128 byte mode!
        FARF(HIGH,"Warning - HVX is reserved but could not be locked. Worker thread bailing!");
        dspCV_worker_pool_synctoken_jobdone(dptr->token);
        return;
    }

    // Set pointers to appropriate line of image for this stripe
    unsigned char *src = dptr->src;
    unsigned int *dst = dptr->dst;
    unsigned char *L2FETCH_ADDR =  src + (ROW-1+AHEAD) * dptr->srcStride;
    int width_ver, width_t = (srcWidth/numThreads + 128 - 1) & -128;
    width_ver = (tid == numThreads - 1)? (srcWidth - tid * width_t) : width_t;
    unsigned *preint = dptr->preint;
    size_t modaddres = 0;
	qurt_sem_t *sem0 = dptr->sem0;
	qurt_sem_t *sem1 = dptr->sem1;

    int i, k, n, k_t;

    // remaining rows
    n = ROW;
    k = n;
    k_t = n / numThreads;

    int srcWidth2a = roundup(srcWidth/2, VLEN);
    int srcWidth2b = roundup(srcWidth - srcWidth2a, VLEN);
    long long L2FETCH_PARA;
    if (tid == 0)
        L2FETCH_PARA = CreateL2pfParam(dptr->srcStride, srcWidth2a, k, 0);
    else
        L2FETCH_PARA = CreateL2pfParam(dptr->srcStride, srcWidth2b, k, 0);

    if (!tid) { // thread = 0
        srcWidth2a += (1<<24);
        L2fetch((unsigned int)(L2FETCH_ADDR), L2FETCH_PARA); L2FETCH_ADDR += n * srcStride;
		qurt_sem_down(sem1);
        IntegrateRow(src, srcWidth2a, srcStride, dst, dstStride, preint + modaddres);
		qurt_sem_up(sem0);
        src += 2 * srcStride;
        dst += 2 * dstStride;
        modaddres = (modaddres+VLEN/sizeof(unsigned))&(VLEN*8/sizeof(unsigned)-1);
        for (i = 2; i < length; i += n)
        {
            WaitForL2fetch();
            if (i+ROW-1+AHEAD < length) {
                L2fetch((unsigned int)(L2FETCH_ADDR), L2FETCH_PARA); L2FETCH_ADDR += n * srcStride;
            }

		    qurt_sem_down(sem1);
            IntegrateRowAcc(src, srcWidth2a, srcStride, dst, dstStride, preint + modaddres);
		    qurt_sem_up(sem0);
            src += n*srcStride;
            dst += n*dstStride;
            modaddres = (modaddres+VLEN/sizeof(unsigned))&(VLEN*8/sizeof(unsigned)-1);
        }
    }
    else { // thread = 1
        src += srcWidth2a;
        L2FETCH_ADDR += srcWidth2a;
        WaitForL2fetch();
        L2fetch((unsigned int)(L2FETCH_ADDR), L2FETCH_PARA); L2FETCH_ADDR += n * srcStride;
		qurt_sem_down(sem0);
        IntegrateRow(src, srcWidth2b, srcStride, dst + srcWidth2a, dstStride, preint + modaddres);
		qurt_sem_up(sem1);
        src += 2 * srcStride;
        dst += 2 * dstStride;
        modaddres = (modaddres+VLEN/sizeof(unsigned))&(VLEN*8/sizeof(unsigned)-1);
        for (i = 2; i < length; i += n)
        {
            WaitForL2fetch();
            if (i+ROW-1+AHEAD < length) {
                L2fetch((unsigned int)(L2FETCH_ADDR), L2FETCH_PARA); L2FETCH_ADDR += n * srcStride;
            }
		    qurt_sem_down(sem0);
            IntegrateRowAcc(src, srcWidth2b, srcStride, dst + srcWidth2a, dstStride, preint + modaddres);
		    qurt_sem_up(sem1);
            src += n*srcStride;
            dst += n*dstStride;
            modaddres = (modaddres+VLEN/sizeof(unsigned))&(VLEN*8/sizeof(unsigned)-1);
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
AEEResult benchmark_integrate(
    remote_handle64 handle,
    const unsigned char* imgSrc,
    int inpLen,
    int srcStride,
    int srcWidth,
    int srcHeight,
    unsigned int* imgDst,
    int outpLen,
    int dstStride,
    int32 LOOPS,
    int32 wakeupOnly,
    int32* dspUsec,
    int32* dspCyc
)
{
#if defined(__HEXAGON_ARCH__) && (__HEXAGON_ARCH__ < 60)
    // only supporting HVX version in this example.
    return AEE_EUNSUPPORTED;
#endif

    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

#ifdef PROFILING_ON
    // record start time (in both microseconds and pcycles) for profiling
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif

    /* -----------------------------------------------------*/
    /*  Initialization                                      */
    /* -----------------------------------------------------*/
    // Only supporting 128-byte aligned!!
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcWidth | srcStride | dstStride) & 127) == 0)
            && (srcHeight >= 3)))
    {
        return AEE_EBADPARM;
    }

    for (int loops = 0; loops < LOOPS; loops++)
    {
        L2fetch((unsigned int)imgSrc, CreateL2pfParam(srcStride, srcWidth, 4, 0));
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

        unsigned int *preint = (unsigned int *)memalign(VLEN, VLEN*8);
        if (!preint)
        {
            FARF(HIGH,"allocation error");
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
        qurt_sem_t sem0, sem1;
        qurt_sem_init_val(&sem0, 0);
        qurt_sem_init_val(&sem1, 7);

        // init job function pointer
        job.fptr = integrate_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        integrate_callback_t dptr;
        dptr.token = &token;
        dptr.sem0 = &sem0;
        dptr.sem1 = &sem1;
        dptr.threadCount = 0;
        dptr.src = (unsigned char *)imgSrc;
        dptr.srcWidth = srcWidth;
        dptr.height = srcHeight;
        dptr.srcStride = srcStride;
        dptr.dst = imgDst;
        dptr.dstStride = dstStride;
        dptr.preint = preint;
        dptr.rowsPerJob = dptr.height;
        dptr.hvxInfo = &hvxInfo;
        job.dptr = (void *)&dptr;

        /* -----------------------------------------------------*/
        /*  Multi-threading dispatch                            */
        /* -----------------------------------------------------*/
        unsigned int i;
        for (i = 0; i < numWorkers; i++)
        {
            // for multi-threaded impl, use this line.
           (void) dspCV_worker_pool_submit(job);
        }
        dspCV_worker_pool_synctoken_wait(&token);

        // clean up hvx configuration - release temporary reservation (if any), turn off power, etc.
        dspCV_hvx_cleanup_mt_job(&hvxInfo);
        qurt_sem_destroy(&sem0);
        qurt_sem_destroy(&sem1);
	}
    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"integrate profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif
    return AEE_SUCCESS;
}

#ifdef __cplusplus
}
#endif
