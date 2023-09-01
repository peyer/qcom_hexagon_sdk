#ifndef ELITE_FWK_EXTNS_LAB_H
#define ELITE_FWK_EXTNS_LAB_H

/**
  @file Elite_fwk_extns_lab.h

  @brief Parameters required to be implemented for Look Ahead Buffering(LAB)

  This file defines a framework extensions and corresponding private messages
  for Look Ahead Buffering(LAB).
*/
/*===========================================================================
NOTE: The @brief description above does not appear in the PDF.
      The description that displays in the PDF is located in the
      Elite_CAPIv2_mainpage.dox file. Contact Tech Pubs for assistance.
===========================================================================*/
/*==============================================================================
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

/*==============================================================================
  Edit History

  $Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/incs/audio/Elite_fwk_extns_lab.h#1 $

  when        who      what, where, why
  --------    ---      -------------------------------------------------------
  11/23/14    sw      (Tech Pubs) Edited Doxygen markup/comments for 2.6.
  08/05/14    SivaNaga   New
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "Elite_CAPI_V2_types.h"

/** @addtogroup capiv2_ext_look_ahead_buf
@{ */

/** Unique identifier used to represent the LAB framework extension for the
    Listen Stream Manager feature.

    This extension supports the #PARAM_ID_KW_END_POSITION and
    #PARAM_ID_KW_END_MAX_BUFFER_REQ parameters.
*/
#define FWK_EXTN_LAB 0x00012C11

/*------------------------------------------------------------------------------
 * Parameter definitions - Needs implementation in module/library
 *----------------------------------------------------------------------------*/
 
/** ID of the parameter that indicates the keyword end position (in samples)
    from the end of the buffer for which detection happened.

    For example, in the Snapdragon@tm Voice Activation (SVA) solution, this
    parameter is used to omit the keyword before sending sending buffers to the
    HLOS. Other algorithms can use this extension in a similar way.

    @msgpayload{kw_end_position_t}
    @table{weak__kw__end__position__t} @newpage
*/
#define PARAM_ID_KW_END_POSITION   0x00012C12

/** @weakgroup weak_kw_end_position_t
@{ */
/* Data structure used when sending a keyword end position.
*/
typedef struct 
{
  uint32_t kw_end_position_samples;
  /**< Keyword end position in samples.

       @values 0 to kw_end_max_delay_t::kw_end_max_delay_samples */

}kw_end_position_t;
/** @} */ /* end_weakgroup weak_kw_end_position_t */

/** ID of the parameter that sends to adjust the lab buffer size

    @msgpayload{lab_adjust_t}
    @table{weak__lab__adjust__t}
*/
#define PARAM_ID_LAB_ADJUST   0x00012C31

  /** @weakgroup weak_lab_adjust_t
@{ */
  /* Data structure used when sending a lab buffer adjustment. 
   */
  typedef struct
  {
    uint32_t offset_samples;
    /**< offset in samples.

       @values 0 to 0xFFFFFFFF*/

  }lab_adjust_t;
  /** @} *//* end_weakgroup weak_lab_adjust_t */
/** ID of the parameter that sends the required maximum buffer size of the
    keyword end position to the framework. The LAB uses this parameter to
    query for the maximum number of samples that it must buffer.

    @msgpayload{kw_end_max_delay_t}
    @table{weak__kw__end__max__delay__t}
*/
#define PARAM_ID_KW_END_MAX_BUFFER_REQ 0x00012C13

/** @weakgroup weak_kw_end_max_delay_t
@{ */
/* Data structure used when querying for the maximum number of samples that
    the Look Ahead Buffer needs to buffer.
*/
typedef struct
{
  uint32_t kw_end_max_delay_samples;
  /**< Maximum delay between the keyword end position and keyword detection in
       the stream.

       This delay is calculated in terms of samples starting from the frame for
       which a keyword is detected. */

}kw_end_max_delay_t;
/** @} */ /* end_weakgroup weak_kw_end_max_delay_t */

/** This is to get unread bytes in lab buffer. querying this information to check
    whether there is enough data in LAB buffer for configured pre-roll duration 

    @msgpayload{unread_bytes_info_t}
    @table{weak__unread__bytes__info__t}
 */
#define PARAM_ID_UNREAD_BYTES   0x00012c2d

/** @weakgroup weak_unread_bytes_info_t
@{ */
/* Data structure used to get unread bytes.
 */
typedef struct
{
	uint32_t unread_bytes;
	/**< unread bytes from Look ahead buffer.

       @values 0 to 0xFFFFFFFF*/ 

}unread_bytes_info_t;
/** @} *//* end_weakgroup weak_unread_bytes_info_t */

/** ID of the parameter that sends the SVA detection best channel to the
    framework. The LAB uses this parameter to send this channel to HLOS,
    depending on the output configuration.

    @msgpayload{best_channel_t}
    @table{weak__best__channel__t}
*/
#define PARAM_ID_BEST_CHANNEL_IDX 0x00012C14

/** @weakgroup weak_best_channel_t
@{ */
/*  Data structure used when querying/sending the best channel returned by
    the SVA algorithm.
*/
typedef struct
{
  uint32_t index;
  /**< Index of the best channel.
    */

}best_channel_t;
/** @} *//* end_weakgroup weak_best_channel_t */

/** @} *//* end_addtogroup capiv2_ext_look_ahead_buf */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef ELITE_FWK_EXTNS_LAB_H*/
