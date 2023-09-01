#ifndef PASSTHRU_V2_CALIB_H
#define PASSTHRU_V2_CALIB_H

/*==============================================================================
  @file audio_passthru_calib.h
  @brief This file contains PASSTHRU
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


#define PASSTHRU_PARAM_MOD_ENABLE 0x11111201
#include "adsp_begin_pack.h"

typedef struct passthru_param_mod_enable_t passthru_param_mod_enable_t;
/** @h2xmlp_parameter   {"PASSTHRU_PARAM_MOD_ENABLE", PASSTHRU_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct passthru_param_mod_enable_t {

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

#define PASSTHRU_V2_MODULE  0x11111200
/**
    @h2xmlm_module       	{"PASSTHRU_V2_MODULE",
                          	  PASSTHRU_V2_MODULE}
    @h2xmlm_displayName  	{"AUDIO PASSTHRU"}
	@h2xmlm_description 	{ passthru in the audio preprocessor path.\n
						 	  This module supports the following parameter IDs \n
						 	  PASSTHRU_PARAM_MOD_ENABLE\n
						 	\n}
   @h2xmlm_ToolPolicy 		{Calibration; RTC}
   @{                   <-- Start of the Module -->

   @h2xml_Select			{passthru_param_mod_enable_t}
   @h2xmlm_InsertParameter


   @}                   <-- End of the Module -->*/

#endif // PASSTHRU_V2_CALIB_H
