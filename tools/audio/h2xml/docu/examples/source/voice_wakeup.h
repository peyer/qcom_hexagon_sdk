#ifndef _VOICE_WAKEUP_H_
#define _VOICE_WAKEUP_H_

/*==============================================================================
  @file voice_wakeup.h
  @brief This file contains Public APIs for Voice Wake-up Module

  Copyright (c) 2016 QUALCOMM Technologies, Inc.  All Rights Reserved.
  QUALCOMM Technologies, Inc Proprietary.  Export of this technology or
  software is regulated by the U.S. Government, Diversion contrary to U.S.
  law prohibited.
==============================================================================*/

/*==============================================================================
  Edit History

  $Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/tools/audio/h2xml/docu/examples/source/voice_wakeup.h#1 $

  when       who        what, where, why
  --------   ---        ------------------------------------------------------
  05/10/16   unni       Created File
==============================================================================*/

#include "mmdefs.h"
/*------------------------------------------------------------------------------
   Inheriting Parameters defined in detection engine
------------------------------------------------------------------------------*/
#include "detection_engine.h"

/*------------------------------------------------------------------------------
   Module ID
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/

/* Global unique Module ID definition.
   Module library is independent of this number, it defined here for static
   loading purpose only */
#define MODULE_ID_VOICE_WAKEUP        0x00012C0D

/* Input Pin Name - '0' */
#define VW_INPUT_PIN_0      0

/* Input Pins Max */
#define VW_INPUT_PINS_MAX   1

/* Output Pin Name - '0' */
#define VW_OUTPUT_PIN_0     0

/* Output Pins Max */
#define VW_OUTPUT_PINS_MAX  1

/** @h2xmlm_module       {"MODULE_ID_VOICE_WAKEUP",MODULE_ID_VOICE_WAKEUP}
    @h2xmlm_InputPins    {IN0=VW_INPUT_PIN_0}  
    @h2xmlm_OutputPins   {OUT0=VW_OUTPUT_PIN_0}
    @h2xmlm_toolPolicy   {Calibration;RTC}
    @h2xmlm_description  {Voice Wakeup Module}
    @{                   <-- Start of the Module --> */

/*------------------------------------------------------------------------------
   Parameters
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_ENDPOINT_DETECT_THRESHOLD      0x00012C01

/* Minor Version of this PARAM */
#define API_VERSION_EPD_THRESHOLD               0x1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_epd_threshold param_id_epd_threshold_t;

/** @h2xmlp_parameter   {"PARAM_ID_ENDPOINT_DETECT_THRESHOLD", 
                         PARAM_ID_ENDPOINT_DETECT_THRESHOLD}  
    @h2xmlp_description {parameter used to set the beginning-of-speech and 
                         end-of-speech thresholds} */
#include "graphite_begin_pack.h"
struct param_id_epd_threshold
{
    uint32_t minor_version;
    /**< @h2xmle_description {Minor version used for tracking the version of 
                              this parameter ID}
         @h2xmle_range       {1..API_VERSION_EPD_THRESHOLD}  
         @h2xmle_default     {API_VERSION_EPD_THRESHOLD} */

    int32_t epd_begin_threshold;
    /**< @h2xmle_description {Threshold for the beginning of speech}
         @h2xmle_default     {0x02710000} 
         @h2xmle_dataFormat  {Q20} */

    uint32_t epd_end_threshold;
    /**< @h2xmle_description {Threshold for the end of speech}
         @h2xmle_default     {0xFA9B62B7} 
         @h2xmle_dataFormat  {Q20} */

}
#include "graphite_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_GAIN                     0x00012C03

/* Minor Version of this PARAM */
#define API_VERSION_GAIN                  0x1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_gain param_id_gain_t;

/** @h2xmlp_parameter   {"PARAM_ID_GAIN", PARAM_ID_GAIN} 
    @h2xmlp_description {parameter used to set the gain that is applied to the 
                         audio incoming data} */
#include "graphite_begin_pack.h"
struct param_id_gain
{
    uint32_t minor_version;
    /**< @h2xmle_description {Minor version used for tracking the version of 
                              this parameter ID}  
         @h2xmle_range       {1..API_VERSION_GAIN}  
         @h2xmle_default     {API_VERSION_GAIN} */

