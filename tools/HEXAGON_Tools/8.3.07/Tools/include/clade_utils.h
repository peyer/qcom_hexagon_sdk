#ifndef __CLADE_UTILS_H__
#define __CLADE_UTILS_H__
/*==========================================================================
 * FILE:         clade_utils.h
 *
 * DESCRIPTION:  DDRCLADE utils
 *
 * Copyright (c) 2016 Qualcomm Technologies Incorporated.
 * All Rights Reserved. QUALCOMM Proprietary and Confidential.
 ===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#include "clade_api.h"
#include "clade_trace.h"


DDRCLADE_API_EXPORT
clade_error_t clade_init_with_data(clade_config_t *config, const char*, int len);

#ifdef __cplusplus
}
#endif

#endif
