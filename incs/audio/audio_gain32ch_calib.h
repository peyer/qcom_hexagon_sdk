#ifndef GAIN_32CH_V2_CALIB_H
#define GAIN_32CH_V2_CALIB_H

/*==============================================================================
  @file audio_gain32ch_calib.h
  @brief This file contains GAIN
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


#define GAIN_32CH_PARAM_MOD_ENABLE 0x11111401
#include "adsp_begin_pack.h"

typedef struct gain_param_mod_enable_t gain_param_mod_enable_t;
/** @h2xmlp_parameter    {"GAIN_32CH_PARAM_MOD_ENABLE", GAIN_32CH_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct gain_param_mod_enable_t {

    uint16_t enable;
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


#define GAIN_32CH_PARAM_MASTER_GAIN 0x11111402
#include "adsp_begin_pack.h"

typedef struct gain_ctrl_master_gain_t gain_ctrl_master_gain_t;
/** @h2xmlp_parameter   {"GAIN_32CH_PARAM_MASTER_GAIN", GAIN_32CH_PARAM_MASTER_GAIN}
    @h2xmlp_description {Gain algorithm.\n
						 Fixed size for this parameter \n
 	 	 	 	 	 	 Version 0 - 4 bytes\n\n} */

struct gain_ctrl_master_gain_t {

    uint16_t master_gain;
    /**< @h2xmle_description {Controls audio gain32ch}
         @h2xmle_default     {0x2000}
         @h2xmle_range       {0..0xFFFF}

         @h2xmle_policy       {Basic} */

    int16_t reserved;
    /**< @h2xmle_description {Client must this field to zero.}
         @h2xmle_default     {0}
	     @h2xmle_readonly	 {true} */

}
#include "adsp_end_pack.h"
;


#define GAIN_32CH_V2_MODULE  0x11111400
/**
    @h2xmlm_module       	{"GAIN_32CH_V2_MODULE",
                          	  GAIN_32CH_V2_MODULE}
    @h2xmlm_displayName  	{"AUDIO GAIN32ch"}
	@h2xmlm_description 	{ Gain32ch in the audio preprocessor path.\n
						 	  This module supports the following parameter IDs \n
						 	  GAIN_32CH_PARAM_MOD_ENABLE\n
						 	  GAIN_32CH_PARAM_MASTER_GAIN
						 	\n}
   @h2xmlm_ToolPolicy 		{Calibration; RTC}
   @{                   <-- Start of the Module -->

   @h2xml_Select			{gain_param_mod_enable_t}
   @h2xmlm_InsertParameter

   @h2xml_Select			{gain_ctrl_master_gain_t}
   @h2xmlm_InsertParameter

   @}                   <-- End of the Module -->*/

#endif // GAIN_32CH_V2_CALIB_H
