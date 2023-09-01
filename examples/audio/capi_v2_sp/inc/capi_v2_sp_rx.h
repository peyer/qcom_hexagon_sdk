/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
#ifndef CAPI_V2_SP_RX_H_
#define CAPI_V2_SP_RX_H_



#include "mmdefs.h"
#include "HAP_farf.h"
#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "audio_sp_rx_calib.h"


// SP_Rx module calibration parameters
#define CAPI_V2_PARAM_ID_SP_RX_ENABLE 		SP_RX_PARAM_MOD_ENABLE
#define CAPI_V2_SP_RX_CFG_1_PARAM_ID   SP_RX_CFG_1_PARAM_ID

#ifdef __cplusplus
extern "C"
{
#endif


capi_v2_err_t capi_v2_sp_rx_get_static_properties (
		capi_v2_proplist_t *init_set_properties,
		capi_v2_proplist_t *static_properties);

capi_v2_err_t capi_v2_sp_rx_init (capi_v2_t* _pif,
		capi_v2_proplist_t      *init_set_properties);



#ifdef __cplusplus
}
#endif

#endif /* CAPI_V2_SP_RX_H_ */
