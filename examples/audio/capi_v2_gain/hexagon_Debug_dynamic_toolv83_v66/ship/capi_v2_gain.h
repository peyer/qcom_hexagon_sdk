#ifndef CAPI_V2_GAIN_H
#define CAPI_V2_GAIN_H
/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "Elite_CAPI_V2.h"
#include "audio_gain_calib.h"


// Gain module calibration parameters
#define CAPI_V2_GAIN_PARAM_ID_ENABLE 		GAIN_PARAM_MOD_ENABLE
#define CAPI_V2_GAIN_PARAM_ID_MASTER_GAIN 	GAIN_PARAM_MASTER_GAIN

// These are the extensions to CAPI_V2
/* NEED_FAR_END_DATA will send reference data to Tx modules in the last port, i.e. 1
   Number of input ports will be set to 2 if module raises this event
   0 - input pcm data
   1 - reference pcm data
*/
#define CAPI_V2_FRAMEWORK_EXTENSIONS_NEED_FAR_END_DATA 0x000000AA
#define CAPI_V2_MAX_EXTENSIONS 1

#ifdef __cplusplus
extern "C"
{
#endif

capi_v2_err_t capi_v2_gain_get_static_properties(
   capi_v2_proplist_t* init_set_properties,
   capi_v2_proplist_t* static_properties);
capi_v2_err_t capi_v2_gain_init(capi_v2_t* _pif,
                                 capi_v2_proplist_t* init_set_properties);

#ifdef __cplusplus
}
#endif

#endif // CAPI_V2_PASSTHRU_H

