#ifndef _DSP_STREAMER_COMMON_H_
#define _DSP_STREAMER_COMMON_H_

#include "qurt.h"
#include "dsp_streamer.h"

#define MAX_NUM_STREAMER 2 /**< maximum number of streamers, for each of
                                streamer could have its own configuration */
#define STREAMER_STATUS_RX_DATA_DROP 0x00008000 /**< RX DATA DROP bit */
#define BYTES_PER_PIXEL 2 /**< default data depth in L2 rx and tx buffer */
#define L2_BUF_ALIGNMENT 128 /**< alignment requirement for HVX */
#define STREAMER_ERROR_MASK 0x4100CB00 /**< streamer status register error
                                            bits */


enum dsp_streamer_state_t {
  DSP_STREAMER_OPEN,
  DSP_STREAMER_RESET,
  DSP_STREAMER_INIT,
  DSP_STREAMER_START,
  DSP_STREAMER_STOP,
  DSP_STREAMER_CLOSE
};

enum tx_timing_mode_t {
  TX_FOLLOW_RX_MODE = 0, /**< TX signals will follow exactly RX signals with a
                              fixed offset */
  TX_PROGRAMMED_MODE = 1 /**< TX signals should be programmed by user. 
                              if set to TX_PROGRAMMED_MODE,
                              please explicity program
                              streamer_config->tx_pixel_cnt[i], 
                              streamer_config->tx_line_cnt[i] and 
                              streamer_config->tx_SOL_interval[i] */
};

typedef struct streamer_request_config_t{
  unsigned int rx_lines[MAX_NUM_STREAMER]; /**< number of lines in RX rotation
                                                buffer */
  unsigned int tx_lines[MAX_NUM_STREAMER]; /**< number of lines in TX rotation
                                                buffer */
  unsigned int rx_linesize[MAX_NUM_STREAMER]; /**< corrected RX line size based
                                                   on padding */
  unsigned int tx_linesize[MAX_NUM_STREAMER]; /**< corrected TX line size based
                                                    on padding */
  unsigned int tx_min_start[MAX_NUM_STREAMER]; /**< TX_MIN_START register
                                                    value */
  unsigned int dynamic_buf_size; /**< size (in byte) of dynamic config
                                      structure */
  unsigned int rx_pad_type; /**< RX side padding pattern */
  unsigned int rx_pad_size; /**< RX side padding size */
  unsigned int tx_pad_size; /**< TX side stripping size*/
  unsigned int streamer_in_format; /**< CAMIF pixel data format (bpp) */
  unsigned int streamer_out_format; /**< VFE pixel data format (bpp) */
  unsigned int tx_timing_mode; /**< TX_TIMGING_MODE value, see enum
                                    tx_timing_mode_t */
  unsigned int tx_pixel_cnt[MAX_NUM_STREAMER]; /**< number of active pixels in
                                                    TX line, in case
                                                    tx_timing_mode is set to 1*/
  unsigned int tx_line_cnt[MAX_NUM_STREAMER]; /**< number of active lines in TX
                                                   line, in case tx_timing_mode
                                                   is set to 1*/
  unsigned int tx_SOL_interval[MAX_NUM_STREAMER]; /**< number of VFE clk between
                                                       subsequent SOLs, in case
                                                       tx_timing_mode is set to
                                                        1*/
}streamer_request_config_t;

typedef struct ife_frame_info_t{
  unsigned int   in_width[MAX_NUM_STREAMER]; /**<input width per streamer*/
  unsigned int   in_height; /**< frame height */
  unsigned int   out_width[MAX_NUM_STREAMER];/**<output width per streamer*/
  unsigned int   out_height; /**< frame height */
  hvx_IFE_mode_t ife_mode; /**< single VFE if equals to HVX_VFE0 or HVX_VFE1,
  dual VFE othewise */
  unsigned int   pixel_bit_depth; /**< obselete. this value inherites VFE
  configuration of bpp*/
  unsigned int   available_l2_size; /**< total available L2 size, up-limit of
  all buffers in L2 */
  tapping_point_select_t tapping_point;
  hvx_pixel_format_t pixel_format;
}ife_frame_info_t;

//#endif
// TODO: Header below this line is Yet to be finalized

enum rx_pad_size_t {
  PAD_SIZE_0 = 0, /**< disable padding or stripping */
  PAD_SIZE_8 = 8 /**< pad or strip 8 pixels */
};

