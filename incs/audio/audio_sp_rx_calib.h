#ifndef SP_Rx_V2_CALIB_H
#define SP_Rx_V2_CALIB_H

/*==============================================================================
  @file audio_sp_rx_calib.h
  @brief This file contains SP_Rx
==============================================================================*/
/*=======================================================================
* Copyright (c) 2018 Qualcomm Technologies, Inc.  All Rights Reserved.
* Confidential and Proprietary – Qualcomm Technologies, Inc.
=========================================================================*/
/*========================================================================
 Edit History

 when       who     what, where, why
 --------   ---     -------------------------------------------------------
 05/25/18   Phanidhar  Added h2xml comments.
 ========================================================================== */
#include "mmdefs.h"


#define SP_RX_PARAM_MOD_ENABLE 0x11111611
#include "adsp_begin_pack.h"

typedef struct sp_rx_enable_cfg_t sp_rx_enable_cfg_t;
/** @h2xmlp_parameter    {"SP_RX_PARAM_MOD_ENABLE", SP_RX_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct sp_rx_enable_cfg_t {

    uint16_t enable_flag;
    /**< @h2xmle_description {Enables/disables this feature}
         @h2xmle_rangeList   {"Disable"= 0;
                              "Enable"=1}
         @h2xmle_default     {0}

         @h2xmle_policy      {Basic} */

    uint16_t reserved;
    /**< @h2xmle_description {Client should set this field to zero.}
         @h2xmle_default     {0}
         @h2xmle_readOnly    {true}
         @h2xmle_policy      {Basic} */

}
#include "adsp_end_pack.h"
;

#define SP_RX_CFG_1_PARAM_ID 0x11111612
typedef struct sp_rx_lib_config_1_t sp_rx_lib_config_1_t;
/** @h2xmlp_parameter    {"SP_RX_PARAM_MOD_ENABLE", SP_RX_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct sp_rx_lib_config_1_t {

    uint32_t data_1;
    /**< @h2xmle_description {Enables or disables a module}
         @h2xmle_rangeList   {"Disable"= 0;
                              "Enable"=1}
         @h2xmle_default     {0}

         @h2xmle_policy      {Basic} */

    uint32_t data_2;
    /**< @h2xmle_description {Client should set this field to zero.}
         @h2xmle_default     {0}
         @h2xmle_readOnly    {true}
         @h2xmle_policy      {Basic} */

}
#include "adsp_end_pack.h"
;

#define SP_Rx_V2_MODULE  0x11111610
/**
    @h2xmlm_module       	{"SP_Rx_V2_MODULE",
                          	  SP_Rx_V2_MODULE}
    @h2xmlm_displayName  	{"AUDIO SP_Rx"}
	@h2xmlm_description 	{ SP in the audio preprocessor path.\n
						 	  This module supports the following parameter IDs \n
						 	  SP_RX_PARAM_MOD_ENABLE\n
						 	  \n}
   @h2xmlm_ToolPolicy 		{Calibration; RTC}
   @{                   <-- Start of the Module -->

   @h2xml_Select			{sp_rx_enable_cfg_t}
   @h2xmlm_InsertParameter


   @}                   <-- End of the Module -->*/

#endif // SP_Rx_V2_CALIB_H
