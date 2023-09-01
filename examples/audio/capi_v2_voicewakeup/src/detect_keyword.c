/* ======================================================================== */
/**
   @file detect_keyword.c

   C source file to implement keyword detection.
 */

/*==============================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "detect_keyword.h"
#include "capi_v2_voicewakeup.h"
#include "Elite_CAPI_V2_types.h"

#include "Elite_fwk_extns_lab.h"
#include "Elite_fwk_extns_detection_engine.h"

/*------------------------------------------------------------------------------
  Function name: detect_keyword

  Performs trivial detection algorithm on input stream sample..
  For this example algorithm, the SoundModel data assumed to contain two fields:
      16-bit sample amplitude Threshold
      16-bit Duration in milliseconds
  This very simple algorithm looks for a consecutive frames of input samples
  spanning over the given Duration whose approximated RMS stays above the
  given amplitude Threshold.

  The actual number of samples that must be above the threshold is calculated
  using a percentage of this given Duration times the minimum confidence
  level set as a parameter and rounds down to a multiple of 10ms frame size
  (160 samples).

  This algorithm uses a very simple approximation of RootMeanSquare by just
  taking the average the sum of the absolute values of each sample of
  each frame.

  The function will be called with 10ms frame(s) of audio data.

  Only a single confidence level (for a single keyword model) is used.
  If more than one is set, the rest are ignored.

  When criteria is met this module sends several CAPI_V2_EVENT_DATA_TO_DSP_SERVICE
  events:
       CAPI_V2_EVENT_DATA_TO_DSP_SERVICE with paramId LSM_VOICE_WAKEUP_STATUS_DISCOVERY
       CAPI_V2_EVENT_DATA_TO_DSP_SERVICE with paramId LSM_VOICE_WAKEUP_STATUS_DETECTION,
       CAPI_V2_EVENT_DATA_TO_DSP_SERVICE with paramId PARAM_ID_KW_END_POSITION
  In this sample implementation, DISCOVERY event is sent when "detection" is successful
  then immediately after this DETECTION event is sent.

  Certain algorithms may be capable of detecting the Keyword End position as well.
  Usually by the time Keyword is detected, Keyword end might have already been
  surpassed due to algorithm delay and other algorithm factors. So in order not
  to lose any data between the Keyword End Position and the actual keyword detection
  an intermediate STATE called “DISCOVERY” is defined. The framework will start
  buffering the data once the DetectionEvent of LSM_VOICE_WAKEUP_STATUS_DISCOVERY
  is received.

  Once Keyword is detected, algorithm gives the Keyword End Position back to framework.
  This Keyword End Position given by algorithm will be in samples from the current frame.
  So suppose if Keyword End Position is 13 then it means that Keyword End is 13 samples
  back from the current frame.  Then Framework when performing LookAheadBuffering
  will flush the older data and keeps only Keyword End frames amount of data in
  it internal buffer.

  It is not mandated that an implementation send this DISCOVERY event.
  If not sent while Look-Ahead Buffering is enabled, then the framework will start
  buffering after detection event is received, and module may return Keyword End
  position as Zero.

  Multiple detections could occur with a single input stream.  So even after
  detection, remaining input buffer continues to be processed until completely consumed.

  LSM_VOICE_WAKEUP_STATUS_REJECTED ("no key word present") events are not process
  by this simple algorithm.
------------------------------------------------------------------------------ */

