/***************************************************************************
 * Copyright (c) 2015-2016 QUALCOMM Technology INCORPORATED
 * All Rights Reserved
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
 * @brief check if vfe id is in range [0, 1]
 *  
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return int: 0 valid, -1 invalid
 */
int process_util_vfe_id_validation(unsigned int vfe_id);

/**
 * @brief check raw status of streamer[vfe_id]
 *  
 * @param vfe_id: vfe_id of current processing thread
 * 
 * @return unsigned int: RAW_STATUS register value
 */
unsigned int process_util_get_streamer_raw_status(unsigned int vfe_id);

/**
 * @brief get rx buffer starting address based on specific vfe 
 *        id
 *  
 * @param vfe_id: vfe_id of current processing thread
 * 
 * @return void*: rx starting address
 */
void* process_util_get_rx_addr(unsigned int vfe_id);

/**
 * @brief get tx buffer starting address based on specific vfe 
 *        id
 * 
 * @param vfe_id: vfe_id of current processing thread
 * 
 * @return void*: tx starting address
 */
void* process_util_get_tx_addr(unsigned int vfe_id);

/**
 * @brief for debugging purpose only! get static config buffer 
 *        address
 *  
 * @param vfe_id: vfe_id of current processing thread
 * 
 * @return void*: static config buffer address
 */
void* process_util_get_static_config_buf(unsigned int vfe_id);

/**
 * @brief get dynamic config buffer address 
 *  
 * dynamic config buffer contains algorithm specified data 
 * structure that is in sync between APPS and ADSP. User would 
 * get the buffer address and parse it to be the data structure. 
 * There is no offset between this address and actual data. 
 *  
 * @param vfe_id: vfe_id of current processing thread
 * 
 * @return void*: dynamic config buffer address
 */
void* process_util_get_dynamic_config_buf(unsigned int vfe_id);

/**
 * @brief get available L2 for processing thread use as scratch 
 *        buffer
 * 
 * @param vfe_id: input, vfe id of current processing thread
 * @param p_addr: output, address of available L2
 * @param p_size: output, size of available L2 
 *  
 * @return None 
 */
void process_util_get_cache(unsigned int vfe_id, unsigned char** p_addr, int* p_size);

/**
 * @brief check if REG_UPDATE register has been updated
 *  
 * Everytime APPS update the dynamic config structure, 
 * REG_UPDATE will be set. Please check this at every SOF, and 
 * use updated dynamic config parameters. Use this API together 
 * with process_util_update_metabuf 
 *  
 * In case of dual VFE (dual streamer), each processing thread 
 * can be updated independently, so please check for both 
 * processing threads 
 * 
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return int: 0 bit not set, 1 bit set
 */
int process_util_check_reg_update(unsigned int vfe_id);

/**
 * @brief update dynamic config buffer to latest 
 *  
 * Upon calling of this API, the content in shadow buffer will 
 * be copied into active dynamic config buffer
 * 
 * @param vfe_id: vfe id of current processing thread
 *  
 * @return None 
 */
void process_util_update_metabuf(unsigned int vfe_id);

/**
 * @brief check if start signal is issued 
 *  
 * After starting processing thread, control thread will soon 
 * issue a start signal, this API checks if this signal is 
 * issued. if yes, proceed with HVX locking, etc.
 * 
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return unsigned int: 0, do not start, 1, OK to start 
 */
unsigned int process_util_get_start_flag(unsigned int vfe_id);

/**
 * @brief check if exit flag is issued
 *  
 * at time of stream-off, control thread will set the exit flag 
 * to 1. OEM can use this API to check that flag in processing 
 * thread. Please exit the processing thread if it is found set.
 * 
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return unsigned int: 1, exit; 0, don't exit
 */
unsigned int process_util_get_force_exit_flag(unsigned int vfe_id);

/**
 * @brief check if SOF got set in streamer RAW_STATUS register 
 *  
 * when a new frame come, streamer will catch SOF signal, and 
 * set the SOF bit in RAW_STATUS register to 1. 
 *  
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return unsigned int, 1 SOF bit set, 0 SOF bit not set
 */
unsigned int process_util_check_rx_sof(unsigned int vfe_id);

