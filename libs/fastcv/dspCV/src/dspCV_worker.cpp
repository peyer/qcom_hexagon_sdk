/**=============================================================================

@file
   dspCV_worker.cpp

@brief
   Utility providing a multi-priority thread worker pool for 
   multi-threaded computer vision (or other compute) applications.

Copyright (c) 2013-2017 Qualcomm Technologies Incorporated.
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

/*===========================================================================
    INCLUDE FILE
===========================================================================*/
#include "dspCV_worker.h"
#include <stdlib.h>
#include "AEEStdErr.h"

#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "qurt.h"
#include "hexagon_protos.h"

#ifdef BUILDING_SO
void dspCV_worker_constructor(void) __attribute__((constructor));
void dspCV_worker_destructor(void) __attribute__((destructor));
#endif

#ifdef __cplusplus
}
#endif

/*===========================================================================
    DEFINE
===========================================================================*/
#define DSPCV_WORKER_THREAD_STACK_SZ  2 *16384                   
#define DSPCV_WORKER_KILL_SIGNAL      31                      // signal to kill the worker threads
#define DSPCV_NUM_JOB_SLOTS           (DSPCV_MAX_NUM_WORKERS + 1) // max queued jobs, slightly more than number of workers.
#define DSPCV_NUM_PRIORITY_LEVELS     3                       // background, normal, elevated priorities.
#define DSPCV_NORMAL_PRIORITY         0
#define DSPCV_LOW_PRIORITY            1
#define DSPCV_HIGH_PRIORITY           2
#define DSPCV_LOWEST_USABLE_QURT_PRIO 254

/*===========================================================================
    TYPEDEF
===========================================================================*/
// internal structure kept in thread-local storage per instance of worker pool
typedef struct
{
    qurt_anysignal_t     empty_jobs;                // available job nodes
    qurt_anysignal_t     queued_jobs;                // jobs that are waiting for a worker
    qurt_mutex_t         empty_jobs_mutex;            // mutex for multiple threads trying to send a job
    qurt_mutex_t         queued_jobs_mutex;            // mutex for multiple threads trying to acquire a job
    unsigned int         job_queue_mask;            // mask for job queue nodes
    unsigned int         num_workers;               // number of workers in this pool
    dspCV_worker_job_t   job[DSPCV_NUM_JOB_SLOTS];    // list of job descriptors
    qurt_thread_t        thread[DSPCV_MAX_NUM_WORKERS]; // thread ID's of the workers
    void *               stack[DSPCV_MAX_NUM_WORKERS];  // thread stack pointers
} dspCV_worker_pool_t;

// internal structure containing OS primitives to sync caller with all its spawned jobs.
typedef union
{
    dspCV_synctoken_t raw;
    struct
    {
        unsigned int atomic_countdown;
        qurt_sem_t   sem;
    } sync;
} internal_synctoken_t;

typedef union {
    unsigned long long int raw;
    struct {
        unsigned int ref_cnt;               // reference counter
        dspCV_worker_pool_t *volatile context;       // Pointer to a context. Pointer itself is volatile.
    }X;
} pool_context_t;

/*===========================================================================
    GLOBAL VARIABLES (per PD)
===========================================================================*/
// initialized in constructor
unsigned int dspCV_num_workers = 1;
unsigned int dspCV_num_hvx128_contexts = 0;

/*===========================================================================
    STATIC VARIABLES
===========================================================================*/
static pool_context_t static_context = {0};
static int constructor_successful = 0;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// in assembly
#ifdef __cplusplus
extern "C"
{
#endif
int dspCV_dec_refcnt_and_nullify_ptr(unsigned long long int *ptr);
#ifdef __cplusplus
}
#endif

