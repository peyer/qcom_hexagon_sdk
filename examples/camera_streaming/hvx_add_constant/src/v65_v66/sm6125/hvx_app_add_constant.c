/***************************************************************************
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************************/

#define  FARF_ERROR 1
#define  FARF_HIGH 0
#define  FARF_LOW 0

#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "hvx_app_common.h"
#include "hvx_app_add_constant.h"
#include "hvx_add_constant_def.h"
#include "hvx_app_process_utils.h"

#define roundup_t(a, m)     (((a)+(m)-1)&(0-m))

l2_buffer_info_t l2_buffer_info;

void enqueue_data_buffer(hvx_evt_cb_type_t type,
                         int handle,
                         int vfe_mode,
                         int buffer_label,
                         hvx_evt_queue_t evt_queue)
{
    hvx_evt_cb_elem_t  * cb_elem;
    cb_elem = (hvx_evt_cb_elem_t*)qurt_malloc(sizeof(hvx_evt_cb_elem_t));
    if ( cb_elem == NULL ) {
      FARF(ERROR, "enqueue_data_buffer malloc err.");
      return;
    }
  	cb_elem->evt_type = type;
  	cb_elem->handle = handle;
  	cb_elem->vfe_mode = vfe_mode;
  	cb_elem->buf_label = buffer_label;
  	hvx_event_queue_enqueue(evt_queue, (void**)&cb_elem, 1);
    return;
}
void hvx_add_constant_get_config(vfe_frame_info_t* frame_info,
                                 adsp_streamer_config_t* streamer_config,
                                 adsp_power_voting_t* OEM_voting){
  int i = 0;
  FARF(ALWAYS, "hvx_add_constant_get_config: E");
  FARF(HIGH, "get vfe configurations: width = %u, height = %u,"
               "vfe_type = %u, pixel bit depth = %u, l2 size = %u,"
               "rx_stride = %u & %u, tx_stride = %u & %u",
       frame_info->width, frame_info->height, frame_info->vfe_type,
       frame_info->pixel_bit_depth, frame_info->available_l2_size,
       frame_info->rx_line_size[0], frame_info->rx_line_size[1],
       frame_info->tx_line_size[0], frame_info->tx_line_size[1]);

  streamer_config->dynamic_buf_size = sizeof(hvx_constant_t);
  streamer_config->rx_pad_size = PAD_SIZE_0;
  streamer_config->rx_pad_type = PAD_0;
  streamer_config->tx_pad_size = PAD_SIZE_0;
  streamer_config->streamer_in_format = BIT_DEPTH_10;
  streamer_config->streamer_out_format = BIT_DEPTH_10;

  l2_buffer_info.height = frame_info->height;
  streamer_config->tx_timing_mode = TX_FOLLOW_RX_MODE;
  for ( i = 0; i < MAX_NUM_STREAMER; i++ ) {
    streamer_config->rx_lines[i] = INPUT_BUF_LINES;
    streamer_config->tx_lines[i] = OUTPUT_BUF_LINES;
    streamer_config->tx_min_start[i] = OUTPUT_START_DELAY;
    l2_buffer_info.width[i] = frame_info->rx_line_width[i];
    //after padding, if the line stride is greater than previous stride
    // align another 128 byte
    if (frame_info->rx_line_width[i] * BYTES_PER_PIXEL
      + 2 * PAD_SIZE_0 * BYTES_PER_PIXEL > frame_info->rx_line_size[i]){
      l2_buffer_info.rx_linesize[i] = frame_info->rx_line_size[i] + L2_BUF_ALIGNMENT;
    } else {
      l2_buffer_info.rx_linesize[i] = frame_info->rx_line_size[i];
    }
    if (streamer_config->tx_timing_mode == TX_PROGRAMMED_MODE) {
      /*
      in case of TX PROGRAMMED MODE, all these 4 parameters needs to be reprogrammed!
      */
      l2_buffer_info.tx_linesize[i] = roundup_t(l2_buffer_info.rx_linesize[i], 128);
      streamer_config->tx_pixel_cnt[i] = frame_info->rx_line_width[i];
      streamer_config->tx_line_cnt[i] = frame_info->height;
      streamer_config->tx_SOL_interval[i] = 3212;
    } else {
      l2_buffer_info.tx_linesize[i] = l2_buffer_info.rx_linesize[i];
    }
    l2_buffer_info.rx_buffsize[i] = l2_buffer_info.rx_linesize[i] * INPUT_BUF_LINES;
    l2_buffer_info.tx_buffsize[i] = l2_buffer_info.tx_linesize[i] * OUTPUT_BUF_LINES;
    streamer_config->rx_linesize[i] = l2_buffer_info.rx_linesize[i];
    streamer_config->tx_linesize[i] = l2_buffer_info.tx_linesize[i];
    FARF(HIGH, "hvx_zzHDR_get_config: corrected linesize %d, %d", streamer_config->rx_linesize[i], streamer_config->tx_linesize[i]);

  }
  OEM_voting->mips_total = 400;
  OEM_voting->mips_per_thread = 240;
  OEM_voting->bus_bw = 150;
  OEM_voting->usage_percentage = 10;
  FARF(ALWAYS, "hvx_add_constant_get_config: X");
  return;
}