/**
 * @brief reset SOF bit in streamer RAW_STATUS register 
 *  
 * streamer will not reset SOF bit itself. In order to get 
 * successive SOF signals, user need to clean the bit to 0. 
 * 
 * @param vfe_id: vfe id of current processing thread 
 *  
 * @return None 
 */
void process_util_reset_rx_sof(unsigned int vfe_id);

/**
 * @brief wait for RX to get certain number of lines
 *  
 * This function will return until RX get linecount+1 lines. it 
 * will keep checking active number of lines(EOLs) RX has 
 * accumulated. 
 *  
 * This is done by reading RX_LINE_COUNT register. This register 
 * will reset to zero automatically at SOF. 
 * 
 * @param vfe_id: vfe id of current processing thread
 * @param linecount: the line number to be waited for
 * 
 * @return unsigned int: will return linecount + 1
 */
unsigned int process_util_rx_wait_for_line(unsigned int vfe_id, unsigned int linecount);

/**
 * @brief inform RX hardware that certain line buffer is 
 *        consumed by algorithm
 *  
 * This is done by programming RX_INDEX_CONSUMED register, 
 * however normally RX will write data into L2 regardlessly of 
 * this register's value. 
 *  
 * if data processing speed is slower than data streaming-in 
 * speed, RX overrun will happen and there will be data loss. 
 *  
 * 
 * @param vfe_id: vfe id of current processing thread
 * @param offset: the start offset of a line in L2 RX buffer 
 *  
 * @return None 
 */
void process_util_rx_done(unsigned int vfe_id, unsigned int offset);

/**
 * @brief tell TX certain line in TX buffer is ready to transmit
 *  
 * This is done by programming TX_INDEX_AVAIL register. However 
 * TX will normally transmit data regardlessly of this register 
 * value.  
 *  
 * if data processing speed is slower than TX transmitting 
 * speed, TX overrun will happen and there will be data loss. 
 *  
 * @param vfe_id: vfe id of current processing thread
 * @param offset: offset of data to transmit in TX 
 *  
 * @return None 
 */
void process_util_tx_done(unsigned int vfe_id, unsigned int offset);

/**
 * @brief wait until EOF bit got set in RAW_STATUS register 
 * 
 * @param vfe_id: vfe id of current processing thread 
 *  
 * @return None 
 */
void process_util_tx_wait_for_eof(unsigned int vfe_id);

/**
 * @brief clear EOF bit in RAW_STATUS register 
 *  
 * regsiter bit won't clear by itself, so use this API to clear 
 * the EOF bit so we can get following EOF signals
 * 
 * @param vfe_id: vfe id of current processing thread 
 *  
 * @return None 
 */
void process_util_tx_clear_eof(unsigned int vfe_id);

/**
 * @brief incrementor with wrap-around functionality 
 *  
 * if the input idx is equal or greater than bufsize, idx will 
 * wrap-around to zero. 
 * 
 * @param idx: idx to be checked
 * @param bufsize: wrap-around point
 * 
 * @return unsigned int: idx after wrap-around
 */
unsigned int process_util_wrap_idx(unsigned int idx, unsigned int bufsize);

/**
 * @brief check if dump flag is set
 *  
 * if set, frame dump is needed, otherwise, not.
 * 
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return unsigned int: the dump flag value
 */
unsigned int process_util_get_dump_flag(unsigned int vfe_id);

/**
 * @brief check if stats flag is set 
 *  
 * if set, stats dump is needed, otherwise, not
 * 
 * @param vfe_id: vfe id of current processing thread
 * 
 * @return unsigned int: the stats flag value
 */
unsigned int process_util_get_stats_flag(unsigned int vfe_id);

/**
 * @brief streamer recovery call
 * 
 * in case overflow happens, call this API, it will do:
 *   stop streamer
 *   reset streamer
 *   reprogram streamer with previous values
 *   start streamer
 *  
 * @param vfe_id: vfe id of current processing thread 
 *  
 * @return None 
 */
void process_util_recover_streamer(unsigned int vfe_id);
hvx_q_buffer_t* hvx_queue_deque(hvx_queue_t * p_q);
#endif
