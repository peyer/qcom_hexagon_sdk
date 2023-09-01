/* ======================================================================== */
/**
   @file decimate_block.c

   C source file to implement CAPIv2 Decimation example
 */

/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
#ifndef CAPI_V2_DECIMATE_H_
#define CAPI_V2_DECIMATE_H_
#include "mmdefs.h"
#include "HAP_farf.h"
#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "audio_decimate_calib.h"

// Decimate module calibration parameters
#define CAPI_V2_PARAM_ID_DECIMATE_ENABLE 		DECIMATE_PARAM_MOD_ENABLE
#define CAPI_V2_PARAM_ID_UPDATE_DECIMATION_FACTOR 	DECIMATE_PARAM_DECIMATION_FACTOR

#ifdef __cplusplus
extern "C"
{
#endif

capi_v2_err_t capi_v2_decimate_get_static_properties (
		capi_v2_proplist_t *init_set_properties,
		capi_v2_proplist_t *static_properties);
capi_v2_err_t capi_v2_decimate_init (capi_v2_t* _pif,
		capi_v2_proplist_t      *init_set_properties);
#ifdef __cplusplus
}
#endif

#endif /* CAPI_V2_DECIMATE_H_ */
