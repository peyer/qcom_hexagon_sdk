/* =========================================================================
 Copyright (c) 2015 QUALCOMM Technologies Incorporated.
 All rights reserved. Qualcomm Technologies Proprietary and Confidential.
 ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef CAPI_V2_SP_TX_UTILS_H
#define CAPI_V2_SP_TX_UTILS_H

#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "HAP_farf.h"
#include "string.h"
#include "Elite_fwk_extns_feedback.h"
#include "sp_common.h"
#include "sp_tx_lib.h"
#include "adsp_api.h"
#include <stdlib.h>
#include "test_profile.h"

#define malloc __wrap_malloc
#define free __wrap_free

#ifdef __cplusplus
extern "C"
{
#endif

#define CAPI_V2_SP_TX_MAX_IN_PORTS     1
#define CAPI_V2_SP_TX_MAX_OUT_PORTS    1

static const uint32_t CAPI_V2_SP_TX_STACK_SIZE = 4096;

typedef struct capi_v2_sp_tx_media_fmt
{
   capi_v2_set_get_media_format_t main;
   capi_v2_standard_data_format_t std_fmt;
} capi_v2_sp_tx_media_fmt_t;

typedef struct capi_v2_sp_tx
{
   /* The virtual table location, should be the first element of the capi_v2 structure*/
   capi_v2_t vtbl;
   /* The callback info used to raise events to the service */
   capi_v2_event_callback_info_t cb_info;
   /* The input media format */
   capi_v2_sp_tx_media_fmt_t input_media_fmt[CAPI_V2_SP_TX_MAX_IN_PORTS];
   /* The output media format */
   capi_v2_sp_tx_media_fmt_t output_media_fmt[CAPI_V2_SP_TX_MAX_OUT_PORTS];
   /* To cache any library params set from the service */
   sp_tx_lib_config_t lib_config;
   /* To store the numbers of speakers this module needs to run on. Calculated from the
    * incoming media format. If 4 channels then 2 speakers and if 2 channel then 1 speaker */
   uint32_t num_spkr;
   /* The pointer to the library structure */
   sp_tx_memory_t lib_mem;
   /* The config of the speaker used for feedback communication */
   sp_spkr_cfg_t spkr_cfg;
   /* The fb path information sent from the service */
   feedback_info_t feedback_path_info;
   /* To identify whether the module is enabled or not */
   uint32_t enable_flag;
} capi_v2_sp_tx_t;

capi_v2_err_t capi_v2_sp_tx_process_set_properties(capi_v2_sp_tx_t* me_ptr,
                                                   capi_v2_proplist_t *proplist_ptr);
capi_v2_err_t capi_v2_sp_tx_process_get_properties(capi_v2_sp_tx_t *me_ptr,
                                                   capi_v2_proplist_t *proplist_ptr);

void capi_v2_sp_tx_release_memory(capi_v2_sp_tx_t *me_ptr);

void capi_v2_sp_tx_pop_fb_data_from_buffer_q(capi_v2_sp_tx_t *me_ptr, void **fb_buf_pptr);

void capi_v2_sp_tx_push_fb_data_to_data_q(capi_v2_sp_tx_t *me_ptr, void *fb_buff_ptr);

#ifdef __cplusplus
}
#endif
#endif /* CAPI_V2_SP_TX_UTILS_H */