// the main workloop for each of the worker threads.
static void dspCV_worker_main(void* context)
{
    // local pointer to owning pool's context
    dspCV_worker_pool_t *me = (dspCV_worker_pool_t *) context;
    
    // some local vars to reduce dereferencing inside loop
    qurt_anysignal_t *signal = &me->queued_jobs;
    unsigned int mask = me->job_queue_mask;
    qurt_mutex_t *mutex = &me->queued_jobs_mutex;

    while(1)
    {
        qurt_mutex_lock(mutex);                     // mutex only allows 1 thread to wait on signal at a time. QuRT restriction.
        (void) qurt_anysignal_wait(signal, mask);    // wait for a job
        unsigned int sig_rx = Q6_R_ct0_R(mask & qurt_anysignal_get(signal)); // count trailing 0's to choose flagged job
        if (sig_rx < DSPCV_NUM_JOB_SLOTS)        // if real job
        {
            dspCV_worker_job_t job = me->job[sig_rx];    // local copy of job descriptor
            (void) qurt_anysignal_clear(signal, (1 << sig_rx));        // clear the queued job signal
            (void) qurt_anysignal_set(&me->empty_jobs, (1 << sig_rx)); // send node back to empty list
            qurt_mutex_unlock(mutex);                // unlock the mutex
            // printf ("worker pool thread %d handling job\n", qurt_thread_get_id());
            job.fptr(job.dptr);                        // issue the callback
        }
        else if (DSPCV_WORKER_KILL_SIGNAL == sig_rx)
        {
            // don't clear the kill signal, leave it for all the workers to see, and exit
            qurt_mutex_unlock(mutex);
            qurt_thread_exit(0);
        }
        else{
            FARF(HIGH,"Worker pool received invalid job %d", sig_rx );
            qurt_mutex_unlock(mutex);
        }
        // else ignore
    }
}

// clean up worker pool
static void
dspCV_worker_pool_deinit_context(dspCV_worker_pool_t *me)
{
    for (int j = 0; j < DSPCV_NUM_PRIORITY_LEVELS; j++)
    {
    // de-initializations
        (void) qurt_anysignal_set(&(me[j].empty_jobs), (1 << DSPCV_WORKER_KILL_SIGNAL));  // notify to stop new jobs.
        (void) qurt_anysignal_set(&(me[j].queued_jobs), (1 << DSPCV_WORKER_KILL_SIGNAL)); // kill worker pool.
        for (unsigned int i = 0; i < me[j].num_workers; i++)                                          // wait for workers to die
        {
            if (me[j].thread[i])
            {
               int status;
               (void) qurt_thread_join(me[j].thread[i], &status);
            }
        }
    
    // release resources
        qurt_mutex_destroy(&me[j].empty_jobs_mutex);
        qurt_mutex_destroy(&me[j].queued_jobs_mutex);
        qurt_anysignal_destroy(&me[j].queued_jobs);
        qurt_anysignal_destroy(&me[j].empty_jobs);
    }
    // free allocated memory (were allocated as a single buffer starting at stack[0])
    if (me[0].stack[0]) free (me[0].stack[0]);
}

