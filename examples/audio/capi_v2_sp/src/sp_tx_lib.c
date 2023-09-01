/* =========================================================================
 Copyright (c) 2015 QUALCOMM Technologies Incorporated.
 All rights reserved. Qualcomm Technologies Proprietary and Confidential.
 ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef _DEBUG  // To enable FARF messages
#define _DEBUG
#endif

#include "sp_tx_lib.h"

sp_lib_err_t sp_tx_get_fb_data_per_spkr_mem_req(sp_tx_lib_config_t *tx_stat_cfg_ptr,
                                                uint32 *fb_data_memsize_ptr)
{
   // Calculate the size per spkr
   *fb_data_memsize_ptr = 20;
   return SP_LIB_EOK;
}

sp_lib_err_t sp_tx_init_fb_data_per_spkr(sp_tx_lib_config_t *tx_stat_cfg_ptr, int8_t *fb_data_ptr,
                                         uint32 memory_size)
{

   // Initialize the memory with any req. from the library
   memset(fb_data_ptr, 0, memory_size);

   // Dummy Data init
   fb_data_ptr[0] = 0xD;
   fb_data_ptr[1] = 0xE;
   fb_data_ptr[2] = 0xA;
   fb_data_ptr[3] = 0xD;
   fb_data_ptr[4] = 0xB;
   fb_data_ptr[5] = 0xA;
   fb_data_ptr[6] = 0xB;
   fb_data_ptr[7] = 0xE;

   return SP_LIB_EOK;
}

sp_lib_err_t sp_tx_process(sp_tx_memory_t *sp_tx_memory_ptr, int16_t *vsen_ptr[MAX_SP_SPKRS],
                           int16_t *isen_ptr[MAX_SP_SPKRS], int32_t num_samples,
                           int8_t *fb_data_ptr[MAX_SP_SPKRS])
{
   return SP_LIB_EOK;
}
