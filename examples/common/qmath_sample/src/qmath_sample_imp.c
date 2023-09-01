/**=============================================================================

@file
   qmath_sample_imp.cpp

@brief
   implementation file for testing qmath.

Copyright (c) 2017 QUALCOMM Technologies Incorporated.
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
#include "HAP_power.h"

#include "qmath_sample.h"
#include "dspCV_hvx.h"
#include "dspCV_worker.h"

#include "AEEStdErr.h"

// includes
#include "qmath.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "hexagon_protos.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/

/*===========================================================================
    DECLARATIONS
===========================================================================*/

/*===========================================================================
    TYPEDEF
===========================================================================*/
typedef struct
{
    dspCV_synctoken_t    *token;            // worker pool token
    int32_t              loops;             // return value
    int32_t              refFunc;           // which reference function to profile
    float                aRef;
    int32_t              retval;            // return value
} quadratic_callback_t;

    // set up test vectors a, b, and c, for the quadratic equation ax^2 + bx + c = 0.
    // Choosing some values with infinite decimal representations.
    const double a_d[32] = {0.0, 1.0, 7.0/1900.0, 23.0/19.0, -1.0/37.0, -11.0/79.0, 5.0/7.0, 
        -50.0/13.0, 1.0/47.0, -1.0/3.0, 1.0/7.0, -1.0/11.0, 10000.0/13.0, 1.0/1700.0, 
        -3.0/7.0, -1.0/130.0, 1.0/1.3, -900.0/7.0, 9.0/7000.0, 65.0/3.0, -5.0/13.0, 
        100.0/3.0, -1.0/17.0, -1.0/71.0, 500.0/71.0, 70.0/71.0, -20.0/3.0, 200.0/31.0, 
        5.0/310.0, -16.0/17.0, 100.0/41.0, 1000.0/37.0};
    const double b_d[32] = {0.0, 0.0, -100.0/71.0, 5000.0/71.0, 7000.0/71.0, -2000.0/3.0, 
        20000.0/31.0, 5000.0/310.0, -1600.0/17.0, 10000.0/41.0, -100.0/19.0, 10000.0/37.0, 
        -10000.0/3.0, 100.0/7.0, -100.0/11.0, 10000.0/13.0, 1000.0/17.0, -3000.0/7.0, 
        700.0/1900.0, 2300.0/19.0, -100.0/37.0, -1100.0/0.79, 500.0/7.0, -5000.0/13.0, 
        10000.0/47.0, -100.0/130.0, 100.0/1.3, -9000.0/7.0, 900.0/7000.0, 6500.0/3.0, 
        -500.0/13.0, 10000.0/3.0};
    const double c_d[32] = {0.0, 0.0, -1.0/3.0, 1.0/7.0, -1.0/11.0, 10000.0/13.0, 1.0/1700.0, 
        -3.0/7.0, 7.0/1900.0, 23.0/19.0, -1.0/37.0, -11.0/79.0, 5.0/7.0, -500.0/13.0, 1.0/47.0, 
        -1.0/130.0, 1.0/1.3, -900.0/7.0, 9.0/7000.0, 65.0/3.0, -5.0/13.0, 100.0/3.0, -1.0/17.0,
        -20.0/3.0, 200.0/31.0, 5.0/310.0, -16.0/17.0, 100.0/41.0, -1.0/19.0, 1000.0/37.0, 
        -1.0/71.0, 70.0/71.0};
        
    float a_f[32], b_f[32], c_f[32];
    double rootqd_minus[32], rootqd_plus[32];
