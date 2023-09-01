#ifndef DECIMATE_V2_CALIB_H
#define DECIMATE_V2_CALIB_H

/*==============================================================================
  @file audio_decimate_calib.h
  @brief This file contains DECIMATE
==============================================================================*/
/*=======================================================================
* Copyright (c) 2019 Qualcomm Technologies, Inc.  All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================*/
/*========================================================================
 Edit History

 when       who     what, where, why
 --------   ---     -------------------------------------------------------
 05/25/18   Phanidhar  Added h2xml comments.
 ========================================================================== */
#include "mmdefs.h"


#define DECIMATE_PARAM_MOD_ENABLE 0x11111101
#include "adsp_begin_pack.h"

typedef struct decimate_filter_enable_cfg_t decimate_filter_enable_cfg_t;
/** @h2xmlp_parameter   {"DECIMATE_PARAM_MOD_ENABLE", DECIMATE_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct decimate_filter_enable_cfg_t {

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


#define DECIMATE_PARAM_DECIMATION_FACTOR 0x11111102
#include "adsp_begin_pack.h"

typedef struct decimate_factor_cfg_t decimate_factor_cfg_t;
/** @h2xmlp_parameter   {"DECIMATE_PARAM_DECIMATION_FACTOR", DECIMATE_PARAM_DECIMATION_FACTOR}
    @h2xmlp_description {Decimate algorithm.\n
						 Fixed size for this parameter \n
 	 	 	 	 	 	 Version 0 - 4 bytes\n\n} */

struct decimate_factor_cfg_t {

    uint32_t decimation_factor;
    /**< @h2xmle_description {Controls audio decimation}
         @h2xmle_default     {0x0000}
         @h2xmle_range       {0..0xFFFFFFFF}

         @h2xmle_policy      {Basic} */

}
#include "adsp_end_pack.h"
;

#define DECIMATE_V2_MODULE  0x11111100
/**
    @h2xmlm_module       	{"DECIMATE_V2_MODULE",
                          	  DECIMATE_V2_MODULE}
    @h2xmlm_displayName  	{"AUDIO DECIMATE"}
	@h2xmlm_description 	{ decimate in the audio preprocessor path.\n
						 	  This module supports the following parameter IDs \n
						 	  DECIMATE_PARAM_MOD_ENABLE\n
						 	  DECIMATE_PARAM_DECIMATION_FACTOR
						 	\n}
   @h2xmlm_ToolPolicy 		{Calibration; RTC}
   @{                   <-- Start of the Module -->

   @h2xml_Select  			{decimate_filter_enable_cfg_t}
   @h2xmlm_InsertParameter

   @h2xml_Select			{decimate_factor_cfg_t}
   @h2xmlm_InsertParameter

   @}                   <-- End of the Module -->*/

#endif // DECIMATE_V2_CALIB_H
