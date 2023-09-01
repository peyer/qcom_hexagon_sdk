#ifndef ELITE_FWK_EXTNS_ECNS_H
#define ELITE_FWK_EXTNS_ECNS_H

/* ======================================================================== */
/**
@file Elite_fwk_extns_ecns.h

@brief Frame work extensions for Echo canceller and Noise suppression (ECNS)

  This file defines a framework extensions and corresponding private propeties
  needed for ECNS in voice.
*/

/* =========================================================================
  Copyright (c) 2010-2018 Qualcomm Technologies, Inc.(QTI)
  All rights reserved. Qualcomm Proprietary and Confidential.
  ========================================================================== */

/* =========================================================================
                             Edit History

   when       who     what, where, why
   --------   ---     ------------------------------------------------------
   09/25/14   abhayg      Initial Version.
   ========================================================================= */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


#include "Elite_CAPI_V2_types.h"

#define FWK_EXTN_ECNS 0x00010E54
/**< Unique identifier to represent custom framework extension for ECNS */

/*------------------------------------------------------------------------------
 * Property definitions - Needs implementation in module/library
 *----------------------------------------------------------------------------*/

#define CAPI_V2_PROPERTY_ID_ECNS_OUTPUT_CAPABILITIES 0x00010E55
/**< Property for querying ecns output capabilities statically>
 * payload structure :ecns_output_capabilities_t */

#define CAPI_V2_PROPERTY_ID_ECNS_MONITORING_CAPABILITIES 0x00010E56
/**< Property for querying ecns monitoring capabilities statically>
 * payload structure :ecns_monitoring_capabilities_t*/

#define CAPI_V2_PROPERTY_ID_ECNS_VP3_CAPABILITIES 0x00010E57
/**< Property for querying ecns VP3 capabilities statically>
 * payload structure :ecns_vp3_capabilities_t*/

#define CAPI_V2_PROPERTY_ID_ECNS_RATE_MATCHING_CAPABILITIES 0x00010E58
/**< Property for querying ecns rate matching capabilities statically>
 * payload structure :ecns_monitoring_capabilities_t*/

#define CAPI_V2_PROPERTY_ID_ECNS_STT_CAPABILITIES 0x00010E60
/**<Property used to statically query for ECNS multichannel reference capabilities>
     payload structure :ecns_stt_capabilities_t */

#define CAPI_V2_PROPERTY_ID_MULTICHANNEL_REF_CAPABILITIES 0x00010EA0
/**< Property used to statically query for ECNS multichannel reference capabilities>
    payload structure :ecns_multichannel_ref_capabilities_t */

#define CAPI_V2_PROPERTY_ID_SPEECH_ACT_DET_CAPABILITIES 0x00010EA2
/**< Property used to statically query for ECNS speech activity detection capabilities>
     payload structure :ecns_speech_act_det_capabilities_t */


#define SS_PARAM_INST_DRIFT (0x00010E4F)
/** param-id to set the instantaneous drift of all ports of ECNS
 */


/**< payload for SS_PARAM_INST_DRIFT param Id */
typedef struct instantaneous_drift_t
{
  int32_t*  inst_drift_in_us;
  /**<  array of instantaneous drift for each port*/
}instantaneous_drift_t;


#define SS_PARAM_DRIFT_SAMPLES (0x00010E50)
/** param-id to get the drift in terms of samples to be corrected
 *  of all ports of ECNS
 */

/**< payload for SS_PARAM_DRIFT_SAMPLES param Id */
typedef struct drift_samples_t
{
  int32_t*  drift_in_samples;
  /**<  array of drift in samples for each port */
}drift_samples_t;



#define SS_PARAM_RESET (0x00010E51)
/** param-id to reset specific port or all the ports of sample slip capi
 *  This param-id does not have any payload
 */

/**<struture for querying ECNS output capabilities
 *  This info needs to be provided by ECNS module
 *  to the service                                      */
typedef struct ecns_output_capabilities_t
{
   bool_t       output_lec;
   /**< enables service to know whether ECNS modules provides
    * LEC (linear Echo cancelled) output
    * if TRUE - ECNS module outputs LEC, FALSE otherwise
    */
   bool_t       output_nr;
   /**< enables service to know whether ECNS modules provides
    * NR (noise refernce) output
    * if TRUE - ECNS module outputs NR, FALSE otherwise */
}ecns_output_capabilities_t;

/**<struture for querying ECNS monitoring capabilities
 *  This info needs to be provided by ECNS module
 *  to the service                                      */
typedef struct ecns_monitoring_capabilities_t
{
   bool_t       is_rtm_supported;
   /**< this enables service to know whether the module supports
    * Real Time monitoring (RTM) functionality. This would help the service to moniter
    * AF coeffs and state of module in real time.
    *  if TRUE - ECNS module supports RTM, FALSE otherwise
    */
}ecns_monitoring_capabilities_t;

/**<struture for querying ECNS VP3 capabilities
 *  This info needs to be provided by ECNS module
 *  to the service                                      */
typedef struct ecns_vp3_capabilities_t
{
   bool_t       is_vp3_supported;
   /**< this eanbles service to know whether the module supports
    * vp3 functionality.
    *  if TRUE - ECNS module supports vp3, FALSE otherwise
    */
}ecns_vp3_capabilities_t;

/**<struture for querying ECNS Source tracking trio capabilities
 *  This info needs to be provided by ECNS module
 *  to the service                                      */
