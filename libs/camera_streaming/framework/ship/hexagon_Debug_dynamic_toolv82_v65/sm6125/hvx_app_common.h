/***************************************************************************
 * Copyright (c) 2015 - 2016 QUALCOMM Technology INCORPORATED
 * All Rights Reserved
 ****************************************************************************/

/** 
 * @file hvx_app_common.h
 * @author Junyuan Shi
 * @date May 13th, 2016
 * @brief This file contains common data strucutures between 
 * HVX camera streaming control thread and processing thread. 
 */
#ifndef _HVX_APP_COMMON_H_
#define _HVX_APP_COMMON_H_

#include "qurt.h"
#include "adsp_hvx.h"
#include "adsp_hvx_callback.h"
#include "hvx_app_event_queue.h"

#define MAX_NUM_STREAMER 2 /**< maximum number of streamers, for each of streamer could have its own configuration */
#define STREAMER_STATUS_RX_DATA_DROP 0x00008000 /**< RX DATA DROP bit */
#define BYTES_PER_PIXEL 2 /**< default data depth in L2 rx and tx buffer */
#define L2_BUF_ALIGNMENT 128 /**< alignment requirement for HVX */
#define STREAMER_ERROR_MASK 0x4200C100 /**< streamer status register error bits */

/**
 * @brief streamer config: streamer padding size 
 *  
 * Streamer will take care of padding in RX and stipping in TX.
 * If streamer_config->rx_pad_size is set to 8, streamer will 
 * pad 8 pixels in front and end of each line before writting 
 * into RX. 
 * if streamer_config->tx_pad_size is set to 8, steamer will 
 * stripe out 8 pixels from front and end of each line when 
 * fetching from TX. 
 * if set to PAD_SIZE_0, it simply means disable padding or 
 * stripping. 
 */
enum rx_pad_size_t {
  PAD_SIZE_0 = 0, /**< disable padding or stripping */
  PAD_SIZE_8 = 8 /**< pad or strip 8 pixels */
};

/**
 * @brief streamer config: streamer padding patterns
 *  
 * User should set streamer_config->rx_pad_type to one of these 
 * if streamer_config->rx_pad_size is set to PAD_SIZE_8 
 */
enum rx_pad_type_t {
  PAD_DUP_1 = 1, /**< duplicate 1st and last pixel 8 times */
  PAD_DUP_2 = 3, /**< duplicate first 2 and last 2 pixels 4 times */
  PAD_DUP_4 = 5, /**< duplicate first 4 and last 4 pixels twice */
  PAD_0 = 6 /**< pad with 0s */
};

/**
 * @brief streamer config: TX timing mode 
 *  
 * User should set streamer_config->tx_timing_mode to one of 
 * this. 
 * TX timing mode specifies how SOF, SOL, EOL, EOF and other 
 * signals will be generated when TX is transmitting data to 
 * VFE. 
 */
enum tx_timing_mode_t {
  TX_FOLLOW_RX_MODE = 0, /**< TX signals will follow exactly RX signals with a fixed offset */
  TX_PROGRAMMED_MODE = 1 /**< TX signals should be programmed by user. 
                              if set to TX_PROGRAMMED_MODE, please explicity program
                              streamer_config->tx_pixel_cnt[i], 
                              streamer_config->tx_line_cnt[i] and 
                              streamer_config->tx_SOL_interval[i] */
};

/**
 * @brief streamer config: streamer pixel format 
 *  
 * Pixel data in L2cache will be 16bpp(bit per pixel). Streamer 
 * will take care of input data format conversion, which is 
 * CAMIF format to 16bpp, and output data format conversion, 
 * which is 16bpp to VFE format. 
 * Program streamer_config->streamer_in_format and 
 * streamer_config->streamer_out_format to one of these bit 
 * depth values.  
 */
enum streamer_data_format {
  BIT_DEPTH_8 = 0, /**< 8bpp */
  BIT_DEPTH_10 = 1, /**< 10bpp */
  BIT_DEPTH_12 = 2, /**< 12bpp */
  BIT_DEPTH_14 = 3 /**< 14bpp */
};