/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
static void quadratic_callback(void* data)
{
    quadratic_callback_t    *dptr = (quadratic_callback_t*)data;

    if (128 != dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0))
    {
        FARF(ERROR,"Could not acquire HVX in 128B mode!!");
        dptr->retval = AEE_EFAILED;
        goto bail;
    }
    
    double root_minus[32], root_plus[32];
    
    // cast down to single-precision float
    for (int i = 0; i < 32; i++)
    {
        a_f[i] = (float)a_d[i];
        b_f[i] = (float)b_d[i];
        c_f[i] = (float)c_d[i];
    }
    
    // test native double precision
    uint64_t t0 = HAP_perf_get_pcycles();
    for (int i = 0; i < 32; i++)
    {
        root_minus[i] = (-b_d[i] - sqrt(b_d[i] * b_d[i] - 4 * a_d[i] * c_d[i])) / (2 * a_d[i]);
        root_plus[i] = (-b_d[i] + sqrt(b_d[i] * b_d[i] - 4 * a_d[i] * c_d[i])) / (2 * a_d[i]);
    }
    uint64_t t1 = HAP_perf_get_pcycles();
    
    // test native single-precision float
    float rootf_minus[32], rootf_plus[32];
    uint64_t t2 = HAP_perf_get_pcycles();
    for (int i = 0; i < 32; i++)
    {
        rootf_minus[i] = (-b_f[i] - sqrt(b_f[i] * b_f[i] - 4 * a_f[i] * c_f[i])) / (2 * a_f[i]);
        rootf_plus[i] = (-b_f[i] + sqrt(b_f[i] * b_f[i] - 4 * a_f[i] * c_f[i])) / (2 * a_f[i]);
    }
    uint64_t t3 = HAP_perf_get_pcycles();

    // test pseudo-float
    qm_vqf32_t a_q = qm_vqf32_from_double((double*)a_d);
    qm_vqf32_t b_q = qm_vqf32_from_double((double*)b_d);
    qm_vqf32_t c_q = qm_vqf32_from_double((double*)c_d);
    
    // solve with inline functions
    uint64_t t4 = HAP_perf_get_pcycles();
    HVX_Vector constexp = Q6_V_vsplat_R(1); // generate vector of 1's
    qm_vqf32_t fourac = qm_vqf32_mpy_inl(a_q, c_q);
    HVX_Vector a_exp = qm_vqf32_getexp(a_q);
    a_exp = Q6_Vw_vadd_VwVw_sat(a_exp, constexp); // compute 2a by adding 1 to exponent (this is faster than a full add or multiply)
    a_q = Q6_W_vcombine_VV(a_exp, qm_vqf32_getmant(a_q));
    
    constexp = Q6_Vw_vadd_VwVw(constexp, constexp); // generate vector of 2's
    HVX_Vector fourac_exp = qm_vqf32_getexp(fourac);
    fourac_exp = Q6_Vw_vadd_VwVw_sat(fourac_exp, constexp); // multiply by 4, by adding 2 to exponent
    fourac = Q6_W_vcombine_VV(fourac_exp, qm_vqf32_getmant(fourac));
    
    qm_vqf32_t sqrtdiff = qm_vqf32_sqrt_inl(qm_vqf32_sub_inl(qm_vqf32_mpy_inl(b_q, b_q), fourac));
    b_q = qm_vqf32_negate_inl(b_q);
    qm_vqf32_t rootq_minus_inl = qm_vqf32_mpy_inl(qm_vqf32_inverse_inl(a_q), qm_vqf32_sub_inl(b_q, sqrtdiff));
    qm_vqf32_t rootq_plus_inl = qm_vqf32_mpy_inl(qm_vqf32_inverse_inl(a_q), qm_vqf32_add_inl(b_q, sqrtdiff));
    uint64_t t5 = HAP_perf_get_pcycles();
    
    // solve with (non-inlined) function calls
    a_q = qm_vqf32_from_double((double*)a_d);
    b_q = qm_vqf32_from_double((double*)b_d);
    c_q = qm_vqf32_from_double((double*)c_d);
    
    uint64_t t6 = HAP_perf_get_pcycles();
    constexp = Q6_V_vsplat_R(1); // generate vector of 1's
    fourac = qm_vqf32_mpy(a_q, c_q);
    a_exp = qm_vqf32_getexp(a_q);
    a_exp = Q6_Vw_vadd_VwVw_sat(a_exp, constexp); // 2a
    a_q = Q6_W_vcombine_VV(a_exp, qm_vqf32_getmant(a_q));
    
    constexp = Q6_Vw_vadd_VwVw(constexp, constexp); // generate vector of 2's
    fourac_exp = qm_vqf32_getexp(fourac);
    fourac_exp = Q6_Vw_vadd_VwVw_sat(fourac_exp, constexp);
    fourac = Q6_W_vcombine_VV(fourac_exp, qm_vqf32_getmant(fourac));
    
    sqrtdiff = qm_vqf32_sqrt(qm_vqf32_sub(qm_vqf32_mpy(b_q, b_q), fourac));
    b_q = qm_vqf32_negate(b_q);
    qm_vqf32_t rootq_minus = qm_vqf32_mpy(qm_vqf32_inverse(a_q), qm_vqf32_sub(b_q, sqrtdiff));
    qm_vqf32_t rootq_plus = qm_vqf32_mpy(qm_vqf32_inverse(a_q), qm_vqf32_add(b_q, sqrtdiff));
    uint64_t t7 = HAP_perf_get_pcycles();
    
    qm_vqf32_to_double(rootq_minus, rootqd_minus);
    qm_vqf32_to_double(rootq_plus, rootqd_plus);

    for (int i = 0; i < 32; i++)
    {
        if (((uint32_t*)&rootq_minus)[i] != ((uint32_t*)&rootq_minus_inl)[i] 
            || ((uint32_t*)&rootq_plus)[i] != ((uint32_t*)&rootq_plus_inl)[i])
        {
            FARF(HIGH, "Difference detected between inline and non-inlined versions of pseudo-float!");
            FARF(HIGH, "i = %d, non-inl = 0x%8x, inl = 0x%8x", i , ((uint32_t*)&rootq_minus)[i], ((uint32_t*)&rootq_minus_inl)[i]);
            FARF(HIGH, "i = %d, non-inl = 0x%8x, inl = 0x%8x", i , ((uint32_t*)&rootq_plus)[i], ((uint32_t*)&rootq_plus_inl)[i]);
            dptr->retval = 1;
            goto bail;
        }
    }
    
    FARF(HIGH, "Profiling result (PCycles for a vector of 32 test cases, single-threaded)");
    FARF(HIGH, "double: %d, float: %d, qfloat: %d, inlined qfloat: %d", (int)(t1-t0),
        (int)(t3-t2), (int)(t7-t6), (int)(t5-t4));
    
    FARF(HIGH, "Comparing result of illegal divide zero by zero. ");
    FARF(HIGH, "Native float/double gives -nan, while qfloat should give zero, per design.");
    FARF(HIGH, "-b-sqrt(b*b-4ac))/2a: double: %e, float: %e, qfloat: %e",root_minus[0], rootf_minus[0], rootqd_minus[0]);
    FARF(HIGH, "-b+sqrt(b*b-4ac))/2a: double: %e, float: %e, qfloat: %e",root_plus[0], rootf_plus[0], rootqd_plus[0]);

    FARF(HIGH, "Comparing result for test case where there is only 1 root, at (0,0)");
    FARF(HIGH, "(-b-sqrt(b*b-4ac))/2a: double: %e, float: %e, qfloat: %e",root_minus[1], rootf_minus[1], rootqd_minus[1]);
    FARF(HIGH, "(-b+sqrt(b*b-4ac))/2a: double: %e, float: %e, qfloat: %e",root_plus[1], rootf_plus[1], rootqd_plus[1]);

    FARF(HIGH, "Comparing error vs. native double for 32 test cases (qfloat performs better than float in most cases).");
    
    for (int i = 2; i < 32; i++)
    {
        FARF(HIGH, "-b-sqrt(b*b-4ac))/2a error: float: %e qfloat: %e", fabs((root_minus[i] - (double)rootf_minus[i])/root_minus[i]), fabs((root_minus[i] - rootqd_minus[i])/root_minus[i]));
        FARF(HIGH, "-b+sqrt(b*b-4ac))/2a error: float: %e qfloat: %e", fabs((root_plus[i] - (double)rootf_plus[i])/root_plus[i]), fabs((root_plus[i] - rootqd_plus[i])/root_plus[i]));
    }

    dptr->retval = 0;

