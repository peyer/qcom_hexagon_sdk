/*========================================================================
 $Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/test/audio/test_capi_v2/src/feedback_fwk.h#1 $

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

#ifndef _FEEDBACK_FWK_H_
#define _FEEDBACK_FWK_H_

/*==========================================================================
 Include files
 ========================================================================== */
#include "Elite_CAPI_V2.h"
#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus


typedef struct fb_client_info_t
{
   void*               buf_q_handle;
   void*               data_q_handle;
} fb_client_info_t;


capi_v2_err_t create_queue(void **queue_handle_ptr);

capi_v2_err_t destroy_queue(void **queue_handle_ptr);

capi_v2_err_t en_queue_back(void *queue_handle, void *element);

capi_v2_err_t de_queue_front(void *queue_handle, void **element_pptr);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* _FEEDBACK_FWK_H_ */