capi_v2_err_t detect_keyword(capi_v2_voicewakeup_t* me_ptr,
                             capi_v2_buf_t* inp_buf_ptr)
{
	capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

    uint32_t inp_indx = 0;   // input buffer sample index
    int16_t* p_inp_buf_short = (int16_t*)inp_buf_ptr->data_ptr;
    uint32_t input_buf_size = inp_buf_ptr->actual_data_len / 2; // samples in frame (in shorts)
    uint32_t num_frames_to_compare = me_ptr->detection_config.num_frames_to_compare;
	detection_engine_status_t algo_status;
	capi_v2_voicewakeup_events_config_t event_data;
	capi_v2_voicewakeup_sound_model_header_t * soundModelHeader = (capi_v2_voicewakeup_sound_model_header_t *)me_ptr->params_config.sound_model_ptr;
    int16_t  threshold_value = soundModelHeader->threshold;
    uint32_t absoluteValues = 0;
    uint32_t approxRMS = 0;

	// calculate and save target number of frames to look for being over threshold
    if (0 == num_frames_to_compare) {
    	num_frames_to_compare =  (soundModelHeader->duration_over_threshold *
    			                  me_ptr->params_config.conf_levels[0]) /
    	                         (100 * FRAME_DURATION_MS);
    	if (0 == num_frames_to_compare) {
	    	FARF(ERROR, "CAPIV2 SVA process called before ready");
            return CAPI_V2_EFAILED;
    	} else {
    		me_ptr->detection_config.frame_count = 0;
    		me_ptr->detection_config.num_frames_to_compare = num_frames_to_compare;
      	    FARF(HIGH, "detect_keyword : confLevel = %d, num_frames_to_compare = %d              ",
      	    		me_ptr->params_config.conf_levels[0], num_frames_to_compare);
    	}
    }
    
    // calculate approximated RMS of all samples in this frame
    // Sum absolute value of all samples /
    for (inp_indx=0; inp_indx < input_buf_size; inp_indx++)
    {
    	if (p_inp_buf_short[inp_indx] > 0)
        	absoluteValues += p_inp_buf_short[inp_indx];
    	else
        	absoluteValues -= p_inp_buf_short[inp_indx];
    }
    approxRMS = absoluteValues / input_buf_size;
    if (approxRMS >= threshold_value)
	    me_ptr->detection_config.frame_count++;
    else
	    me_ptr->detection_config.frame_count = 0;  // restart

    if (me_ptr->detection_config.frame_count >= num_frames_to_compare)
    {
    	/*
    	 * detection criteria met
    	 */
        // send PARAM_ID_DETECTION_ENGINE_STATUS:LSM_VOICE_WAKEUP_STATUS_DISCOVERY event
        algo_status.is_debug_mode = FALSE;
        algo_status.status        = LSM_DETECTION_STATUS_DISCOVERY;
        algo_status.payload_size  = 0;
        algo_status.payload_ptr   = NULL;
        event_data.type = CAPI_V2_EVENT_DATA_TO_DSP_SERVICE;
        event_data.param_id = PARAM_ID_DETECTION_ENGINE_STATUS;
        event_data.payload_ptr = (void *)&algo_status;
        event_data.payload_size = (uint8_t)sizeof(detection_engine_status_t);
        FARF(HIGH, "detect_keyword : send DATA_TO_DSP_SERVICE: PARAM_ID_DETECTION_ENGINE_STATUS - DISCOVERY event    ");
        capi_v2_voicewakeup_raise_event(me_ptr, &event_data);

        // send PARAM_ID_DETECTION_ENGINE_STATUS:LSM_VOICE_WAKEUP_STATUS_DETECTED event
    	// set output confidence level to a value >= min_confidence_level
    	int8_t confidence_levels[1];
    	confidence_levels[0] = me_ptr->params_config.conf_levels[0] + 1;
        algo_status.is_debug_mode = FALSE;
        algo_status.status        = LSM_DETECTION_STATUS_DETECTED;
        algo_status.payload_size  = sizeof(confidence_levels);
        algo_status.payload_ptr   = confidence_levels;
  	    FARF(HIGH, "detect_keyword : send DATA_TO_DSP_SERVICE: PARAM_ID_DETECTION_ENGINE_STATUS - DETECT event         ");
  	    FARF(HIGH, "detect_keyword : conf levels ptr %p should match ptr in event payload    ",
  	    		confidence_levels);

   	    event_data.type = CAPI_V2_EVENT_DATA_TO_DSP_SERVICE;
  	    event_data.param_id = PARAM_ID_DETECTION_ENGINE_STATUS;
   	    event_data.payload_ptr = (void *)&algo_status;
  	    event_data.payload_size = (uint8_t)sizeof(detection_engine_status_t);
  	    capi_v2_voicewakeup_raise_event(me_ptr, &event_data);

        // send PARAM_ID_KW_END_POSITION event to framework
        /* Indicates the keyword end position in samples from the end of this current buffer
  	     * For example, in the Snapdragon™ Voice Activation (SVA) solution, this parameter is used
  	     * to omit the keyword before sending sending buffers to the HLOS.
   	     */
  	    kw_end_position_t kw_end_position;
  	    memset(&kw_end_position,0,sizeof(kw_end_position_t));
  	    // %%% me_ptr->params_config.epd_end_threshold used to derive samples?
  	    kw_end_position.kw_end_position_samples = input_buf_size - inp_indx;

  	    event_data.param_id = PARAM_ID_KW_END_POSITION;
  	    event_data.payload_ptr = (void *)&kw_end_position;
  	    event_data.payload_size = (uint8_t)sizeof(kw_end_position_t);
  	    FARF(HIGH, "detect_keyword : send DATA_TO_DSP_SERVICE: PARAM_ID_KW_END_POSITION event         ");
  	    FARF(HIGH, "detect_keyword : EPD samples 0x%x is in event payload    ",
  	    		kw_end_position.kw_end_position_samples);
  	    capi_v2_voicewakeup_raise_event(me_ptr, &event_data);

  	    me_ptr->detection_config.frame_count = 0; // reset and continue looking
   	    me_ptr->detection_config.number_of_detections++;
  		FARF(HIGH, "CAPIV2 SVA process: %d-th detection                       ",
  				me_ptr->detection_config.number_of_detections);

    }


    return capi_v2_result;
}

