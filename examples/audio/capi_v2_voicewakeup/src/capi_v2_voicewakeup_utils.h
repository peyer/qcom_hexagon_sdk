/* ======================================================================== */
/**
   @file capi_v2_voicewakeup_utils.h

   Header file to implement utility functions for CAPIv2 Interface for
   VoiceWakeup example
 */

/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef CAPI_V2_VOICEWAKEUP_UTILS_H
#define CAPI_V2_VOICEWAKEUP_UTILS_H

#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "HAP_farf.h"
#include "HAP_mem.h"
#include "string.h"

#ifdef __cplusplus
extern "C"
{
#endif

// forward declaration of function in test_utils.c
size_t memscpy(void* dst, size_t dst_size, const void* src, size_t src_size);

#define CAPI_V2_VOICEWAKEUP_MAX_IN_PORTS     1
#define CAPI_V2_VOICEWAKEUP_MAX_OUT_PORTS    0
#define CAPI_V2_VOICEWAKEUP_MAX_CHANNELS     1

/* A sound model can contain data for one or more keyword phrases
 * and optional individual users.
 * Denotes the maximum confidence levels the module can handle.
 * This should be sum of all active keyword phrases and user data defined
 * within the active sound model.
 * For this example implementation SoundModel contains data for a single keyword.
 */
#define MAX_CONFIDENCE_LEVELS 1

#define VW_STACK_SIZE                        2048  // stack size for custom module
#define PCM_16BIT_Q_FORMAT                   15
// 10 msec 16 Khz mono
#define BLOCK_SIZE_IN_SAMPLES                160
// 2 bytes per sample
#define BYTES_PER_SAMPLE                     2
// 10 msec 16 Khz mono, 160*2 =320 bytes
#define BLOCK_SIZE_IN_BYTES                  BLOCK_SIZE_IN_SAMPLES*BYTES_PER_SAMPLE

/*
 * Constants for KPS and Bandwidth calculations
 */
//scaling factor to convert to kilo
#define SCALING_KILO                         1000
//scaling factor to convert from bytes to bits
#define SCALING_BYTE_TO_BIT                  8
//scaling factor to convert from bits to kilo bits
#define SCALING_BITS_TO_KILO_BITS            1024
// sampling rate that Voice Wake Up algo supports
#define SAMPLING_RATE                        16000
//Bits per sample
#define BITS_PER_SAMPLE                      16
//Duration taken by this VoiceWakeup_V2 module to process a frame
#define FRAME_DURATION_MS          			 10
//Maximum models supported by algo
#define SVA_MAX_REGISTERED_MODELS            1
// VoiceWakeup algo frame duration in msec
#define VW_FRAME_SHIFT                       10
//scaling factor to calculate number of frames in a sec
#define SCALING_FRAME_DURATION_TO_SECONDS    (1000/VW_FRAME_SHIFT) // VW_FRAME_SHIFT is in ms
// samples per 1 millisecond and frame (given 16KHz,mono data)
#define SAMPLES_PER_MS                       16
#define SAMPLES_PER_FRAME					 (SAMPLES_PER_MS * FRAME_DURATION_MS)

typedef struct capi_v2_voicewakeup_media_fmt
{
	capi_v2_set_get_media_format_t main;
	capi_v2_standard_data_format_t std_fmt;

} capi_v2_voicewakeup_media_fmt_t;

// data structure passed to capi_v2_voicewakeup_raise_event()
typedef struct capi_v2_voicewakeup_events_config
{
	capi_v2_event_id_t type;  // event type: CAPI_V2_EVENT_KPPI, _BANDWIDTH, _DATA_TO_DSP
	uint32_t param_id;        // event sub-type - zero for most events
	                          // use detection event values are:
							  //    LSM_VOICE_WAKEUP_STATUS_NONE       0x0
	  	  	  	  	  	  	  //    LSM_VOICE_WAKEUP_STATUS_DETECTED   0x2
	  	  	  	  	  	  	  //    LSM_VOICE_WAKEUP_STATUS_DISCOVERY  0x5
	uint8_t  payload_size;    // size of payload data
	void *   payload_ptr;     // ptr to payload data
} capi_v2_voicewakeup_events_config_t;

typedef struct capi_v2_voicewakeup_detection_config
{
	uint32_t frame_count;              // current count of samples over threshold
	uint32_t num_frames_to_compare;  // percentage of total samples to compare calculated
	                                   //  using min_confidence level and SM time_to_compare
	uint32_t number_of_detections;
} capi_v2_voicewakeup_detection_config_t;


typedef struct capi_v2_voicewakeup_params_config
{
	uint8_t operation_mode;			// operation mode flag set: MODE_KEYWORD_DETECTION,...
	int16_t gain;					// gain parameter to compensate the audio path gain mismatch [L16Q8]
	int32_t epd_begin_threshold;	// end point detection start threshold [L32Q20]
	int32_t epd_end_threshold;		// end point detection end threshold [L32Q20]
    // minimum confidence levels for each keyword/user model in soundmodel
	uint8_t num_conf_levels;        // number of elements in confidence level array that are set
    uint8_t conf_levels[MAX_CONFIDENCE_LEVELS];
	uint8_t* sound_model_ptr;       // SoundModel Data ptr
	uint32_t sound_model_size;      // in bytes
} capi_v2_voicewakeup_params_config_t;

typedef struct capi_v2_voicewakeup
{
	capi_v2_t                               vtbl;
	capi_v2_event_callback_info_t           cb_info;
	capi_v2_voicewakeup_media_fmt_t         input_media_fmt[CAPI_V2_VOICEWAKEUP_MAX_IN_PORTS];
	capi_v2_voicewakeup_detection_config_t  detection_config;
	capi_v2_voicewakeup_params_config_t     params_config;
} capi_v2_voicewakeup_t;

/* example SoundModel contains threshold value and
 * time input samples should be great than this threshold before detection is raised.
 */
typedef struct capi_v2_voicewakeup_sound_model_header
{
	uint16_t           threshold;
	uint16_t           duration_over_threshold;  // saved in milliseconds
} capi_v2_voicewakeup_sound_model_header_t;

capi_v2_err_t  capi_v2_voicewakeup_raise_event(capi_v2_voicewakeup_t *me_ptr, capi_v2_voicewakeup_events_config_t *event_config);

bool_t capi_v2_voicewakeup_is_ready_to_process(capi_v2_voicewakeup_t* me_ptr);

void capi_v2_voicewakeup_get_kpps(capi_v2_voicewakeup_t* me_ptr, uint32_t *kpps );
void capi_v2_voicewakeup_get_bandwidth(capi_v2_voicewakeup_t* me_ptr,
		uint32_t * code_bandwidth, uint32_t * data_bandwidth);

void capi_v2_voicewakeup_init_media_fmt(capi_v2_voicewakeup_t *me_ptr);
void capi_v2_voicewakeup_release_memory(capi_v2_voicewakeup_t *me_ptr);

#ifdef __cplusplus
}
#endif
#endif /* CAPI_V2_VOICEWAKEUP_UTILS_H */
