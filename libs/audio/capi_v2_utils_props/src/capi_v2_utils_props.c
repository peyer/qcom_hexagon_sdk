/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "capi_v2_utils_props.h"

#include "HAP_farf.h"

capi_v2_err_t capi_v2_utils_props_get_init_memory_requirement(capi_v2_buf_t* payload,
                                                              uint32_t* size_in_bytes)
{
  capi_v2_init_memory_requirement_t* init_mem_req = 
     (capi_v2_init_memory_requirement_t*)payload->data_ptr;
  *size_in_bytes = init_mem_req->size_in_bytes;
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_init_memory_requirement(capi_v2_buf_t* payload,
                                                              uint32_t size_in_bytes)
{
  capi_v2_init_memory_requirement_t* init_mem_req =
     (capi_v2_init_memory_requirement_t*)payload->data_ptr;
  init_mem_req->size_in_bytes = size_in_bytes;
  payload->actual_data_len = sizeof(capi_v2_init_memory_requirement_t);
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_get_stack_size(capi_v2_buf_t* payload,
                                                 uint32_t* size_in_bytes)
{
  capi_v2_stack_size_t* stack_size =
     (capi_v2_stack_size_t*)payload->data_ptr;
  *size_in_bytes = stack_size->size_in_bytes;
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_stack_size(capi_v2_buf_t* payload,
                                                 uint32_t size_in_bytes)
{
  capi_v2_stack_size_t* stack_size =
     (capi_v2_stack_size_t*)payload->data_ptr;
  stack_size->size_in_bytes = size_in_bytes;
  payload->actual_data_len = sizeof(capi_v2_stack_size_t);
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_get_max_metadata_size(capi_v2_buf_t* payload,
                                                        uint32_t* output_port_index,
                                                        uint32_t* size_in_bytes)
{
  capi_v2_max_metadata_size_t* meta_size =
  (capi_v2_max_metadata_size_t*)payload->data_ptr;
   *output_port_index = meta_size->output_port_index;
   *size_in_bytes = meta_size->size_in_bytes;
   return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_max_metadata_size(capi_v2_buf_t* payload,
                                                        uint32_t output_port_index,
                                                        uint32_t size_in_bytes)
{
  capi_v2_max_metadata_size_t* meta_size =
     (capi_v2_max_metadata_size_t*)payload->data_ptr;
  meta_size->output_port_index = output_port_index;
  meta_size->size_in_bytes = size_in_bytes;
  payload->actual_data_len = sizeof(capi_v2_max_metadata_size_t);
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_get_is_inplace(capi_v2_buf_t* payload,
                                                 bool_t* is_inplace)
{
  capi_v2_is_inplace_t* in_place =
  (capi_v2_is_inplace_t*)payload->data_ptr;
   *is_inplace = in_place->is_inplace;
   return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_is_inplace(capi_v2_buf_t* payload,
                                                 bool_t is_inplace)
{
  capi_v2_is_inplace_t* in_place =
     (capi_v2_is_inplace_t*)payload->data_ptr;
  in_place->is_inplace = is_inplace;
  payload->actual_data_len = sizeof(capi_v2_is_inplace_t);
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_get_requires_data_buffering(capi_v2_buf_t* payload,
                                                              bool_t* requires_data_buffering)
{
  capi_v2_requires_data_buffering_t* req_data_buffering =
     (capi_v2_requires_data_buffering_t*)payload->data_ptr;
  *requires_data_buffering = req_data_buffering->requires_data_buffering;
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_requires_data_buffering(capi_v2_buf_t* payload,
                                                              bool_t requires_data_buffering)
{
  capi_v2_requires_data_buffering_t* req_data_buffering =
     (capi_v2_requires_data_buffering_t*)payload->data_ptr;
  req_data_buffering->requires_data_buffering = requires_data_buffering;
  payload->actual_data_len = sizeof(capi_v2_requires_data_buffering_t);
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_num_framework_extensions(capi_v2_buf_t* payload,
                                                               uint32_t num)
{
  capi_v2_num_needed_framework_extensions_t* num_extns =
     (capi_v2_num_needed_framework_extensions_t*)payload->data_ptr;
  num_extns->num_extensions = num;
  return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_set_framework_extensions(capi_v2_buf_t* payload,
                                                           uint32_t num, capi_v2_framework_extension_id_t* list)
{
  uint32_t i=0;
  capi_v2_framework_extension_id_t* extensions_list =
  (capi_v2_framework_extension_id_t*)(payload->data_ptr);
   for(i=0; i < num; i++){
    extensions_list->id = list->id;
    list++;
    extensions_list++;
  }
   return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_utils_props_process_properties(capi_v2_proplist_t* prop,
                                                     capi_v2_utils_props_process_cb cb,
                                                     void* ctx)
{
  capi_v2_err_t result = CAPI_V2_EOK;
  uint32_t i = 0;

  for (i = 0; i < prop->props_num; i++) {
    capi_v2_buf_t* payload = &(prop->prop_ptr[i].payload);

    if (CAPI_V2_FAILED(result = cb(ctx, prop->prop_ptr[i].id, payload))) {
      return result;
    }
  }
  return result;
}
