#ifndef ADSP_API_H
#define ADSP_API_H

/* ======================================================================== */
/**
 @file Elite_fwk_extns_feedback.h

 @brief Frame work extensions for feedback header file

 This file defines a framework extensions and corresponding private propeties
 needed for feedback communication between modules.
 */

/* =========================================================================
 Copyright (c) 2010-2014 Qualcomm Technologies, Inc.(QTI)
 All rights reserved. Qualcomm Proprietary and Confidential.
 ========================================================================== */

/* =========================================================================
 Edit History

 when       who     what, where, why
 --------   ---     ------------------------------------------------------
 10/28/14   roverma      Initial Version.
 ========================================================================= */

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

/** @ingroup afe_module_feedback
 ID of the Feedback module, which is used to set up the feedback path
 between modules on different AFE ports for communicating custom data.
 To use this framework, these modules must support custom data
 communication.

 This module supports the following parameter ID:
 - #AFE_PARAM_ID_FEEDBACK_PATH_CFG

 @apr_hdr_fields
 Opcode -- AFE_MODULE_FEEDBACK
 */
#define AFE_MODULE_FEEDBACK                                     0x00010257

/** @} *//* end_addtogroup afe_module_feedback */

/** @addtogroup afe_module_feedback
 @{ */
/** ID of the parameter used by #AFE_MODULE_FEEDBACK to configure
 a feedback connection between speaker protection Tx and Rx processing.
 Only #AFE_PORT_CMD_SET_PARAM_V2 can use this parameter ID.

 @apr_msgpayload{afe_fbsp_feedback_path_cfg_v1_t}
 @table{weak__afe__fbsp__feedback__path__cfg__v1__t}
 */
#define AFE_PARAM_ID_FEEDBACK_PATH_CFG                         0x0001022C

/** Version information used to handle future additions to configuration of
 the feedback connection between speaker protection Tx and Rx processing
 (for backward compatibility).
 */
#define AFE_API_VERSION_FEEDBACK_PATH_CFG                      0x1

/** Vsens comes from the left speaker. */
#define AFE_FBSP_VSENS_LEFT_CHAN          1

/** Isens comes from the left speaker. */
#define AFE_FBSP_ISENS_LEFT_CHAN          2

/** Vsens comes from the right speaker. */
#define AFE_FBSP_VSENS_RIGHT_CHAN            3

/** Isens comes from the right speaker. */
#define AFE_FBSP_ISENS_RIGHT_CHAN            4

/** Maximum number of VI channels supported. @newpage */
#define AFE_FBSP_VI_CHAN_MAX              4

/** @} *//* end_addtogroup afe_module_feedback_spkr_vi */

typedef struct afe_fbsp_feedback_path_cfg_v1_t afe_fbsp_feedback_path_cfg_v1_t;

/** @weakgroup weak_afe_fbsp_feedback_path_cfg_v1_t
 @{ */
/* Configuration structure for the
 AFE_PARAM_ID_FEEDBACK_PATH_CFG parameter (version 1).
 */
struct afe_fbsp_feedback_path_cfg_v1_t
{
   uint32_t feedback_path_cfg_minor_version;
   /**< Minor version used for tracking feedback speaker protection Tx and Rx
    processing configuration.

    @values #AFE_API_VERSION_FEEDBACK_PATH_CFG */

   int32_t dst_portid;
   /**< Destination Rx port ID. It must be the same Rx port ID for
    which feedback speaker protection is configured.

    @values See <i>Supported Hardware Ports</i> for the applicable
    chipset in Chapter @xref{sec:portIDs} */

   int32_t num_channels;
   /**< Number of Tx port channels. Each Vsens and Isens is considered an
    individual channel.

    @values
    - 2 -- Mono speaker
    - 4 -- Stereo speakers @tablebulletend */

   int32_t chan_info[AFE_FBSP_VI_CHAN_MAX];
   /**< Channel mapping array that provides information on the order in
    which the Vsens and Isens of different speakers come into a Tx port.
    Some possible configs are (1,2,3,4) (4,2,1,3) (1,2,0,0) (4,3,0,0) (2,1,0,0), etc
    All channels should be unique. Order doesn't matter as long as they are valid.
    In case only 2 channels are sent (V and I), it should be first two like (4,3,0,0) (1,2,0,0).

    @values
    - #AFE_FBSP_VSENS_LEFT_CHAN
    - #AFE_FBSP_ISENS_LEFT_CHAN
    - #AFE_FBSP_VSENS_RIGHT_CHAN
    - #AFE_FBSP_ISENS_RIGHT_CHAN @tablebulletend */
};
/** @} *//* end_weakgroup weak_afe_fbsp_feedback_path_cfg_v1_t */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef ADSP_API_H */
