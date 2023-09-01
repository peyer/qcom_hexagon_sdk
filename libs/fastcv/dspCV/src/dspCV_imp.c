/**=============================================================================

@file
   dspCV_imp.c

@brief
   implementation of dspCV interface. Initializes/configures the DSP for 
   compute sessions.

Copyright (c) 2013-2016 Qualcomm Technologies Incorporated.
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

#include "dspCV.h"
#include "dspCV_worker.h"
#include "dspCV_hvx.h"
#include "dspCV_concurrency.h"
#include "remote.h"
#include <stdlib.h>
#include <string.h>

// legacy clock setting defaults (prior to HAP_power_set API)
#define DEFAULT_MIN_DSP        800   // MHz DSP core clock
#define DEFAULT_MIN_BUS        250   // MHz SNOC frequency

// new clock setting defaults (used with HAP_power_set)
#define DEFAULT_TOTAL_MCPS                       900   // total MCPS requirement
#define DEFAULT_MCPS_PER_THREAD DEFAULT_TOTAL_MCPS/2   // peak per-thread MCPS requirement
#define DEFAULT_PEAK_BUS_BW                     8000   // peak MB/sec requirement
#define DEFAULT_BUS_PERCENT                       50   // usage percent
#if (__HEXAGON_ARCH__ < 62)
#define DEFAULT_MAX_LATENCY                     1000   // uSec tolerable wakeup latency upon RPC (or other) interrupt
#else
#define DEFAULT_MAX_LATENCY                     100    // uSec tolerable wakeup latency upon RPC (or other) interrupt
#endif

#include "qurt_mutex.h"

#include "AEEStdErr.h"
#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "HAP_power.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "HAP_pls.h"
extern uint32_t HAP_get_chip_family_id(void);


void dspCV_imp_constructor(void) __attribute__((constructor));
void dspCV_imp_destructor(void) __attribute__((destructor));

extern int __attribute__((weak)) HAP_get_dsp_domain(void);

#ifdef __cplusplus
}
#endif

// global (per PD) variables
unsigned int dspCV_hvx_powered = 0;

/*===========================================================================
    TYPEDEF
===========================================================================*/
typedef struct 
{
    qurt_mutex_t mutex;                 // Pointer to a mutex. Pointer itself is volatile.
} dspCV_syncobj_t;

/*===========================================================================
    STATIC VARIABLES
===========================================================================*/
static int constructor_successful = 0;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/

static void dspCV_deinitQ6_helper(dspCV_syncobj_t *dspCV_sync)
{
    qurt_mutex_t *mutex = &(dspCV_sync->mutex);
    qurt_mutex_lock(mutex);
        
    // revoke clock requests.
    {
        HAP_power_request_t request;
        //To remove any dcvs_v2 vote 
#if (__HEXAGON_ARCH__ >= 62)
        memset(&request, 0, sizeof(HAP_power_request_t)); //Remove all votes.
        request.type = HAP_power_set_DCVS_v2;
        request.dcvs_v2.dcvs_enable = TRUE;
        request.dcvs_v2.dcvs_option = HAP_DCVS_V2_POWER_SAVER_MODE;
        (void) HAP_power_set(NULL, &request);
#endif
        //To remove any mips_bw vote 
        memset(&request, 0, sizeof(HAP_power_request_t)); //Remove all votes.
        request.type = HAP_power_set_mips_bw;
        request.mips_bw.set_mips = TRUE;
        request.mips_bw.set_bus_bw = TRUE;
        request.mips_bw.set_latency = TRUE;
        request.mips_bw.latency = -1;
        (void) HAP_power_set(NULL, &request);
    }
   
    // reset HVX config and release reservation (if any)
    dspCV_hvx_reset_config();
    dspCV_hvx_unreserve();
    
    qurt_mutex_unlock(mutex);
}

