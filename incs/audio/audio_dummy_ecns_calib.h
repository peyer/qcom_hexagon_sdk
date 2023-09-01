#ifndef CAPI_V2_DUMMY_ECNS_CALIB_H
#define CAPI_V2_DUMMY_ECNS_CALIB_H

/*==============================================================================
  @file audio_dummy_encs_calib.h
  @brief This file contains DUMMY_ECNS
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


#define DUMMY_ECNS_PARAM_MOD_ENABLE 0x11111501
#include "adsp_begin_pack.h"

typedef struct dummy_ecns_param_mod_enable_t dummy_ecns_param_mod_enable_t;
/** @h2xmlp_parameter    {"DUMMY_ECNS_PARAM_MOD_ENABLE", DUMMY_ECNS_PARAM_MOD_ENABLE}
    @h2xmlp_description {Enables or disables a module.} */

struct dummy_ecns_param_mod_enable_t {

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

#define DUMMY_ECNS_V2_MODULE  0x11111500
/**
    @h2xmlm_module       	{"DUMMY_ECNS_V2_MODULE",
                          	  DUMMY_ECNS_V2_MODULE}
    @h2xmlm_displayName  	{"AUDIO DUMMY_ECNS"}
	@h2xmlm_description 	{ Dummy_encs in the audio preprocessor path.\n
						 	  This module supports the following parameter IDs \n
						 	  DUMMY_ECNS_PARAM_MOD_ENABLE\n
						 	\n}
   @h2xmlm_ToolPolicy 		{Calibration; RTC}
   @{                   <-- Start of the Module -->

   @h2xml_Select			{dummy_ecns_param_mod_enable_t}
   @h2xmlm_InsertParameter

   @}                   <-- End of the Module -->*/

#endif // CAPI_V2_DUMMY_ECNS_CALIB_H
