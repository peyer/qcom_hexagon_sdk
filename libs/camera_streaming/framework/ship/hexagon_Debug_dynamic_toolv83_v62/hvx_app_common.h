/***************************************************************************
 * Copyright (c) 2015 QUALCOMM Technology INCORPORATED
 * All Rights Reserved
 ****************************************************************************/
#ifndef _HVX_APP_COMMON_H_
#define _HVX_APP_COMMON_H_

#include "qurt.h"
#include "adsp_hvx.h"
#include "adsp_hvx_callback.h"
#include "hvx_app_event_queue.h"

#define MAX_NUM_STREAMER 2
#define STREAMER_STATUS_RX_DATA_DROP 0x00008000
#define BYTES_PER_PIXEL 2
#define L2_BUF_ALIGNMENT 128
#define STREAMER_ERROR_MASK 0x4200C100
enum rx_pad_type_t {
  PAD_DUP_1 = 1,
  PAD_DUP_2 = 3,
  PAD_DUP_4 = 5,
  PAD_0 = 6
};

enum rx_pad_size_t {
  PAD_SIZE_0 = 0,
  PAD_SIZE_8 = 8
};

enum tx_timing_mode_t {
  TX_FOLLOW_RX_MODE = 0,
  TX_PROGRAMMED_MODE = 1
};

enum streamer_data_format {
  BIT_DEPTH_8 = 0,
  BIT_DEPTH_10 = 1,
  BIT_DEPTH_12 = 2,
  BIT_DEPTH_14 = 3
};

typedef enum hvx_evt_cb_type_t{
  HVX_DATA_CB_STATS_BUF_READY = 0,
  HVX_DATA_CB_DUMP_BUF_READY,
  HVX_ERROR_CB_NULLPTR,
  HVX_ERROR_CB_RX_DATA_DROP
}hvx_evt_cb_type_t;

typedef struct hvx_q_buffer_t{
  unsigned char label;
  int length;
	uint8* data;
}hvx_q_buffer_t;

typedef struct hvx_queue_t{
	int capacity;
	int size;
	int front;
	int rear;
	hvx_q_buffer_t* elements;
	qurt_mutex_t queue_mutex;
}hvx_queue_t;

typedef struct thread_data_str_t{
  int             thread_id;
  int             handle;
  int             vfe_id;
  hvx_vfe_type_t  vfe_mode;
  qurt_hvx_mode_t vector_mode;
  hvx_queue_t*    stats_buf_queue;
  hvx_queue_t*    dump_buf_queue;
  hvx_evt_queue_t evt_queue;
}thread_data_str_t;

typedef struct vfe_frame_info_t{
  unsigned int width;
  unsigned int height;
  hvx_vfe_type_t vfe_type;
  unsigned int pixel_bit_depth;
  unsigned int available_l2_size;
  unsigned int rx_line_size[MAX_NUM_STREAMER];
  unsigned int tx_line_size[MAX_NUM_STREAMER];
  unsigned int rx_line_width[MAX_NUM_STREAMER];
  unsigned int tx_line_width[MAX_NUM_STREAMER];
}vfe_frame_info_t;

typedef struct adsp_streamer_config_t{
  unsigned int rx_lines[MAX_NUM_STREAMER];
  unsigned int tx_lines[MAX_NUM_STREAMER];
  unsigned int rx_linesize[MAX_NUM_STREAMER];
  unsigned int tx_linesize[MAX_NUM_STREAMER];
  unsigned int tx_min_start[MAX_NUM_STREAMER];
  unsigned int dynamic_buf_size;
  unsigned int rx_pad_type;
  unsigned int rx_pad_size;
  unsigned int tx_pad_size;
  unsigned int streamer_in_format;
  unsigned int streamer_out_format;
  unsigned int tx_timing_mode;
  unsigned int tx_pixel_cnt[MAX_NUM_STREAMER];
  unsigned int tx_line_cnt[MAX_NUM_STREAMER];
  unsigned int tx_SOL_interval[MAX_NUM_STREAMER];
}adsp_streamer_config_t;

typedef struct adsp_power_voting_t{
    int mips_total; //in MHz
    int mips_per_thread; //in MHz
    int bus_bw; //in Hz
    int usage_percentage; //percentage number
}adsp_power_voting_t;

typedef struct hvx_evt_cb_elem_t{
  hvx_evt_cb_type_t evt_type;
  int handle;
  int vfe_mode;
  unsigned int frame_id;
  //for data callback, use buf_label
  //for error callback, use error_info
  union{
    int buf_label;
    char error_info[64];
  };
}hvx_evt_cb_elem_t;

typedef struct l2_buffer_info_t{
  unsigned int height;
  unsigned int width[MAX_NUM_STREAMER];
  unsigned int rx_linesize[MAX_NUM_STREAMER];
  unsigned int tx_linesize[MAX_NUM_STREAMER];
  unsigned int rx_buffsize[MAX_NUM_STREAMER];
  unsigned int tx_buffsize[MAX_NUM_STREAMER];
}l2_buffer_info_t;

void hvx_queue_status(hvx_queue_t *q);
hvx_q_buffer_t * hvx_queue_deque(hvx_queue_t * p_q);

#endif