void
dspCV_worker_constructor()
{
    FARF(HIGH, "In dspCV_worker constructor");

    qurt_sysenv_max_hthreads_t num_threads;
    if (QURT_EOK != qurt_sysenv_get_max_hw_threads(&num_threads))
    {
        dspCV_num_workers = 4; // Couldn't get number of threads from QuRT, default to 4.
        FARF(HIGH, "Failed to get number of threads. Defaulting to %u", dspCV_num_workers);
    }
    else
    {
        dspCV_num_workers = num_threads.max_hthreads;
    }

    /* Verify that number of hw threads isn't greater than max supported number of hw threads.
       Max threads is used as a constant value for array size. */
    if (dspCV_num_workers > DSPCV_MAX_NUM_WORKERS)
    {
        dspCV_num_workers = DSPCV_MAX_NUM_WORKERS;
        FARF(HIGH, "Limiting number of threads to maximum supported value %u", dspCV_num_workers);
    }
    
#if (__HEXAGON_ARCH__ >= 60)
    dspCV_num_hvx128_contexts = (qurt_hvx_get_units() >> 8) & 0xFF;
#else
    dspCV_num_hvx128_contexts = 0;
#endif    
    // name for the first worker, useful in debugging threads
    char name[6] = "cmp0";
    int nErr = 0;

    // do all allocations in one blob
    int total_workers = dspCV_num_workers + (DSPCV_NUM_PRIORITY_LEVELS - 1);
    int size = (DSPCV_WORKER_THREAD_STACK_SZ * total_workers) + (sizeof(dspCV_worker_pool_t) * DSPCV_NUM_PRIORITY_LEVELS);
    unsigned char *mem_blob = (unsigned char*)malloc(size);
    if (!mem_blob) 
    {
        FARF(ERROR,"Could not allocate memory for worker pool!!");
        return;
    }
    
    dspCV_worker_pool_t *me = (dspCV_worker_pool_t *)(mem_blob + DSPCV_WORKER_THREAD_STACK_SZ * total_workers);
    
    for (int j = 0; j < DSPCV_NUM_PRIORITY_LEVELS; j++)
    {
        // number of workers for this prio level
        me[j].num_workers = (j == 0 ? dspCV_num_workers : 1);
    
    // initializations
        for (unsigned int i = 0; i < me[j].num_workers; i++)
        {
            me[j].stack[i] = NULL;
            me[j].thread[i] = 0;
        }

        // initialize job queue
        qurt_anysignal_init(&me[j].queued_jobs);
        qurt_anysignal_init(&me[j].empty_jobs);
        qurt_mutex_init(&me[j].empty_jobs_mutex);
        qurt_mutex_init(&me[j].queued_jobs_mutex);
        me[j].job_queue_mask = (1 << DSPCV_NUM_JOB_SLOTS) - 1;  // set a bit for each job node, number of job nodes = num_workers + 1
        (void) qurt_anysignal_set(&(me[j].empty_jobs), me[j].job_queue_mask); // fill the empty pool.
        me[j].job_queue_mask |= (1 << DSPCV_WORKER_KILL_SIGNAL);  // add the kill signal to the mask.

        // launch the workers
        qurt_thread_attr_t attr;
        qurt_thread_attr_init (&attr);

        for (unsigned int i = 0; i < me[j].num_workers; i++)
        {
            // set up stack
            me[j].stack[i] = mem_blob;
            mem_blob += DSPCV_WORKER_THREAD_STACK_SZ;
            qurt_thread_attr_set_stack_addr(&attr, me[j].stack[i]);
            qurt_thread_attr_set_stack_size(&attr, DSPCV_WORKER_THREAD_STACK_SZ);

            // set up name
            qurt_thread_attr_set_name(&attr, name);
            name[3] = (name[3] + 1);
            if (name[3] > '9') name[3] = '0';  // name threads cmp0, cmp1, ... (recycle at 9, but num threads should be less than that anyway)

                // set up priority - by default, match the creating thread's prio
                int prio = qurt_thread_get_priority(qurt_thread_get_id());
                
                // adjust priority for elevated or background pools
                prio += (j == DSPCV_NORMAL_PRIORITY ? 0 : (j == DSPCV_LOW_PRIORITY ? 1 : -1));
                
                if (prio < 1) prio = 1;
                if (prio > DSPCV_LOWEST_USABLE_QURT_PRIO) prio = DSPCV_LOWEST_USABLE_QURT_PRIO;

                qurt_thread_attr_set_priority(&attr, prio);

            // launch
                nErr = qurt_thread_create(&me[j].thread[i], &attr, dspCV_worker_main, (void *) &(me[j]));
            if (nErr)
            {
                FARF(ERROR, "Could not launch worker threads!");
                dspCV_worker_pool_deinit_context(me);
                return;
            }
        }
    }
    static_context.X.context = me;
    constructor_successful = 1;
}

