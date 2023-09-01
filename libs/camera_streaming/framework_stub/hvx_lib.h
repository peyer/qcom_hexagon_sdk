 /* hvx_lib.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __HVX_LIB_H__
#define __HVX_LIB_H__



/* Start of Macros */

#define MAX_ISP 2
#define HVX_TRUE 1
#define HVX_FALSE 0

/* End of Macros */

/* Start of Enums */

typedef enum {
  HVX_SINGLE_ISP,
  HVX_DUAL_ISP,
  HVX_INVALID_ISP
} hvx_lib_isp_type_t;

typedef enum {
  HVX_RET_SUCCESS,
  HVX_RET_FAILURE,
  HVX_RET_INVALID
} hvx_ret_type_t;

typedef enum {
  HVX_SENSOR_BGGR,
  HVX_SENSOR_GBRG,
  HVX_SENSOR_GRBG,
  HVX_SENSOR_RGGB,
  HVX_SENSOR_UYVY,
  HVX_SENSOR_YUYV,
  HVX_SENSOR_MAX
} hvx_sensor_filter_arrangement;

typedef enum {
   HVX_LIB_VECTOR_NULL,
   HVX_LIB_VECTOR_32,
   HVX_LIB_VECTOR_64,
   HVX_LIB_VECTOR_INVALID,
   HVX_LIB_VECTOR_128,
   HVX_LIB_VECTOR_MAX,
} hvx_lib_vector_mode_t;

/* End of Enums */

/* Start of struct definition */

/** hvx_lib_sensor_info_t
 *
 *  @bayer_format: bayer format for sensor output
 *  @bits_per_pixel: number of bits per pixel
 *
 **/
typedef struct _hvx_lib_sensor_info_t {
  hvx_sensor_filter_arrangement bayer_format;
  unsigned int                  bits_per_pixel;
} hvx_lib_sensor_info_t;

/** hvx_lib_isp_info_t
 *
 *  @camif_width: camif width
 *  @camif_height: camif height
 *  @sensor_offset: offset for sensor output width
 *
 **/
typedef struct _hvx_lib_isp_info_t {
  unsigned int camif_width;
  unsigned int camif_height;
  unsigned int sensor_offset;
} hvx_lib_isp_info_t;

/** hvx_lib_single_isp_info_t:
 *
 *  @isp_info: single isp info
 *
 **/
typedef struct _hvx_lib_single_isp_info_t {
  hvx_lib_isp_info_t isp_info;
} hvx_lib_single_isp_info_t;

/** hvx_lib_dual_isp_info_t:
 *
 *  @isp_info: dual isp info
 *
 **/
typedef struct _hvx_lib_dual_isp_info_t {
  hvx_lib_isp_info_t isp_info[MAX_ISP];
} hvx_lib_dual_isp_info_t;

/** hvx_lib_config_t:
 *
 *  @sensor_info: sensor information
 *  @single_isp_info: single isp info
 *  @dual_isp_info: dual isp info
 *  @isp_type: ISP type (single / dual)
 *
 **/
typedef struct _hvx_lib_config_t {
 hvx_lib_sensor_info_t       sensor_info;
 union {
   hvx_lib_single_isp_info_t single_isp_info;
   hvx_lib_dual_isp_info_t   dual_isp_info;
 } u;
  hvx_lib_isp_type_t         isp_type;
} hvx_lib_config_t;

//remove typedef struct _hvx_lib_adsp_config_t hvx_lib_adsp_config_t;

/** hvx_lib_adsp_config_t:
 *
 *  @adsp_config_call: function pointer to pass params to
 *                   ADSP
 *
 **/
typedef struct _hvx_lib_adsp_config_t {
  hvx_ret_type_t (*adsp_config_call)(void *adsp_data,
    unsigned int adsp_data_size, void *caller_data);
} hvx_lib_adsp_config_t;

/** hvx_lib_stats_t:
 *
 *  @stats_left: left stats pointer
 *  @stats_left_size: left stats pointer size
 *  @stats_right: right stats pointer
 *  @stats_right_size: right stats pointer size
 *
 **/
