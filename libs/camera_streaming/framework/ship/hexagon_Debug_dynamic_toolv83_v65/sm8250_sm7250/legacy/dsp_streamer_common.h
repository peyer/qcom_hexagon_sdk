/***************************************************************************
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 ****************************************************************************/
/***************************************************************************
 * Common definitions to be used by both the user APP and  DSP streamer
 * framework.
 ****************************************************************************/
#ifndef _DSP_STREAMER_COMMON_H_
#define _DSP_STREAMER_COMMON_H_

#include "qurt.h"
#include "dsp_streamer.h"

#define MAX_NUM_STREAMER 2 /**< maximum number of streamers, for each of
                                streamer could have its own configuration */
#define BYTES_PER_PIXEL 2 /**< default data depth in L2 rx and tx buffer */
#define L2_BUF_ALIGNMENT 128 /**< alignment requirement for HVX */
#ifdef HANA_STREAMER_FW
#define STREAMER_STATUS_RX_DATA_DROP 0x00008000 /**< RX DATA DROP bit */
#define STREAMER_ERROR_MASK 0x4100CB00 /**< streamer status register error bits */
#else
#define STREAMER_STATUS_RX_DATA_DROP 0x00000008 /**< RX DATA DROP bit */
#define STREAMER_ERROR_MASK 0xff031f /**< streamer status register error bits */
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// sync_flag_shift_t
///
/// @brief  Bit-shifts used to set sync_flag.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
    SYNC_FLAG_SHIFT_START = 0,
    SYNC_FLAG_SHIFT_FORCE_EXIT,
    SYNC_FLAG_SHIFT_BUF_DUMP,
    SYNC_FLAG_SHIFT_STATS,
///////////////////////////////////////
    SYNC_FLAG_SHIFT_NUM
}sync_flag_shift_t;

typedef enum tx_timing_mode_t {
    TX_FOLLOW_RX_MODE = 0, /**< TX signals will follow exactly RX signals with a
                                fixed offset */
    TX_PROGRAMMED_MODE = 1, /**< TX signals should be programmed by user.
                                if set to TX_PROGRAMMED_MODE,
                                please explicity program
                                streamer_config->tx_pixel_cnt[i],
                                streamer_config->tx_line_cnt[i] and
                                streamer_config->tx_SOL_interval[i] */
    RX_ONLY_MODE = 2, /** 1way streaming, no TX mode needed **/
    INT_LOOPBACK_DEBUG = 3, /** Debug only HW mode**/
    IO_LOOPBACK_DEBUG = 4 /** Debug only HW mode**/
}tx_timing_mode_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// streamer_request_config_t
///
/// @brief  Streamer request configuration.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct streamer_request_config_t{
    unsigned int rx_lines; /**< number of lines in RX rotation
                                                  buffer */
    unsigned int tx_lines; /**< number of lines in TX rotation
                                                  buffer */
    unsigned int rx_linesize; /**< corrected RX line size based
                                                     on padding */
    unsigned int tx_linesize; /**< corrected TX line size based
                                                      on padding */
    unsigned int tx_min_start; /**< TX_MIN_START register
                                                      value */
    unsigned int dynamic_buf_size; /**< size (in byte) of dynamic config
                                        structure */
    unsigned int rx_pad_type; /**< RX side padding pattern */
    unsigned int rx_pad_size; /**< RX side padding size */
    unsigned int tx_pad_size; /**< TX side stripping size*/
    unsigned int streamer_in_format; /**< CAMIF pixel data format (bpp) */
    unsigned int streamer_out_format; /**< VFE pixel data format (bpp) */
    unsigned int tx_timing_mode; /**< TX_TIMGING_MODE value, see enum tx_timing_mode_t */
    unsigned int tx_pixel_cnt; /**< number of active pixels in
                                                      TX line, in case
                                                      tx_timing_mode is set to 1*/
    unsigned int tx_line_cnt;           /**< number of active lines in TX
                                                     line, in case tx_timing_mode
                                                     is set to 1*/
    unsigned int tx_SOL_interval;       /**< number of VFE clk between
                                                         subsequent SOLs, in case
                                                         tx_timing_mode is set to
                                                          1*/
    unsigned int thread_stack_size;         /**< Size of the thread app stack. min=2048 */
    unsigned int rx_first_fetch_numlines;   /**< Number of lines to fetch before calling the
                                              algorithm processing function the first time. */
    unsigned int rx_fetch_numlines;            /**< Number of lines to fetch before calling the
                                              algorithm processing function after the first time. */

    unsigned int tx_first_fetch_numlines;     /**< Reserved For future development. */
    unsigned int tx_fetch_numlines;            /**< Number of lines algo need to write to tx buffer. */
