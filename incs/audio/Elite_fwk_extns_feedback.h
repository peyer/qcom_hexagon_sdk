#ifndef ELITE_FWK_EXTNS_FEEDBACK_H
#define ELITE_FWK_EXTNS_FEEDBACK_H

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
extern "C" {
#endif /*__cplusplus*/

#include "Elite_CAPI_V2_types.h"


#define FWK_EXTN_FEEDBACK 0x00010273
/**< Unique identifier to represent custom framework extension for feedback between modules. We can pass custom data
     between multiple modules. Currently, it supports multiple source modules but there should only be one destination module. */

#define CAPI_V2_PARAM_ID_CLIENT_INFO 0x00010274
/**< Unique identifier to send the custom client list information to the algorithm capi v2 framework
   to use it for passing custom data between two capi v2 modules on different threads. It contains the client list of
   all the source modules that connect want to send data to a dest module. This info is passed to the dest module from
   the service layer.

  For example:
  CALLER
      feedback_client_list_t *cust_client_list_ptr;
      capi_v2_buf_t client_buf;
      client_buf.data_ptr = (int8_t*)cust_client_list_ptr;
      client_buf.actual_data_len = sizeof(feedback_client_list_t*);
      client_buf.max_data_len = sizeof(feedback_client_list_t*);
      capi_ptr->vtbl_ptr->set_param(so.capi_ptr, CAPI_V2_PARAM_ID_CLIENT_INFO, &port_info, &client_buf);

  CALLEE
    // Read data_ptr and max_data_len
	 // Read and Cache the cust_client_list_ptr for future use
 
	Refer to /adsp_proc/avs/afe/services/static/inc/AFEInternal.h for the definition of bufQList
 */

/**@brief Feedback client list: It represents the feedback path client list in 
  * which each node is feedback path client.
 */
typedef struct feedback_client_list_t
{
   void *element;
   /**< This element. */
   struct feedback_client_list_t *next;
   /**< Pointer to the next node. */
} feedback_client_list_t;


#define CAPI_V2_PARAM_ID_FB_PATH_INFO 0x00010275
/**< Unique identifier to send the feedback path information to the algorithm capi v2 framework for the use of feedback path between two capi v2 modules on different threads.
  For example:

  CALLER
    capi_v2_buf_t params_buf;
    feedback_info_t feedback_path_info;

    params_buf.data_ptr = (int8_t*)&feedback_path_info;
    params_buf.actual_data_len = sizeof(feedback_info_t);
    params_buf.max_data_len = sizeof(feedback_info_t);
    capi_ptr->vtbl_ptr->set_param (
    so.capi_ptr,
    CAPI_V2_PARAM_ID_FB_PATH_INFO,
    &port_info,
    &params_buf);

  CALLEE
      // Read data_ptr and max_data_len
  	  while (payload_size)
      {
		  // Cache the feedback path information for future use

      };
 */

/* Maximum number of channels supported in feedback */
#define FEEDBACK_MAX_CHANNELS 4

/* Feedback channel info contains information about the channels and their mapping */
typedef struct feedback_config_t
{
   int32_t    num_channels;
   /**< Number of channels. Each Vsens and Isens is considered an
    individual channel.                          */

   int32_t    chan_info[FEEDBACK_MAX_CHANNELS];
   /**< Channel mapping array that provides information on the order of channels */
} feedback_config_t;

/**@brief Feedback information: It is used for communication between two modules.
 * This info is sent from the Service to the Source module using CAPI_V2_PARAM_ID_FB_PATH_INFO.
 */
typedef struct feedback_info_t
{
   /**< Feedback path configuration. */
   feedback_config_t feedback_cfg;
   /**< Handle to the buf queue of the service. */
   void  *bufQ_ptr;
   /**< Handle to the data queue of the service. */
   void  *dataQ_ptr;
   /**< Number of buffers in the bufQ */
   uint32_t          *num_buffers_ptr;
} feedback_info_t;


#define CAPI_V2_PARAM_ID_FB_SRC_PUSH_BUF 0x00010291
/**< Unique identifier to push the buffer from source of the feedback path to Queue using the CAPI_V2_EVENT_DATA_TO_DSP_SERVICE. 
  Payload Structure: capi_v2_fb_src_push_buf_t
 
  For example:

  CALLER
    capi_v2_event_info_t      event_info;
    capi_v2_fb_src_push_buf_t    push_buf;
    capi_v2_event_data_to_dsp_service_t data_event;
    
    push_buf.queue_ptr = (void*)queue_ptr;
    push_buf.buf_ptr = cx_buff_ptr;
 
    data_event.param_id = CAPI_V2_PARAM_ID_SRC_PUSH_BUF;
    data_event.payload.actual_data_len = sizeof(capi_v2_fb_src_push_buf_t);
    data_event.payload.max_data_len = sizeof(capi_v2_fb_src_push_buf_t);
    data_event.payload.data_ptr = (int8_t *)&push_buf; 

    event_info.payload.actual_data_len = sizeof(data_event);
    event_info.payload.max_data_len = sizeof(data_event);
    event_info.payload.data_ptr = (int8_t *)&data_event; 

    (*event_callback_info.event_cb)(event_callback_info.event_context,
                   CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, &event_info);
  
 */

typedef struct capi_v2_fb_src_push_buf_t
{
  void              *queue_ptr;
  /**< pointer to the queue. */
  void              *buf_ptr;
  /**< pointer to the buffer. */
}capi_v2_fb_src_push_buf_t;


#define CAPI_V2_PARAM_ID_FB_SRC_POP_BUF 0x00010292
/**< Unique identifier to push the buffer at source of the feedback path from Queue using the CAPI_V2_EVENT_DATA_TO_DSP_SERVICE. 
  Payload Structure: capi_v2_fb_src_pop_buf_t
 
  For example:

  CALLER
    capi_v2_event_info_t      event_info;
    capi_v2_fb_src_pop_buf_t     pop_buf;
    capi_v2_event_data_to_dsp_service_t data_event;
    
    pop_buf.buf_ptr_ptr = &buff_ptr;
    pop_buf.queue_ptr = queue_ptr;
 
    data_event.param_id = CAPI_V2_PARAM_ID_FB_SRC_POP_BUF;
    data_event.payload.actual_data_len = sizeof(capi_v2_fb_src_pop_buf_t);
    data_event.payload.max_data_len = sizeof(capi_v2_fb_src_pop_buf_t);
    data_event.payload.data_ptr = (int8_t *)&pop_buf; 

    event_info.payload.actual_data_len = sizeof(data_event);
    event_info.payload.max_data_len = sizeof(data_event);
    event_info.payload.data_ptr = (int8_t *)&data_event; 

    (*event_callback_info.event_cb)(event_callback_info.event_context,
                   CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, &event_info);
  
 */

typedef struct capi_v2_fb_src_pop_buf_t
{
  void              *queue_ptr;
  /**< pointer to the queue. */
  void              **buf_ptr_ptr;
  /**< pointer to the buffer. */
}capi_v2_fb_src_pop_buf_t;


#define CAPI_V2_PARAM_ID_FB_DST_RTN_BUF_TO_SRC 0x00010293
/**< Unique identifier to return the buffer at destination of the feedback path to the feedback path client using the CAPI_V2_EVENT_DATA_TO_DSP_SERVICE. 
  Payload Structure: capi_v2_fb_dst_rtn_buf_to_client_t
 
  For example:

  CALLER
    capi_v2_event_info_t      event_info;
    capi_v2_fb_dst_return_buf_to_client_t     return_buf;
    capi_v2_event_data_to_dsp_service_t data_event;
    
    return_buf.buf_ptr = buff_ptr;
    return_buf.queue_ptr = client_ptr;
 
    data_event.param_id = CAPI_V2_PARAM_ID_FB_DST_RETURN_BUF_TO_CLIENT;
    data_event.payload.actual_data_len = sizeof(capi_v2_fb_dst_rtn_buf_to_src_t);
    data_event.payload.max_data_len = sizeof(capi_v2_fb_dst_rtn_buf_to_src_t);
    data_event.payload.data_ptr = (int8_t *)&return_buf; 

    event_info.payload.actual_data_len = sizeof(data_event);
    event_info.payload.max_data_len = sizeof(data_event);
    event_info.payload.data_ptr = (int8_t *)&data_event; 

    (*event_callback_info.event_cb)(event_callback_info.event_context,
                   CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, &event_info);
  
 */

typedef struct capi_v2_fb_dst_rtn_buf_to_src_t
{
  void              *client_ptr;
  /**< pointer to the client. */
  void              *buf_ptr;
  /**< pointer to the buffer. */
}capi_v2_fb_dst_rtn_buf_to_src_t;


#define CAPI_V2_PARAM_ID_FB_DST_GET_BUF_FROM_SRC 0x00010294
/**< Unique identifier to get the buffer at destination of the feedback path from the feedback path client using the CAPI_V2_EVENT_DATA_TO_DSP_SERVICE. 
  Payload Structure: capi_v2_fb_dst_get_buf_from_client_t
 
  For example:

  CALLER
    capi_v2_event_info_t      event_info;
    capi_v2_fb_dst_get_buf_from_client_t     return_buf;
    capi_v2_event_data_to_dsp_service_t data_event;
    
    return_buf.buf_ptr_ptr = &buff_ptr;
    return_buf.queue_ptr = client_ptr;
 
    data_event.param_id = CAPI_V2_PARAM_ID_FB_DST_GET_BUF_FROM_CLIENT;
    data_event.payload.actual_data_len = sizeof(capi_v2_fb_dst_get_buf_from_src_t);
    data_event.payload.max_data_len = sizeof(capi_v2_fb_dst_get_buf_from_src_t);
    data_event.payload.data_ptr = (int8_t *)&return_buf; 

    event_info.payload.actual_data_len = sizeof(data_event);
    event_info.payload.max_data_len = sizeof(data_event);
    event_info.payload.data_ptr = (int8_t *)&data_event; 

    (*event_callback_info.event_cb)(event_callback_info.event_context,
                   CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, &event_info);
  
 */

typedef struct capi_v2_fb_dst_get_buf_from_src_t
{
  void              *client_ptr;
  /**< pointer to the client. */
  void              **buf_ptr_ptr;
 /**< pointer to the buffer. */
}capi_v2_fb_dst_get_buf_from_src_t;


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef ELITE_FWK_EXTNS_FEEDBACK_H */
