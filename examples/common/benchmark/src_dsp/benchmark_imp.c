/**=============================================================================

@file
   benchmark_imp.c

@brief
   implementation file for dilate filter RPC interface.

Copyright (c) 2016-2017 QUALCOMM Technologies Incorporated.
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

#include <string.h>

// profile DSP execution time (without RPC overhead) via HAP_perf api's.
#include "HAP_perf.h"
#include "HAP_power.h"
#include "dspCV_worker.h"
#include "dspCV.h"

#include "AEEStdErr.h"

// includes
#include "benchmark.h"
#include "benchmark_asm.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

// (128-byte is only mode supported in this example)
#define VLEN 128 
#define AHEAD 1

/*===========================================================================
    DECLARATIONS
===========================================================================*/

/*===========================================================================
    TYPEDEF
===========================================================================*/

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult benchmark_open(const char *uri, remote_handle64 *h) 
{
    // benchmark has no state requiring a handle. Set to a dummy value.
    *h = 0x00DEAD00;
    return 0;
}

AEEResult benchmark_close(remote_handle64 h) 
{
    return 0;
}

AEEResult benchmark_setClocks(remote_handle64 h, int32 powerLevel, int32 latency, int32 dcvsEnable) 
{
    // Set client class (useful for monitoring concurrencies)
    HAP_power_request_t request;
    memset(&request, 0, sizeof(HAP_power_request_t)); //Important to clear the structure if only selected fields are updated.
    request.type = HAP_power_set_apptype;
    request.apptype = HAP_POWER_COMPUTE_CLIENT_CLASS;
    int retval = HAP_power_set(NULL, &request);
    if (retval) return AEE_EFAILED;

    // convert benchmark application power levels to dcvs_v2 clock levels.
    const uint32_t numPowerLevels = 6;
    const HAP_dcvs_voltage_corner_t voltageCorner[numPowerLevels] 
        = { HAP_DCVS_VCORNER_TURBO,
            HAP_DCVS_VCORNER_NOMPLUS,
            HAP_DCVS_VCORNER_NOM,
            HAP_DCVS_VCORNER_SVSPLUS,
            HAP_DCVS_VCORNER_SVS,
            HAP_DCVS_VCORNER_SVS2 };

    if ((uint32_t)powerLevel >= numPowerLevels) powerLevel = numPowerLevels - 1;

    // Configure clocks & DCVS mode
    memset(&request, 0, sizeof(HAP_power_request_t)); //Important to clear the structure if only selected fields are updated.
    request.type = HAP_power_set_DCVS_v2;
    request.dcvs_v2.dcvs_enable = dcvsEnable;   // enable dcvs if desired, else it locks to target corner
    request.dcvs_v2.dcvs_option = HAP_DCVS_V2_POWER_SAVER_MODE;
    request.dcvs_v2.set_dcvs_params = TRUE;
    request.dcvs_v2.dcvs_params.min_corner = HAP_DCVS_VCORNER_DISABLE; // no minimum
    request.dcvs_v2.dcvs_params.max_corner = HAP_DCVS_VCORNER_DISABLE; // no maximum
    request.dcvs_v2.dcvs_params.target_corner = voltageCorner[powerLevel];
    request.dcvs_v2.set_latency = TRUE;
    request.dcvs_v2.latency = latency;
    retval = HAP_power_set(NULL, &request);
    if (retval) return AEE_EFAILED;
    
// vote for HVX power
    memset(&request, 0, sizeof(HAP_power_request_t)); //Important to clear the structure if only selected fields are updated.
    request.type = HAP_power_set_HVX;
    request.hvx.power_up = TRUE;
    retval = HAP_power_set(NULL, &request);
    if (retval) return AEE_EFAILED;

    return AEE_SUCCESS;
}


