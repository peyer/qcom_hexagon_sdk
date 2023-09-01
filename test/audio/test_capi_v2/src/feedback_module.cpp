/*========================================================================
 $Header: //components/dev/avs.adsp/2.7/roverma.AVS.ADSP.2.7.jun_17_27/elite/module_interfaces/utils/src/feedback_module.cpp#1 $

 Edit History

 when       who     what, where, why
 --------   ---     -------------------------------------------------------
 7/22/2014   rv       Created

 ==========================================================================*/

/*-----------------------------------------------------------------------
 Copyright (c) 2012-2015 Qualcomm  Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------*/

/*============================================================================
 *                       INCLUDE FILES FOR MODULE
 *==========================================================================*/
#include "feedback_module.h"
#include "feedback_fwk.h"



#define FEEDBACK_BUFFER 0xFEEDBACC

capi_v2_err_t feedback_push_buf_to_buf_q(void *client_info_ptr, void *pPayLoad)
{
   return en_queue_back(((fb_client_info_t *)client_info_ptr)->buf_q_handle, pPayLoad);
}


capi_v2_err_t feedback_pop_buf_from_data_q(void *client_info_ptr, void **pPayLoad)
{
   return de_queue_front(((fb_client_info_t *)client_info_ptr)->data_q_handle, pPayLoad);
}


capi_v2_err_t feedback_pop_buf_from_buf_q(void *buf_q_ptr, void **payload_pptr)
{
   return de_queue_front(buf_q_ptr, payload_pptr);
}


capi_v2_err_t feedback_send_buf_to_data_q(void *data_q_ptr, void *payload_ptr)
{
   return en_queue_back(data_q_ptr, payload_ptr);
}

