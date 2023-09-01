/* hvx_add_constant.c
*
* Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

/* std headers */
#include <stdio.h>
#include <stdlib.h>
#include "android/log.h"
#include <string.h>

/* hvx headers */
#include "hvx_lib.h"

#define ILOG(fmt, args...) \
   __android_log_print(ANDROID_LOG_ERROR, 0, "%s:%d hvx_debug "fmt"\n", __func__, __LINE__, ##args)

#define INLOG(fmt, args...) \
  __android_log_print(ANDROID_LOG_ERROR, 0, "hvx_debug "fmt"\n", ##args)

struct hvx_update_t {
  uint32_t constant;
};

static hvx_ret_type_t hvx_lib_open(void **oem_data)
{
  uint32_t *data = (uint32_t *)calloc(1, sizeof(uint32_t));
  if (!data) {
    /* log error */
    ILOG("failed: data %p\n", data);
    return HVX_RET_FAILURE;
  }
  *oem_data = (void *)data;
  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_get_hvx_info(void *oem_data,
    hvx_lib_get_hvx_info_t *hvx_info)
{
  if (!oem_data || !hvx_info) {
   ILOG("failed: oem_data %p hvx_info %p\n", oem_data, hvx_info);
   return HVX_RET_FAILURE;
  }

  hvx_info->hvx_enable = HVX_TRUE;

  strlcpy(hvx_info->algo_name, "hvx_add_pixel_test",
    sizeof(hvx_info->algo_name));
  hvx_info->hvx_out_width = hvx_info->sensor_width;
  hvx_info->hvx_out_height = hvx_info->sensor_height;
  if (hvx_info->available_hvx_units >= 1) {
    hvx_info->request_hvx_units = 1;
  }
  if (hvx_info->available_hvx_vector_mode >= HVX_LIB_VECTOR_64) {
    hvx_info->request_hvx_vector_mode = HVX_LIB_VECTOR_64;
  }
  hvx_info->is_pix_intf_needed = HVX_TRUE;
  hvx_info->is_stats_needed = HVX_FALSE;
  hvx_info->is_dump_frame_needed = HVX_TRUE;

  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_set_config(void *oem_data,
  const hvx_lib_config_t *lib_config,
  const hvx_lib_adsp_config_t *adsp_config,
  void *caller_data)
{
  hvx_ret_type_t               ret = HVX_RET_SUCCESS;
  uint32_t                    *data = NULL;
  struct hvx_update_t          dynamic_config;

  if (!oem_data || !lib_config || !adsp_config ||
    !adsp_config->adsp_config_call) {
    ILOG("failed: %p %p %p\n", oem_data,
      lib_config, adsp_config);
    return HVX_RET_FAILURE;
  }

  data = (uint32_t *)oem_data;

  dynamic_config.constant = *data = 5;

  /* Call adsp_config to pass call to ADSP */
  ret = adsp_config->adsp_config_call(&dynamic_config, sizeof(dynamic_config),
    caller_data);
  if (ret != HVX_RET_SUCCESS) {
    /* log error */
    ILOG("failed: adsp_config_call ret %d\n", ret);
  }

  return ret;
}
#define PROPERTY_VALUE_MAX 100

static hvx_ret_type_t hvx_lib_consume_dump(void *oem_data,
  unsigned int frame_id, hvx_lib_stats_t *stats_data,
  const hvx_lib_adsp_config_t *adsp_config, void *caller_data)
{
  //struct hvx_lib_private_data_t *private_data = NULL;
  //hvx_ret_type_t                 ret = HVX_RET_SUCCESS;
  uint32_t                      *data = NULL;
  //char                           value[PROPERTY_VALUE_MAX];
  char                           buf[100];
  FILE                          *fptr = NULL;
  //uint32_t                       i = 0;
  int                            write_length;

  //ILOG("%s:%d: E", __func__, __LINE__);
  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !stats_data || !caller_data) {
    /* log error */
    ILOG("failed: %p %p %p %p\n", oem_data,
      adsp_config, stats_data, caller_data);
    return HVX_RET_FAILURE;
  }

  ILOG("buffer info: left addr %p, left size %d, right addr %p, right size %d",
        stats_data->stats_left, stats_data->stats_left_size,
        stats_data->stats_right, stats_data->stats_right_size);

  if (1) {
    /* Dump frame buffer */
    if (stats_data->stats_left && stats_data->stats_left_size) {
      data = (uint32_t *)stats_data->stats_left;
      snprintf(buf, sizeof(buf), "/data/misc/camera/hvx_ac_dump_left_frame%d.raw", frame_id);
      fptr = fopen(buf, "w+");
      if (!fptr) {
        ILOG("failed fptr %p buf %s\n",fptr, buf);
      } else {
        write_length = fwrite(data, 1, stats_data->stats_left_size, fptr);
		//write_length = fwrite(data, 1, 128, fptr);
        ILOG("write length for frame id %d is %d", frame_id, write_length);
        fclose(fptr);
      }
    }
    if (stats_data->stats_right && stats_data->stats_right_size) {
      data = (uint32_t *)stats_data->stats_right;
      snprintf(buf, sizeof(buf), "/data/misc/camera/hvx_ac_dump_right_frame%d.raw", frame_id);
      fptr = fopen(buf, "w+");
      if (!fptr) {
        ILOG("failed fptr %p buf %s\n",fptr, buf);
      } else {
        write_length = fwrite(data, 1, stats_data->stats_right_size, fptr);
		//write_length = fwrite(data, 1, 128, fptr);
        ILOG("write length for frame id %d is %d", frame_id, write_length);
        fclose(fptr);
      }
    }
  }
  
  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_sof(void *oem_data,
  const hvx_lib_sof_params_t *sof_params,
  const hvx_lib_adsp_config_t *adsp_config,
  void *caller_data)
{
  hvx_ret_type_t       ret = HVX_RET_SUCCESS;
  uint32_t            *data = NULL;
  struct hvx_update_t  hvx_update;

  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !sof_params) {
    /* log error */
    ILOG("failed: %p %p %p\n", oem_data,
      adsp_config, sof_params);
    return HVX_RET_FAILURE;
  }

  data = (uint32_t *)oem_data;

  hvx_update.constant = *data;
  *data = *data + 5;
  if (*data >= 100) {
    *data = 5;
  }

  /* Call adsp_config to pass call to ADSP */
  ret = adsp_config->adsp_config_call(&hvx_update, sizeof(hvx_update),
    caller_data);
  if (ret != HVX_RET_SUCCESS) {
    /* log error */
    ILOG("failed: adsp_config_call ret %d\n", ret);
  }

  return HVX_RET_SUCCESS;
}
static   hvx_ret_type_t hvx_lib_get_dump_buffer_size(int output_width,
												 int output_height,
												 int right_stripe_offset,
												 int overlap,
												 int * dump_frame_size)
{
	int vfe0_width, vfe1_width;
	hvx_ret_type_t               ret = HVX_RET_SUCCESS;
    vfe0_width = right_stripe_offset + overlap;
    vfe1_width = output_width - right_stripe_offset;
    //ILOG("JS: in set config, request frame buffers, sensor w %d, sensor h %d, vfe0 w %d, vfe1 w %d",
    //     full_camif_w, full_camif_h, vfe0_width, vfe1_width);

    if (vfe0_width > vfe1_width) {
      *dump_frame_size = (((vfe0_width*2 + 16*2 + 127) >> 7) << 7) * (output_height);
    } else {
      *dump_frame_size = (((vfe1_width*2 + 16*2 + 127) >> 7) << 7) * (output_height);
    }

	return ret;
}


static hvx_ret_type_t hvx_lib_close(void *oem_data)
{
  if (!oem_data) {
     /* log error */
     ILOG("failed: hvx_lib_close oem_data %p\n", oem_data);
     return HVX_RET_FAILURE;
  }

  free(oem_data);

  return HVX_RET_SUCCESS;
}

hvx_ret_type_t hvx_lib_fill_function_table(
  hvx_lib_function_table_t *func_table)
{
  ILOG("");
  if (!func_table) {
    ILOG("failed func_table %p\n", func_table);
    return HVX_RET_FAILURE;
  }

  func_table->hvx_lib_open = hvx_lib_open;
  func_table->hvx_lib_get_hvx_info = hvx_lib_get_hvx_info;
  func_table->hvx_lib_set_config = hvx_lib_set_config;
  func_table->hvx_lib_consume_dump = hvx_lib_consume_dump;
  func_table->hvx_lib_consume_stats = hvx_lib_consume_dump;
  func_table->hvx_lib_sof = hvx_lib_sof;
  func_table->hvx_lib_close = hvx_lib_close;
  func_table->hvx_lib_get_dump_buffer_size =  hvx_lib_get_dump_buffer_size;

  return HVX_RET_SUCCESS;
}