/**
 * @brief algo can request frame to vote ADSP to a certain level 
 *  
 * During streaming, it is possible that algorithm wants ADSP to 
 * run at a certain clk level. OEM_skel lib implementation can 
 * use event callback to do this. 
 */
enum algo_voting_level {
  ALGO_VOTING_TURBO = 0,
  ALGO_VOTING_NOM = 1,
  ALGO_VOTING_SVS = 2,
  ALGO_VOTING_SVS2 = 3
};

/**
 * @brief event handler queue element event type 
 *  
 * When constructing a callback element to enqueue into event 
 * handler queue, a specific event type is needed. Put 
 * hvx_evt_cb_elem_t -> type to one of these.  
 */
typedef enum hvx_evt_cb_type_t{
  HVX_DATA_CB_STATS_BUF_READY = 0, /**< stats buffer ready */
  HVX_DATA_CB_DUMP_BUF_READY, /**< frame dump buffer ready */
  HVX_ERROR_CB_NULLPTR, /**< null pointer exception */
  HVX_ERROR_CB_RX_DATA_DROP, /**< RX data drop error occurs */
  HVX_REQ_CLK_VOTING
}hvx_evt_cb_type_t;

/**
 * @brief buffer structure used for frame dump and stats dump or 
 *        general purpose buffer request
 *  
 * From the queue, deque a buffer of this type, use it's size 
 * and pointer. When dumping is concluded, buffer label will be 
 * used for HVX_DATA_CB_DUMP_BUF_READY or 
 * HVX_DATA_CB_STATS_BUF_READY event. 
 */
typedef struct hvx_q_buffer_t{
  unsigned char label; /**< buffer label */
  int length; /**< buffer size */
	uint8* data; /**< buffer address */
}hvx_q_buffer_t;

typedef struct hvx_queue_t{
	int capacity;
	int size;
	int front;
	int rear;
	hvx_q_buffer_t* elements;
	qurt_mutex_t queue_mutex;
}hvx_queue_t;

/**
 * @brief processing thread payload 
 *  
 * Processing thread will get this payload from control thread. 
 * It has necessary VFE information as well as communication 
 * queues. 
 */
typedef struct thread_data_str_t{
  int             thread_id; /**< current processing thread thread ID */
  int             handle; /**< current camera id */
  int             vfe_id; /**< current vfe id, left VFE: 0, right VFE: 1 */
  hvx_vfe_type_t  vfe_mode; /**< single VFE if equals to HVX_VFE0 or HVX_VFE1, dual VFE othewise */
  qurt_hvx_mode_t vector_mode; /**< HVX vector mode, QURT_HVX_MODE_64B or QURT_HVX_MODE_128B */
  hvx_queue_t*    stats_buf_queue; /**< pointer to stats buffer queue */
  hvx_queue_t*    dump_buf_queue; /**< pointer to frame dump buffer queue */
  hvx_evt_queue_t evt_queue;  /**< pointer to event handler queue */
}thread_data_str_t;

/**
 * @brief input data structure to hvx_xx_get_config function 
 *  
 * Contains VFE information as well as basic frame information 
 * User will use these information to determine # of RX lines 
 * and # of TX lines.
 */
typedef struct vfe_frame_info_t{
  unsigned int width; /**< total frame width before dual VFE split */
  unsigned int height; /**< frame height */
  hvx_vfe_type_t vfe_type; /**< single VFE if equals to HVX_VFE0 or HVX_VFE1, dual VFE othewise */
  unsigned int pixel_bit_depth; /**< obselete. this value inherites VFE configuration of bpp*/
  unsigned int available_l2_size; /**< total available L2 size, up-limit of all buffers in L2 */
  unsigned int rx_line_size[MAX_NUM_STREAMER]; /**< pre-calculated rx line size by ISP, 128byte aligned */
  unsigned int tx_line_size[MAX_NUM_STREAMER]; /**< pre-calculated tx line size by ISP 128byte aligned */
  unsigned int rx_line_width[MAX_NUM_STREAMER]; /**< # of pixels in RX after VFE split, calculated based on widht, right frame offset and overlap */
  unsigned int tx_line_width[MAX_NUM_STREAMER]; /**< # of pixels in TX after VFE split */
}vfe_frame_info_t;

