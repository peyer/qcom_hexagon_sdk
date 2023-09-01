/***************************************************************************
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
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
//#include "hwio.h"

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
 * @brief get raw status of current streamer
 *  
 * @param dev_p: pointer to streamer device.
 * 
 * @return unsigned int: RAW_STATUS register value
 */
uint32_t process_util_get_streamer_raw_status(void *dev_p);
uint32_t process_util_get_error_status_mask(void *dev_p);
uint32_t process_util_get_RX_bad_frame_error_status_mask(void *dev_p);

/**
 * @brief get status of currnt streamer
 *  
 * @param dev_p: pointer to streamer device.
 * 
 * @return unsigned int: STATUS register value
 */
uint32_t process_util_get_streamer_status(void *dev_p);

/**
 * @brief set SOL and SOF bit in STATUS register, configure this 
 *        for interrupts
 *  
 * @param dev_p: pointer to streamer device.
 * 
 * @return None
 */
void process_util_set_status_mask(void *dev_p);

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
 * @param dev_p: pointer to streamer device.
  *
  * Description:
  *     check if REG_UPDATE bit is set
  *
  * Return Value:
  *     0: bit not set
  *     1: bit set
  **/
uint32_t process_util_check_reg_update(void *dev_p);

/**
  * Function: process_util_update_metabuf()
  *
  * Arguments:
  *     @arg dev_p: pointer to device context.
  *
  * Description:
  *     copy shadow CX content to CX
  *
  * Return Value:
  *     see process_util_err_type
  **/
int process_util_update_metabuf(void *dev_p);

/**
  * Function: process_util_get_start_flag()
  *
  * Arguments:
  *     @arg dev_p: pointer to device context.
  *     @arg start_flag: return value, 0 means not set, 1 means
  *          start flag set, ready to start
  *
  * Description:
  *     return start flag set by control thread
  *
  * Return Value:
  *     see process_util_err_type
  **/
uint32 process_util_get_start_flag(void *dev_p);

/**
  * Function: process_util_get_<>_flag()
  *
  * Arguments:
  *     @arg dev_p: pointer to device context.
  *
  * Description:
  *     return flag set by control thread
  *
  * Return Value:
  *     1: exit
  *     0: do not exit
  **/
uint32 process_util_get_force_exit_flag(void *dev_p);

uint32 process_util_get_dump_flag(void *dev_p);

uint32 process_util_get_stats_flag(void *dev_p);


/**
  * Function: process_util_get_metabuf()
  *     @arg dev_p: pointer to device context.
  *
  * Description:
  *     return value of metabuf pointer
  *
  * Return void*:
  **/
void * process_util_get_metabuf(void *dev_p);
/**
  * Function: process_util_get_rxbuf_addr
  *     @arg dev_p: pointer to device context.
  *
  * Description:
  *     return address pointer of RX circular buffer
  *
  * Return void*:
  **/
void * process_util_get_rxbuf_addr(void *dev_p);
/**
  * Function: process_util_get_txbuf_addr
  *     @arg dev_p: pointer to device context.
  *
  * Description:
  *     return address pointer of TX circular buffer
  *
  * Return void*:
  **/
void * process_util_get_txbuf_addr(void *dev_p);

/**
  * Function: process_util_check_rx_<>()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  *
  * Description:
  *     Check if <> bit got updated in STATUS register
  *
  * Return Value:
  *     1: <> bit set
  *     0: <> bit not set
  **/
uint32_t process_util_check_rx_sof(void *dev_p);

uint32_t process_util_check_rx_sol(void *dev_p);

uint32_t process_util_check_rx_eol(void *dev_p);

/**
  * Function: process_util_reset_rx_<>()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  *
  * Description:
  *     Reset <> bit to 0 in STATUS register
  *
  * Return Value:
  *     None
  **/
void process_util_reset_rx_sof(void *dev_p);
void process_util_reset_rx_sol(void *dev_p);

/**
  * Function: process_util_rx_wait_for_line()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  * @param linecount: waiting period (in num. of lines).
  *
  * Description:
  *     Wait until rx get linecount+1 lines(EOLs)
  *     eg: wait until RX_LINE_COUNT register reach linecount+1
  *
  * Return Value:
  *     linecount+1
  **/
uint32_t process_util_rx_wait_for_line(void *dev_p, uint32_t linecount);

/**
  * Function: process_util_rx_done()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  * @arg offset: offset of consumed data in rx
  *
  * Description:
  *     Update RX_INDEX_CONSUMED register to offset
  *
  * Return Value:
  *     None
  **/
void process_util_rx_done(void *dev_p, uint32_t offset);

/**
  * Function: process_util_rx_done()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  * @arg offset: offset of transmit data in tx
  *
  * Description:
  *     Update TX_INDEX_AVAIL register to offset
  *
  * Return Value:
  *     None
  **/
void process_util_tx_done(void *dev_p, uint32_t offset);

/**
  * Function: process_util_tx_wait_for_eof()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  *
  * Description:
  *     wait until EOF bit got set in STATUS register
  *
  * Return Value:
  *     None
  **/
void process_util_tx_wait_for_eof(void *dev_p);

/**
  * Function: process_util_tx_clear_eof()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
  *
  * Description:
  *     reset EOF bit to 0 in STATUS register
  *
  * Return Value:
  *     None
  **/
void process_util_tx_clear_eof(void *dev_p);

/**
  * Function: process_util_recover_streamer()
  *
  * Arguments:
  * @param dev_p: pointer to streamer device.
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
void process_util_recover_streamer(void *dev_p);

void process_util_set_TX_SOL_interval(void *dev_p, uint32_t interval);

uint32_t process_util_get_rx_SOL_interval(void *dev_p);

uint32_t process_util_get_rx_lines(void *dev_p);

void* process_util_get_streamer_config(void *dev_p);

void* process_util_get_frame_info(void *dev_p);

void* process_util_get_config_from_app(void *dev_p);

void* process_util_get_client_cfg(void *dev_p);

void* process_util_get_app_context(void *dev_p);

#endif