typedef struct _hvx_lib_stats_t {
  void         *stats_left;
  unsigned int  stats_left_size;
  void         *stats_right;
  unsigned int  stats_right_size;
} hvx_lib_stats_t;

/** hvx_lib_get_hvx_info_t:
 *
 *  @sensor_width: [INPUT] sensor output width
 *  @sensor_height: [INPUT] sensor output height
 *  @available_hvx_units: [INPUT] currently available hvx units
 *                      on DSP side (1 / 2 / 3 / 4)
 *  @available_vector_mode: [INPUT] currently available hvx
 *                        vector mode
 *  @hvx_enable: [OUTPUT] enable / disable HVX for current
 *             configuration
 *  @algo_name: [OUTPUT] algorithm name to run on ADSP
 *  @hvx_out_width: [OUTPUT] HVX output width
 *  @hvx_out_height: [OUTPUT] HVX output height
 *  @request_hvx_units: [OUTPUT] hvx units to be used for this
 *                    sensor
 *  @request_hvx_vector_mode: [OUTPUT] HVX vector mode to be
 *                          used for this sensor
 *  @is_pix_intf_needed: [OUTPUT] set 1 if output of HVX should
 *                     be connected to PIXEL interface
 *                     (Linearization / rolloff etc)
 *  @is_stats_needed: [OUTPUT] enable / disable stats output
 *                  from HVX
 *  @stats_data_size: [OUTPUT] size of each stats data buffer
 *
 **/
typedef struct _hvx_lib_get_hvx_info_t {
  unsigned int          sensor_width;
  unsigned int          sensor_height;
  unsigned int          available_hvx_units;
  hvx_lib_vector_mode_t available_hvx_vector_mode;
  unsigned int          hvx_enable;
  char                  algo_name[32];
  unsigned int          hvx_out_width;
  unsigned int          hvx_out_height;
  unsigned int          request_hvx_units;
  hvx_lib_vector_mode_t request_hvx_vector_mode;
  unsigned int          is_pix_intf_needed;
  unsigned int          is_stats_needed;
  unsigned int          stats_data_size;
  unsigned int          is_dump_frame_needed;
  unsigned int          dump_frame_size;
} hvx_lib_get_hvx_info_t;

/** hvx_lib_sof_params_t:
 *
 *  @frame_id : current frame id
 *  @real_gain: global real gain
 *  @exposure_time: exposure time in ns
 *  @short_real_gain: short real gain during HDR
 *  @short_exposure_time: short exposure time in ns
 *  @target_luma: targed luma to be achieved to settle AEC
 *  @cur_luma: current luma value
 *  @avg_luma: average luma value
 *  @lux_idx: lux index
 *  @aec_settled: boolean flag to indicate whether AEC
 *              settled
 *  @r_gain: AWB r gain
 *  @g_gain: AWB g gain
 *  @b_gain: AWB b gain
 *  @color_temp: color temperature
 **/
typedef struct _hvx_lib_sof_params_t {
  unsigned int frame_id;

  /* AEC params */
  float        real_gain;
  long         exposure_time;
  float        short_real_gain;
  long         short_exposure_time;
  float        long_real_gain;
  unsigned int target_luma;
  unsigned int cur_luma;
  unsigned int avg_luma;
  float        lux_idx;
  int          aec_settled;
  float        hdr_exp_time_ratio;
  float        hdr_sensitivity_ratio;


  /* AWB params */
  float        r_gain;
  float        g_gain;
  float        b_gain;
  unsigned int color_temp;
 
} hvx_lib_sof_params_t;