enum rx_pad_type_t {
  PAD_DUP_1 = 1, /**< duplicate 1st and last pixel 8 times */
  PAD_DUP_2 = 3, /**< duplicate first 2 and last 2 pixels 4 times */
  PAD_DUP_4 = 5, /**< duplicate first 4 and last 4 pixels twice */
  PAD_0 = 6 /**< pad with 0s */
};

enum streamer_data_format {
  BIT_DEPTH_8 = 0, /**< 8bpp */
  BIT_DEPTH_10 = 1, /**< 10bpp */
  BIT_DEPTH_12 = 2, /**< 12bpp */
  BIT_DEPTH_14 = 3 /**< 14bpp */
};

enum algo_voting_level {
  ALGO_VOTING_TURBO = 0,
  ALGO_VOTING_NOM = 1,
  ALGO_VOTING_SVS = 2,
  ALGO_VOTING_SVS2 = 3
};

typedef enum hvx_evt_cb_type_t{
  HVX_DATA_CB_STATS_BUF_READY = 0, /**< stats buffer ready */
  HVX_DATA_CB_DUMP_BUF_READY, /**< frame dump buffer ready */
  HVX_ERROR_CB_NULLPTR, /**< null pointer exception */
  HVX_ERROR_CB_RX_DATA_DROP, /**< RX data drop error occurs */
  HVX_REQ_CLK_VOTING
}hvx_evt_cb_type_t;

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

typedef struct thread_data_str_t{
  int             thread_id; /**< current processing thread thread ID */
  int             handle; /**< current session identifier */
  int             ife_id; /**< current vfe id, left VFE: 0, right VFE: 1 */
  hvx_IFE_mode_t  ife_mode; /**< single VFE if equals to HVX_IFE0 or HVX_IFE1,
                                 dual IFE othewise */
  unsigned int    vfe_clk; /**< current VFE clk frequency */
  qurt_hvx_mode_t vector_mode; /**< HVX vector mode, QURT_HVX_MODE_64B or
                                    QURT_HVX_MODE_128B */
  hvx_queue_t*    stats_buf_queue; /**< pointer to stats buffer queue */
  hvx_queue_t*    dump_buf_queue; /**< pointer to frame dump buffer queue */
  //hvx_evt_queue_t evt_queue;  /**< pointer to event handler queue */
  ife_frame_info_t frame_info;
  streamer_request_config_t streamer_request_config;
  qurt_mutex_t*  mutex_force_exit;
  unsigned char* stack_mem;
  unsigned char* inbuf;
  unsigned char* outbuf;
  unsigned char* metabuf;
}thread_data_str_t;


typedef struct dsp_power_voting_t{
    int mips_total; /**< total ADSP clk required, in MHz */
    int mips_per_thread; /**< ADSP clk required per thread, in MHz */
    uint64_t bus_bw; /**< bus bandwidth required, in Hz */
    int usage_percentage; /**< bus useage percentage, 0~100 */
}dsp_power_voting_t;

typedef struct hvx_evt_cb_elem_t{
  hvx_evt_cb_type_t evt_type; /**< callback element type,
                                   see hvx_evt_cb_type_t */
  int handle; /**< camera id, please inherit this from processing thread
                   payload */
  int vfe_mode; /**< vfe mode, please inherit this from processing thread
                     payload */
  unsigned int frame_id; /**< current frame id, processing thread should
                              bookkeeping frame id */
  union{
    int buf_label; /**< for buffer ready callback, use buf_label */
    char error_info[64]; /**< for error callback, use error message*/
    int voting_level; /**< for voting request callback, use voting level*/
  };
}hvx_evt_cb_elem_t;

typedef struct l2_buffer_info_t{
  unsigned int height; /**< frame height */
  unsigned int width[MAX_NUM_STREAMER]; /**< width for each VFE */
  unsigned int rx_linesize[MAX_NUM_STREAMER]; /**< corrected rx line size */
  unsigned int tx_linesize[MAX_NUM_STREAMER]; /**< corrected tx line size */
  unsigned int rx_buffsize[MAX_NUM_STREAMER]; /**< corrected rx buffer size */
  unsigned int tx_buffsize[MAX_NUM_STREAMER]; /**< corrected tx buffer size */
}l2_buffer_info_t;
#endif