void
dspCV_worker_destructor()
{
    FARF(HIGH, "In dspCV_worker destructor");

    dspCV_worker_pool_t *me = static_context.X.context;
    dspCV_worker_pool_deinit_context(me);
}
// submit a prioritized job to the pool.
static int
dspCV_worker_pool_prio_submit(dspCV_worker_job_t job, int prio)
{
    dspCV_worker_pool_t *context = static_context.X.context;

    // if no worker pool exists, perform job in-context.
    if (NULL == context)
    {
        FARF(HIGH, "WARNING- dspCV worker pool is not detected. Using caller context.");
        job.fptr(job.dptr);                     // issue the callback in caller's context
        return 0;
    }

    dspCV_worker_pool_t *me = &context[prio];
    
    // if a worker thread tries to submit a job, call it in-context to avoid recursion deadlock.
    unsigned int i;
    qurt_thread_t id = qurt_thread_get_id();
    for (i = 0; i < me->num_workers; i++)
    {
        if (id == me->thread[i])
        {
            job.fptr(job.dptr);                     // issue the callback in caller's context
            return 0;
        }
    }

    // local vars to reduce dereferencing
    qurt_mutex_t *mutex = &me->empty_jobs_mutex;
    qurt_anysignal_t *signal =  &me->empty_jobs;
    unsigned int mask = me->job_queue_mask;

    qurt_mutex_lock(mutex);                             // lock empty queue
    (void) qurt_anysignal_wait(signal, mask);            // wait for an empty job node
    unsigned int bitfield = qurt_anysignal_get(signal);

    // check if pool is being killed and return early
    if (bitfield & (1 << DSPCV_WORKER_KILL_SIGNAL))
    {
        qurt_mutex_unlock(mutex);
        return 1;
    }

    // send the job to the queue.
    unsigned int sig_rx = Q6_R_ct0_R(mask & bitfield); // count trailing 0's to find first avail node
    me->job[sig_rx] = job;            // copy job descriptor
    (void) qurt_anysignal_clear(signal, (1 << sig_rx));        // clear the empty job node flag
    (void) qurt_anysignal_set(&me->queued_jobs, (1 << sig_rx)); // notify of pending job
    qurt_mutex_unlock(mutex);                // unlock the mutex

    return 0;
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
// lookup for whether worker pool has been created for the calling thread. Optional to call this.
// If worker pool is used when not initialized, jobs will seamlessly happen in caller's context.
int
dspCV_worker_pool_available()
{
    // local pointer to owning pool's context
    return (NULL != static_context.X.context);
}

// function to create a worker pool for the calling thread.
// worker pool is created in constructor. Stubbing this function for backward compatibility
int
dspCV_worker_pool_init()
{
    if (constructor_successful) return 0;

    FARF(HIGH,"Warning - dspCV worker pool constructor failed to run. Initializing worker pool now.");
    FARF(HIGH,"Please check your linking order. dspCV needs to be linked somewhere in between init.o and fini.o.");
    
#if (__HEXAGON_ARCH__ >= 60)
    dspCV_num_hvx128_contexts = (qurt_hvx_get_units() >> 8) & 0xFF;
#else
    dspCV_num_hvx128_contexts = 0;
#endif
    qurt_sysenv_max_hthreads_t num_threads;
    if (QURT_EOK != qurt_sysenv_get_max_hw_threads(&num_threads))
    {
        dspCV_num_workers = 4; // Couldn't get number of threads from QuRT, default to 4.
        FARF(HIGH, "Failed to get number of threads. Defaulting to %u", dspCV_num_workers);
    }
    else
    {
        dspCV_num_workers = num_threads.max_hthreads;
    }

    /* Verify that number of hw threads isn't greater than max supported number of hw threads.
       Max threads is used as a constant value for array size. */
    if (dspCV_num_workers > DSPCV_MAX_NUM_WORKERS)
    {
        dspCV_num_workers = DSPCV_MAX_NUM_WORKERS;
        FARF(HIGH, "Limiting number of threads to maximum supported value %u", dspCV_num_workers);
    }
    
    // name for the first worker, useful in debugging threads
    char name[6] = "cmp0";
    int nErr = 0;

    // do all allocations in one blob
    int total_workers = dspCV_num_workers + (DSPCV_NUM_PRIORITY_LEVELS - 1);
    int size = (DSPCV_WORKER_THREAD_STACK_SZ * total_workers) + (sizeof(dspCV_worker_pool_t) * DSPCV_NUM_PRIORITY_LEVELS);
    unsigned char *mem_blob = (unsigned char*)malloc(size);
    if (!mem_blob) return AEE_ENOMEMORY;
    
    dspCV_worker_pool_t *me = (dspCV_worker_pool_t *)(mem_blob + DSPCV_WORKER_THREAD_STACK_SZ * total_workers);
    
    for (int j = 0; j < DSPCV_NUM_PRIORITY_LEVELS; j++)
    {
        // number of workers for this prio level
        me[j].num_workers = (j == 0 ? dspCV_num_workers : 1);
    
    // initializations
        for (unsigned int i = 0; i < me[j].num_workers; i++)
        {
            me[j].stack[i] = NULL;
            me[j].thread[i] = 0;
        }

        // initialize job queue
        qurt_anysignal_init(&me[j].queued_jobs);
        qurt_anysignal_init(&me[j].empty_jobs);
        qurt_mutex_init(&me[j].empty_jobs_mutex);
        qurt_mutex_init(&me[j].queued_jobs_mutex);
        me[j].job_queue_mask = (1 << DSPCV_NUM_JOB_SLOTS) - 1;  // set a bit for each job node, number of job nodes = num_workers + 1
        (void) qurt_anysignal_set(&(me[j].empty_jobs), me[j].job_queue_mask); // fill the empty pool.
        me[j].job_queue_mask |= (1 << DSPCV_WORKER_KILL_SIGNAL);  // add the kill signal to the mask.

        // launch the workers
        qurt_thread_attr_t attr;
        qurt_thread_attr_init (&attr);

        for (unsigned int i = 0; i < me[j].num_workers; i++)
        {
            // set up stack
            me[j].stack[i] = mem_blob;
            mem_blob += DSPCV_WORKER_THREAD_STACK_SZ;
            qurt_thread_attr_set_stack_addr(&attr, me[j].stack[i]);
            qurt_thread_attr_set_stack_size(&attr, DSPCV_WORKER_THREAD_STACK_SZ);

            // set up name
            qurt_thread_attr_set_name(&attr, name);
            name[3] = (name[3] + 1);
            if (name[3] > '9') name[3] = '0';  // name threads cmp0, cmp1, ... (recycle at 9, but num threads should be less than that anyway)

                // set up priority - by default, match the creating thread's prio
                int prio = qurt_thread_get_priority(qurt_thread_get_id());
                
                // adjust priority for elevated or background pools
                prio += (j == DSPCV_NORMAL_PRIORITY ? 0 : (j == DSPCV_LOW_PRIORITY ? 1 : -1));
                
                if (prio < 1) prio = 1;
                if (prio > DSPCV_LOWEST_USABLE_QURT_PRIO) prio = DSPCV_LOWEST_USABLE_QURT_PRIO;

                qurt_thread_attr_set_priority(&attr, prio);

            // launch
                nErr = qurt_thread_create(&me[j].thread[i], &attr, dspCV_worker_main, (void *) &(me[j]));
            if (nErr)
            {
                FARF(ERROR, "Could not launch worker threads!");
                dspCV_worker_pool_deinit_context(me);
                return AEE_EFAILED;
            }
        }
    }
    unsigned int my_ref_count = dspCV_atomic_inc_return(&static_context.X.ref_cnt);
    if (1 == my_ref_count)
    {
        static_context.X.context = me;
    }
    else
    {
        // there is already a worker pool.
        dspCV_worker_pool_deinit_context(me);
    }
    return 0;
}

// clean up a worker pool
// worker pool is destroyed in destructor. Stubbing this function for backward compatibility
void
dspCV_worker_pool_deinit()
{
    if (constructor_successful) return;
    
    dspCV_worker_pool_t *me = static_context.X.context;
    unsigned int my_ref_count = dspCV_dec_refcnt_and_nullify_ptr(&static_context.raw);
    if (0 == my_ref_count)
    {
        dspCV_worker_pool_deinit_context(me);
    }
    // check and clean up of deinit() called too many times.
    else if (0 > (int)my_ref_count)
    {
        (void) dspCV_atomic_inc_return(&static_context.X.ref_cnt);
    }
    return;
}

// submit a job to the pool at normal priority.
int
dspCV_worker_pool_submit(dspCV_worker_job_t job)
{
    return dspCV_worker_pool_prio_submit(job, DSPCV_NORMAL_PRIORITY);
}

// submit a job to the pool at elevated priority.
int
dspCV_worker_pool_boosted_submit(dspCV_worker_job_t job)
{
    return dspCV_worker_pool_prio_submit(job, DSPCV_HIGH_PRIORITY);
}

// submit a job to the pool at lower priority.
int
dspCV_worker_pool_bkgd_submit(dspCV_worker_job_t job)
{
    return dspCV_worker_pool_prio_submit(job, DSPCV_LOW_PRIORITY);
}

// initialize a synctoken - caller will wait on the synctoken and each job will release it.
// caller wakes when all jobs have released.
void
dspCV_worker_pool_synctoken_init(dspCV_synctoken_t *token, unsigned int njobs)
{
    // cast input to usable struct
    internal_synctoken_t *internal_token = (internal_synctoken_t *) token;

    // initialize atomic counter and semaphore
    internal_token->sync.atomic_countdown = njobs;
    qurt_sem_init_val(&internal_token->sync.sem, 0);
}

// worker job responsible for calling this function to count down completed jobs.
void
dspCV_worker_pool_synctoken_jobdone(dspCV_synctoken_t *token)
{
    // cast input to usable struct
    internal_synctoken_t *internal_token = (internal_synctoken_t *) token;

    // count down atomically, and raise semaphore if last job.
    if (0 == dspCV_atomic_dec_return(&internal_token->sync.atomic_countdown))
    {
        (void) qurt_sem_up(&internal_token->sync.sem);
    }
}

// job submitter waits on this function for all jobs to complete.
void
dspCV_worker_pool_synctoken_wait(dspCV_synctoken_t *token)
{
    // cast input to usable struct
    internal_synctoken_t *internal_token = (internal_synctoken_t *) token;

    // Wait for all jobs to finish and raise the semaphore
    (void) qurt_sem_down(&internal_token->sync.sem);

    // clean up the semaphore
    (void) qurt_sem_destroy(&internal_token->sync.sem);
}

int
dspCV_worker_pool_set_thread_priority(unsigned int prio)
{
    dspCV_worker_pool_t *me = static_context.X.context;

    // if no worker pool exists, perform job in-context.
    if (NULL == me)
    {
        return -2;
    }

    int result = 0;
    for (int j = 0; j < DSPCV_NUM_PRIORITY_LEVELS; j++)
    {
        // adjust priority for elevated or background pools
        int adjusted_prio = (int)prio + (j == DSPCV_NORMAL_PRIORITY ? 0 : (j == DSPCV_LOW_PRIORITY ? 1 : -1));
        if (adjusted_prio < 1) adjusted_prio = 1;
        if (adjusted_prio > DSPCV_LOWEST_USABLE_QURT_PRIO) adjusted_prio = DSPCV_LOWEST_USABLE_QURT_PRIO;
        for (unsigned int i = 0; i < me[j].num_workers; i++)
        {
            int res = qurt_thread_set_priority(me[j].thread[i], (unsigned short)adjusted_prio);
            if (0 != res) result = res;
        }
    }
    return result;
}

int
dspCV_worker_pool_get_thread_priority(unsigned int *prio)
{
    dspCV_worker_pool_t *me = static_context.X.context;

    // if no worker pool exists, perform job in-context.
    if (NULL == me)
    {
        return -2;
    }

    int priority = qurt_thread_get_priority(me[DSPCV_NORMAL_PRIORITY].thread[0]);
    if (priority > 0)
    {
        *prio = priority;
        return 0;
    }
    else
    {
        *prio = 0;
        return -1;
    }
}