/* main processing function */
void hvx_add_constant(void * p_para)
{
  FARF(ALWAYS, "hvx_add_constant: E");
  thread_data_str_t * p;
  volatile hvx_constant_t* hvx_constant;
  unsigned char* inbuf;
  unsigned char* outbuf;
  qurt_hvx_mode_t vector_mode;
  unsigned int linecount = 0;
  unsigned int processed_linecount = 0;
  unsigned int rx_bufsize, tx_bufsize;
  unsigned int in_idx = 0;
  unsigned int out_idx = 0;
  unsigned int cx_idx;
  unsigned int runahead_linecount;
  unsigned int width, height;
  unsigned int VLEN, shift_num;
  unsigned int force_exit_flag;
//  unsigned int rc, retval;
  unsigned int vfe_id, thread_id;
  unsigned int tx_linesize, rx_linesize, half_linesize;
  char fw_version[32];
  uint32_t raw_status;
  hvx_evt_cb_elem_t* cb_elem;
  int frame_cnt = 0;
  int overflow_recovery = 0;
  unsigned int frame_dump_flag;
  hvx_q_buffer_t* dump_buf;
  unsigned char* dump_buffer_addr;
  unsigned char dump_buffer_label;

  get_framework_version(fw_version);
  FARF(HIGH, "hvx_add_constant: framework version: %s", fw_version);

  p = (thread_data_str_t*) p_para;
  thread_id = p->thread_id;
  vfe_id = p->vfe_id;
  vector_mode = qurt_hvx_get_mode();

  if( -1 == process_util_vfe_id_validation(vfe_id)) {
    FARF(HIGH, "processing thread %d got bad vfe_id %d, exit",
         thread_id, vfe_id);
  }

  //get tx and rx buffer address
  inbuf = (unsigned char*)process_util_get_rx_addr(vfe_id);
  outbuf = (unsigned char*)process_util_get_tx_addr(vfe_id);
  //get static config buffer address
  //p_static = (hvx_static_config_t*)process_util_get_static_config_buf(vfe_id);
  //get dynamic config buffer address
  hvx_constant = (hvx_constant_t*)process_util_get_dynamic_config_buf(vfe_id);
  //update dynamic config buffer content at init step
  process_util_update_metabuf(vfe_id);

  FARF(LOW, "hvx_add_constant%d: rx address: %p", vfe_id, inbuf);
  FARF(LOW, "hvx_add_constant%d: tx address: %p", vfe_id, outbuf);
  FARF(LOW, "hvx_add_constant%d: dynamic buf address: %p", vfe_id, hvx_constant);


  height = l2_buffer_info.height;
  runahead_linecount = 4;

  if (p->vfe_mode == HVX_VFE0 || p->vfe_mode == HVX_VFE1) {
     cx_idx = 0;
  } else {
     cx_idx = vfe_id;
  }
  width = l2_buffer_info.width[cx_idx];
  rx_linesize = l2_buffer_info.rx_linesize[cx_idx];
  tx_linesize = l2_buffer_info.tx_linesize[cx_idx];
  rx_bufsize = l2_buffer_info.rx_buffsize[cx_idx];
  tx_bufsize = l2_buffer_info.tx_buffsize[cx_idx];

  if (rx_linesize != tx_linesize) {
    FARF(ERROR, "rx line size and tx line size doesn't match! return");
    return;
  }
  // wait for start signal
  while ( !process_util_get_start_flag(vfe_id) );
  force_exit_flag = process_util_get_force_exit_flag(vfe_id);

  if (force_exit_flag) {
    return;
  }
//For V65 V66 qurt_hvx_lock() not needed
//  retval = qurt_hvx_lock(vector_mode);
//  if (retval) {
//    return;
//  }

  switch (vector_mode) {
    case QURT_HVX_MODE_64B:
      VLEN = 64;
      shift_num = 6;
      break;
    case QURT_HVX_MODE_128B:
      VLEN = 128;
      shift_num = 7;
      break;
    default:
      return;
  }

  /* We want to wait for four lines to come in, so start with > 4-1 = 3 */
  linecount = runahead_linecount - 1;
  /* We want to compare received lines to processed lines, but we have two rows of boundary. */
  processed_linecount = 0;
  /* We should be waiting for 4 lines, then processing 2, then waiting
   * for pairs of lines and processing them in pairs.
   * This does almost that.  It waits for 4 lines, and processes them,
   * but starts TX right after the first batch is done
   */

  FARF(LOW,  "hvx_add_constant%d: rx_linesize = %d, tx_linesize = %d",
       vfe_id, rx_linesize, tx_linesize);
  FARF(LOW,  "hvx_add_constant%d: rx_bufsize = %d", vfe_id, rx_bufsize);
  FARF(LOW,  "hvx_add_constant%d: tx_bufsize = %d", vfe_id, tx_bufsize);
  FARF(LOW,  "hvx_add_constant%d: width = %d",   vfe_id, width);
  FARF(LOW,  "hvx_add_constant%d: height = %d",  vfe_id, height);

  while (1) {
    linecount = runahead_linecount - 1;
    /* We want to compare received lines to processed lines, but we have two rows of boundary. */
    processed_linecount = 0;
    //wait for rx sof
    while (!process_util_check_rx_sof(vfe_id)){
      force_exit_flag = process_util_get_force_exit_flag(vfe_id);
      if ( force_exit_flag ) {
        FARF(HIGH,  "hvx_add_constant%d: hit force exit flag in loop", vfe_id);
        break;
      }
    }
    raw_status = process_util_get_streamer_raw_status(vfe_id);
    frame_cnt++;
    if ( raw_status & STREAMER_ERROR_MASK ) {
      FARF(HIGH, "hvx_add_constant%d RAW STATUS: %x, frame %d", vfe_id, raw_status, frame_cnt);
    }
    if (raw_status & STREAMER_STATUS_RX_DATA_DROP ) {
      cb_elem = (hvx_evt_cb_elem_t*)qurt_malloc(sizeof(hvx_evt_cb_elem_t));
      cb_elem->evt_type = HVX_ERROR_CB_RX_DATA_DROP;
      cb_elem->handle = p->handle;
      cb_elem->vfe_mode = p->vfe_mode;
      sprintf(cb_elem->error_info, "vfe id %u, raw status %lu", vfe_id, raw_status);
      hvx_event_queue_enqueue(p->evt_queue, (void**)&cb_elem, 1);
      overflow_recovery = 1;
    }
    if (process_util_check_reg_update(vfe_id)) {
      process_util_update_metabuf(vfe_id);
    }

    frame_dump_flag = process_util_get_dump_flag(vfe_id);
    FARF(HIGH, "hvx_add_constant%d: dump flag %d", vfe_id, frame_dump_flag);
    if ( frame_dump_flag  && ( (frame_cnt % 10) == 0) && ( overflow_recovery == 0 ) ) {
      dump_buf = hvx_queue_deque( p->dump_buf_queue);
      if ( dump_buf != NULL ) {
        dump_buffer_addr = (unsigned char  *)dump_buf->data;
        dump_buffer_label = dump_buf->label;
        FARF(HIGH, "hvx_add_constant%d: get dump buffer addr %p, buffer label %x, data size %d ",
           vfe_id, dump_buffer_addr, dump_buffer_label, tx_linesize);
      } else {
        frame_dump_flag = 0;
      }
    } else {
      FARF(HIGH, "hvx_add_constant%d: dump flag not set or not in sync (frame cnt %d)", vfe_id, frame_cnt);
      frame_dump_flag = 0;
    }
    // reset rx sof
    half_linesize = tx_linesize;
    process_util_reset_rx_sof(vfe_id);
    FARF(LOW, "hvx_add_constant%d constant %d", vfe_id, hvx_constant->constant);
    while (linecount < height) {
      linecount = process_util_rx_wait_for_line(vfe_id,(linecount | 1));
      do {
          hvx_add_constant_asm(outbuf + out_idx,
                                 inbuf + in_idx,
                                 rx_linesize,
                                 hvx_constant->constant,
                                 shift_num);
          if (frame_dump_flag) {
            memcpy(dump_buffer_addr + rx_linesize * processed_linecount, outbuf + out_idx, rx_linesize);
          }
          process_util_rx_done(vfe_id,in_idx);
          process_util_tx_done(vfe_id,out_idx);
          in_idx  = process_util_wrap_idx(in_idx  + rx_linesize, rx_bufsize);
          out_idx = process_util_wrap_idx(out_idx + tx_linesize, tx_bufsize);
        processed_linecount += 1;
      } while (linecount > processed_linecount);
    }

    process_util_tx_done(vfe_id, out_idx);
    process_util_tx_wait_for_eof(vfe_id);
    process_util_tx_clear_eof(vfe_id);
    if (overflow_recovery) {
      process_util_recover_streamer(vfe_id);
      out_idx = 0;
      in_idx = 0;
      overflow_recovery = 0;
      frame_cnt = 0;
    }
    if ( frame_dump_flag ) {
      FARF(HIGH,  "hvx_zzhdr%d: enqueue data dump at frame %d", vfe_id, frame_cnt);
      enqueue_data_buffer(HVX_DATA_CB_DUMP_BUF_READY,
                          p->handle,
                          p->vfe_mode,
                          dump_buffer_label,
                          p->evt_queue);
  	}
    //check if need to exit
    force_exit_flag = process_util_get_force_exit_flag(vfe_id);
    if ( force_exit_flag ) {
        FARF(ALWAYS,  "hvx_add_constant%d: hit force exit flag in loop", vfe_id);
        break;
    }
  }

//For V65 V66 qurt_hvx_unlock() not needed
//  rc = qurt_hvx_unlock();
  FARF(ALWAYS, "hvx_add_constant%d: X", vfe_id);
  return;
}
