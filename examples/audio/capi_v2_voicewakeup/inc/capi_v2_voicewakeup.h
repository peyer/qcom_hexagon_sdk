/* ======================================================================== */
/**
   @file capi_v2_voicewakeup.h

   C source file to implement CAPIv2 VoiceWakeup example
 */

/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
#ifndef CAPI_V2_VOICEWAKEUP_H_
#define CAPI_V2_VOICEWAKEUP_H_
#include "mmdefs.h"
#include "HAP_farf.h"
#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * %%% Duplicated in
 *    audio/api/lsm/inc/cpe_lsm_service_params.h
 *    avs/api/listen/inc/adsp_lsm_session_commands.h
 */

/** ID of the Voice Wakeup V2 module, which configures multiple
    keyword and multiple user calibration data.

    This module supports the following parameter IDs:
    - #LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD
    - #LSM_PARAM_ID_OPERATION_MODE
    - #LSM_PARAM_ID_GAIN
    - #LSM_PARAM_ID_MIN_CONFIDENCE_LEVELS

    This module supports the #LSM_SESSION_EVENT_DETECTION_STATUS_V2 event.
*/
#define LSM_MODULE_VOICE_WAKEUP_V2        0x00012C0D

/** This param id will be used to set the beginning of speech and end of
  speech thresholds. This param id will be accepted when the session is in INIT state
*/
#define LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD      0x00012C01


/**  This version information is used to handle the new
     additions to the config interface in future in backward
     compatible manner.
 */
#define LSM_API_VERSION_EPD_THRESHOLD 									0x1

/* Payload of the LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD command. */
typedef struct lsm_param_id_epd_threshold_t lsm_param_id_epd_threshold_t;

/** Payload of the #LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD parameter,
 *  which sets the beginning of speech and end of speech thresholds .
 */
struct lsm_param_id_epd_threshold_t
{
    uint32_t minor_version;
    /**< Minor version used for tracking the version of this param id.
             Supported values:
               #LSM_API_VERSION_EPD_THRESHOLD  */

    int32_t epd_begin_threshold;

   /**< Threshold for beginning of speech in Q20 format
         Supported Values: Range of Q20 values
        */

    int32_t epd_end_threshold;

   /**< Threshold for end of speech in Q20 format
        Supported Values: Range of Q20 values
	  */

};

/** This param id will be used to set the different operations
    modes to be operated by voice wakeup module.
*/
#define LSM_PARAM_ID_OPERATION_MODE                0x00012C02

/**  This version information is used to handle the new
     additions to the config interface in future in backward
     compatible manner.
 */
#define LSM_API_VERSION_OPERATION_MODE 									0x1

/* Payload of the LSM_PARAM_ID_OPERATION_MODE command. */
typedef struct lsm_param_id_operation_mode_t lsm_param_id_operation_mode_t;

/** Payload of the #LSM_PARAM_ID_OPERATION_MODE parameter,
 *  which sets the different types of operation mode of voice wakeup block
*/
struct lsm_param_id_operation_mode_t
{
    uint32_t minor_version;
    /**< Minor version used for tracking the version of this param id.
             Supported values:
               #LSM_API_VERSION_OPERATION_MODE  */

    uint16_t mode;

   /**< Mode flags that configure the voice wake up module to run in different types of modes.

         Supported values for bit 0:
         - 0 - Keyword Detection Mode is disabled
         - 1 - Keyword Detection Mode is  enabled

         Supported values for bit 1:
         This bit is applicable only if bit 0 is set as 1. Otherwise this
         values are ignored.
         - 0 -  User Verification Mode is disabled
         - 1 -  User Verification Mode is  enabled

        Supported values for bit 2:
         - 0 - Events will be sent to HLOS only in case of succesful detection
         - 1 - Events will be sent to HLOS irresepective of success or failure */

    uint16_t reserved;
       /**< Reserved for 32-bit alignment.
          This field must be set to 0. */

};

/** This param id will be used to set the gain which is applied
    on the data coming from SW MAD.
*/
#define LSM_PARAM_ID_GAIN                         0x00012C03


/**  This version information is used to handle the new
    additions to the config interface in future in backward
    compatible manner.
 */
#define LSM_API_VERSION_GAIN 									0x1

/* Payload of the LSM_PARAM_ID_GAIN command. */
typedef struct lsm_param_id_gain_t lsm_param_id_gain_t;

/** Payload of the LSM_PARAM_ID_GAIN parameter */
struct lsm_param_id_gain_t
{
    uint32_t minor_version;
    /**< Minor version used for tracking the version of this param id.
             Supported values:
               #LSM_API_VERSION_GAIN  */

    int16_t gain;
   /**< Gain in Q8 format.
     Supported Values: Range of Q8 values */

    uint16_t reserved;
       /**< Reserved for 32-bit alignment.
          This field must be set to 0. */

};


/** This param id is used to set the minimum confidence levels
    for registered keyword models and active user models.This is
    applicable when app_id variable in LSM_SESSION_CMD_OPEN_TX
    is LSM_VOICE_WAKEUP_APP_V2.
*/
#define LSM_PARAM_ID_MIN_CONFIDENCE_LEVELS                                 0x00012C07


/* Payload of the LSM_PARAM_ID_MIN_CONFIDENCE_LEVELS command. */
typedef struct lsm_param_id_min_confidence_levels_t lsm_param_id_min_confidence_levels_t;

/** Payload of the LSM_PARAM_ID_MIN_CONFIDENCE_LEVELS parameter */
struct lsm_param_id_min_confidence_levels_t
{
    uint8_t num_active_models;
    /**< This is the sum of all keyword models plus the active user models.
         The active user models are the users registered for each target keyword.
         Immediately following this structure "num_active_models" bytes of data will
         be followed.
         Here HLOS need to make sure that they should add enough padding bytes while sending
         this payload such that the whole payload size should be multiple of 4 bytes
         Supported values:
         #1 - 20  */
};

/** Register a sound model with a module. The sound model is a blob of data
    interpreted by the module supporting this parameter. A sound model is
    design to contain the unique sound characteristics/signatures which is used
    by algorithm hosted in the module.

    The payload must be an out of band payload. It should persist across the
    detection mode and can be free up once LSM_PARAM_ID_DEREGISTER_SOUND_MODEL
    is invoked.

    param_id : LSM_PARAM_ID_REGISTER_SOUND_MODEL
    param_size : variable size
*/
#define LSM_PARAM_ID_REGISTER_SOUND_MODEL                0x00012C14

/** De-register a sound model from a module. This is useful if client wants to
    replace already registered sound model or free up resource without closing
    a session.[Note: If the client closes a session, all registered models and
    other resources claimed by the module are freed implicitly]

    param_id : LSM_PARAM_ID_DEREGISTER_SOUND_MODEL
    param_size : none
*/
#define LSM_PARAM_ID_DEREGISTER_SOUND_MODEL              0x00012C15


capi_v2_err_t capi_v2_voicewakeup_get_static_properties (
		capi_v2_proplist_t *init_set_properties,
		capi_v2_proplist_t *static_properties);
capi_v2_err_t capi_v2_voicewakeup_init (capi_v2_t* _pif,
		capi_v2_proplist_t      *init_set_properties);


#ifdef __cplusplus
}
#endif

#endif /* CAPI_V2_VOICEWAKEUP_H_ */
