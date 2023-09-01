#ifndef TEST_CAPI_V2_H
#define TEST_CAPI_V2_H
/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "Elite_CAPI_V2.h"

#ifdef __cplusplus
extern "C"
{
#endif

capi_v2_err_t test_capi_v2_main(capi_v2_get_static_properties_f get_static_properties_f,
                             capi_v2_init_f init_f,
                             capi_v2_proplist_t* init_set_properties,
                             capi_v2_proplist_t* static_properties,
                             const char* filename_in, const char* filename_out,
                             const char* filename_config);


capi_v2_err_t test_capi_v2_main_feedback(capi_v2_get_static_properties_f get_static_properties_rx_f,
                                capi_v2_init_f init_rx_f,
                                capi_v2_get_static_properties_f get_static_properties_tx_f,
                                capi_v2_init_f init_tx_f,
                                capi_v2_proplist_t* init_set_properties,
                                capi_v2_proplist_t* static_properties,
                                const char* filename_rx_in,
                                const char* filename_rx_out,
                                const char* filename_rx_config,
                                const char* filename_tx_in,
                                const char* filename_tx_out,
                                const char* filename_tx_config
                                );

#ifdef __cplusplus
}
#endif

#endif // TEST_CAPI_V2_H
