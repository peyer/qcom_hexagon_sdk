/***************************************************************************
 * Copyright (c) 2015 QUALCOMM Technology INCORPORATED
 * All Rights Reserved
 ****************************************************************************/

#ifndef _HVX_APP_PROCESS_UTILS_H_
#define _HVX_APP_PROCESS_UTILS_H_

/**
  * Function: get_framework_version()
  *
  * Arguments:
  *     none
  *
  * Description:
  *     return a string of current framework version
  *
  * Return Value:
  *     string of version
  **/
void get_framework_version(char*);

/**
  * Function: process_util_vfe_id_validation()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     check if vfe id is in range [0, 1]
  *
  * Return Value:
  *     0: valid vfe id
  *     -1: invalid vfe id
  **/
int process_util_vfe_id_validation(unsigned int vfe_id);

/**
  * Function: process_util_get_streamer_raw_status()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     returns raw status of streamer
  *
  * Return Value:
  *     raw status register value
  **/
unsigned int process_util_get_streamer_raw_status(unsigned int vfe_id);

/**
  * Function: process_util_get_rx_addr()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     get rx buffer starting address based on specific vfe id
  *
  * Return Value:
  *     rx address
  **/
void* process_util_get_rx_addr(unsigned int vfe_id);

/**
  * Function: process_util_get_tx_addr()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     get tx buffer starting address based on specific vfe id
  *
  * Return Value:
  *     tx address
  **/
void* process_util_get_tx_addr(unsigned int vfe_id);

/**
  * Function: process_util_get_static_config_buf()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     get static config buffer starting address based
  *     on specific vfe id
  *
  * Return Value:
  *     buffer address
  **/
void* process_util_get_static_config_buf(unsigned int vfe_id);

/**
  * Function: process_util_get_dynamic_config_buf()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     get dynamic config buffer starting address based on
  *     specific vfe id
  *
  * Return Value:
  *     buffer address
  **/
void* process_util_get_dynamic_config_buf(unsigned int vfe_id);

/**
  * Function: process_util_get_cache()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *     @arg p_addr: return starting address of availabel L2
  *          cache
  *     @arg p_size: return size of available L2 cache
  *
  * Description:
  *     get available L2 cache section for processing thread use
  *
  * Return Value:
  *     None
  **/
void process_util_get_cache(unsigned int vfe_id, unsigned char** p_addr, int* p_size);

/**
  * Function: process_util_check_reg_update()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     check if REG_UPDATE bit is set
  *
  * Return Value:
  *     0: bit not set
  *     1: bit set
  **/
int process_util_check_reg_update(unsigned int vfe_id);

/**
  * Function: process_util_update_metabuf()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     copy shadow CX content to CX
  *
  * Return Value:
  *     None
  **/
void process_util_update_metabuf(unsigned int vfe_id);

/**
  * Function: process_util_get_start_flag()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     return start flag set by control thread
  *
  * Return Value:
  *     0: do not start
  *     1: OK to start
  **/
unsigned int process_util_get_start_flag(unsigned int vfe_id);

/**
  * Function: process_util_get_force_exit_flag()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     return force exit flag set by control thread
  *
  * Return Value:
  *     1: exit
  *     0: do not exit
  **/
unsigned int process_util_get_force_exit_flag(unsigned int vfe_id);

/**
  * Function: process_util_check_rx_sof()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     check if SOF bit got updated in STATUS register
  *
  * Return Value:
  *     1: SOF bit set
  *     0: SOF bit not set
  **/
unsigned int process_util_check_rx_sof(unsigned int vfe_id);

/**
  * Function: process_util_reset_rx_sof()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     reset SOF bit to 0 in STATUS register
  *
  * Return Value:
  *     None
  **/
void process_util_reset_rx_sof(unsigned int vfe_id);

/**
  * Function: process_util_rx_wait_for_line()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     wait until rx get linecount+1 lines(EOLs)
  *     say, wait until RX_LINE_COUNT register reach linecount+1
  *
  * Return Value:
  *     linecount+1
  **/
unsigned int process_util_rx_wait_for_line(unsigned int vfe_id, unsigned int linecount);

/**
  * Function: process_util_rx_done()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *     @arg offset: offset of consumed data in rx
  *
  * Description:
  *     update RX_INDEX_CONSUMED register to offset
  *
  * Return Value:
  *     None
  **/
void process_util_rx_done(unsigned int vfe_id, unsigned int offset);

/**
  * Function: process_util_rx_done()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *     @arg offset: offset of transmit data in tx
  *
  * Description:
  *     update TX_INDEX_AVAIL register to offset
  *
  * Return Value:
  *     None
  **/
void process_util_tx_done(unsigned int vfe_id, unsigned int offset);

/**
  * Function: process_util_tx_wait_for_eof()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     wait until EOF bit got set in STATUS register
  *
  * Return Value:
  *     None
  **/
void process_util_tx_wait_for_eof(unsigned int vfe_id);

/**
  * Function: process_util_tx_clear_eof()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     reset EOF bit to 0 in STATUS register
  *
  * Return Value:
  *     None
  **/
void process_util_tx_clear_eof(unsigned int vfe_id);

/**
  * Function: process_util_wrap_idx()
  *
  * Arguments:
  *     @arg idx: next idx in TX/RX buffer
  *     @arg bufsize: TX/RX size
  *
  * Description:
  *     if idx hit buffer end, rotate to buffer head
  *
  * Return Value:
  *    incremented idx
  **/
unsigned int process_util_wrap_idx(unsigned int idx, unsigned int bufsize);

/**
  * Function: process_util_get_dump_flag()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     check if frame dump is needed
  *
  * Return Value:
  *    flag value
  **/
unsigned int process_util_get_dump_flag(unsigned int vfe_id);

unsigned int process_util_get_stats_flag(unsigned int vfe_id);

/**
  * Function: process_util_get_dump_addr()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     get the buffer addr to be dumped to
  *
  * Return Value:
  *    buffer address
  **/
void* process_util_get_dump_addr(unsigned int vfe_id);

/**
  * Function: process_util_get_dump_label()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     get the buffer label to be dumped to
  *
  * Return Value:
  *     buffer label
  **/
unsigned char process_util_get_dump_label(unsigned int vfe_id);

/**
  * Function: process_util_clear_dump_flag()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     clear the dump flag once caught in SOF event
  *
  * Return Value:
  *     none
  **/
void process_util_clear_dump_flag(unsigned int vfe_id);

/**
  * Function: process_util_reset_streamer()
  *
  * Arguments:
  *     @arg vfe_id: vfe id of current processing thread
  *
  * Description:
  *     in case of overflow happens, call this function which
  *     will do:
  *       stop streamer
  *       reset streamer
  *       reprogram streamer with previous values
  *       start streamer
  *
  * Return Value:
  *     none
  **/
void process_util_recover_streamer(unsigned int vfe_id);
#endif
