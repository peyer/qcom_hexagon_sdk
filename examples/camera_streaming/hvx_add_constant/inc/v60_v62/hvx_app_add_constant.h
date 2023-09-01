/***************************************************************************
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************************/
#ifndef _HVX_APP_ADD_CONSTANT_H_
#define _HVX_APP_ADD_CONSTANT_H_

void hvx_add_constant_asm(void* dst,
                          void* src,
                          unsigned int linesize,
                          unsigned int constant,
                          int shift);

void hvx_add_constant(void* p_para);
void hvx_add_constant_get_config(vfe_frame_info_t* frame_info,
                                 adsp_streamer_config_t* streamer_config, adsp_power_voting_t* OEM_voting);

#endif
