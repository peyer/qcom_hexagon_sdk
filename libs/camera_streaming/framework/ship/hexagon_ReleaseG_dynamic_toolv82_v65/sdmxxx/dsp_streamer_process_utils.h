/***************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 ****************************************************************************/

/** 
 * @file hvx_app_process_utils.h
 * @author Junyuan Shi
 * @date May 16th, 2016
 * @brief This file contains API between HVX camera streaming
 * control thread and processing thread.
 * The implementation of those APIs are in libadsp_hvx_skel.so.
 * User can call APIs to get buffer addresses, synchronization 
 * signals, etc. 
 */
#ifndef _HVX_APP_PROCESS_UTILS_H_
#define _HVX_APP_PROCESS_UTILS_H_

#include "AEEStdDef.h"
#include "hwio.h"
enum process_util_err_type {
  PROC_UTIL_SUCCESS = 0,
  PROC_UTIL_BAD_HANDLE,
  PROC_UTIL_BAD_THREAD_ID,
  PROC_UTIL_TIME_OUT
};
/**
 * @brief get framework version
 *  
 * This function will return the HVX camera streaming framework 
 * verison number as a string. 
 *  
 * @param v output string of version number 
 * @return None 
 */
void get_framework_version(char* v);

/**
 * @brief get raw status of currnt streamer
 *  
 * @param handle: current camera session identifier 
 * @param thread_id: current processing thread identifier
 * 
 * @return unsigned int: RAW_STATUS register value
 */
uint32_t process_util_get_streamer_raw_status(int handle, int thread_id);

/**
 * @brief get status of currnt streamer
 *  
 * @param handle: current camera session identifier 
 * @param thread_id: current processing thread identifier
 * 
 * @return unsigned int: STATUS register value
 */
uint32_t process_util_get_streamer_status(int handle, int thread_id);

/**
 * @brief set SOL and SOF bit in STATUS register, configure this 
 *        for interrupts
 *  
 * @param handle: current camera session identifier 
 * @param thread_id: current processing thread identifier
 * 
 * @return None
 */
void process_util_set_status_mask(int handle, int thread_id);

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
//void process_util_get_cache(unsigned int vfe_id, unsigned char** p_addr, int* p_size);

/**
  * Function: process_util_check_reg_update()
  *
  * Arguments:
  *     @arg handle: current camera session identifier
  *     @arg thread_id: current processing thread identifier
  *
  * Description:
  *     check if REG_UPDATE bit is set
  *
  * Return Value:
  *     0: bit not set
  *     1: bit set
  **/
uint32_t process_util_check_reg_update(int handle, int thread_id);

/**
  * Function: process_util_update_metabuf()
  *
  * Arguments:
  *     @arg handle: current camera session identifier
  *     @arg thread_id: current processing thread identifier
  *
  * Description:
  *     copy shadow CX content to CX
  *
  * Return Value:
  *     see process_util_err_type
  **/
int process_util_update_metabuf(int handle, int thread_id);

/**
  * Function: process_util_get_start_flag()
  *
  * Arguments:
  *     @arg handle: current camera session identifier
  *     @arg thread_id: current processing thread identifier
  *     @arg start_flag: return value, 0 means not set, 1 means
  *          start flag set, ready to start
  *
  * Description:
  *     return start flag set by control thread
  *
  * Return Value:
  *     see process_util_err_type
  **/
int process_util_get_start_flag(int handle, int thread_id, uint32_t* start_flag);

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
int process_util_get_force_exit_flag(int handle, int thread_id, uint32_t* stop_flag);
int process_util_get_dump_flag(int handle, int thread_id, uint32_t* dump_flag);
int process_util_get_stats_flag(int handle, int thread_id, uint32_t* stats_flag);
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
uint32_t process_util_check_rx_sof(int handle, int thread_id);
uint32_t process_util_check_rx_sol(int handle, int thread_id);
uint32_t process_util_check_rx_eol(int handle, int thread_id);

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
void process_util_reset_rx_sof(int handle, int thread_id);
void process_util_reset_rx_sol(int handle, int thread_id);

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
uint32_t process_util_rx_wait_for_line(int handle, int thread_id, uint32_t linecount);

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
void process_util_rx_done(int handle, int thread_id, uint32_t offset);

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
void process_util_tx_done(int handle, int thread_id, uint32_t offset);

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
void process_util_tx_wait_for_eof(int handle, int thread_id);

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
void process_util_tx_clear_eof(int handle, int thread_id);


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

int process_util_clear_dump_flag(int handle, int thread_id, uint32_t* stats_flag);

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
void process_util_recover_streamer(int handle, int thread_id);
void process_util_set_TX_SOL_interval(int handle, int thread_id, uint32_t interval);
uint32_t process_util_get_rx_SOL_interval(int handle, int thread_id);
uint32_t process_util_get_rx_lines(int handle, int thread_id);
#endif
