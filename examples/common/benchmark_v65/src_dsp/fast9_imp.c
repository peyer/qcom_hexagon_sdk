/**=============================================================================
Copyright (c) 2014-2015, 2019 QUALCOMM Technologies Incorporated.
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

// fast9 includes
#include "benchmark_asm.h"
#include "benchmark.h"

#include "q6cache.h"

#include "dspCV_worker.h"

#include <stdlib.h>
#include <string.h>

#include "HAP_compute_res.h"

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define VLEN 128
#define roundup(x,m)            (((x)+(m)-1)&(-(m)))

#define PROFILING_ON
#define AHEAD 1

/*===========================================================================
    DECLARATIONS
===========================================================================*/
void sort(
    short *array,
    int n
    );

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
    unsigned int          numThreads;       // number of workers being used
    unsigned int          threadCount;      // thread counter shared by all workers

    unsigned char        *im;               // source image pointer
    unsigned int          xsize;            // source image width
    unsigned int          ysize;            // number of output rows
    unsigned int          stride;           // source image stride
    unsigned int          barrier;          // barrier
    unsigned int          border;           // border
    short int            *xy;               // destination coordinate
    int                   maxnumcorners;    // max num of corners
    int                   numcorners[4];    // num corners found
} fast9_callback_t;


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// multi-threading callback function
static void fast9_callback(
    void* data
)
{
    fast9_callback_t    *dptr = (fast9_callback_t*)data;
    int tid              = dspCV_atomic_inc_return(&(dptr->threadCount)) - 1;
    int numThreads       = dptr->numThreads;
    unsigned char *im    = dptr->im;
    unsigned int xsize   = dptr->xsize;
    unsigned int ysize   = dptr->ysize;
    unsigned int stride  = dptr->stride;
    unsigned int barrier = dptr->barrier;
    unsigned int border  = dptr->border;
    short int *xy        = dptr->xy;
    int maxnumcorners    = dptr->maxnumcorners;
    int numcorners       = 0;

    int rowsPerJob = dptr->rowsPerJob;
    int ystart, yend, length;

    if (tid == 0)
    {
        ystart = dptr->border + 0 * rowsPerJob;
        length = rowsPerJob;
    }
    else
    {
        ystart = dptr->border + tid * rowsPerJob;
        length = rowsPerJob;
        if (tid == numThreads - 1)
        {
            length = ysize - 2 * dptr->border - tid * rowsPerJob;
        }
    }
    yend = ystart + length;
    xy += tid * 2 * maxnumcorners;

    unsigned int width_t    = roundup(xsize,VLEN);
    long long l2fetchparam = CreateL2pfParam(stride, width_t, 1, 0);

    numcorners = 0;
    int boundary = border;
    unsigned int xstart = boundary&(-VLEN);
    unsigned int num_pixels = xsize - xstart - boundary;
    num_pixels = (num_pixels+8*VLEN-1)&(-8*VLEN); // roundup to 8*VLEN
    unsigned int num_pixels32 = num_pixels >> 5;

    unsigned int *bitmask = (unsigned int *)memalign(VLEN, num_pixels32*sizeof(unsigned int));
    short        *xpos    = (short        *)malloc(xsize*sizeof(short));
    if (!bitmask || !xpos)
    {
         FARF(HIGH,"allocation error!");
         goto bail;
    }

    int y, num, n, k;

    for (y = ystart; y < yend; ++y)
    {
        unsigned char* p = (unsigned char *)im + y*stride;

        if (y + 3 + AHEAD < yend)
        {
            L2fetch((unsigned int)(p+(3+AHEAD)*stride), l2fetchparam); 
        }
        fast9_detect_coarse(p, xsize, stride, barrier, bitmask, boundary);

        num = fast9_detect_fine(p, num_pixels32, stride, barrier, bitmask, xpos, xstart);
        sort(xpos, num);

        k = maxnumcorners - (numcorners);
        n = ( k > num ) ? num : k;

        for (k = 0; k < n; k++)
        {
            *(xy++) = xpos[k];
            *(xy++) = y;
        }

        numcorners += n;

        if( numcorners >= maxnumcorners )
        {
            break;
        }
    }

    dptr->numcorners[tid] = numcorners;

bail:
    free(xpos   );
    free(bitmask);

    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token);
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
void sort( short *a, int n )
{
    int i, j;
    short temp;

    for (i = 1; i < n; ++i)
    {
        for (j = 0; j < (n-i); ++j)
        {
            if(a[j] > a[j+1])
            {
                temp  = a[j  ];
                a[j  ]= a[j+1];
                a[j+1]= temp;
            }
        }
    }
}

/* ======================================================================== */
AEEResult benchmark_fast9(
    remote_handle64 handle,
    const unsigned char* im, 
    int imLen, 
    unsigned int stride, 
    unsigned int xsize, 
    unsigned int ysize, 
    unsigned int barrier, 
    unsigned int border, 
    short int* xy, 
    int xyLen, 
    int maxnumcorners, 
    int* numcorners,
    int32 LOOPS,
    int32 wakeupOnly,
    int32 useComputRes,
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
    if (!(im && xy && numcorners && ((((uint32)im | (uint32)stride) & 127) == 0)
            && (ysize >= 7)))
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

        short int *output = (short int *)memalign(VLEN, maxnumcorners*2*sizeof(output[0])*numWorkers);
        if (!output)
        {
            return AEE_ENOMEMORY;
        }

        /* -----------------------------------------------------*/
        /*  Multi-threading setup                               */
        /* -----------------------------------------------------*/
        // split src image into horizontal stripes, for multi-threading.
        dspCV_worker_job_t   job;
        dspCV_synctoken_t    token;

        // init the synchronization token for this dispatch.
        dspCV_worker_pool_synctoken_init(&token, numWorkers);

        // init job function pointer
        job.fptr = fast9_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        fast9_callback_t dptr;
        dptr.token = &token;
        dptr.jobCount = 0;
        dptr.threadCount = 0;
        dptr.im = (unsigned char *)im;
        dptr.xsize = xsize;
        dptr.ysize = ysize;
        dptr.stride = stride;
        dptr.barrier = barrier;
        dptr.border = border>3 ? border : 3;
        dptr.xy = output;
        dptr.maxnumcorners = maxnumcorners;
        dptr.rowsPerJob = (ysize-2*border) / numWorkers;
        dptr.numThreads = numWorkers;
        job.dptr = (void *)&dptr;

        /* -----------------------------------------------------*/
        /*  Multi-threading dispatch                            */
        /* -----------------------------------------------------*/
        unsigned int i;
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

        // combine them
        int num = 0;
        for (i = 0; i< numWorkers; i++)
        {
            int corners = (dptr.numcorners[i] > maxnumcorners - num ? maxnumcorners - num : dptr.numcorners[i]);
            memcpy((void *)(xy + 2*num), (void *)(output + 2*maxnumcorners*i), corners*2*sizeof(output[0]));
            num += corners;
            if (num == maxnumcorners) break;
        }
        *numcorners = num;
        free(output);

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
    FARF(HIGH,"fast9 profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif
    return AEE_SUCCESS;
}

#ifdef __cplusplus
}
#endif
