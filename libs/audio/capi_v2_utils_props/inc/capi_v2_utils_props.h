/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "Elite_CAPI_V2.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef capi_v2_err_t(* capi_v2_utils_props_process_cb)(void* ctx,
                                                  capi_v2_property_id_t id,
                                                  capi_v2_buf_t* payload);
capi_v2_err_t capi_v2_utils_props_process_properties(capi_v2_proplist_t* prop,
                                               capi_v2_utils_props_process_cb cb,
                                               void* ctx);

capi_v2_err_t capi_v2_utils_props_get_init_memory_requirement(capi_v2_buf_t* payload,
                                                        uint32_t* size_in_bytes);
capi_v2_err_t capi_v2_utils_props_set_init_memory_requirement(capi_v2_buf_t* payload,
                                                        uint32_t size_in_bytes);
capi_v2_err_t capi_v2_utils_props_get_stack_size(capi_v2_buf_t* payload,
                                           uint32_t* size_in_bytes);
capi_v2_err_t capi_v2_utils_props_set_stack_size(capi_v2_buf_t* payload,
                                           uint32_t size_in_bytes);
capi_v2_err_t capi_v2_utils_props_get_max_metadata_size(capi_v2_buf_t* payload,
                                                  uint32_t* output_port_index,
                                                  uint32_t* size_in_bytes);
capi_v2_err_t capi_v2_utils_props_set_max_metadata_size(capi_v2_buf_t* payload,
                                                  uint32_t output_port_index,
                                                  uint32_t size_in_bytes);
capi_v2_err_t capi_v2_utils_props_get_is_inplace(capi_v2_buf_t* payload,
                                           bool_t* is_inplace);
capi_v2_err_t capi_v2_utils_props_set_is_inplace(capi_v2_buf_t* payload,
                                           bool_t is_inplace);
capi_v2_err_t capi_v2_utils_props_get_requires_data_buffering(capi_v2_buf_t* payload,
                                                        bool_t* requires_data_buffering);
capi_v2_err_t capi_v2_utils_props_set_requires_data_buffering(capi_v2_buf_t* payload,
                                                        bool_t requires_data_buffering);
capi_v2_err_t capi_v2_utils_props_set_num_framework_extensions(capi_v2_buf_t* payload,
                                              uint32_t num);
capi_v2_err_t capi_v2_utils_props_set_framework_extensions(capi_v2_buf_t* payload,
                                              uint32_t num, capi_v2_framework_extension_id_t* list);

#ifdef __cplusplus
}
#endif // __cplusplus
