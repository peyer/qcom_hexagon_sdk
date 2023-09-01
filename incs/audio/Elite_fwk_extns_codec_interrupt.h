#ifndef ELITE_FWK_EXTNS_CODEC_INTERRUPT_H
#define ELITE_FWK_EXTNS_CODEC_INTERRUPT_H

/* ======================================================================== */
/**
@file Elite_fwk_extns_codec_interrupt.h

@brief frame work extensions for codec interrupts header file

  This file defines a framework extensions and corresponding private propeties
  needed for codec interrupts needed by modules.
*/

/*===========================================================================
NOTE: The @brief description above does not appear in the PDF.

      The Elite_APPI_mainpage.dox file contains the group/module descriptions that
      are displayed in the output PDF generated using Doxygen and LaTeX. To
      edit or update any of the group/module text in the PDF, edit the
      Elite_APPI_mainpage.dox file or contact Tech Pubs.
===========================================================================*/

/* =========================================================================
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.(QTI)
  All rights reserved. Qualcomm Proprietary and Confidential.
  ========================================================================== */

/* =========================================================================
                             Edit History

   when       who     what, where, why
   --------   ---     ------------------------------------------------------
   08/08/14   mk      Initial Version.
   ========================================================================= */

#include "Elite_CAPI_V2_types.h"

#define FWK_EXTN_CDC_INTERRUPT 0x00010279
/**< Unique identifier to represent custom framework extension for codec interrupts */

#define CAPI_V2_PARAM_ID_NUM_NEEDED_CDC_INT 0x0001027A
/**< Unique identifier to represent number of needed codec interrupt by a module 
  Payload Structure: capi_v2_num_needed_cdc_int_t
 
 CALLER
 
  capi_v2_buf_t buf;
  capi_v2_num_needed_cdc_int_t num_cdc_int;
 
  buf.data_ptr = (int8_t *)&num_cdc_int.num_cdc_int;
  buf.max_data_len = sizeof(capi_v2_num_needed_cdc_int_t);

  CALLEE
      capi_v2_cdc_int_t *cfg_ptr = (capi_v2_num_needed_cdc_int_t *)params_ptr->data_ptr;
 
      uint32_t cfg_size = sizeof(capi_v2_num_needed_cdc_int_t);

      cfg_ptr->num_cdc_int = 4;
 
      params_ptr->actual_data_len = cfg_size;
 
  */

#define CAPI_V2_PARAM_ID_CDC_INT_LIST 0x0001027B
/**< Unique identifier to get using the CAPI V2 get param the codec interrupts list from the module.
  Payload Structure: capi_v2_cdc_int_t
 
  CALLER
 
  capi_v2_buf_t buf;
  capi_v2_cdc_int_t *cdc_int_ptr;
  capi_v2_num_needed_cdc_int_t num_cdc_int;
 
  buf.data_ptr = (int8_t *)cdc_int_ptr;
  buf.max_data_len = sizeof(capi_v2_cdc_int_t) * num_cdc_int.num_cdc_int;

  CALLEE
      capi_v2_num_needed_cdc_int_t num_cdc_int;
      capi_v2_cdc_int_t *cfg_ptr = (capi_v2_cdc_int_t *)params_ptr->data_ptr;
 
      uint32_t cfg_size = sizeof(capi_v2_cdc_int_t) * num_cdc_int.num_cdc_int;

      cfg_ptr[0].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP;
      cfg_ptr[1].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP;
      cfg_ptr[2].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK;
      cfg_ptr[3].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE;
 
      params_ptr->actual_data_len = cfg_size;
   */

#define CAPI_V2_PARAM_ID_CDC_INT_MAD_AUDIO 0x0001027C
/**< Unique identifier to set the MAD audio interrupt to the module. 
  Payload Structure: None */

#define CAPI_V2_PARAM_ID_CDC_INT_MAD_BEACON 0x0001027D
/**< Unique identifier to set the MAD beacon interrupt to the module. 
  Payload Structure: None */

#define CAPI_V2_PARAM_ID_CDC_INT_MAD_ULT 0x0001027E
/**< Unique identifier to set the MAD ultrasound interrupt to the module. 
  Payload Structure: None */

#define CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP 0x0001027F
/**< Unique identifier to set the left speaker clip interrupt to the module. 
    Payload Structure: capi_v2_cdc_clip_pcm_info_t */

#define CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK 0x00010280
/**< Unique identifier to set the Vbat attack interrupt to the module. 
  Payload Structure: None */

#define CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE 0x00010281
/**< Unique identifier to set the Vbat release interrupt to the module. 
  Payload Structure: None */

#define CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP 0x00010282
/**< Unique identifier to set the right speaker clip interrupt to the module. 
    Payload Structure: capi_v2_cdc_clip_pcm_info_t */


typedef struct capi_v2_cdc_clip_pcm_info_t
{
  capi_v2_buf_t cdc_clip_pcm_level_buf;
  /**< Clip pcm levels buffer */
}capi_v2_cdc_clip_pcm_info_t; 


typedef struct capi_v2_num_needed_cdc_int_t capi_v2_num_needed_cdc_int_t;
struct capi_v2_num_needed_cdc_int_t
{
   uint32_t num_cdc_int;
};

typedef struct capi_v2_cdc_int_t capi_v2_cdc_int_t;
struct capi_v2_cdc_int_t
{
   uint32_t cdc_int;
};

#endif /* #ifndef ELITE_FWK_EXTNS_CODEC_INTERRUPT_H */
