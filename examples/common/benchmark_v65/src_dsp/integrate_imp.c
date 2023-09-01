/**=============================================================================
Copyright (c) 2016-2019 QUALCOMM Technologies Incorporated.
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

#include "HAP_compute_res.h"

#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define VLEN             (128)
#define NUM_ROW_PER_PROC (2)
#define NUM_COL_PER_PROC (VLEN)

#define PROFILING_ON

#define BUF_LOC_POOL_SZ  (8)
#define MAX_PREINT_SZ    (8)

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
    unsigned int          jobCount;         // the counter of the finished job
    unsigned int          jobNum;           // total number of jobs, two consecutive rows are considered as one job.
                                            // If the height is an odd number, the last row is considered as an additional job.    
                                             
    unsigned int          height;           // source image height   
    unsigned char         *pSrc;            // source image pointer
    unsigned int          srcWidth;         // source image width
    unsigned int          srcStride;        // source image stride, in byte unit    
    unsigned int          *pDst;            // destination image pointer
    unsigned int          dstStride;        // destination image stride, in byte unit
    
                                             // the following three pointer to use to refer the corresponding arrays defined outside of integrate_callback.
    qurt_sem_t            *pSem;             // pointer to the sem array
    unsigned int          *pAhead;           // pointer to the ahead array
    unsigned char         (*pPreint)[VLEN];  // pointer to the preint array
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

    unsigned int height=dptr->height;
    unsigned int jobNum=dptr->jobNum;      
    unsigned int srcStride=dptr->srcStride;
    unsigned int dstStrideDword=dptr->dstStride/sizeof(unsigned int);    
    int numBlock=dptr->srcWidth/VLEN; // the number of blocks, to be horizontally processed
    qurt_sem_t *pSem=dptr->pSem;
    unsigned int *pAhead=dptr->pAhead;
    
    int i;
    while((i = dspCV_atomic_inc_return(&(dptr->jobCount)) - 1) < jobNum)
    {
        int rowIdx=i*NUM_ROW_PER_PROC;
        void (*fp)(unsigned char *restrict ,int ,int ,unsigned int *restrict ,int ,unsigned int *restrict);     

        // There are three cases: 1) The first two rows, which means there is no need to do the accumulation from the upside rows.
        //                        2) The last row, need to call a separate function only processes a single row, only happens when the height is odd.
        //                        3) The other remained rows.
        if(0==rowIdx)
        {
            fp=IntegrateRow;
        }
        else if((height%2==1)&&(rowIdx==height-1))
        {
            fp=IntegrateRowAccSingleRow;
        }
        else
        {
            fp=IntegrateRowAcc;                    
        }     
        
        unsigned char *pSrc=dptr->pSrc+rowIdx*srcStride;
        unsigned int  *pDst=dptr->pDst+rowIdx*dstStrideDword;        
        unsigned int  *pPreint=(unsigned int *)dptr->pPreint[i%MAX_PREINT_SZ];
                
        // prefetch the first block
        L2fetch((unsigned int)pSrc, CreateL2pfParam(srcStride, NUM_COL_PER_PROC, NUM_ROW_PER_PROC, 0));

        // IntegrateRow/IntegrateRowAcc/IntegrateRowAccSingleRow abstract bit24 of the width, value 1 for the current block is the left most block
        //                                                                                    value 0 for the current block is NOT the left most block
        int blkWidth=NUM_COL_PER_PROC+(1<<24);                                                                        
        for(int j=0; j<numBlock; j++)
        {   
            // prefetch the next block if it is not the last block
            if(j<numBlock-1)
            {
                L2fetch((unsigned int)(pSrc+NUM_COL_PER_PROC), CreateL2pfParam(srcStride, NUM_COL_PER_PROC, NUM_ROW_PER_PROC, 0));
            }
            
            // There are two cases: 1) If it is the first row, then there is no dependency on other rows.
            //                      2) Else if the pAhead is zero, it means the processing of the upper two rows is already no faster than the current
            //                         two rows, then directly wait semaphore.
            if(0!=rowIdx&&dspCV_atomic_dec_return(&pAhead[(i-1)%BUF_LOC_POOL_SZ])==-1)
            {                               
                qurt_sem_down(&pSem[(i-1)%BUF_LOC_POOL_SZ]);
            }
            
            fp(pSrc, blkWidth, srcStride, pDst, dstStrideDword, pPreint);
            blkWidth=NUM_COL_PER_PROC;  // update the value for all subsequent blocks
            pSrc += NUM_COL_PER_PROC;
            pDst += NUM_COL_PER_PROC;
          
            if(dspCV_atomic_inc_return(&pAhead[i%BUF_LOC_POOL_SZ])==0)  
            {
                qurt_sem_up(&pSem[i%BUF_LOC_POOL_SZ]);
            }
        }
    }

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
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst | srcWidth | srcStride | dstStride) & 127) == 0)
            && (srcHeight >= 3)))
    {
        return AEE_EBADPARM;
    }

    // Actually it needs unsigned char [MAX_PREINT_SZ][VLEN] __attribute__ ((aligned (128)));
    // Because it does not support __attribute__ ((aligned (128))); defined on the stack, extra 127 elements are used and align the start address manually.
    // After that, each preint points to the the accumulated results left to the current block.
    unsigned char preint_tmp[MAX_PREINT_SZ*VLEN+127];
    unsigned char (*preint)[VLEN]=(unsigned char (*)[VLEN])(((unsigned int)preint_tmp+127)&0xFFFFFF80); 
        
    // Elements of ahead presents the number of blocks of last two rows that are processed ahead of the current two rows. 
    // ahead is used first in atomic_inc/dec, when it is zero, elements of sem are used to suspect the thread.
    qurt_sem_t sem[BUF_LOC_POOL_SZ];
    unsigned int ahead[BUF_LOC_POOL_SZ];

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

        /* -----------------------------------------------------*/
        /*  Multi-threading setup                               */
        /* -----------------------------------------------------*/
        dspCV_worker_job_t   job;
        dspCV_synctoken_t    token;

        // init the synchronization token for this dispatch.
        dspCV_worker_pool_synctoken_init(&token, numWorkers);
        
        for(int i=0; i<sizeof(sem)/sizeof(sem[0]); i++)
        {
            qurt_sem_init_val(&sem[i], 0);
        }
        memset(ahead, 0, sizeof(ahead));        

        // init job function pointer
        job.fptr = integrate_callback;

        // init job data pointer. In this case, all the jobs can share the same copy of job data.
        integrate_callback_t dptr;
        dptr.token = &token;
        dptr.jobCount=0;
        dptr.height=srcHeight;
        if(0==srcHeight%2)
        {
            dptr.jobNum=srcHeight/2;
        }
        else
        {
            dptr.jobNum=srcHeight/2+1;
        }     
        
        dptr.pSrc = (unsigned char *)imgSrc;
        dptr.srcWidth=srcWidth;
        dptr.srcStride=srcStride;
        dptr.pDst=imgDst;
        dptr.dstStride = dstStride;        
        dptr.pPreint=preint;
        dptr.pAhead=ahead;
        dptr.pSem=sem;
        
        job.dptr = (void *)&dptr;
        
        /* -----------------------------------------------------*/
        /*  Multi-threading dispatch                            */
        /* -----------------------------------------------------*/        
        for (int i = 0; i < numWorkers; i++)
        {
           (void) dspCV_worker_pool_submit(job);
        }
        
        dspCV_worker_pool_synctoken_wait(&token);

        // clean up all the semaphores
        for(int i=0; i<sizeof(sem)/sizeof(sem[0]); i++)
        {
            qurt_sem_destroy(&sem[i]);
        }                

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
    FARF(HIGH,"integrate profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif
    return AEE_SUCCESS;
}

#ifdef __cplusplus
}
#endif
