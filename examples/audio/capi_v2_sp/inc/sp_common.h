/*-----------------------------------------------------------------------
   Copyright (c) 2012-2015 Qualcomm  Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------*/

#ifndef SP_COMMON_H_
#define SP_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* This is the number of buffers to use for feedback communication */
#define NUM_CXDATA_BUFFERS 3

/* Number of speakers that is supported by Speaker Protection */
#define MAX_SP_SPKRS 2

/* Following are prototype for algorithm error status */
typedef uint32_t sp_lib_err_t;

/** Success. The operation completed with no errors. */
#define SP_LIB_EOK 0
/** General failure. */
#define SP_LIB_EFAILED             ((uint32_t)1)



/** @brief To represent the Tx spkr type to the Rx processing.
 */
typedef enum sp_spkr_cfg_t
{
   SPKR_L   = 0,
   SPKR_R,
   SPKR_LR, /* fb the buffers and set into this format for 2 spkr configuration */
   SPKR_MAX = (0xFFFFFFFF)  /* Maximum value to ensure that the enum is 4 bytes long */
}sp_spkr_cfg_t;

/**
 * Payload for fb data received from Tx thermal blocks by Speaker protection Rx processing
 */
typedef struct sp_v2_tx_fb_data_payload_t
{
   /**< Spkr config */
   sp_spkr_cfg_t       spkr_cfg;
   /**< Beginning of the control data buffer. */
   int8_t                 *fb_data_ptr[MAX_SP_SPKRS];
}sp_v2_tx_fb_data_payload_t;


#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* SP_COMMON_H_ */
