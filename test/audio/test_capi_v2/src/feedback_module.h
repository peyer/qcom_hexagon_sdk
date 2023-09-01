/*========================================================================
 $Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/test/audio/test_capi_v2/src/feedback_module.h#1 $

 Edit History

 when       who     what, where, why
 --------   ---     -------------------------------------------------------
 7/22/2014   rv       Created

 ==========================================================================*/

/*-----------------------------------------------------------------------
 This file contains Feedback implementation to be used by CAPI V2 modules for
 communicating between them.  

 Copyright (c) 2012-2015 Qualcomm  Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------*/

#ifndef _FEEDBACK_MODULE_H_
#define _FEEDBACK_MODULE_H_

/*==========================================================================
 Include files
 ========================================================================== */
#include "Elite_CAPI_V2.h"



#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

/*==========================================================================
 Function Declarations
 ========================================================================== */



/**
 * This function is used to return the buffer to the return Q.
 */
capi_v2_err_t feedback_push_buf_to_buf_q(void *client_info_ptr, void *pPayLoad);

/**
 * This function is used to pop a buffer from the data Q.
 */
capi_v2_err_t feedback_pop_buf_from_data_q(void *client_info_ptr, void **pPayLoad);

/**
 * This function is used to pop a buffer from the buffer Q.
 */
capi_v2_err_t feedback_pop_buf_from_buf_q(void *buf_q_ptr, void **payload_pptr);

/**
 * This function is used to send the buffer to the data Q.
 */
capi_v2_err_t feedback_send_buf_to_data_q(void *data_q_ptr, void *payload_ptr);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* _FEEDBACK_MODULE_H_ */