/**
 * @brief output data structure from hvx_xx_get_config function 
 *  
 * Contains final L2 buffer allocation information and streamer 
 * configuration. Control thread will take these information and 
 * configure L2 and streamer HW accordingly. 
 */
typedef struct adsp_streamer_config_t{
  unsigned int rx_lines[MAX_NUM_STREAMER]; /**< number of lines in RX rotation buffer */
  unsigned int tx_lines[MAX_NUM_STREAMER]; /**< number of lines in TX rotation buffer */
  unsigned int rx_linesize[MAX_NUM_STREAMER]; /**< corrected RX line size based on padding */
  unsigned int tx_linesize[MAX_NUM_STREAMER]; /**< corrected TX line size based on padding */
  unsigned int tx_min_start[MAX_NUM_STREAMER]; /**< TX_MIN_START register value */
  unsigned int dynamic_buf_size; /**< size (in byte) of dynamic config structure */
  unsigned int rx_pad_type; /**< RX side padding pattern */
  unsigned int rx_pad_size; /**< RX side padding size */
  unsigned int tx_pad_size; /**< TX side stripping size*/
  unsigned int streamer_in_format; /**< CAMIF pixel data format (bpp) */
  unsigned int streamer_out_format; /**< VFE pixel data format (bpp) */
  unsigned int tx_timing_mode; /**< TX_TIMGING_MODE value, see enum tx_timing_mode_t */
  unsigned int tx_pixel_cnt[MAX_NUM_STREAMER]; /**< number of active pixels in TX line, in case tx_timing_mode is set to 1*/
  unsigned int tx_line_cnt[MAX_NUM_STREAMER]; /**< number of active lines in TX line, in case tx_timing_mode is set to 1*/
  unsigned int tx_SOL_interval[MAX_NUM_STREAMER]; /**< number of VFE clk between subsequent SOLs, in case tx_timing_mode is set to 1*/
}adsp_streamer_config_t;

/**
 * @brief output data structure from hvx_xx_get_config function 
 *  
 * Contains user configuration of ADSP clk and bus BW voting 
 */
typedef struct adsp_power_voting_t{
    int mips_total; /**< total ADSP clk required, in MHz */
    int mips_per_thread; /**< ADSP clk required per thread, in MHz */
    uint64_t bus_bw; /**< bus bandwidth required, in Hz */
    int usage_percentage; /**< bus useage percentage, 0~100 */
}adsp_power_voting_t;

/**
 * @brief callback element definition 
 *  
 * When processing thread is ready to send a callback event, 
 * please construct a callback element following this sturcture 
 * and enque it into event handler queue 
 */
typedef struct hvx_evt_cb_elem_t{
  hvx_evt_cb_type_t evt_type; /**< callback element type, see hvx_evt_cb_type_t */
  int handle; /**< camera id, please inherit this from processing thread payload */
  int vfe_mode; /**< vfe mode, please inherit this from processing thread payload */
  unsigned int frame_id; /**< current frame id, processing thread should bookkeeping frame id */
  union{
    int buf_label; /**< for buffer ready callback, use buf_label */
    char error_info[64]; /**< for error callback, use error message*/
    int voting_level; /**< for voting request callback, use voting level*/
  };
}hvx_evt_cb_elem_t;

/**
 * @brief processing thread internal data 
 *  
 * hvx_xx_get_config function should wirte into this structure 
 * with following infomation, hvx_xx function will later read 
 * from it, instead of getting the info from processing thread 
 * payload, to avoid extra data passing
 */
typedef struct l2_buffer_info_t{
  unsigned int height; /**< frame height */
  unsigned int width[MAX_NUM_STREAMER]; /**< width for each VFE */
  unsigned int rx_linesize[MAX_NUM_STREAMER]; /**< corrected rx line size */
  unsigned int tx_linesize[MAX_NUM_STREAMER]; /**< corrected tx line size */
  unsigned int rx_buffsize[MAX_NUM_STREAMER]; /**< corrected rx buffer size */
  unsigned int tx_buffsize[MAX_NUM_STREAMER]; /**< corrected tx buffer size */
}l2_buffer_info_t;

#endif