static void dspCV_dtor(void *data) 
{
//    FARF(HIGH, "Entering dspCV_dtor()...");
    dspCV_syncobj_t *dspCV_sync = (dspCV_syncobj_t*) data;
        dspCV_deinitQ6_helper(dspCV_sync);
    dspCV_worker_pool_deinit();
    qurt_mutex_destroy(&(dspCV_sync->mutex));
}

static int dspCV_ctor(void* context, void* data) 
{
//    FARF(HIGH, "Entering dspCV_ctor()...");
    dspCV_syncobj_t *dspCV_sync = (dspCV_syncobj_t*) data;
    qurt_mutex_init(&(dspCV_sync->mutex));
    return dspCV_SUCCESS;
}

void
dspCV_imp_constructor()
{
    FARF(HIGH,"Reached dspCV_imp_constructor.");
    int retVal;
    retVal = dspCV_hvx_power_on();
    constructor_successful = (0 == retVal);
}

void
dspCV_imp_destructor()
{
    FARF(HIGH,"Reached dspCV_imp_destructor.");
    if (dspCV_hvx_powered) 
    {
        HAP_power_request_t request;
        request.type = HAP_power_set_HVX;
        request.hvx.power_up = FALSE;
        (void) HAP_power_set(&dspCV_hvx_powered, &request);
        dspCV_hvx_powered = 0;
    }
}    

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
/* This function retained for backward compatibility */
DSPCV_API int dspCV_initQ6(void) 
{
    return dspCV_initQ6_with_attributes(NULL, 0);
}

