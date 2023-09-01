#ifndef CAPI_V2_PASSTHRU_H
#define CAPI_V2_PASSTHRU_H
/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "Elite_CAPI_V2.h"
#include "audio_passthru_calib.h"

// Passthru module calibration parameters
#define CAPI_V2_PARAM_PASSTHRU_MODULE_ENABLE PASSTHRU_PARAM_MOD_ENABLE

#ifdef __cplusplus
extern "C"
{
#endif

capi_v2_err_t capi_v2_passthru_get_static_properties(
   capi_v2_proplist_t* init_set_properties,
   capi_v2_proplist_t* static_properties);
capi_v2_err_t capi_v2_passthru_init(capi_v2_t* _pif,
                                 capi_v2_proplist_t* init_set_properties);

#ifdef __cplusplus
}
#endif

#endif // CAPI_V2_PASSTHRU_H

