#ifndef DSPCV_WORKER_H
#define DSPCV_WORKER_H

/**=============================================================================

@file
   dspCV_worker.h

@brief
   Utility providing a thread worker pool for multi-threaded computer vision
   (or other compute) applications.

Copyright (c) 2013-2015 Qualcomm Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary

Export of this technology or software is regulated by the U.S.
Government. Diversion contrary to U.S. law prohibited.

All ideas, data and information contained in or disclosed by
this document are confidential and proprietary information of
Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
By accepting this material the recipient agrees that this material
and the information contained therein are held in confidence and in
trust and will not be used, copied, reproduced in whole or in part,
nor its contents revealed in any manner to others without the express
written permission of Qualcomm Technologies Incorporated.

=============================================================================**/
//==============================================================================
// Defines
//==============================================================================
#ifdef BUILDING_SO
/// MACRO enables function to be visible in shared-library case.
#define DSPCV_API __attribute__ ((visibility ("default")))
#define DSPCV_WORKERPOOL_API __attribute__ ((visibility ("default")))
#else
/// MACRO empty for non-shared-library case.
#define DSPCV_API
#define DSPCV_WORKERPOOL_API
#endif

//==============================================================================
// Include Files
//==============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
    TYPEDEF
===========================================================================*/
/// signature of callbacks to be invoked by worker threads
typedef void ( *dspCV_worker_callback_t )( void* );

/// descriptor for requested callback
typedef struct
{
    dspCV_worker_callback_t fptr;   // function pointer
    void* dptr;                     // data pointer
} dspCV_worker_job_t;

/// opaque client view of synchronization token for job submitter and workers. Internals hidden in implementation.
typedef struct
{
    unsigned int dummy[5];   // large enough to hold a counter and a semaphore
} dspCV_synctoken_t;

/*===========================================================================
    CONSTANTS
===========================================================================*/
// Maximum supported number of worker threads in the default priority level.
// There is also 1 higher priority, and 1 lower priority worker thread 
// available.

#define DSPCV_MAX_NUM_WORKERS   4
DSPCV_WORKERPOOL_API extern unsigned int dspCV_num_workers;
DSPCV_WORKERPOOL_API extern unsigned int dspCV_num_hvx128_contexts;

// Worker pool supports multiple worker thread priorities as follows:
// - Typical jobs are all submitted to the main job queue, via 
//   dspCV_worker_pool_submit(). These jobs are serviced by the normal 
//   worker threads (of which there are dspCV_num_workers).
// - Jobs meant to pre-empt existing busy workers can be sent to the 
//   boosted priority queue via dspCV_worker_pool_prio_submit(). 
// - Jobs meant to run in the background at lower priority than the main
//   worker pool can be submitted via dspCV_worker_pool_bkgd_submit().


//==============================================================================
// Declarations
//==============================================================================

//---------------------------------------------------------------------------
/// @brief
///   Function to determine if there is an established worker pool available to
///   the calling thread. This is an optional call - if no pool is available
///   but attempted to be used, everything works seamlessly, in the client's
///   context (instead of worker context).
///
/// @detailed
///    TBD.
///
/// @return
///   0 - no worker pool available.
///   any other value - worker pool available.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_available(void);

//---------------------------------------------------------------------------
/// @brief
///   Initialize a worker pool. Should be called by each control thread that
///   requires its own worker pool. Typically, this would be an RPC server
///   thread for a given session.
///
/// @detailed
///    TBD.
///
/// @return
///   0 - success.
///   any other value - failure.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_init(void);

//---------------------------------------------------------------------------
/// @brief
///   Kill worker threads and release worker pool resources. Must be called
///   when pool owner no longer requires the pool, such as the RPC session
///   ending.
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API void
dspCV_worker_pool_deinit(void);

//---------------------------------------------------------------------------
/// @brief
///   Submit a job to the worker pool.
///
/// @detailed
///    TBD.
///
/// @param job
///   callback function pointer and data.
///
/// @return
///   0 - success.
///   any other value - failure.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_submit(dspCV_worker_job_t job);

//---------------------------------------------------------------------------
/// @brief
///   Submit a priority-boosted job to the worker pool.
///
/// @detailed
///    TBD.
///
/// @param job
///   callback function pointer and data.
///
/// @return
///   0 - success.
///   any other value - failure.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_boosted_submit(dspCV_worker_job_t job);