typedef struct _hvx_lib_function_table_t {
  /** hvx_lib_open:
   *
   *  @oem_data: double pointer to oem_data, to be
   *               filled by OEM stub
   *
   *  Allocate oem_data and return to caller
   *
   *  Return type of hvx_ret_type_t
   *
   *  Finish in <= 1 ms
   **/
  hvx_ret_type_t (*hvx_lib_open)(void **oem_data);

  /** hvx_lib_get_hvx_info:
   *
   *  @oem_data: OEM specific private data
   *  @hvx_info: hvx information
   *
   *  Send sensor output information and get hvx information
   *
   *  Return type of hvx_ret_type_t
   *
   *  Finish in <= 1 ms
   **/
  hvx_ret_type_t (*hvx_lib_get_hvx_info)(void *oem_data,
    hvx_lib_get_hvx_info_t *hvx_info);

  /** hvx_lib_set_config:
   *
   *  @oem_data: OEM specific private data
   *  @lib_config: sensor output info
   *  @adsp_config: ADSP config handle
   *  @caller_data: caller private data to be passed back in
   *              adsp_config_call
   *
   *  Set sensor output along with Single / dual vfe information
   *
   *  Optional: Pass dynamic config to ADSP
   *
   *  Return type of hvx_ret_type_t
   *
   *  Finish in <= 1 ms
   **/
  hvx_ret_type_t (*hvx_lib_set_config)(void *oem_data,
    const hvx_lib_config_t *lib_config,
    const hvx_lib_adsp_config_t *adsp_config,
    void *caller_data);

  /** hvx_lib_consume_stats:
   *
   *  @oem_data: OEM specific private data
   *  @frame_id: frame id
   *  @stats_data: stats data
   *  @adsp_config: ADSP config handle
   *  @caller_data: caller private data to be passed back in
   *              adsp_config_call
   *
   *  Consume stats filled by ADSP
   *
   *  Optional: Pass dynamic config to ADSP
   *
   *  Return type of hvx_ret_type_t
   *
   *  Finish in <= 1 ms
   **/
  hvx_ret_type_t (*hvx_lib_consume_stats)(void *oem_data,
    unsigned int frame_id, hvx_lib_stats_t *stats_data,
    const hvx_lib_adsp_config_t *adsp_config, void *caller_data);


  hvx_ret_type_t (*hvx_lib_consume_dump)(void *oem_data,
    unsigned int frame_id, hvx_lib_stats_t *stats_data,
    const hvx_lib_adsp_config_t *adsp_config, void *caller_data);
  /** hvx_lib_sof:
   *
   *  @oem_data: OEM specific private data
   *  @sof_params: AEC and AWB feedback params
   *  @adsp_config: ADSP config handle
   *  @caller_data: caller private data to be passed back in
   *              adsp_config_call
   *
   *  SOF trigger indication along with frame id
   *
   *  Optional: Pass dynamic config to ADSP
   *
   *  Return type of hvx_ret_type_t
   *
   *  Finish in <= 1 ms
   **/
  hvx_ret_type_t (*hvx_lib_sof)(void *oem_data,
    const hvx_lib_sof_params_t *sof_params,
    const hvx_lib_adsp_config_t *adsp_config,
    void *caller_data);

  hvx_ret_type_t (*hvx_lib_get_dump_buffer_size)(int output_width,
												 int output_height,
												 int right_stripe_offset,
												 int overlap,
												 int * buffer_size);


  /** hvx_lib_close:
   *
   *  @oem_data: private data specific to OEM stub
   *
   *  Free oem_data
   *
   *  Return type of hvx_ret_type_t
   *
   *  Finish in <= 1 ms
   **/
  hvx_ret_type_t (*hvx_lib_close)(void *oem_data);
} hvx_lib_function_table_t;



/* End of struct definition */

/* Start of function declaration */

/** hvx_lib_fill_function_table:
 *
 *  @func_table: [INPUT]function table handle which has place holders to
 *  get other HVX functions
 *
 *  OEM library implement this function. Call will open this function using
 *  dlsym to get other HVX function pointers
 *
 *  Return type of hvx_ret_type_t
 *
 *  Finish in <= 1 ms
 **/
hvx_ret_type_t hvx_lib_fill_function_table(
  hvx_lib_function_table_t *func_table);

/* End of function declaration */

#endif
