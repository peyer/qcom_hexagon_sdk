#ifndef CAPI_V2_GAIN_V2_H
#define CAPI_V2_GAIN_V2_H
/*==============================================================================
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "Elite_CAPI_V2.h"
#include "audio_gain32ch_calib.h"


// Gain32ch module calibration parameters
#define CAPI_V2_GAIN_PARAM_ID_ENABLE 		GAIN_32CH_PARAM_MOD_ENABLE
#define CAPI_V2_GAIN_PARAM_ID_MASTER_GAIN 	GAIN_32CH_PARAM_MASTER_GAIN


#ifdef __cplusplus
extern "C"
{
#endif

capi_v2_err_t capi_v2_gain_32ch_get_static_properties(capi_v2_proplist_t* init_set_properties, capi_v2_proplist_t* static_properties);

capi_v2_err_t capi_v2_gain_32ch_init(capi_v2_t* _pif, capi_v2_proplist_t* init_set_properties);

#ifdef __cplusplus
}
#endif

#endif // CAPI_V2_GAIN_V2_H