    int16_t gain;
    /**< @h2xmle_description {Gain applied to the data}
         @h2xmle_default     {0x100} 
         @h2xmle_dataFormat  {Q8}   */

    uint16_t reserved;
    /**< @h2xmle_description {This field must be set to zero}
         @h2xmle_readonly    {true}
         @h2xmle_default     {0}  */
}
#include "graphite_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramete id */
#define PARAM_ID_SWMAD_ENABLE                        0x00012C1A

/* Minor version of this PARAM */
#define API_VERSION_SWMAD_ENABLE                            0x1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct swmad_enable_param_v1 swmad_enable_param_v1_t;

/** @h2xmlp_parameter   {"PARAM_ID_SWMAD_ENABLE", PARAM_ID_SWMAD_ENABLE}
    @h2xmlp_description {parameter used to enable Software Mic Activity 
                         Detection (SWMAD)} */
#include "graphite_begin_pack.h"
struct swmad_enable_param_v1
{
    uint32_t minor_version;
    /**< @h2xmle_description {Tracks the current version of this parameter}
         @h2xmle_range       {1..API_VERSION_SWMAD_ENABLE}  
         @h2xmle_default     {API_VERSION_SWMAD_ENABLE} */

    int32_t enable;
    /**< @h2xmle_description {Specifies whether the module is enabled}
         @h2xmle_rangeList   {disable=0; enable=1} 
         @h2xmle_default     {0} */
}
#include "graphite_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/
/* Unique Parameter id */
#define PARAM_ID_SWMAD_CFG                           0x00012C18

/* Minor version of this PARAM */
#define API_VERSION_SWMAD_CONFIG                            0x1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct swmad_cfg_param_v1 swmad_cfg_param_v1_t;

/** @h2xmlp_parameter   {"PARAM_ID_SWMAD_CFG", PARAM_ID_SWMAD_CFG} 
    @h2xmlp_description {Parameter used to configure Software Mic Activity 
                         Detection (SWMAD)}  */
#include "graphite_begin_pack.h"
struct swmad_cfg_param_v1
{
    uint32_t minor_version;
    /**< @h2xmle_description {Tracks the current version of this parameter}
         @h2xmle_range       {1..API_VERSION_SWMAD_CONFIG}  
         @h2xmle_default     {API_VERSION_SWMAD_CONFIG} */

    int32_t sw_mad_thresh_Q23;
    /**< @h2xmle_description {Score threshold for the binary classifier}
         @h2xmle_range       {-2147483648..2139095040}
         <!--@h2xmle_dataFormat  {Q23}  tool issue-->
         @h2xmle_default     {-511573}  */

    int16_t sw_mad_gain_Q8;
    /**< @h2xmle_description {Gain value to multiply to input samples}
         @h2xmle_range       {-32768..32767} 
         @h2xmle_dataFormat  {Q8}   
         @h2xmle_default     {256}    */

    int16_t reserved;
    /**< @h2xmle_description {This field must be set to 0}
         @h2xmle_readonly    {true}
         @h2xmle_default     {0} */
}
#include "graphite_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Parameter id */
#define PARAM_ID_SWMAD_MODEL               0x00012C19

/*==============================================================================
   Type definitions
==============================================================================*/

/** @h2xmlp_parameter   {"PARAM_ID_SWMAD_MODEL", 
                         PARAM_ID_SWMAD_MODEL}    
    @h2xmlp_description {parameter used to register the SWMAD model. The SWMAD 
                         model contains the unique sound characteristics or 
                         signatures used by the algorithm hosted in the 
                         module.}*/
struct param_id_swmad_model
{
  uint8_t bolb[0];
  /**< Blob: Payload is Blob which represents the sound model and it's size is
             mention in param_size

       @h2xmle_description  {Payload is Blob which represents the sound model
                             and it's size is mentioned in param_size} */  
};
/*==============================================================================
   Constants
==============================================================================*/

/* Unique Parameter id */
#define PARAM_ID_BUFFERING_MODULE_INFO 0x00012c23

/* Minor version of this PARAM */
#define API_VERSION_BUFFERING_MODULE_INFO 1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_buffering_module_info
               param_id_buffering_module_info_t;

