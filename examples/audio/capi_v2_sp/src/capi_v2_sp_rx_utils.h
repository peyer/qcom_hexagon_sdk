/* =========================================================================
 Copyright (c) 2015 QUALCOMM Technologies Incorporated.
 All rights reserved. Qualcomm Technologies Proprietary and Confidential.
 ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef CAPI_V2_SP_RX_UTILS_H
#define CAPI_V2_SP_RX_UTILS_H

#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "HAP_farf.h"
#include "HAP_mem.h"
#include "string.h"
#include "Elite_fwk_extns_feedback.h"
#include "Elite_fwk_extns_codec_interrupt.h"
#include "sp_common.h"
#include "sp_rx_lib.h"
#include "adsp_api.h"
#include <stdlib.h>
#include "test_profile.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define malloc __wrap_malloc
#define free __wrap_free

#define CAPI_V2_SP_RX_MAX_IN_PORTS     1
#define CAPI_V2_SP_RX_MAX_OUT_PORTS    1
#define MAX_PCM_BUF_LEN 16

static const uint32_t CAPI_V2_SP_RX_STACK_SIZE = 4096;

typedef struct capi_v2_sp_rx_media_fmt
{
   capi_v2_set_get_media_format_t main;
   capi_v2_standard_data_format_t std_fmt;
} capi_v2_sp_rx_media_fmt_t;

typedef struct capi_v2_sp_rx
{
   /* The virtual table location, should be the first element of the capi_v2 structure*/
   capi_v2_t vtbl;
   /* The callback info used to raise events to the service */
   capi_v2_event_callback_info_t cb_info;
   /* The input media format */
   capi_v2_sp_rx_media_fmt_t input_media_fmt[CAPI_V2_SP_RX_MAX_IN_PORTS];
   /* The output media format */
   capi_v2_sp_rx_media_fmt_t output_media_fmt[CAPI_V2_SP_RX_MAX_OUT_PORTS];
   /* To cache any library params set from the service */
   sp_rx_lib_config_t lib_config;
   /* To store the numbers of speakers this module needs to run on. Calculated from the
    * incoming media format. If 2 channels then 2 speakers and if 1 channel then 1 speaker */
   uint32_t num_spkr;
   /* The pointer to the library structure */
   sp_rx_memory_t lib_mem;
   /* The config of the speaker used for feedback communication */
   sp_spkr_cfg_t spkr_cfg;
   /* The client list used for feedback communication */
   void **client_list_ptr_ptr;
   /* To identify whether the module is enabled or not */
   uint32_t enable_flag;
   /**< Clip pcm level buffer */
   int16_t clip_pcm_buf[MAX_SP_SPKRS][MAX_PCM_BUF_LEN];
   /**< Interrupt status */
   uint32_t rx_intr[MAX_SP_SPKRS];
   /**< Number of codec interrupts needed by Module */
   uint32_t num_cdc_int;
} capi_v2_sp_rx_t;

capi_v2_err_t capi_v2_sp_rx_process_set_properties(capi_v2_sp_rx_t* me_ptr,
                                                   capi_v2_proplist_t *proplist_ptr);
capi_v2_err_t capi_v2_sp_rx_process_get_properties(capi_v2_sp_rx_t *me_ptr,
                                                   capi_v2_proplist_t *proplist_ptr);

void capi_v2_sp_rx_get_fb_data(capi_v2_sp_rx_t *me_ptr, int8_t *fb_data_ptr[MAX_SP_SPKRS],
                               void *fb_data_buf_rcvd_ptr[MAX_SP_SPKRS],
                               void *fb_data_buf_client_ptr[MAX_SP_SPKRS]);

void capi_v2_sp_rx_return_fb_data(capi_v2_sp_rx_t *me_ptr, void *fb_data_buf_rcvd_ptr[MAX_SP_SPKRS],
                                  void *fb_data_buf_client_ptr[MAX_SP_SPKRS]);

void capi_v2_sp_rx_release_memory(capi_v2_sp_rx_t *me_ptr);

capi_v2_err_t capi_v2_sp_v2_rx_intr_handler(capi_v2_sp_rx_t *me_ptr, uint32_t int_id);
capi_v2_err_t capi_v2_sp_v2_rx_copy_pcmlevels_to_libbuf(capi_v2_sp_rx_t *me_ptr, uint32_t int_id,
                                                        int8_t *cdc_clip_pcm_level_buf);

#ifdef __cplusplus
}
#endif
#endif /* CAPI_V2_SP_RX_UTILS_H */
