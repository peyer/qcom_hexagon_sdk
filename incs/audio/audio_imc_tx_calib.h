#ifndef IMC_Tx_V2_CALIB_H
#define IMC_Tx_V2_CALIB_H

/*==============================================================================
  @file audio_IMC_Tx_calib.h
  @brief This file contains IMC_Tx
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


#define IMC_Tx_PARAM_MOD_ENABLE 0x11111711
#include "adsp_begin_pack.h"

typedef struct imc_tx_param_mod_enable_t imc_tx_param_mod_enable_t;
/** @h2xmlp_parameter    {"IMC_Tx_PARAM_MOD_ENABLE", IMC_Tx_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct IMC_Tx_param_mod_enable_t {

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

#define IMC_Tx_V2_MODULE  0x11111710
/**
    @h2xmlm_module       	{"IMC_Tx_V2_MODULE",
                          	  IMC_Tx_V2_MODULE}
    @h2xmlm_displayName  	{"AUDIO IMC_Tx"}
	@h2xmlm_description 	{ voice imc in the audio preprocessor path.\n
						 	  This module supports the following parameter IDs \n
						 	  IMC_Tx_PARAM_MOD_ENABLE\n
						 	  \n}
   @h2xmlm_ToolPolicy 		{Calibration; RTC}
   @{                   <-- Start of the Module -->

   @h2xml_Select			{imc_tx_param_mod_enable_t}
   @h2xmlm_InsertParameter


   @}                   <-- End of the Module -->*/

#endif // IMC_Tx_V2_CALIB_H
