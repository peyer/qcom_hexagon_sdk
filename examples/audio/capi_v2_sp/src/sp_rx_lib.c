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

#include "sp_rx_lib.h"

sp_lib_err_t sp_rx_process(sp_rx_memory_t *sp_rx_memory_ptr, int16_t *vsen_ptr[MAX_SP_SPKRS],
                           int16_t *isen_ptr[MAX_SP_SPKRS], int32_t num_samples,
                           int8_t *fb_data_ptr[MAX_SP_SPKRS])
{
   return SP_LIB_EOK;
}
