/**=============================================================================

@file
   dspCV_hvx.cpp

@brief
   Utility providing a multi-priority thread worker pool for 
   multi-threaded FastCV applications.

Copyright (c) 2014-2017 Qualcomm Technologies Incorporated.
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

#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"

#include "dspCV_hvx.h"
#include "dspCV_concurrency.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include "HAP_power.h"

#ifdef __cplusplus
extern "C"
{
#endif
#if (__HEXAGON_ARCH__ >= 60)
#include "qurt_hvx.h"
#endif
#ifdef __cplusplus
}
#endif

extern unsigned int dspCV_hvx_powered;

/*===========================================================================
    DEFINE
===========================================================================*/
// due to how long it takes to power HVX on/off (~160 uSec) and how little leakage is consumed while HVX is gated (~1mW),
// it is preferable to leave HVX power on throughout session, starting from first power-on request, up until session closes.
// therefore, do not actually power it off.
#define DSPCV_KEEP_HVX_POWER_ON

#define DSPCV_DEFAULT_HVX_RESERVED 0
#define DSPCV_DEFAULT_HVX_ENABLED 1
#if (__HEXAGON_ARCH__ >= 65)
#define DSPCV_DEFAULT_HVX_MODE DSPCV_HVX_MODE_128B
#else
#define DSPCV_DEFAULT_HVX_MODE DSPCV_HVX_MODE_DONT_CARE
#endif
/*===========================================================================
    TYPEDEF
===========================================================================*/

/*===========================================================================
    STATIC VARIABLES
===========================================================================*/

static int reserved_hvx_units = DSPCV_DEFAULT_HVX_RESERVED;
static int hvx_enabled = DSPCV_DEFAULT_HVX_ENABLED;
static dspCV_hvx_mode_t default_hvx_mode = DSPCV_DEFAULT_HVX_MODE;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

void
dspCV_hvx_reset_config(void)
{
    reserved_hvx_units = DSPCV_DEFAULT_HVX_RESERVED;
    hvx_enabled = DSPCV_DEFAULT_HVX_ENABLED;
    default_hvx_mode = DSPCV_DEFAULT_HVX_MODE;
}

int
dspCV_hvx_reserve(unsigned int num_units)
{
#if (__HEXAGON_ARCH__ < 60)
    return -1; // legacy targets with no HVX support 
#else
    if (!hvx_enabled || qurt_hvx_get_units() <= 0) return -1; // HVX not supported in this target

    if (0 == num_units) num_units = QURT_HVX_RESERVE_ALL_AVAILABLE;

    // determine if camera is using (or requesting to use) HVX contexts, and don't reserve those.
    dspCV_ConcurrencyAttribute attrib[] = 
    {
        {EXISTING_CONCURRENCIES, 0},  // query for compute concurrency recommendation
    };
    dspCV_concurrency_query(attrib, sizeof(attrib)/sizeof(attrib[0]));
    
    if (dspCV_CONCURRENCY_ATTRIBUTE_UNSUPPORTED != attrib[0].value)
    {
        if (attrib[0].value & dspCV_DUAL_HVX_CAMERA_STREAMING_CONCURRENCY_BITMASK)
        {   
            int remaining = (int)(qurt_hvx_get_units() & 0xFF) - 4;
            if (remaining <= 0) 
            {
                reserved_hvx_units = 0;
                return 0;
            }
            else num_units = (unsigned int) remaining;
        }
        
        if (attrib[0].value & dspCV_SINGLE_HVX_CAMERA_STREAMING_CONCURRENCY_BITMASK)
        {   
            int remaining = (int)(qurt_hvx_get_units() & 0xFF) - 2;
            if (remaining <= 0) 
            {
                reserved_hvx_units = 0;
                return 0;
            }
            else num_units = (unsigned int) remaining;
        }
    }
    
    int retVal = qurt_hvx_reserve(num_units);
    switch (retVal)
    {
        case QURT_HVX_RESERVE_ALREADY_MADE:
            return reserved_hvx_units;
            break;
            
        case QURT_HVX_RESERVE_NOT_SUPPORTED:
        case QURT_HVX_RESERVE_NOT_SUCCESSFUL:
            reserved_hvx_units = 0;
            return 0;
            break;
            
        default:
            if (retVal > 0)
            {
                reserved_hvx_units = retVal;
                return retVal;
            }
            else
            {
                reserved_hvx_units = 0;
                return -1;
            }
    }
#endif
}

void
dspCV_hvx_unreserve(void)
{
#if (__HEXAGON_ARCH__ < 60)
    return;
#else
    (void) qurt_hvx_cancel_reserve();
    reserved_hvx_units = 0;
#endif
}

int
dspCV_hvx_num_reserved(void)
{
    return reserved_hvx_units;
}

int
dspCV_hvx_power_on(void)
{
#if (__HEXAGON_ARCH__ < 60)
    return -1; // legacy targets with no HVX support 
#else
    if (!dspCV_hvx_powered) 
    {
        HAP_power_request_t request;
        request.type = HAP_power_set_HVX;
        request.hvx.power_up = TRUE;
        int retval = HAP_power_set(&dspCV_hvx_powered, &request);
        if (0 != retval)
        {
            FARF(ERROR,"dspCV unable to power on HVX, status %d!", retval);
            return -1;
        }
        dspCV_hvx_powered = 1;
    }
    return 0;
#endif
}

void
dspCV_hvx_power_off(void)
{
#if (__HEXAGON_ARCH__ >= 60)
#ifndef DSPCV_KEEP_HVX_POWER_ON
    if (dspCV_hvx_powered) 
    {
        HAP_power_request_t request;
        request.type = HAP_power_set_HVX;
        request.hvx.power_up = FALSE;
        (void) HAP_power_set(&dspCV_hvx_powered, &request);
        dspCV_hvx_powered = 0;
    }
#endif
#endif    
    return;
}

