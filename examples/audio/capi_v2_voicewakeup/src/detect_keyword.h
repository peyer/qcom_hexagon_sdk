#ifndef DETECT_KEYWORD_H
#define DETECT_KEYWORD_H
/* ======================================================================== */
/**
   @file detect_keyword.h

   Header file to implement keyword detection.
 */

/*==============================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "mmdefs.h"
#include "capi_v2_voicewakeup_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

capi_v2_err_t detect_keyword(capi_v2_voicewakeup_t* me_ptr,
		                     capi_v2_buf_t* inp_buf_ptr);

#ifdef __cplusplus
}
#endif

#endif // DETECT_KEYWORD_H

