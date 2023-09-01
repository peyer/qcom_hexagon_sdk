/* =========================================================================
 Copyright (c) 2015 QUALCOMM Technologies Incorporated.
 All rights reserved. Qualcomm Technologies Proprietary and Confidential.
 ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef SP_TX_LIB_H
#define SP_TX_LIB_H

#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "HAP_farf.h"
#include "HAP_mem.h"
#include "string.h"
#include "sp_common.h"
#include "adsp_api.h"
#include "audio_sp_tx_calib.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Prototype structure of the algorithm library */
typedef struct sp_tx_memory
{
   int8_t* lib_ptr;
} sp_tx_memory_t;

/* Prototype structure of the algorithm configuration parameters */
typedef struct sp_tx_lib_config
{
   sp_tx_lib_config_1_t cfg1;
} sp_tx_lib_config_t;

/* Prototype function to obtain the size of the memory needed for feedback buffer per speaker */
sp_lib_err_t sp_tx_get_fb_data_per_spkr_mem_req(sp_tx_lib_config_t *tx_stat_cfg_ptr,
                                                uint32_t *fb_data_memsize_ptr);

/* Prototype function to initialize the feedback data buffer, it is optional */
sp_lib_err_t sp_tx_init_fb_data_per_spkr(sp_tx_lib_config_t *tx_stat_cfg_ptr, int8_t *fb_data_ptr,
                                         uint32 memory_size);

/* Prototype function for the algorithm process function */
sp_lib_err_t sp_tx_process(sp_tx_memory_t *sp_tx_memory_ptr, int16_t *vsen_ptr[MAX_SP_SPKRS],
                           int16_t *isen_ptr[MAX_SP_SPKRS], int32_t num_samples,
                           int8_t *fb_data_ptr[MAX_SP_SPKRS]);

#ifdef __cplusplus
}
#endif
#endif /* SP_TX_LIB_H */