int
dspCV_hvx_lock(dspCV_hvx_mode_t mode, unsigned int block)
{
#if (__HEXAGON_ARCH__ < 60)
    return -1; // legacy targets with no HVX support 
#else
    if (!hvx_enabled) return -1;

// Starting with v65, the RTOS supports HVX context switching. 
// Treat all hvx locks as blocking now, so they can succeed, and 
// be scheduled according to RTOS scheduler via thread priority.
#if (__HEXAGON_ARCH__ >= 65)
    block = 1;
#endif

    qurt_hvx_mode_t qurt_mode;
    int VLEN;
    
    if (DSPCV_HVX_MODE_DONT_CARE == mode) mode = default_hvx_mode;
    
    switch (mode)
    {
        case DSPCV_HVX_MODE_DONT_CARE:
            qurt_mode = qurt_hvx_get_mode();
            switch (qurt_mode)
            {
                case QURT_HVX_MODE_64B:
                    VLEN = 64;
                    break;
                    
                case QURT_HVX_MODE_128B:
                    VLEN = 128;
                    break;
                   
                default:
                FARF(HIGH,"Unknown mode %d",qurt_mode);
                    return -2;
                    break;
            }
            break;
        
        case DSPCV_HVX_MODE_64B:
            qurt_mode = QURT_HVX_MODE_64B;
            VLEN = 64;
            break;
            
        case DSPCV_HVX_MODE_128B:
            qurt_mode = QURT_HVX_MODE_128B;
            VLEN = 128;
            break;
            
        default:
                FARF(HIGH,"Unknown mode %d",qurt_mode);
            return -3;
    }
    
    int retval;
    if (block)
    {
    //FARF(HIGH,"Calling blocking lock with qurt_mode %d",qurt_mode);
        retval = qurt_hvx_lock(qurt_mode);
    }
    else
    {
    //FARF(HIGH,"Calling non-blocking try_lock with qurt_mode %d",qurt_mode);
        retval = qurt_hvx_try_lock(qurt_mode);
    }
    
    if (retval)
    {
//        FARF(HIGH,"qurt return val %d\n",retval);
        return -4;
    }
    else
    {
      //FARF(HIGH,"lock SUCCEEDED, VLEN %d",VLEN);
        return VLEN;
    }
#endif
}

void
dspCV_hvx_unlock(void)
{
#if (__HEXAGON_ARCH__ < 60)
    return; // legacy targets with no HVX support 
#else
    (void) qurt_hvx_unlock();
#endif
}

void
dspCV_hvx_prepare_mt_job(dspCV_hvx_config_t *hvx_config)
{
#if (__HEXAGON_ARCH__ >= 60)
    if (!hvx_enabled)
    {
        hvx_config->tempReserve = 0;
        hvx_config->numUnits = 0;
        hvx_config->numThreads = 0;
        return;
    }
        
    // check whether HVX is reserved for this protection domain
    hvx_config->numUnits = dspCV_hvx_num_reserved();
    
    // if not, see if we can temporarily reserve them for this invocation only
    // Note this is one of many possible logics for choosing whether to use HVX or 
    // scalar version (or some mixture of both across worker threads) of algorithm, 
    // depending on end-to-end use case design.
    hvx_config->tempReserve = 0;
    if (0 == hvx_config->numUnits)
    {
        hvx_config->numUnits = dspCV_hvx_reserve(0); // reserve all units
        if (0 < hvx_config->numUnits) hvx_config->tempReserve = 1;
    }
    
    // if client doesn't specify required mode, fallback to default
    if (DSPCV_HVX_MODE_DONT_CARE == hvx_config->mode) hvx_config->mode = default_hvx_mode;
    
    // choose 64byte or 128 byte mode, based on whether there are odd or even number of units
    if ((hvx_config->mode == DSPCV_HVX_MODE_64B) || 
        (hvx_config->mode == DSPCV_HVX_MODE_DONT_CARE && (hvx_config->numUnits & 1)))
    {
        hvx_config->VLEN = 64;
        hvx_config->mode = DSPCV_HVX_MODE_64B;
        hvx_config->numThreads = hvx_config->numUnits;
    }
    else
    {
        hvx_config->VLEN = 128;
        hvx_config->mode = DSPCV_HVX_MODE_128B;
        hvx_config->numThreads = (qurt_hvx_get_units() >> 8) & 0xFF;
        // handle case where only 1 64-byte unit was available
        if (0 == hvx_config->numThreads)
        {
            if (hvx_config->tempReserve) 
            {
                dspCV_hvx_unreserve();
                hvx_config->tempReserve = 0;
            }
            hvx_config->numUnits = 0;
        }
    }
    
    // if using HVX, make sure it turns on properly
    if (hvx_config->numUnits > 0 && 0!= dspCV_hvx_power_on()) hvx_config->numUnits = 0;
#endif
}

void
dspCV_hvx_cleanup_mt_job(dspCV_hvx_config_t *hvx_config)
{
#if (__HEXAGON_ARCH__ >= 60)
    // if HVX was used, indicate it can be turned off
    if (hvx_config->numUnits > 0) dspCV_hvx_power_off();
    // if HVX was temporarily reserved, unreserve it
    if (hvx_config->tempReserve) dspCV_hvx_unreserve();
#endif
}

void
dspCV_hvx_disable(void)
{
    hvx_enabled = 0;
}

void
dspCV_hvx_enable(void)
{
    hvx_enabled = 1;
}

void
dspCV_hvx_set_default_mode(dspCV_hvx_mode_t mode)
{
    default_hvx_mode = mode;
}
