/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef SP_RX_LIB_H
#define SP_RX_LIB_H

#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "HAP_farf.h"
#include "HAP_mem.h"
#include "string.h"
#include "sp_common.h"
#include "adsp_api.h"
#include "audio_sp_rx_calib.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* Prototype structure of the algorithm library */
typedef struct sp_rx_memory
{
   int8_t* lib_ptr;
} sp_rx_memory_t;

/* Prototype structure of the algorithm configuration parameters */
typedef struct sp_rx_lib_config
{
   sp_rx_lib_config_1_t cfg1;
} sp_rx_lib_config_t;


/* Prototype function for the algorithm process function */
sp_lib_err_t sp_rx_process(
      sp_rx_memory_t *sp_rx_memory_ptr,
      int16_t *vsen_ptr[MAX_SP_SPKRS],
      int16_t *isen_ptr[MAX_SP_SPKRS],
      int32_t num_samples,
      int8_t *fb_data_ptr[MAX_SP_SPKRS]
   );

#ifdef __cplusplus
}
#endif
#endif /* SP_RX_LIB_H */
