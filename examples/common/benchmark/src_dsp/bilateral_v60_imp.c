/**=============================================================================

@file
   bilateral9x9_v60_imp.cpp

@brief
   implementation file for bilateral9x9 RPC interface.

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

#if (__HEXAGON_ARCH__ >= 65)
#include "HAP_vtcm_mgr.h"
#endif

#include "AEEStdErr.h"

// bilateral9x9 includes
#include "benchmark.h"
#include "benchmark_asm.h"

#include "q6cache.h"

#include "dspCV_worker.h"
#include "dspCV_hvx.h"

#include "hexagon_types.h"
#include "hexagon_protos.h"

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

#if (__HEXAGON_ARCH__ >= 65)
const uint8 rangeLUT[256] = {
    255,254,254,254,254,253,253,252,
    251,251,250,249,248,246,245,244,
    242,241,239,237,236,234,232,230,
    228,226,224,221,219,217,214,212,
    209,206,204,201,198,196,193,190,
    187,184,181,178,175,172,169,166,
    163,160,157,154,151,148,145,142,
    139,136,133,130,127,124,121,119,
    116,113,110,107,104,102,99,96,
    94,91,89,86,84,81,79,76,
    74,72,69,67,65,63,61,59,
    57,55,53,51,49,48,46,44,
    43,41,39,38,37,35,34,32,
    31,30,29,27,26,25,24,23,
    22,21,20,19,18,17,16,16,
    15,14,13,13,12,11,11,10,
    10,9,9,8,8,7,7,6,
    6,6,5,5,5,4,4,4,
    3,3,3,3,2,2,2,2,
    2,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};
#endif

/*===========================================================================
    TYPEDEF
===========================================================================*/

// multi-threading context structure. This structure contains everything 
// the multi-threaded callback needs to process the right portion of the 
// input image.
typedef struct
{
    dspCV_synctoken_t    *token;            // worker pool token
    unsigned int          workerCount;         // atomic counter shared by all workers
    unsigned char        *src;              // source image pointer
    unsigned int          width;         // source image width
    unsigned int          height;           // number of output rows
    unsigned int          stride;        // source image stride
    unsigned char        *dst;              // destination image pointer
    unsigned char        *rangeLUT;          // look-up table (v65 only)
    unsigned char        *scratch;           // scratch for v65 implementation
    int                  nThreads;          // number of threads
    dspCV_hvx_config_t   *hvxInfo;          // HVX configuration information
} bilateral9x9_callback_t;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void bilateral9x9_callback(void* data)
{
    bilateral9x9_callback_t    *dptr = (bilateral9x9_callback_t*)data;

    // lock HVX. Main thread has already confirmed HVX reservation.
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
    if (0 > lockResult)
    {
         // this example doesn't handle cases where 128B mode could not be locked
         FARF(ERROR,"Warning - HVX is reserved but could not be locked. Worker thread bailing!");
         return;
    }

    unsigned char *src, *dst;
    int id, width, stride, height, NUM_THREADS;
    int y, ystart, yend, rows;

    bilateral9x9_callback_t *pTD = (bilateral9x9_callback_t *)dptr;

    width     = pTD->width;
    stride    = pTD->stride;

    int l2fsize= roundup_t(width, VLEN)*sizeof(unsigned char);
    long long L2FETCH_PARA = CreateL2pfParam(stride*sizeof(unsigned char), l2fsize, 1, 0);

    id       = dspCV_atomic_inc_return(&(pTD->workerCount)) - 1;
    src      = pTD->src;
    dst      = pTD->dst;
    height   = pTD->height;
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

#if (__HEXAGON_ARCH__ >= 65)
    if (NULL != pTD->rangeLUT)
    {
        unsigned char *scratch = pTD->scratch + id*4*128;
        for (y = ystart; y < yend; y++)
        {
            if ( (y+5)<height )
            {
                L2fetch( (unsigned int)src + 5*stride, L2FETCH_PARA);
            }
            bilateral9x9PerRow_v65(src, stride, width, NULL, pTD->rangeLUT, dst, scratch);
            
            src += stride;
            dst += stride;
        }
        // unlock HVX.
        dspCV_hvx_unlock();
        // release multi-threading job token
        dspCV_worker_pool_synctoken_jobdone(dptr->token); 
        return;
    }
#endif    
    for (y = ystart; y < yend; y++)
    {
        if ( (y+5)<height )
        {
            L2fetch( (unsigned int)src + 5*stride, L2FETCH_PARA);
        }
        bilateral9x9PerRow(src, stride, width, dst);
        
        src += stride;
        dst += stride;
    }
        
    // unlock HVX.
    dspCV_hvx_unlock();
    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token); 
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult benchmark_bilateral9x9(remote_handle64 handle,const uint8* src, 
    int srcLen, int32 stride, int32 width, int32 height, uint8* dst, int dstLen, 
    int32 LOOPS, int32 wakeupOnly, int32 *dspUsec, int32 *dspCyc) 
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

// for v65, set up VTCM
#if (__HEXAGON_ARCH__ >= 65)
    unsigned char *vtcm = HAP_request_VTCM(256*1024, 1); 
    int vtcmAvailable = (NULL != vtcm);
    
    // temporary for VTCM bringup - bail out if can't get VTCM.
    if (!vtcm) return AEE_ENOMEMORY;
    
    HVX_Vector *vtcmRangeLUT = (HVX_Vector*)vtcm;
    unsigned char *rangeTemp = (unsigned char *)vtcmRangeLUT + 256*2*64;
    // make 128 copies of each rangeLUT element, to avoid bank conflicts during VGATHER.
    int retVal = dspCV_hvx_power_on();
    if (!retVal)
    {
        retVal = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0);
    }
    if (128 != retVal)
    {
        retVal = HAP_release_VTCM(vtcm); 
        if (retVal)
        {
            FARF(ERROR,"Unable to release VTCM, error code %d",retVal);
        }
        vtcmAvailable = 0;
    }
    else
    {
        for (int i = 0; i < 256; i++)
        {
            int32_t temp32 = Q6_R_vsplatb_R(rangeLUT[i]);
            HVX_Vector temp = Q6_V_vsplat_R(temp32);
            *vtcmRangeLUT++ = temp;
        }
        dspCV_hvx_unlock();
    }
#endif

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif

    for (int loops = 0; loops < LOOPS; loops++)
    {
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
        job.fptr = bilateral9x9_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        bilateral9x9_callback_t dptr;
        dptr.token = &token;
        dptr.workerCount = 0;
        dptr.src = (unsigned char *)src;
        dptr.width = width;
        dptr.height = height;
        dptr.stride = stride;
        dptr.dst = dst;
        dptr.stride = stride;
        dptr.nThreads = numWorkers;  
        dptr.hvxInfo = &hvxInfo;
        dptr.rangeLUT = NULL;
        dptr.scratch = NULL;
#if (__HEXAGON_ARCH__ >= 65)
        if (vtcmAvailable) 
        {
            dptr.rangeLUT = vtcm;
            dptr.scratch = rangeTemp;
        }
#endif
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

#if (__HEXAGON_ARCH__ >= 65)
        if (vtcmAvailable) 
        {
            retVal = HAP_release_VTCM(vtcm); 
            if (retVal)
            {
                FARF(ERROR,"Unable to release VTCM, error code %d",retVal);
            }
        }
#endif
    }

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int32)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"bilateral9x9 profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