// new for Turing Streamer
    unsigned int rx_l2_pack_msb_aligned;    /** data store in L2$ pack, 0->lsb aligned, 1->msb aligned,*/
    unsigned int rx_l2_format;               /** data store in L2$ format, 0->8bit, 1->16bit,  */
    unsigned int tx_l2_pack_msb_aligned;    /** data store in L2$ pack, 0->lsb aligned, 1->msb aligned,*/
    unsigned int tx_l2_format;               /** data store in L2$ format, 0->8bit, 1->16bit,  */
}streamer_request_config_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ife_frame_info_t
///
/// @brief  Frame information.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ife_frame_info_t{
    unsigned int   in_width;        /**<input width for this streamer*/
    unsigned int   in_height;       /**< frame height */
    unsigned int   out_width;       /**<output width for this streamer*/
    unsigned int   out_height;      /**< frame height */
    unsigned int   pixel_bit_depth; /**< This value inherits VFE configuration of bpp*/
    hvx_pixel_format_t pixel_format;
}ife_frame_info_t;

typedef enum rx_pad_size_t {
    PAD_SIZE_0 = 0, /**< disable padding or stripping */
    PAD_SIZE_8 = 8, /**< pad or strip 8 pixels */
    PAD_SIZE_SOL_ONLY = 0x10, /**< pad or strip 8 on SOL onlypixels NOTE: this only apply to new Turing streamer*/
    PAD_SIZE_EOL_ONLY = 0x20 /**< pad or strip 8 pixels on EOL only NOTE: this only apply to new Turing streamer*/
}rx_pad_size_t;

typedef enum rx_pad_type_t {
    PAD_DUP_1 = 1, /**< duplicate 1st and last pixel 8 times */
    PAD_DUP_2 = 3, /**< duplicate first 2 and last 2 pixels 4 times */
    PAD_DUP_4 = 5, /**< duplicate first 4 and last 4 pixels twice */
    PAD_0 = 6 /**< pad with 0s */
}rx_pad_type_t;

typedef enum streamer_data_format {
    BIT_DEPTH_8 = 0,    /**< 8bpp */
    BIT_DEPTH_10 = 1,   /**< 10bpp */
    BIT_DEPTH_12 = 2,   /**< 12bpp */
    BIT_DEPTH_14 = 3,   /**< 14bpp */
	BIT_DEPTH_16 = 4,	/**< 16bpp Reserve for future use*/
    BIT_DEPTH_20 = 6    /**< 20bpp Reserve for future use */
}streamer_data_format;

typedef enum streamer_L2_format { // stored data into L2 format
    BIT_L2_DEPTH_8 = 0,     /**< 8bpp */
    BIT_L2_DEPTH_16 = 1,    /**< 16bpp */
    BIT_L2_DEPTH_32 = 2     /**< 32bpp */
}streamer_L2_format;

typedef enum algo_voting_level {
    ALGO_VOTING_TURBO = 0,
    ALGO_VOTING_NOM = 1,
    ALGO_VOTING_SVS = 2,
    ALGO_VOTING_SVS2 = 3
}algo_voting_level;

typedef enum hvx_evt_cb_type_t {
    HVX_DATA_CB_STATS_BUF_READY = 0, /**< stats buffer ready */
    HVX_DATA_CB_DUMP_BUF_READY,     /**< frame dump buffer ready */
    HVX_ERROR_CB_NULLPTR,           /**< null pointer exception */
    HVX_ERROR_CB_RX_DATA_DROP,      /**< RX data drop error occurs */
    HVX_REQ_CLK_VOTING
} hvx_evt_cb_type_t;

typedef struct dsp_power_voting_t{
    int mips_total;             /**< total ADSP clk required, in MHz */
    int mips_per_thread;        /**< ADSP clk required per thread, in MHz */
    uint64_t bus_bw;            /**< bus bandwidth required, in Hz */
    int usage_percentage;       /**< bus useage percentage, 0~100 */
}dsp_power_voting_t;

#endif
