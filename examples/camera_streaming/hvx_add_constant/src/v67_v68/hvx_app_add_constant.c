////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////
// From Hexagon SDK environment
// If User want to control the workloop then uncomment the below #define USE_CUSTOM_WORKLOOP
//#define USE_CUSTOM_WORKLOOP

#define  FARF_ERROR 1
#define  FARF_HIGH 1
#define  FARF_LOW 1

#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "dsp_streamer_if_app.h"
#include "dsp_streamer_common.h"
#include "hvx_app_add_constant.h"
#include "hvx_add_constant_def.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_CUSTOM_WORKLOOP
#include "dsp_streamer_process_utils.h"
#endif


#define roundup_t(a, m)     (((a)+(m)-1)&(0-m))

uint32_t wrap_buffer_idx(uint32_t idx, uint32_t bufsize) {
    if (idx >= bufsize) {
        idx -= bufsize;
    }
    return idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Wrapper for function used to dump buffers.
///
/// @param  strm_dev_p  Streamer device.
/// @param  callbacks_p Framework callback pointers.
/// @param  buf_p  Pointer to buffer to be dumped.
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int buf_get_empty(void* strm_dev_p, framework_callback_functns_t *callbacks_p, dsp_streamer_buf_desc_t* buf_p)
{
    int ret = HVX_SUCCESS;
    unsigned int blocking = 0;

    if(!strm_dev_p || !callbacks_p || !buf_p)
    {
        FARF(ERROR, "%s: NULL parameter (%p/%p)!", __func__, strm_dev_p, buf_p);
        return HVX_FAILURE;
    }

    if(callbacks_p->buf_get_empty_func_ptr)
    {
        int rc = callbacks_p->buf_get_empty_func_ptr(strm_dev_p, (void*)buf_p, blocking);
        FARF(HIGH, "%s: buf_get_empty_func_ptr callback returned %d!", __func__, rc);
        if(rc != 0)//TODO: use ret code
        {
            ret = HVX_FAILURE;
        }
    }
    else
    {
        FARF(ERROR, "%s: callback func not present!", __func__);
        ret = HVX_FAILURE;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Wrapper for function used to retrieve buffers.
///
/// @param  strm_dev_p  Streamer device.
/// @param  callbacks_p Framework callback pointers.
/// @param  buf_p  Pointer to buffer to be dumped.
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int buf_send_full(void* strm_dev_p, framework_callback_functns_t *callbacks_p, dsp_streamer_buf_desc_t* buf_p)
{
    int ret = HVX_SUCCESS;
    unsigned int blocking = 1;

    if(!strm_dev_p || !callbacks_p || !buf_p)
    {
        FARF(ERROR, "%s: NULL parameter!", __func__);
        return HVX_FAILURE;
    }

    if(callbacks_p->buf_send_full_func_ptr)
    {
        int rc = callbacks_p->buf_send_full_func_ptr(strm_dev_p, (void*)buf_p, blocking);
        FARF(HIGH, "%s: buf_send_full_func_ptr callback returned %d!", __func__, rc);
        if(rc != 0)//TODO: use ret code
        {
            ret = HVX_FAILURE;
        }
    }
    else
    {
        FARF(ERROR, "%s: callback func not present!", __func__);
        ret = HVX_FAILURE;
    }

    return ret;
}

static unsigned int linesize_128b_aligned_get(unsigned int width, unsigned int pad_size, unsigned int byteperpixel)
{ 
    unsigned int padsize=0,linesize=0;
    switch(pad_size)
    {
        case PAD_SIZE_0:
            padsize =0;
        break;
        case PAD_SIZE_8:
            padsize =16;
        break;
        case PAD_SIZE_SOL_ONLY:
        case PAD_SIZE_EOL_ONLY:
            padsize =8;
        break;
        default:
            padsize =0;
    }
    linesize = (unsigned int) roundup_t(width* byteperpixel + padsize * byteperpixel, 128);
    return linesize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Get configuration: 
///
/// @param  app_context_p   Pointer to app context allocate at entry point.
///
/// @param config_in_p: pointer to the (input) configuration data set by the client.
///
/// @param config_out_p: pointer to the (output) configuration data set by the app and returned to the client.
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int hvx_add_constant_get_config(void *app_context_p,
        config_from_client_t* config_in_p,
        config_from_app_t* config_out_p)
{
    int ret = HVX_SUCCESS;
    FARF(ALWAYS, "%s: E", __func__);

    FARF(ALWAYS, "%s:=========== CUSTOM get Config =============", __func__);
    FARF(HIGH, "%s: input height = %d,"
            "output height = %d, ife_type = %u,"
            "pixel bit depth = %u, tapping point %d,"
            "frame pixel format %u", __func__,
            config_in_p->frame_info.in_height,
            config_in_p->frame_info.out_height,
            config_in_p->ife_mode,
            config_in_p->frame_info.pixel_bit_depth,
            config_in_p->tapping_point,
            config_in_p->frame_info.pixel_format);

    config_out_p->streamer_config.dynamic_buf_size = sizeof(hvx_constant_t);
    config_out_p->streamer_config.rx_pad_size = PAD_SIZE_0;
    config_out_p->streamer_config.rx_pad_type = PAD_0;
    config_out_p->streamer_config.tx_pad_size = PAD_SIZE_0;
    config_out_p->streamer_config.streamer_in_format = BIT_DEPTH_10;
    config_out_p->streamer_config.streamer_out_format = BIT_DEPTH_10;

    config_out_p->streamer_config.thread_stack_size = 4096;

    config_out_p->streamer_config.tx_timing_mode =TX_PROGRAMMED_MODE;// TX_FOLLOW_RX_MODE;

    FARF(HIGH, "%s: streamer %d input width = %d, output width = %d,",
            __func__, config_in_p->id,
            config_in_p->frame_info.in_width,config_in_p->frame_info.out_width);
    config_out_p->streamer_config.rx_lines = INPUT_BUF_LINES;
    config_out_p->streamer_config.tx_lines = OUTPUT_BUF_LINES;
    config_out_p->streamer_config.tx_min_start = OUTPUT_START_DELAY;

    config_out_p->streamer_config.rx_first_fetch_numlines = 4;
    config_out_p->streamer_config.rx_fetch_numlines = 2;

    config_out_p->streamer_config.tx_first_fetch_numlines = 4;
    config_out_p->streamer_config.tx_fetch_numlines =2;

    /*** calculate rx and tx linesize based on width and padding ***/
    config_out_p->streamer_config.rx_linesize = linesize_128b_aligned_get(
            config_in_p->frame_info.in_width, config_out_p->streamer_config.rx_pad_size,
            BYTES_PER_PIXEL);
	config_out_p->streamer_config.rx_l2_format=1;
	config_out_p->streamer_config.tx_l2_format=1;
	config_out_p->streamer_config.rx_l2_pack_msb_aligned=0;
	config_out_p->streamer_config.tx_l2_pack_msb_aligned=0;

	config_out_p->streamer_config.tx_linesize = linesize_128b_aligned_get(
	config_in_p->frame_info.out_width, config_out_p->streamer_config.tx_pad_size,
                BYTES_PER_PIXEL);
	config_out_p->streamer_config.tx_pixel_cnt = config_in_p->frame_info.out_width;
	config_out_p->streamer_config.tx_line_cnt = config_in_p->frame_info.out_height;
	config_out_p->streamer_config.tx_SOL_interval = 3212;
	FARF(HIGH, "%s: for streamer %d, TX_PROGRAMMED_MODE enabled",
                __func__, config_in_p->id);

    FARF(HIGH, "%s: for streamer %d, rx_linesize is %d, tx_linesize is %d",
            __func__, config_in_p->id, config_out_p->streamer_config.rx_linesize,
            config_out_p->streamer_config.tx_linesize);

    if (config_in_p->ife_mode == HVX_IFE_BOTH) {
        // in case of dual vfe, vote for NOM ADSP clk
        config_out_p->power_votes.mips_total = 600;
        config_out_p->power_votes.mips_per_thread = 150;
    } else {
        // in case of single vfe, vote for TURBO ADSP clk
        config_out_p->power_votes.mips_total = 900;
        config_out_p->power_votes.mips_per_thread = 300;
    }

    config_out_p->power_votes.bus_bw = 4800000000;
    config_out_p->power_votes.usage_percentage = 50;

    /* Update internal data */
    ((hvx_add_constant_context_t*) app_context_p)->num_lines_processed = 0;
    ((hvx_add_constant_context_t*) app_context_p)->num_lines_total =
            config_in_p->frame_info.out_height;

    FARF(ALWAYS, "%s: X", __func__);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Main processing function. 
///
/// @param
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hvx_add_constant_process_lines(
                            void *app_context_p,
                            void* dst_p,
                            void* src_p,
                            process_lines_params_t* params_p)
{
    int32 shift = 7;
    uint32 num_lines_to_process =
            /* TH: uncomment this after fixing rx_first_fetch_numlines: ((hvx_add_constant_context_t*)app_context_p)->num_lines_processed == 0 ?
                    params_p->app_cfg_p->streamer_config.rx_first_fetch_numlines : */
                    params_p->app_cfg_p->streamer_config.rx_fetch_numlines;
    uint32 Tx_buffpointer = (uint32)dst_p;
    uint32 Rx_buffpointer = (uint32)src_p;
    hvx_constant_t* hvx_constant = (hvx_constant_t*)params_p->client_cfg_p->metadata_p;
    uint32 linesize_bytes = params_p->app_cfg_p->streamer_config.tx_linesize;

#ifdef BUF_SHARING_EN
    dsp_streamer_buf_desc_t buf;
    unsigned int send_buf = 0;
    if(!((hvx_add_constant_context_t*)app_context_p)->num_lines_processed)
    {
        if(!buf_get_empty(
                ((hvx_add_constant_context_t*)app_context_p)->client_p,
                ((hvx_add_constant_context_t*)app_context_p)->callbacks_p,
                &buf))
        {
            FARF(LOW, "%s: Received buf with fd %d, size %u!\n",
                                __func__,
                                buf.hndl,
                                buf.len);
            *((unsigned char*)(buf.vaddr)) = 0xAB;
            *((unsigned char*)(buf.vaddr+1)) = 0xCD;
            send_buf = 1;
        }
        else
        {
            FARF(ERROR, "%s: buf_get_empty failed!\n",
                                __func__);
            buf.hndl = -1;
        }
    }
#endif //BUF_SHARING_EN

    hvx_add_constant_asm((void*)(Tx_buffpointer),
                            (void*)(Rx_buffpointer),
                            (linesize_bytes*num_lines_to_process),
                            hvx_constant->constant,
                            shift);

    ((hvx_add_constant_context_t*)app_context_p)->num_lines_processed += num_lines_to_process;

    if(((hvx_add_constant_context_t*)app_context_p)->num_lines_processed >=
            ((hvx_add_constant_context_t*)app_context_p)->num_lines_total)
    {
        FARF(HIGH, "%s: lines processed %u out of %u. Resetting!\n",
                    __func__,
                    ((hvx_add_constant_context_t*)app_context_p)->num_lines_processed,
                    ((hvx_add_constant_context_t*)app_context_p)->num_lines_total);
        ((hvx_add_constant_context_t*)app_context_p)->num_lines_processed = 0;
    }

#ifdef BUF_SHARING_EN
     if(send_buf)
     {
         if(buf_send_full(
                 ((hvx_add_constant_context_t*)app_context_p)->client_p,
                 ((hvx_add_constant_context_t*)app_context_p)->callbacks_p,
                 &buf))
         {
             FARF(ERROR, "%s: buf_send_full failed!\n",
                                 __func__);
         }
     }
#endif // BUF_SHARING_EN
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Main processing loop.
///
/// @param  p  Thread payload.
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hvx_add_constant(void *p)
{
#ifdef USE_CUSTOM_WORKLOOP

    uint32 raw_status = 0,raw_status_mask=0,raw_status_Rx_data_loss_mask=0;
    uint32 overflow_recovery = 0;
    uint32 linecount = 0;
    uint32 in_idx = 0;
    uint32 out_idx = 0;
    uint32 sess_p_stats_frame_cnt = 0;
    uint32 skip_processing = 0;
	streamer_request_config_t  *config_p = (streamer_request_config_t*)process_util_get_streamer_config(p);
    ife_frame_info_t* frame_info_p = process_util_get_frame_info(p);

    /* TX/RX pointers*/
    uint8 *tx_buf_p = (uint8*)process_util_get_txbuf_addr(p);
    uint8 *rx_buf_p = (uint8*)process_util_get_rxbuf_addr(p);

    process_lines_params_t params;
    params.app_cfg_p =(config_from_app_t*)process_util_get_config_from_app(p);
    params.client_cfg_p =(config_from_client_t*) process_util_get_client_cfg(p);
    params.client_cfg_p->metadata_p =(void*) process_util_get_metabuf(p);
    params.client_cfg_p->num_lines_processed = 0;
	void* app_context_p= process_util_get_app_context(p);
	
    /**
     * RX and TX buffer size
     * these values are whatever got calculated in get_config
     * function
     * in dual IFE case, two processing thread will get separate buf
     * size based on thread_id.
     * in single IFE case, thread_id will be 0
     */
    uint32 rx_linesize = config_p->rx_linesize;
    uint32 tx_linesize = config_p->tx_linesize;
    uint32 rx_bufsize = rx_linesize * config_p->rx_lines;
    uint32 tx_bufsize = tx_linesize * config_p->tx_lines;

//    FARF(LOW, "Entering CUSTOM workloop for dev %u!", dev_p->resources.id);

    /** Frame layout **/
    uint32 width = frame_info_p->in_width;
    uint32 height = frame_info_p->in_height;
//    uint32 RX_SOL_interval = 0;

    /** Copy shadow_metabuf->metabuf **/
    process_util_update_metabuf(p);

    /** Wait for config_p->rx_first_fetch_numlines before calling the processing function the first time.
     * Wait for config_p->fetch_numlines before calling the processing function all subsequent times.
     */
//    uint32 rx_line_count = config_p->rx_first_fetch_numlines;
    uint32 processed_linecount = 0;

    /******** Wait here before entering main loop ****/
//    osal_sem_down(&dev_p->workloop_ctrl.sync, NULL, blocking);
    FARF(HIGH, "Workloop for . rx_first_fetch_numlines %u, fetch_numlines %u, width %u, height %u!",
                     config_p->rx_first_fetch_numlines, config_p->rx_fetch_numlines, width, height);
    FARF(HIGH, "Workloop for . rx_linesize %u, tx_linesize %u, rx_bufsize %u, tx_bufsize %u, tx_min_start %u, tx_timing_mode %u!",
                     rx_linesize, tx_linesize, rx_bufsize, tx_bufsize, config_p->tx_min_start, config_p->tx_timing_mode);
    uint32 internal_exit_flag = 0;
    while(!process_util_get_force_exit_flag(p) && !internal_exit_flag){
        processed_linecount = 0;
        overflow_recovery = 0;

        /* Wait for rx sof */
        while (!process_util_check_rx_sof(p)) {
            if (process_util_get_force_exit_flag(p)) {
            FARF(HIGH,  "Worker loop for dev id hit force exit flag. Exiting loop!" );
                goto exit_pre_sof;
            }
        }

        sess_p_stats_frame_cnt++;

        /* At SOF, program TX SOL interval if TX is in programmed mode */
        //TODO: why not use TX_START_CONDITION??
        if (config_p->tx_timing_mode == TX_PROGRAMMED_MODE) {
//            while (process_util_get_rx_lines(p) < ((uint32)config_p->rx_first_fetch_numlines[0]/2));//
//            RX_SOL_interval = process_util_get_rx_SOL_interval(p);
//            process_util_set_TX_SOL_interval(p, RX_SOL_interval );//+ (uint32)config_p->tx_first_fetch_numlines[0]
        }

        /* At SOF, read streamer status and determine error status */
        raw_status = process_util_get_streamer_raw_status(p);
        raw_status_mask = process_util_get_error_status_mask(p);

        if (raw_status & raw_status_mask) {//STREAMER_ERROR_MASK
            FARF(ERROR, "STREAMER_ERROR_MASK set in RAW STATUS: raw_status %x, frame %d!",
                    raw_status, sess_p_stats_frame_cnt);
        }
        raw_status_Rx_data_loss_mask = process_util_get_RX_bad_frame_error_status_mask(p);
        if (raw_status & raw_status_Rx_data_loss_mask) {//STREAMER_STATUS_RX_DATA_DROP
            FARF(ERROR, "Enabling overflow_recovery: frame %d!",
                    raw_status, sess_p_stats_frame_cnt);
            overflow_recovery = 1;
        }

        /* At SOF, update metabuf to apply the latest per-frame parameters */
        if (process_util_check_reg_update(p)) {
            process_util_update_metabuf(p);
            skip_processing = 0;
        }
        else{
            skip_processing = 1;
        }

        //TODO: if buffer exchange enabled: obtain an empty buffer here

        /* At SOF, reset rx sof for next SOF check */
        process_util_reset_rx_sof(p);

        /* Process lines */
        while (processed_linecount < height) {
            linecount = processed_linecount +
                    ((linecount > config_p->rx_first_fetch_numlines) ? config_p->rx_fetch_numlines : config_p->rx_first_fetch_numlines)
                    - 1;
            linecount = process_util_rx_wait_for_line(p,(linecount ));
            do {
                params.client_cfg_p->num_lines_processed = processed_linecount;
                if(!skip_processing){
                    hvx_add_constant_process_lines(
                        app_context_p,
                        tx_buf_p + out_idx,
                        rx_buf_p + in_idx,
						&params  );
                }
                processed_linecount += (uint32)(config_p->rx_fetch_numlines);
                //TODO: if buffer exchange enabled: write into empty buffer here

                in_idx = wrap_buffer_idx(in_idx + (config_p->rx_fetch_numlines)*rx_linesize, rx_bufsize);
                out_idx = wrap_buffer_idx(out_idx + (config_p->tx_fetch_numlines)*tx_linesize, tx_bufsize);
                process_util_rx_done(p, 64*(int)(in_idx/64));
                if (config_p->tx_timing_mode <= TX_PROGRAMMED_MODE) {
                    process_util_tx_done(p, 64*(int)(out_idx/64));
                    }

            } while (linecount >( processed_linecount+(uint32)(config_p->rx_fetch_numlines)-1));
        }

        if (config_p->tx_timing_mode <= TX_PROGRAMMED_MODE) {
            process_util_tx_done(p, out_idx);
            process_util_tx_wait_for_eof(p);
            process_util_tx_clear_eof(p);
			process_util_tx_done(p, 0);// reset counter
        }
        if (overflow_recovery) {
            FARF(HIGH, " Overflow!!! recover in process");
            process_util_recover_streamer(p);
            out_idx = 0;
            in_idx = 0;
            overflow_recovery = 0;
            sess_p_stats_frame_cnt = 0;
        }

    }//while(1)

exit_pre_sof:
#endif
    return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Termination function.
///
/// @param app_context_p: pointer to the app context (originally allocated by the app at entry point).
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hvx_add_constant_terminate(void *app_context_p)
{
    FARF(ALWAYS, "%s: E", __func__);

#ifdef BUF_SHARING_EN
    //Flush buffer queues
    dsp_streamer_buf_desc_t buf;
    unsigned int send_buf = 0;
    do{
        if(!buf_get_empty(
                ((hvx_add_constant_context_t*)app_context_p)->client_p,
                ((hvx_add_constant_context_t*)app_context_p)->callbacks_p,
                &buf))
        {
            FARF(LOW, "%s: Received buf with fd %d, size %u!\n",
                                __func__,
                                buf.hndl,
                                buf.len);
            send_buf = 1;
        }
        else
        {
            FARF(ERROR, "%s: buf_get_empty failed!\n",
                                __func__);
            buf.hndl = -1;
            send_buf = 0;
        }

        if(send_buf)
        {
            if(buf_send_full(
                    ((hvx_add_constant_context_t*)app_context_p)->client_p,
                    ((hvx_add_constant_context_t*)app_context_p)->callbacks_p,
                    &buf))
            {
                FARF(ERROR, "%s: buf_send_full failed!\n",
                                    __func__);
            }
        }
    } while(send_buf);
#endif // BUF_SHARING_EN

    if(app_context_p) free(app_context_p);
    FARF(ALWAYS, "%s: X", __func__);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// app_entry
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void app_entry(app_callbac_functns_t *app_callbacks_p, void** app_context_pp, void *client_p, framework_callback_functns_t *fwk_callbacks_p)
{
    FARF(ALWAYS, "%s: E", __func__);
    *app_context_pp = malloc(sizeof(hvx_add_constant_context_t));
    ((hvx_add_constant_context_t*)(*app_context_pp))->client_p = client_p;
    ((hvx_add_constant_context_t*)(*app_context_pp))->callbacks_p = fwk_callbacks_p;

    app_callbacks_p->get_config_func_ptr =  &hvx_add_constant_get_config;
    app_callbacks_p->process_lines_func_ptr = &hvx_add_constant_process_lines;
    app_callbacks_p->exit_func_ptr = &hvx_add_constant_terminate;
#ifdef USE_CUSTOM_WORKLOOP
    app_callbacks_p->work_loop_func_ptr = &hvx_add_constant;
#else
    app_callbacks_p->work_loop_func_ptr = NULL;
#endif
    FARF(ALWAYS, "%s: X", __func__);
}


