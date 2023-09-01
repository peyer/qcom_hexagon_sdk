/**=============================================================================

@file
   qprintf_example_imp.cpp

@brief
   implementation file for testing qprintf.

Copyright (c) 2017 QUALCOMM Technologies Incorporated.
_all Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif

#include "HAP_farf.h"
#undef FARF_HIGH
#define FARF_HIGH 1

#include "HAP_perf.h"
#include "HAP_power.h"

#include "qprintf_example.h"
#if (__HEXAGON_ARCH__ >= 60)
#include "dspCV_hvx.h"
#endif
#include "dspCV_worker.h"

#include "AEEStdErr.h"

// includes
#include "qprintf.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "hexagon_protos.h"


#define MY_CENTER 5

extern void qprintf_example_asm();


/*===========================================================================
    DEFINITIONS
===========================================================================*/

typedef struct
{
    dspCV_synctoken_t    *token;            // worker pool token
    int32_t              retval;            // return value
} run_callback_t;

/*===========================================================================
    DECLARATIONS
===========================================================================*/

static int setClocks()
{
#if (__HEXAGON_ARCH__ < 62)
    return 0;
#endif
    HAP_power_request_t request;
    memset(&request, 0, sizeof(HAP_power_request_t)); //Important to clear the structure if only selected fields are updated.
    request.type = HAP_power_set_apptype;
    request.apptype = HAP_POWER_COMPUTE_CLIENT_CLASS;
    int retval = HAP_power_set(NULL, &request);
    if (retval) return AEE_EFAILED;

    // Configure clocks & DCVS mode
    memset(&request, 0, sizeof(HAP_power_request_t)); //Important to clear the structure if only selected fields are updated.
    request.type = HAP_power_set_DCVS_v2;
    request.dcvs_v2.dcvs_enable = 0;   // enable dcvs if desired, else it locks to target corner
    request.dcvs_v2.set_dcvs_params = TRUE;
    request.dcvs_v2.dcvs_params.target_corner = HAP_DCVS_VCORNER_NOM; // nominal voltage corner.
    request.dcvs_v2.set_latency = TRUE;
    request.dcvs_v2.latency = 100;
    retval = HAP_power_set(NULL, &request);
    if (retval) return AEE_EFAILED;

    return 0;
}

static void run_callback(void* data) {
	run_callback_t    *dptr = (run_callback_t*)data;

#if (__HEXAGON_ARCH__ >= 60)
    if (128 != dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0))
    {
        FARF(ERROR,"Could not acquire HVX in 128B mode!!");
        dptr->retval = AEE_EFAILED;
        goto bail;
    }
#endif

	// 1. Demonstrating how to use qprintf functions from C

	qprintf_set_mask(QPRINTF_MASK_ODD_32,QPRINTF_MASK_EVEN_32);

	// 2. Demonstrating how to use qprintf functions from assembly
	qprintf_example_asm();
    
#if (__HEXAGON_ARCH__ >= 60)
    HVX_Vector x = Q6_V_vsplat_R(-1);
    HVX_VectorPred pred = Q6_Q_vand_VR(x,-1);
    qprintf_V("x = %d\n",x);
    qprintf_V("%d is x\n",x);
    qprintf_Q("%x is pred\n",pred);
    qprintf_Q("pred = %dddd\n",pred);
	
    printf("Printing all V registers\n");
    qprintf_V_all();    
#endif
    printf("Printing all R registers\n");
    qprintf_R_all();    

    FARF(HIGH, "SUCCESS");
        
    dptr->retval = 0;

#if (__HEXAGON_ARCH__ >= 60)
bail:
    dspCV_hvx_unlock();
#endif
    dspCV_worker_pool_synctoken_jobdone(dptr->token);	
}

	
AEEResult qprintf_example_open(const char *uri, remote_handle64 *h) 
{
    *h = 0x00DEAD00;
    return 0;
}

AEEResult qprintf_example_close(remote_handle64 h) 
{
    return 0;
}

AEEResult qprintf_example_run(remote_handle64 h)
{
#if (__HEXAGON_ARCH__ >= 60)
    // power on HVX (this may be redundant on recent targets, but harmless).
    (void) dspCV_hvx_power_on();
#endif    
    // initiate worker pool (this may also be redundant, but harmless).
    (void) dspCV_worker_pool_init();

    // setClocks will boost clocks for 
    if (setClocks()) return AEE_EFAILED;

    // execute test case from worker thread pool (because these threads have 
    // larger stacks than the native thread from the RPC call).
    dspCV_worker_job_t   job;
    dspCV_synctoken_t    token;
    run_callback_t dptr;
    dptr.retval = 0;

    int numWorkers = 1;
    dptr.token = &token;
    dptr.retval = -1;
    job.dptr = (void *)&dptr;

    dspCV_worker_pool_synctoken_init(&token, numWorkers);
    job.fptr = run_callback;
    for (int i = 0; i < numWorkers; i++) (void) dspCV_worker_pool_submit(job);
    dspCV_worker_pool_synctoken_wait(&token);
    if (dptr.retval)
    {
        FARF(HIGH, "qprintf example test FAILURE %d", dptr.retval);
        goto bail;
    }

    FARF(HIGH, "qprintf example test SUCCESS");
bail:
    dspCV_worker_pool_deinit();
    return dptr.retval;
    
}