/** @h2xmlp_parameter   {"PARAM_ID_BUFFERING_MODULE_INFO", 
                         PARAM_ID_BUFFERING_MODULE_INFO}   
    @h2xmlp_description {parameter used to share related buffering module 
                         info}*/
#include "graphite_begin_pack.h"
struct param_id_buffering_module_info
{
  uint32_t minor_version;
  /**< @h2xmle_description {Minor version used for tracking the version of 
                           this parameter}
       @h2xmle_range       {1..API_VERSION_BUFFERING_MODULE_INFO}  
       @h2xmle_default     {API_VERSION_BUFFERING_MODULE_INFO} */

  uint32_t module_id;
  /**< @h2xmle_description {Buffering module id}
       @h2xmle_default  {0} */

  uint16_t instance_id;
  /**< @h2xmle_description {Buffering module instance id}
       @h2xmle_default  {0} */

  uint16_t reserved;
  /**< @h2xmle_description {Reserved space for alignment}
       @h2xmle_readonly {true}
       @h2xmle_default  {0} */

}
#include "graphite_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/
/* Unique Paramter id */   
#define PARAM_ID_DETECTION_ENGINE_CONFIG_VOICE_WAKEUP  \
        PARAM_ID_DETECTION_ENGINE_CONFIG

/* Minor version of this PARAM */
#define API_VERSION_DETECTION_ENGINE_CONFIG_VOICE_WAKEUP   0x1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_detection_engine_config_voice_wakeup
               param_id_detection_engine_config_voice_wakeup_t;

/** @h2xmlp_parameter   {"PARAM_ID_DETECTION_ENGINE_CONFIG_VOICE_WAKEUP", 
                         PARAM_ID_DETECTION_ENGINE_CONFIG_VOICE_WAKEUP}
    @h2xmlp_description {PARAM_ID_DETECTION_ENGINE_CONFIG is getting redefined 
                         here. These details are specific to Voice Wakeup 
                         while inheriting the original definition} */
#include "graphite_begin_pack.h"
struct param_id_detection_engine_config_voice_wakeup
{
  uint16_t mode;
  /**< @h2xmle_description {Flags that configure the Voice Wakeup module to 
                            run in different modes}
       
       @h2xmle_bitField    {0x00000001}
       @h2xmle_bitName     {detectionMode}
       @h2xmle_description {Keyword/Pattern Detection mode}
       @h2xmle_rangeList   {disabled=0;enabled=1}
       @h2xmle_bitFieldEnd
       
       @h2xmle_bitField    {0x00000002}
       @h2xmle_bitName     {verificationMode}
       @h2xmle_description {User Verification mode}
       @h2xmle_rangeList   {disabled=0;enabled=1}
       @h2xmle_bitFieldEnd
       
       @h2xmle_bitField    {0x00000004}
       @h2xmle_bitName     {eventMode}
       @h2xmle_description {Detection Events are sent to the HLOS}
       @h2xmle_rangeList   {success=0;always=1}
       @h2xmle_bitFieldEnd  

       @h2xmle_default     {1} */

  uint16_t custom_payload_size;
  /**< @h2xmle_description {Size of addition custom configuration sent to 
                            detection engine module. If custom_payload_size 
                            isn’t 4byte aligned size, then caller needs to fill 
                            padding bytes to ensure that entire calibration is 
                            4byte aligned. Immediately following this variable 
                            is the custom configuration Payload of size 
                            custom_payload_size.}
       @h2xmle_range       {0..0xFFFF} 
       @h2xmle_default     {0} */

  uint8_t minor_version;
  /**< @h2xmle_description {Tracks the current version of this parameter}
       @h2xmle_range       {1..API_VERSION_DETECTION_ENGINE_CONFIG_VOICE_WAKEUP}  
       @h2xmle_default     {API_VERSION_DETECTION_ENGINE_CONFIG_VOICE_WAKEUP} */

  uint8_t num_active_models;
  /**< @h2xmle_description {Sum of all keyword models plus the active user 
                            models}
       @h2xmle_range       {1..20}  
       @h2xmle_default     {1} */