typedef struct ecns_stt_capabilities_t
{
   bool_t       is_stt_supported;
   /**< this eanbles service to know whether the module supports
    *  source tracking functionality.
    *  TRUE - ECNS module supports Source tracking trio,
    *  FALSE - ECNS module does not support Source tracking trio,
    */
}ecns_stt_capabilities_t;

typedef struct ecns_rate_matching_capabilities_t
{
   bool_t     is_rate_matching_supported;
   /**< Indicates whether the ECNS module supports internal rate matching
        functionality. If ECNS supports rate matching, service will proide the drift information
        and seek the number of samples to be corrected
       *  if TRUE - ECNS module supports rate matching, FALSE otherwise
       */
}ecns_rate_matching_capabilities_t;


/** @weakgroup weak_ecns_multichannel_ref_capabilities_t
@{ */
typedef struct ecns_multichannel_ref_capabilities_t
{
   bool_t is_multichannel_ref_supported;
   /**< Specifies whether the ECNS module supports multichannel far reference
        functionality.

        @values
        - TRUE -- Module supports multichannel far reference
        - FALSE -- Otherwise

        If the module supports this functionality, the service provides an
        available far reference. CAPIv2 must have an appropriate media format
        converter to convert to the number of reference channels that the EC
        library requires.

        If the module does not support this functionality, the service provides
        a mono reference. */

}/** @cond */ecns_multichannel_ref_capabilities_t /** @endcond */;
/** @} *//* end_weakgroup weak_ecns_multichannel_ref_capabilities_t */


/*  struture for querying ECNS speech activity detection capabilities
 * The ECNS module must provide this information to the aDSP service.
 */
typedef struct ecns_speech_act_det_capabilities_t
{
   bool_t       is_speech_activity_supported;
   /**< Specifies whether the ECNS module supports speech activity detection.

        @values
        - TRUE -- Speech activity detection is supported
        - FALSE -- Speech activity detection is not supported

        If the ECNS module supports detection, it must raise the activity event
        when speech activity changes.

        ParamID for raising event: #PARAM_ID_SPEECH_ACTIVITY_UPDATE */

}/** @cond */ecns_speech_act_det_capabilities_t/** @endcond */;
/** @} *//* end_weakgroup weak_ecns_speech_act_det_capabilities_t */


/** Parameter for raising speech activity update event. End point of this
     event is DSP service.

     @msgpayload{speech_activity_update_payload_t}
     @tablens{weak__speech__activity__update__payload__t}

     @sa #CAPI_V2_EVENT_DATA_TO_DSP_SERVICE */
     /* in Elite_CAPI_V2_events.h */
#define PARAM_ID_SPEECH_ACTIVITY_UPDATE 0x00010E81

/** @weakgroup weak_speech_activity_update_payload_t
@{ */
/** Data structure for reporting speech activity. This structure can be
  extended in the future.
*/
typedef struct speech_activity_update_payload_t
{
  uint16_t activity_class;
  /**< Class of main activies supported.

       @values speech_main_activity_t */

  uint16_t sub_activity;
  /**< Provides a subactivity type for the main activity (to support backward
       compatibility).

       If an unknown subactivity is received by the service, it requires
       minimum handling.

       @values
       - #speech_mute_sub_activity_t
       - #speech_break_sub_activity_t @tablebulletend*/

}/** @cond */speech_activity_update_payload_t/** @endcond */;
/** @} *//* end_weakgroup speech_activity_update_payload_t */

/** Valid speech activity class types.
*/
typedef enum speech_activity_class_t
{
   SPEECH_ACTIVITY_CLASS_MUTE = 0,    /**< Speech signal mute detection. */
   SPEECH_ACTIVITY_CLASS_BREAK,       /**< Speech signal break detection. */
   SPEECH_ACTIVITY_CLASS_MAX=0xFFFF   /**< Maximum number of speech activity
                                           classes. */
}/** @cond */speech_activity_class_t/** @endcond */;

/** Valid speech subactivities for mute class types.
*/
typedef enum speech_mute_sub_activity_t
{
   MUTE_ACTIVITY_NOT_DETECTED = 0, /**< Mute activity is not detected by the
                                        module. Raised to revert a previously
                                        raised event for the mute class. */
   MUTE_ACTIVITY_DETECTED,         /**< Mute activity is detected by the module. */
   MUTE_ACTIVITY_MAX=0xFFFF,       /**< Maximum number of subactivities for mute. */
}/** @cond */speech_mute_sub_activity_t/** @endcond */;

/** Valid speech subactivities for break class types.
*/
typedef enum speech_break_sub_activity_t
{
   BREAK_ACTIVITY_NOT_DETECTED = 0, /**< Break activity is not detected by the
                                         module. Raised to revert a previously
                                         raised event for mute class. */
   BREAK_ACTIVITY_DETECTED,         /**< Break activity is detected by the module. */
   BREAK_ACTIVITY_MAX=0xFFFF,       /**< Maximum number of subactivities for break. */
}/** @cond */speech_break_sub_activity_t/** @endcond */;


/** Used by the aDSP service to statically query the ECNS properties for
  their capabilities.
*/
typedef struct capi_v2_get_ecns_property_t
{
   uint32_t                  sec_property_id;
   /**< secondary property id for the capability of ECNS which service desires to know */

   void*                     ecns_info;
   /**< this will be point to structure for corresponding to sec_proper_id
    * the structure elements will be updated by ecns module providing its capabilites to service */
}capi_v2_get_ecns_property_t;


#ifdef __cplusplus
}
#endif //__cplusplus
#endif
