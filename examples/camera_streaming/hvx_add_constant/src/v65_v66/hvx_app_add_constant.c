/***************************************************************************
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************************/

#define  FARF_ERROR 1
#define  FARF_HIGH 1
#define  FARF_LOW 1

#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "dsp_streamer_common.h"
#include "hvx_app_add_constant.h"
#include "hvx_add_constant_def.h"
#include "dsp_streamer_process_utils.h"

#define roundup_t(a, m)     (((a)+(m)-1)&(0-m))

uint32_t wrap_buffer_idx(uint32_t idx, uint32_t bufsize) {
  if (idx >= bufsize) {
    idx -= bufsize;
  }
  return idx;
}

void hvx_add_constant_get_config(int handle, ife_frame_info_t* frame_info,
                                 streamer_request_config_t* streamer_config,
                                 dsp_power_voting_t* OEM_voting){
  int i = 0;
  int num_ife;
  FARF(ALWAYS, "%s: E", __func__);
  FARF(HIGH, "%s: input width = %d, input height = %d,"
             "output width = %d, output height = %d, ife_type = %u,"
             "pixel bit depth = %u, max l2 size = %u, tapping point %d,"
             "frame pixel format %u", __func__,
       frame_info->in_width[0], frame_info->in_height, frame_info->out_width[0],
       frame_info->out_height, frame_info->ife_mode,
       frame_info->pixel_bit_depth, frame_info->available_l2_size, 
       frame_info->tapping_point, frame_info->pixel_format);

  streamer_config->dynamic_buf_size = sizeof(hvx_constant_t);
  streamer_config->rx_pad_size = PAD_SIZE_0;
  streamer_config->rx_pad_type = PAD_0;
  streamer_config->tx_pad_size = PAD_SIZE_0;
  streamer_config->streamer_in_format = BIT_DEPTH_10;
  streamer_config->streamer_out_format = BIT_DEPTH_10;

  streamer_config->tx_timing_mode = TX_FOLLOW_RX_MODE;
  num_ife = (frame_info->ife_mode == HVX_IFE_BOTH)? 2 : 1;
  for ( i = 0; i < num_ife; i++ ) {
    streamer_config->rx_lines[i] = INPUT_BUF_LINES;
    streamer_config->tx_lines[i] = OUTPUT_BUF_LINES;
    streamer_config->tx_min_start[i] = OUTPUT_START_DELAY;

    /*** calculate rx and tx linesize based on width and padding ***/
    streamer_config->rx_linesize[i] = roundup_t(frame_info->in_width[i]* BYTES_PER_PIXEL + 2 * streamer_config->rx_pad_size * BYTES_PER_PIXEL, 128);
    
    /*** configuration of TX timing mode ***/
    if (streamer_config->tx_timing_mode == TX_PROGRAMMED_MODE) {
      /*** in case of TX PROGRAMMED MODE, all these 4 parameters needs to be reprogrammed! ***/
      streamer_config->tx_linesize[i] = roundup_t(frame_info->out_width[i]* BYTES_PER_PIXEL + 2 * streamer_config->tx_pad_size * BYTES_PER_PIXEL, 128);
      streamer_config->tx_pixel_cnt[i] = frame_info->out_width[i];
      streamer_config->tx_line_cnt[i] = frame_info->out_height;
      streamer_config->tx_SOL_interval[i] = 3212;
      FARF(HIGH, "%s: for streamer %d, TX_PROGRAMMED_MODE enabled", __func__,
        i);

    } else {
      streamer_config->tx_linesize[i] = streamer_config->rx_linesize[i];
    }
    FARF(HIGH, "%s: for streamer %d, rx_linesize is %d, tx_linesize is %d",
         __func__, i, streamer_config->rx_linesize[i], streamer_config->tx_linesize[i]);

  }

  if (frame_info->ife_mode == HVX_IFE_BOTH) {
    // in case of dual vfe, vote for NOM ADSP clk
    OEM_voting->mips_total = 600;
    OEM_voting->mips_per_thread = 150;
  } else {
    // in case of single vfe, vote for TURBO ADSP clk
    OEM_voting->mips_total = 900;
    OEM_voting->mips_per_thread = 300;
  }
  
  OEM_voting->bus_bw = 4800000000;
  OEM_voting->usage_percentage = 50;
  FARF(ALWAYS, "%s: X", __func__);
  return;
}

