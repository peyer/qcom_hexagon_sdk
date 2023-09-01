/**=============================================================================

@file
   dspCV_imp.c

@brief
   implementation of dspCV interface. Initializes/configures the DSP for 
   compute sessions.

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

#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "HAP_power.h"

#include "dspCV.h"
#include "dspCV_concurrency.h"
#include "dspCV_hvx.h"

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

/*===========================================================================
    TYPEDEF
===========================================================================*/

/*===========================================================================
    STATIC VARIABLES
===========================================================================*/
// default threshold at which audio concurrency may be considered too high
// to continue running CV (or other compute) on the DSP, based on multiple
// concurrency considerations. 
static int dspCV_audio_mpps_threshold_1_hvx = 110;
static int dspCV_audio_mpps_threshold_2_hvx = 110;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
DSPCV_CONCURRENCY_API void dspCV_concurrency_query(dspCV_ConcurrencyAttribute* attrib, int attribLen)
{
    if (NULL == attrib) return;

    HAP_power_response_t response;
    
    int i, numAvail;
    for (i = 0; i < attribLen; i++)
    {
        attrib[i].value = 0;    // default init value
        switch (attrib[i].ID)
        {
        case NUM_TOTAL_HVX_UNITS:
#if (__HEXAGON_ARCH__ >= 60)
            attrib[i].value = qurt_hvx_get_units() & 0xFF; // number of 64-byte units
#endif
            break;
            
        case NUM_AVAILABLE_HVX_UNITS:
            numAvail = dspCV_hvx_reserve(0);
            if (numAvail > 0)
            {
                attrib[i].value = numAvail;
                dspCV_hvx_unreserve();
            }
            break;

        case COMPUTE_RECOMMENDATION:
            attrib[i].value = COMPUTE_RECOMMENDATION_OK;

#if (__HEXAGON_ARCH__ < 65)
            response.type = HAP_power_get_client_class;
            int retval = HAP_power_get(NULL, &response);
            
            // advise to evict if camera streaming is running, in addition to either voice, or heavy audio
            // concurrency, where "heavy" is determined by comparing to a MPPS threshold.
            if (response.client_class & (HAP_POWER_STREAMING_1HVX_CLIENT_CLASS | HAP_POWER_STREAMING_2HVX_CLIENT_CLASS))
            {
                if (response.client_class & HAP_POWER_VOICE_CLIENT_CLASS)
                {
                    attrib[i].value = COMPUTE_RECOMMENDATION_NOT_OK;
                }
                else if (response.client_class & HAP_POWER_AUDIO_CLIENT_CLASS)
                {
                    unsigned int numHvx = (response.client_class & HAP_POWER_STREAMING_2HVX_CLIENT_CLASS) ? 2 : 1;
                    response.type = HAP_power_get_aggregateAVSMpps;
                    retval |= HAP_power_get(NULL, &response);
                    if (((2 == numHvx) && (response.aggregateAVSMpps > dspCV_audio_mpps_threshold_2_hvx))
                        || ((1 == numHvx) && (response.aggregateAVSMpps > dspCV_audio_mpps_threshold_1_hvx)))
                    {
                        attrib[i].value = COMPUTE_RECOMMENDATION_NOT_OK;
                    }
                }
            
                attrib[i].value = (0 == retval) ? attrib[i].value : dspCV_CONCURRENCY_ATTRIBUTE_UNSUPPORTED;
            }
#endif
            break;
        
        case CURRENT_DSP_MHZ_SETTING:
            response.type = HAP_power_get_clk_Freq;
            (void) HAP_power_get(NULL, &response);
            attrib[i].value = response.clkFreqHz / 1000;
            break;
        
        case EXISTING_CONCURRENCIES:
            response.type = HAP_power_get_client_class;
            (void) HAP_power_get(NULL, &response);
            attrib[i].value = response.client_class;
            break;
        
        default:
            attrib[i].value = dspCV_CONCURRENCY_ATTRIBUTE_UNSUPPORTED;
            break;
        }
    }

    return;
}

DSPCV_CONCURRENCY_API void
dspCV_concurrency_set_audio_mpps_1_hvx_threshold(int threshold)
{
    dspCV_audio_mpps_threshold_1_hvx = threshold;
}

DSPCV_CONCURRENCY_API void
dspCV_concurrency_set_audio_mpps_2_hvx_threshold(int threshold)
{
    dspCV_audio_mpps_threshold_2_hvx = threshold;
}