  uint8_t list[0];
  /**< @h2xmle_description {This list contains two arrays (Confidence levels and
                            Keyword/User Enables).

                            Confidence Levels: This is an array of size 
                            num_active_models and each element is 1 byte long. 
                            This field enumerates the confidence level for each 
                            keywords and active user pair. Assuming there are 
                            N keywords and M active user models included in the 
                            model, then the first N elements of this parameter 
                            corresponds to N keywords confidence. The N+1 to 
                            N+M elements controls M active user model's 
                            confidence level. Each element maps confidence 
                            values for entries (keyword and/or user specific 
                            keyword) whose order is specified in sound model.
 
                            Keyword/User enable:
                            Immediately following this structure is an array of 
                            size num_active_models and each element is 1 byte 
                            long. This field enumerates the 
                            enabling(1)/disabling(0) status for each keywords
                            and active user pair. Assuming there are N keywords 
                            and M active user models included in the model, then
                            the first N elements of this parameter corresponds 
                            to N keywords enabling/disabling. The N+1 to N+M 
                            elements controls M active user model's 
                            enabling/disabling. Each element maps 
                            enable(1)/disable(0) values for entries (keyword 
                            and/or user specific keyword) whose order is 
                            specified in sound model.} */   
}
#include "graphite_end_pack.h"
;


/*==============================================================================
   Constants
==============================================================================*/
/* Referencing to already defined Parameter id */
#define PARAM_ID_DETECTION_ENGINE_EVENT_VOICE_WAKEUP  \
        PARAM_ID_DETECTION_ENGINE_EVENT

/* Minor version of this PARAM */
#define API_VERSION_DETECTION_ENGINE_EVENT_VOICE_WAKEUP   0x1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_detection_engine_event_voice_wakeup
               param_id_detection_engine_event_voice_wakeup_t;

/** @h2xmlp_parameter   {"PARAM_ID_DETECTION_ENGINE_EVENT_VOICE_WAKEUP", 
                         PARAM_ID_DETECTION_ENGINE_EVENT_VOICE_WAKEUP} 
    @h2xmlp_description {PARAM_ID_DETECTION_ENGINE_EVENT is getting redefined 
                         here. These details are specific to Voice Wakeup 
                         while inheriting the original definition} */
#include "graphite_begin_pack.h"               
struct param_id_detection_engine_event_voice_wakeup
{
  uint16_t status;
  /**< @h2xmle_description {Status returned by algorithm}
       @h2xmle_rangeList   {success=DETECTION_ENGINE_EVENT_SUCCESS; 
                            failed=DETECTION_ENGINE_EVENT_FAILED} */

  uint16_t custom_payload_size;
  /**< @h2xmle_description {Size of addition custom configuration sent to 
                            detection engine module. If custom_payload_size 
                            isn’t 4byte aligned size, then caller needs to 
                            fill padding bytes to ensure that entire 
                            calibration is 4byte aligned. Immediately following 
                            this variable  is the custom configuration Payload 
                            of size custom_payload_size.}
       @h2xmle_range       {0..0xFFFF} 
       @h2xmle_default     {0} */

  uint8_t minor_version;
  /**< @h2xmle_description {Tracks the current version of this parameter}
       @h2xmle_range       {1..API_VERSION_DETECTION_ENGINE_EVENT_VOICE_WAKEUP}  
       @h2xmle_default     {API_VERSION_DETECTION_ENGINE_EVENT_VOICE_WAKEUP} */

  uint8_t num_active_models;
  /**< @h2xmle_description {Sum of all keyword models plus the active user 
                            models}
       @h2xmle_range       {1..20}  
       @h2xmle_default     {1} */

  uint8_t list[0];
  /**< @h2xmle_description {This list is an array of size num_active_models and 
                            each element is 1 byte long. This field enumerates
                            the detection confidence level for each keywords and
                            active user pair. Assuming there are N keywords and 
                            M active user models included in the model, then the
                            first N elements of this parameter corresponds to
                            N keywords detection confidence level. The N+1 to 
                            N+M elements highlights the M active user model's 
                            detection confidence level. Each element maps it's 
                            confidence level values for entries (keyword and/or 
                            user specific keyword) whose order is specified in 
                            sound model.} */

}
#include "graphite_end_pack.h"
;

/** @} */

#endif /* _VOICE_WAKEUP_H_ */