/* main processing function */
void hvx_add_constant(void *p)
{
  FARF(ALWAYS, "%s: E", __func__);
  /*** extract parameters from payload ***/
  thread_data_str_t *payload = (thread_data_str_t*)p;
  int thread_id = payload->thread_id;
  /** current streaming session identifier **/
  int sess = payload->handle;
  /** start of RX/TX buffer in L2 **/
  uint8_t* inbuf = payload->inbuf;
  uint8_t* outbuf = payload->outbuf;

  qurt_mutex_t* mutex_force_exit = payload->mutex_force_exit;

  /** algorithm per-frame update parameter buffer **/
  hvx_constant_t* hvx_constant = (hvx_constant_t*)payload->metabuf;
  FARF(LOW, "%s:%d: rx addr: %d, tx addr: %d, metabuf: %d",
       __func__, thread_id, inbuf, outbuf, hvx_constant);
  /**
   * RX and TX buffer size 
   * these values are whatever got calculated in get_config 
   * function 
   * in dual IFE case, two processing thread will get separate buf 
   * size based on thread_id. 
   * in single IFE case, thread_id will be 0 
   */
  uint32_t rx_linesize = payload->streamer_request_config.rx_linesize[thread_id];
  uint32_t tx_linesize = payload->streamer_request_config.tx_linesize[thread_id];
  uint32_t rx_bufsize = payload->streamer_request_config.rx_linesize[thread_id] * payload->streamer_request_config.rx_lines[thread_id];
  uint32_t tx_bufsize = payload->streamer_request_config.tx_linesize[thread_id] * payload->streamer_request_config.tx_lines[thread_id];

  FARF(LOW,  "%s:%d: rx_linesize = %d, tx_linesize = %d", __func__, thread_id, rx_linesize, tx_linesize);
  FARF(LOW,  "%s:%d: rx_bufsize = %d, tx_bufsize = %d", __func__, thread_id, rx_bufsize, tx_bufsize);
  /** frame layout **/
  uint32_t width = payload->frame_info.in_width[thread_id];
  uint32_t height = payload->frame_info.in_height;
  FARF(LOW,  "%s:%d: width = %d, height = %d", __func__, thread_id, width, height);

  /** hvx mode to be locked, 128byte mode by default **/
  qurt_hvx_mode_t vector_mode = payload->vector_mode;
  uint8_t shift_num = 7; // = 7 for 128 byte vector mode


  /*** algorithm execution utilities ***/
  /** loop control **/
  uint32_t frame_cnt = 0;
  uint32_t linecount = 0;
  uint32_t processed_linecount = 0;
  uint32_t runahead_linecount;
  uint32_t in_idx = 0;
  uint32_t out_idx = 0;

  /** flags **/
  uint32_t start_flag;
  uint32_t force_exit_flag;
  uint32_t frame_dump_flag;
  uint32_t rc;
  uint32_t raw_status;
  uint32_t overflow_recovery = 0;
  uint32_t status;

  /** buffer dump **/
  // disable buffer dump for now

  /*** profiling utilities ***/

  /*** tx programmed timing mode utilities ***/
  // tx timing mode is whatever was set in get_config function
  uint32_t tx_timing_mode = payload->streamer_request_config.tx_timing_mode;
  uint32_t RX_SOL_interval;
  
  // update dynamic config buffer content at init step
  process_util_update_metabuf(sess, thread_id);

  // wait for start signal
  process_util_get_start_flag(sess, thread_id, &start_flag);
  while (!start_flag) {
    process_util_get_start_flag(sess, thread_id, &start_flag);
  }

  qurt_mutex_lock(mutex_force_exit);
  // check for exit flag before processing starts
  process_util_get_force_exit_flag(sess, thread_id, &force_exit_flag);
  qurt_mutex_unlock(mutex_force_exit);
  if (force_exit_flag) {
    FARF(ERROR, "%s:%d: hit force exit flag before processing, exiting", __func__, thread_id);
    return;
  }

  rc = qurt_hvx_lock(vector_mode);
  if (rc) {
    FARF(ERROR, "%s:%d: hvx unit lock failure, mode %d", __func__, thread_id, vector_mode);
    return;
  }

  runahead_linecount = 4;
  /* We want to wait for four lines to come in, so start with > 4-1 = 3 */
  linecount = runahead_linecount - 1;
  /* We want to compare received lines to processed lines, but we have two rows of boundary. */
  processed_linecount = 0;
  /* We should be waiting for 4 lines, then processing 2, then waiting
   * for pairs of lines and processing them in pairs.
   * This does almost that.  It waits for 4 lines, and processes them,
   * but starts TX right after the first batch is done
   */
  status = process_util_get_streamer_status(sess, thread_id);
  raw_status = process_util_get_streamer_raw_status(sess, thread_id);
  //process_util_set_status_mask(vfe_id);
  while (1) {
    linecount = runahead_linecount - 1;
    /* We want to compare received lines to processed lines, but we have two rows of boundary. */
    processed_linecount = 0;
    //wait for rx sof
    while (!process_util_check_rx_sof(sess, thread_id)){
      qurt_mutex_lock(mutex_force_exit);
      process_util_get_force_exit_flag(sess, thread_id, &force_exit_flag);
      qurt_mutex_unlock(mutex_force_exit);
      if ( force_exit_flag ) {
        FARF(HIGH,  "%s:%d: hit force exit flag in loop %d", __func__, thread_id, force_exit_flag);
        break;
      }
    }
    if ( force_exit_flag ) {
      break;
    }
    FARF(HIGH, "%s:%d:sof in loop %d \n", __func__, thread_id, force_exit_flag);
    // counting frame
    frame_cnt++;
    
    // at SOF, program tx SOL interval if tx is in programmed mode
    if (tx_timing_mode == TX_PROGRAMMED_MODE) {
      while (process_util_get_rx_lines(sess, thread_id) < 2); 
      RX_SOL_interval = process_util_get_rx_SOL_interval(sess, thread_id);
      process_util_set_TX_SOL_interval(sess, thread_id, RX_SOL_interval + 4);
    }

    // at SOF, read streamer status and determine error status
    raw_status = process_util_get_streamer_raw_status(sess, thread_id);
    // FARF(HIGH, "hvx_add_constant%d RAW STATUS after reset SOF: %x, frame %d", vfe_id, raw_status, frame_cnt);
    if ( raw_status & STREAMER_ERROR_MASK ) {
      FARF(HIGH, "%s:%d RAW STATUS: %x, frame %d mask %x", __func__, thread_id, raw_status, frame_cnt, STREAMER_ERROR_MASK);
    }
    if (raw_status & STREAMER_STATUS_RX_DATA_DROP ) {
      overflow_recovery = 1;
    }
    // at SOF, update metabuf to apply the latest per-frame parameters
    if (process_util_check_reg_update(sess, thread_id)) {
      process_util_update_metabuf(sess, thread_id);
    }

    // at SOF, check frame dump flag
    process_util_get_dump_flag(sess, thread_id, &frame_dump_flag);
//    FARF(HIGH, "%s:%d: dump flag %d, frame %d", __func__, thread_id, frame_dump_flag, frame_cnt);
    if ( frame_dump_flag  && ( (frame_cnt % 10) == 0) && ( overflow_recovery == 0 ) ) {
      // disable frame dump for now
      frame_dump_flag = 0;
    } else {
      frame_dump_flag = 0;
    }
    // at SOF, reset rx sof for next SOF check
    process_util_reset_rx_sof(sess, thread_id);
    
    //FARF(LOW, "%s%d constant %d", __func__, thread_id, hvx_constant->constant);
    // hard code constant for now
    //hvx_constant->constant = 25;
    while (linecount < height) {
      linecount = process_util_rx_wait_for_line(sess, thread_id,(linecount | 1));
      do {
         hvx_add_constant_asm(outbuf + out_idx,
                                 inbuf + in_idx,
                                 tx_linesize,
                                 hvx_constant->constant,
                                 shift_num);
          if (frame_dump_flag) {
            //memcpy(dump_buffer_addr + rx_linesize * processed_linecount, outbuf + out_idx, rx_linesize);
          }
          in_idx  = wrap_buffer_idx(in_idx  + rx_linesize, rx_bufsize);
          out_idx = wrap_buffer_idx(out_idx + tx_linesize, tx_bufsize);
          process_util_rx_done(sess, thread_id, 64*(int)(in_idx/64));
          process_util_tx_done(sess, thread_id, 64*(int)(out_idx/64));
          processed_linecount += 1;
      } while (linecount > processed_linecount);
    }
    process_util_tx_done(sess, thread_id, out_idx);
    process_util_tx_wait_for_eof(sess, thread_id);
    process_util_tx_clear_eof(sess, thread_id);
    if (overflow_recovery) {
      FARF(HIGH," %s: overflow!!! recover in process", __func__);
      process_util_recover_streamer(sess, thread_id);
      out_idx = 0;
      in_idx = 0;
      overflow_recovery = 0;
      frame_cnt = 0;
    }
    if ( frame_dump_flag ) {
      FARF(HIGH,  "%s:%d: enqueue data dump at frame %d", __func__, thread_id, frame_cnt);
  	}
    qurt_mutex_lock(mutex_force_exit);
    //check if need to exit
    rc = process_util_get_force_exit_flag(sess, thread_id, &force_exit_flag);
    qurt_mutex_unlock(mutex_force_exit);
    if ( rc | force_exit_flag ) {
        FARF(ALWAYS,  "%s:%d: hit force exit flag in loop", __func__, thread_id);
        break;
    }
  }
  FARF(ALWAYS, "%s: unlock HVX", __func__);
  rc = qurt_hvx_unlock();
  FARF(ALWAYS, "%s:%d: X", __func__, thread_id);
  return;
}