DSPCV_API int dspCV_initQ6_with_attributes(const dspCV_Attribute* attrib, int attribLen)
{

    int retval = 0;

    FARF(HIGH, "Entering dspCV_initQ6()...");
	

    int minDSP = 0;
    int minBus = 0;
    int totalMips = 0;
    int mipsPerThread = 0;
    int peakBusBw = 0;
    int busBwPercent = 0;
    int latencyTolerance = DEFAULT_MAX_LATENCY;
    unsigned int useClockPreset = 0;
    
    // local table to translate clock presets to percentages of max rates. 
    // values are for POWER_SAVING_MODE, NORMAL_MODE, MAX_PERFORMANCE_MODE, respectively.
#if (__HEXAGON_ARCH__ < 62)

    static const int clockPreset[NUM_AVAIL_CLOCK_PRESET_MODES] = {400, 600, 1000};
    static const int clockFloorPreset[NUM_AVAIL_CLOCK_PRESET_MODES] = {50, 100, 500};
    static const int busPreset[NUM_AVAIL_CLOCK_PRESET_MODES] = {3000, 6000, 12000};
#else
    static const HAP_dcvs_params_t dcvsPresets[NUM_AVAIL_CLOCK_PRESET_MODES] 
        = { {HAP_DCVS_VCORNER_SVS, HAP_DCVS_VCORNER_SVS2, HAP_DCVS_VCORNER_NOM, 0, 0, 0},
            {HAP_DCVS_VCORNER_NOM, HAP_DCVS_VCORNER_SVS, HAP_DCVS_VCORNER_TURBO, 0, 0, 0},
            {HAP_DCVS_VCORNER_TURBO, HAP_DCVS_VCORNER_TURBO, HAP_DCVS_VCORNER_TURBO, 0, 0, 0} };
    HAP_dcvs_params_t dcvsPreset = dcvsPresets[NORMAL_MODE];
    int dcvsEnable = 1;
#endif

    int hvxDisable = 0;

    int i;
    for (i = 0; i < attribLen; i++)
    {
        switch (attrib[i].ID)
        {
        case MINIMUM_DSP_MHZ:
            if (attrib[i].value > 0) minDSP = attrib[i].value;
            break;
            
        case MINIMUM_BUS_MHZ:
            if (attrib[i].value > 0) minBus = attrib[i].value;
            break;
            
        case DSP_TOTAL_MCPS:
            if (attrib[i].value > 0) totalMips = attrib[i].value;
            break;
            
        case DSP_MCPS_PER_THREAD:
            if (attrib[i].value > 0) mipsPerThread = attrib[i].value;
            break;
            
        case PEAK_BUS_BANDWIDTH_MBPS:
            if (attrib[i].value > 0) peakBusBw = attrib[i].value;
            break;
            
        case BUS_USAGE_PERCENT:
            if (attrib[i].value > 0) busBwPercent = attrib[i].value;
            break;
            
        case LATENCY_TOLERANCE:
            if (attrib[i].value > 0) latencyTolerance = attrib[i].value;
            if (attrib[i].value == 0) latencyTolerance = 1;
            break;
            
        case CLOCK_PRESET_MODE:
            if (0 <= attrib[i].value && attrib[i].value < NUM_AVAIL_CLOCK_PRESET_MODES) 
            {
                useClockPreset = 1;
#if (__HEXAGON_ARCH__ < 62)
                totalMips = clockPreset[attrib[i].value];
                peakBusBw = busPreset[attrib[i].value];
                mipsPerThread = clockFloorPreset[attrib[i].value];
                busBwPercent = DEFAULT_BUS_PERCENT;
#else
                dcvsPreset = dcvsPresets[attrib[i].value];
                dcvsEnable = (MAX_PERFORMANCE_MODE == attrib[i].value) ? 0 : 1;
#endif
            }
            break;
            
        case RESERVE_HVX_UNITS:
            retval = dspCV_hvx_reserve(attrib[i].value);
            if (0 > retval) return dspCV_ERR_HVX_UNSUPPORTED;
            if (0 == retval) return dspCV_ERR_HVX_BUSY;
            break;
            
        case DISABLE_HVX_USAGE:
            hvxDisable = 1;
            break;

        case DEFAULT_HVX_MODE:

            switch (attrib[i].value)
            {
                case HVX_MODE_DONT_CARE:
                    dspCV_hvx_set_default_mode(DSPCV_HVX_MODE_DONT_CARE);
                break;

                case HVX_MODE_64B:
                    dspCV_hvx_set_default_mode(DSPCV_HVX_MODE_64B);
                break;

                case HVX_MODE_128B:
                    dspCV_hvx_set_default_mode(DSPCV_HVX_MODE_128B);
                break;
                
                default:
                break;
            }
            break;
        
        case AUDIO_MPPS_EVICTION_THRESHOLD_1_STREAMING_HVX:
            dspCV_concurrency_set_audio_mpps_1_hvx_threshold(attrib[i].value);
            break;

        case AUDIO_MPPS_EVICTION_THRESHOLD_2_STREAMING_HVX:
            dspCV_concurrency_set_audio_mpps_2_hvx_threshold(attrib[i].value);
            break;

        default:
            FARF(HIGH,"Unsupported attribute ID %d",(int)(attrib[i].ID));
            return dspCV_ERR_UNSUPPORTED_ATTRIBUTE;
            break;
        }
    }

    dspCV_syncobj_t *dspCV_sync;
    retval = HAP_pls_add_lookup((uint32)dspCV_ctor, 0, sizeof(dspCV_syncobj_t),
                                    dspCV_ctor, 0,
                                    dspCV_dtor, (void**)&dspCV_sync);
                 
    if (retval)
    {
        FARF(HIGH, "dspCV_ctor FAILED, retval %d!",retval);
        return dspCV_ERR_CONSTRUCTOR_FAILED;
    }
    
    qurt_mutex_t *mutex = &(dspCV_sync->mutex);
    qurt_mutex_lock(mutex);
    
    // set HAP app type to COMPUTE.
    HAP_power_request_t request;
    request.type = HAP_power_set_apptype;
    request.apptype = HAP_POWER_COMPUTE_CLIENT_CLASS;
    retval = HAP_power_set(NULL, &request);
    if (0 != retval && HAP_POWER_ERR_UNSUPPORTED_API != retval)
    {
        FARF(ERROR, "dspCV_initQ6 app type setting FAILED, result %d\n", retval );
        retval = dspCV_ERR_CLIENT_CLASS_SETTING_FAILED;
        goto bail;
    }

    // vote for clocks. In case multiple (possibly conflicting) attributes were given, prioritize 
    // 1. clock preset
    // 2. peak/average attributes
    // 3. deprecated min clock/bus
    if (1 == useClockPreset)
    {
#if (__HEXAGON_ARCH__ < 62)
        request.type = HAP_power_set_mips_bw;
        request.mips_bw.set_mips = request.mips_bw.set_bus_bw = request.mips_bw.set_latency = TRUE;
        request.mips_bw.mipsPerThread = mipsPerThread;
        request.mips_bw.mipsTotal = totalMips;
        request.mips_bw.bwBytePerSec = (uint64)peakBusBw * 1000000;
        request.mips_bw.busbwUsagePercentage = (unsigned short) busBwPercent;
        request.mips_bw.latency = latencyTolerance;
        
        retval = HAP_power_set(NULL, &request);
        FARF(HIGH, "dspCV_initQ6 votes %d total MCPS, %d MCPS per thread.", totalMips, mipsPerThread);
        FARF(HIGH, "dspCV_initQ6 votes bus BW %d MB/sec, bus usage %d percent, and max latency %d uSec", 
            peakBusBw, busBwPercent, latencyTolerance );
        
#else
        //To remove any mips_bw vote 
        memset(&request, 0, sizeof(HAP_power_request_t)); //Remove all votes.
        request.type = HAP_power_set_mips_bw;
        request.mips_bw.set_mips = TRUE;
        request.mips_bw.set_bus_bw = TRUE;
        request.mips_bw.set_latency = TRUE;
        request.mips_bw.latency = -1;
        (void) HAP_power_set(NULL, &request);

        request.type = HAP_power_set_DCVS_v2;
        request.dcvs_v2.dcvs_enable = dcvsEnable;
        request.dcvs_v2.dcvs_option = HAP_DCVS_V2_POWER_SAVER_MODE;
        request.dcvs_v2.set_latency = TRUE;
        request.dcvs_v2.latency = latencyTolerance;
        request.dcvs_v2.set_dcvs_params = TRUE;
        request.dcvs_v2.dcvs_params = dcvsPreset;
        
        retval = HAP_power_set(NULL, &request);
        FARF(HIGH, "dspCV_initQ6 votes for clocks via voltage corners- target=%d, min=%d, max=%d.", 
            (int)dcvsPreset.target_corner, (int)dcvsPreset.min_corner, (int)dcvsPreset.max_corner);
        FARF(HIGH, "(0=disabled, 1=SVS2, 2=SVS, 3=SVSPLUS, 4=NOMINAL, 5=NOMINALPLUS, 6=TURBO)" );
#endif
    }
    else
    {
        // choose DSP clock vote
        if (0 == totalMips && 0 == mipsPerThread)
        {
            // guess reasonable values based on specified minimum DSP frequency (if available), else vote for default clock.
            totalMips = (0 < minDSP) ? minDSP * 2 : DEFAULT_TOTAL_MCPS;
            mipsPerThread = (0 < minDSP) ? minDSP : DEFAULT_MCPS_PER_THREAD;
            minDSP = (0 < minDSP) ? minDSP : DEFAULT_MIN_DSP;
        }
        else
        {
            minDSP = (0 < mipsPerThread) ? mipsPerThread : totalMips / 2;
            totalMips = (0 < totalMips) ? totalMips : mipsPerThread * 2;
            mipsPerThread = (0 < mipsPerThread) ? mipsPerThread : totalMips;  // set equal in case client is single-threaded
        }
        
        // choose bus BW vote
        if (0 == peakBusBw && 0 == busBwPercent)
        {
            // guess reasonable values based on specified minimum bus frequency (if available), else vote for default bus bw.
            peakBusBw = (0 < minBus) ? minBus * 16 : DEFAULT_PEAK_BUS_BW;
            busBwPercent = DEFAULT_BUS_PERCENT;
            minBus = (0 < minBus) ? minBus : DEFAULT_MIN_BUS;
        }
        else
        {
            minBus = (0 < peakBusBw) ? peakBusBw : DEFAULT_MIN_BUS;
            peakBusBw = (0 < peakBusBw) ? peakBusBw : DEFAULT_PEAK_BUS_BW;
            busBwPercent = (0 < busBwPercent) ? busBwPercent : DEFAULT_BUS_PERCENT; 
        }

        // remove any DCVSv2 vote
#if (__HEXAGON_ARCH__ >= 62)
        memset(&request, 0, sizeof(HAP_power_request_t)); //Remove all votes.
        request.type = HAP_power_set_DCVS_v2;
        request.dcvs_v2.dcvs_enable = TRUE;
        request.dcvs_v2.dcvs_option = HAP_DCVS_V2_POWER_SAVER_MODE;
        (void) HAP_power_set(NULL, &request);
#endif
        
        request.type = HAP_power_set_mips_bw;
        request.mips_bw.set_mips = request.mips_bw.set_bus_bw = request.mips_bw.set_latency = TRUE;
        request.mips_bw.mipsPerThread = mipsPerThread;
        request.mips_bw.mipsTotal = totalMips;
        request.mips_bw.bwBytePerSec = (uint64)peakBusBw * 1000000;
        request.mips_bw.busbwUsagePercentage = (unsigned short) busBwPercent;
        request.mips_bw.latency = latencyTolerance;
        
        retval = HAP_power_set(NULL, &request);
        FARF(HIGH, "dspCV_initQ6 votes %d total MCPS, %d MCPS per thread.", totalMips, mipsPerThread);
        FARF(HIGH, "dspCV_initQ6 votes bus BW %d MB/sec, bus usage %d percent, and max latency %d uSec", 
            peakBusBw, busBwPercent, latencyTolerance );
    }
    if (0 != retval) 
    {
        FARF(ERROR, "dspCV_initQ6 power/clock settings FAILED, result %d\n", retval );
        retval = dspCV_ERR_CLOCK_SETTING_FAILED;
        goto bail;
    }

        // initialize the worker pool
    if (!dspCV_worker_pool_available())
    {
        retval = dspCV_worker_pool_init();
        if (retval) 
        {
            FARF(ERROR, "dspCV_initQ6 creating worker pool, FAILED status %d" , retval);
            retval = dspCV_ERR_WORKER_POOL_FAILED;
            goto bail;
        }
    }
        
        if (hvxDisable)
        {
            dspCV_hvx_unreserve();
            dspCV_hvx_disable();
        }
 
bail:
    qurt_mutex_unlock(mutex);
    if (retval) dspCV_hvx_unreserve();
    return retval;
}


DSPCV_API int dspCV_deinitQ6(void)
{
    FARF(HIGH, "Entering dspCV_deinitQ6()...\n" );

    dspCV_syncobj_t *dspCV_sync;
    int retval = HAP_pls_add_lookup((uint32)dspCV_ctor, 0, sizeof(dspCV_syncobj_t),
                                    dspCV_ctor, 0,
                                    dspCV_dtor, (void**)&dspCV_sync);
                 
    if (retval)
    {
        FARF(HIGH, "dspCV_deinit FAILED to find context, retval %d!",retval);
        return dspCV_ERR_BAD_STATE;
    }

    dspCV_deinitQ6_helper(dspCV_sync);

    dspCV_hvx_unreserve();
    return dspCV_SUCCESS;

}

DSPCV_API int dspCV_getQ6_concurrency_attributes(dspCV_ConcurrencyAttribute* attrib, int attribLen)
{
    dspCV_concurrency_query(attrib, attribLen);
    return dspCV_SUCCESS;
}