bail:
    dspCV_hvx_unlock();
    dspCV_worker_pool_synctoken_jobdone(dptr->token);
}

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

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
AEEResult qmath_sample_open(const char *uri, remote_handle64 *h) 
{
    *h = 0x00DEAD00;
    return 0;
}

AEEResult qmath_sample_close(remote_handle64 h) 
{
    return 0;
}
AEEResult qmath_sample_quadratic(remote_handle64 h)
{
    // power on HVX (this may be redundant on recent targets, but harmless).
    (void) dspCV_hvx_power_on();
    
    // initiate worker pool (this may also be redundant, but harmless).
    (void) dspCV_worker_pool_init();

    // setClocks will boost clocks for 
    if (setClocks()) return AEE_EFAILED;

    // execute test case from worker thread pool (because these threads have 
    // larger stacks than the native thread from the RPC call).
    dspCV_worker_job_t   job;
    dspCV_synctoken_t    token;
    quadratic_callback_t dptr;
    dptr.retval = 0;

    int numWorkers = 1;
    dptr.token = &token;
    dptr.retval = -1;
    job.dptr = (void *)&dptr;

    dspCV_worker_pool_synctoken_init(&token, numWorkers);
    job.fptr = quadratic_callback;
    for (int i = 0; i < numWorkers; i++) (void) dspCV_worker_pool_submit(job);
    dspCV_worker_pool_synctoken_wait(&token);
    if (dptr.retval)
    {
        FARF(HIGH, "quadratic test FAILURE %d", dptr.retval);
        goto bail;
    }

    FARF(HIGH, "quadratic test SUCCESS");
bail:
    dspCV_worker_pool_deinit();
    return dptr.retval;
}