//---------------------------------------------------------------------------
/// @brief
///   Submit a lower-priority job to the worker pool.
///
/// @detailed
///    TBD.
///
/// @param job
///   callback function pointer and data.
///
/// @return
///   0 - success.
///   any other value - failure.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_bkgd_submit(dspCV_worker_job_t job);

//---------------------------------------------------------------------------
/// @brief
///   Initialize a synchronization token for job submitter and workers to use.
///   Each worker callback must be given access to the token to release it, and
///   job submitter will wait for all jobs to release the token. Internals are
///   hidden from client.
///
/// @detailed
///    TBD.
///
/// @param token
///   pointer to the synctoken structure.
///
/// @param njobs
///   number of jobs that will be releasing the token
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API void
dspCV_worker_pool_synctoken_init(dspCV_synctoken_t *token, unsigned int njobs);

//---------------------------------------------------------------------------
/// @brief
///   Needs to be called by the worker in the callback before exiting. The
///   token must be available to the callback via the data pointer given
///   to the callback during job submission.
///
/// @detailed
///    TBD.
///
/// @param token
///   pointer to the synctoken structure held by the job submitter
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API void
dspCV_worker_pool_synctoken_jobdone(dspCV_synctoken_t *token);

//---------------------------------------------------------------------------
/// @brief
///   Job submitter calls this function after submitting all jobs to await
///   their completion.
///
/// @detailed
///    TBD.
///
/// @param token
///   pointer to the synctoken structure
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API void
dspCV_worker_pool_synctoken_wait(dspCV_synctoken_t *token);

//---------------------------------------------------------------------------
/// @brief
///   Set the thread priority of the worker threads. Specified priority will
///   be applied to all threads in the default worker pool. The threads
///   that service boosted and background job requests will also be adjusted to be relative
///   to the new default thread priority.
///
/// @detailed
///    TBD.
///
/// @param priority
///   desired priority. 1 is the highest priority allowed. 255 is the lowest priority allowed.
///
/// @return
///   0 - success.
///   any other value - failure.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_set_thread_priority(unsigned int prio);

//---------------------------------------------------------------------------
/// @brief
///   Query the thread priority of the default worker threads. This will return 
///   the current priority for one of the workers, which are all created
///   with the same priority. If a user callback has changed one or more worker threads independently,
///   there is no guarantee on which worker's priority is returned by this function. 
///
/// @detailed
///    TBD.
///
/// @param priority
///   desired priority. 1 is the highest priority allowed. 255 is the lowest priority allowed.
///
/// @return
///   0 - success.
///   any other value - failure.
//---------------------------------------------------------------------------
DSPCV_WORKERPOOL_API int
dspCV_worker_pool_get_thread_priority(unsigned int *prio);

//---------------------------------------------------------------------------
/// @brief
///   Utility inline to atomically increment a variable. Useful in
///   synchronizing jobs among worker threads, in cases where all
///   job-related info can be determined by the job number.
///
/// @detailed
///    TBD.
///
/// @param token
///   pointer to the variable being incremented
///
/// @return
///   the value after incrementing
//---------------------------------------------------------------------------
static inline unsigned int
dspCV_atomic_inc_return(unsigned int *target)
{
    unsigned int result;
    __asm__ __volatile__(
        "1:     %0 = memw_locked(%2)\n"
        "       %0 = add(%0, #1)\n"
        "       memw_locked(%2, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result),"+m" (*target)
        : "r" (target)
        : "p0");
    return result;
}

//---------------------------------------------------------------------------
/// @brief
///   Utility inline to atomically decrement a variable.
///
/// @detailed
///    TBD.
///
/// @param token
///   pointer to the variable being incremented
///
/// @return
///   the value after decrementing
//---------------------------------------------------------------------------
static inline unsigned int
dspCV_atomic_dec_return(unsigned int *target)
{
    unsigned int result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%2)\n"
        "       %0 = add(%0, #-1)\n"
        "       memw_locked(%2, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result),"+m" (*target)
        : "r" (target)
        : "p0");
    return result;
}


#ifdef __cplusplus
}
#endif

#endif  // #ifndef DSPCV_WORKER_H
