/*========================================================================*/
/**
@file IEC61937.h

@brief IEC61937 header file.

This file contains IEC 61937 related utilities.
*/

/*========================================================================
Copyright (c) 2009-2013 Qualcomm Technologies, Incorporated.  All rights reserved.
Qualcomm Confidential and Proprietary.  Export of this technology or software is regulated
by the U.S. Government, Diversion contrary to U.S. law prohibited.
====================================================================== */

/*========================================================================
Edit History

$Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/test/audio/test_capi_v2_voice/src/IEC61937.h#1 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
4/9/2013    rbhatnk      Created file.
========================================================================== */


#ifndef _IEC61937_H_
#define _IEC61937_H_

#if defined CAPI_V2_STANDALONE
#include "capi_v2_test.h"
#include "capi_v2_util.h"
#elif defined APPI_EXAMPLE_STANDALONE
#include "appi_test.h"
#include "appi_util.h"
#else
#include "qurt_elite_types.h"
#include "adsp_error_codes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


/* =======================================================================
INCLUDE FILES FOR MODULE
========================================================================== */
/** @addtogroup iec61937
@{ */

/* -----------------------------------------------------------------------
** Global definitions/forward declarations
** ----------------------------------------------------------------------- */
#define IEC61937_BYTES_PER_IEC60958_FRAME  4
/** samples from 2 channels together. */
#define IEC61937_NUM_SAMPLES_PER_IEC60958_FRAME  2
#define IEC61937_BYTES_PER_IEC_60958_SAMPLE  (IEC61937_BYTES_PER_IEC60958_FRAME/IEC61937_NUM_SAMPLES_PER_IEC60958_FRAME)

/** returns the pause repitition period for the corresponding format.
 */
uint32_t IEC61937_get_pause_rep_period(uint32_t format);

/** rounds up given number of IEC60958 samples to a multiple of pause repetition period of the
 * format.
 */
uint32_t IEC61937_align_samples_to_pause_repetition_period(uint32_t samples, uint32_t format_id);

/** rounds up given number of bytes to a multiple of pause repetition period of the
 * format.
 */
uint32_t IEC61937_align_bytes_to_pause_repetition_period(uint32_t bytes, uint32_t format_id);
/**
 * 60958 samples per channel to bytes (for all channels)
 */
uint32_t IEC61937_60958_samples_to_bytes(uint32_t samples, uint32_t num_channels);

/**
 * bytes for all channels to samples per channel.
 */
uint32_t IEC61937_bytes_to_60958_samples(uint32_t bytes, uint32_t num_channels);

/**
 * finds the first sync word location (in terms of byte index)
 */
ADSPResult IEC61937_get_first_syncword_locn(uint8_t *data_buf_ptr,
                                                         uint32_t buf_size,
                                                         uint32_t *locn);
/**
 * finds the first sync word location in terms of byte index.
 * The location will be after the specified limit.
 */
ADSPResult IEC61937_get_syncword_locn_after_limit(uint8_t *data_buf_ptr,
                                                         uint32_t buf_size,
                                                         uint32_t limit,
                                                         uint32_t *locn);
/**
 * fills pause specified samples per channel of bursts if possible
 * (constraints: out buf size & integral multiple of pause repetition period)
 */
void IEC61937_fill_pause_bursts(uint32_t format_id,
                                     uint8_t *data_buf_ptr,
                                     uint32_t *buf_size,
                                     uint32_t *num_pause_samples_per_ch,
                                     uint32_t num_channels);

/**
 * fills pause burst for the entire buffer size provided. pause_burst_start_sample_index will
 * indicate the position to start in the pause burst. Also updates the same pointer with
 * number of samples filled of a pause burst in the current frame of buffer.
 */
void IEC61937_fill_misaligned_pause_bursts(uint32_t format_id,
                                     	uint8_t *data_buf_ptr,
                                        uint32_t buf_size,
                                        uint32_t *pause_burst_start_sample_index);

/**
 * buffer size requirement depending on format id.
 */
ADSPResult IEC61937_get_buffer_size(uint32_t fmt, uint32_t *buf_size);

/*Find the next syncword and update the current location,
 * note it preserves the initial position value
 * Used for 16-bit pointer only
*/

ADSPResult IEC61937_get_next_syncword_locn_and_update(uint16_t *inputbuf,
      const uint32_t bufsize,
      uint32_t *samples_read);
/*
 * Parse the Pc value in the compressed header
 * to tell the media format and repetition period
 */
ADSPResult IEC61937_parse_media_info(uint16_t Pc, uint32_t *samples_per_frame, uint32_t *media_format);

/** @} */ /* end_addtogroup iec61937 */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // #ifndef _IEC61937_H_

