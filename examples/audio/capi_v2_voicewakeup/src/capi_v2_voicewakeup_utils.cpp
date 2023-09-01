/* ======================================================================== */
/**
   @file capi_v2_voicewakeup_utils.cpp

   C source file to implement the utility functions for
   CAPIv2 for VoiceWakeup example.
 */

/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/
#include <stdlib.h>

#include "Elite_CAPI_V2_properties.h"
#include "Elite_CAPI_V2.h"
#include "Elite_fwk_extns_lab.h"

#include "capi_v2_voicewakeup_utils.h"
#include "capi_v2_voicewakeup.h"


/*===========================================================================
	Function name: capi_v2_voicewakeup_init_media_fmt
	DESCRIPTION: Function to set the default media format for VoiceWakeup
	during the initialization
===========================================================================*/
void capi_v2_voicewakeup_init_media_fmt(capi_v2_voicewakeup_t *me_ptr)
{
	capi_v2_voicewakeup_media_fmt_t *media_fmt_ptr = &(me_ptr->input_media_fmt[0]);

	uint32_t j;

	/**< Set the media format to default state */
	media_fmt_ptr->main.format_header.data_format = CAPI_V2_FIXED_POINT;
	media_fmt_ptr->std_fmt.bits_per_sample = 16;
	media_fmt_ptr->std_fmt.bitstream_format = CAPI_V2_DATA_FORMAT_INVALID_VAL;
	media_fmt_ptr->std_fmt.data_interleaving = CAPI_V2_DEINTERLEAVED_UNPACKED;
	media_fmt_ptr->std_fmt.data_is_signed = 1;
	media_fmt_ptr->std_fmt.num_channels = CAPI_V2_DATA_FORMAT_INVALID_VAL;
	media_fmt_ptr->std_fmt.q_factor = PCM_16BIT_Q_FORMAT;
	media_fmt_ptr->std_fmt.sampling_rate = CAPI_V2_DATA_FORMAT_INVALID_VAL;

	for (j=0; (j<CAPI_V2_MAX_CHANNELS); j++)
	{
		media_fmt_ptr->std_fmt.channel_type[j] = (uint16_t)CAPI_V2_DATA_FORMAT_INVALID_VAL;
	}
}



/*===========================================================================
FUNCTION : capi_v2_voicewakeup_release_memory
DESCRIPTION: Function to release allocated memory
        Free all memory, allocated internal buffers, and soundModel data
===========================================================================*/
void capi_v2_voicewakeup_release_memory(capi_v2_voicewakeup_t *me_ptr)
{
	if (me_ptr->params_config.sound_model_ptr) {
		free((void *)me_ptr->params_config.sound_model_ptr);
		me_ptr->params_config.sound_model_size = 0;
	}
    return;
}


/*===========================================================================
	FUNCTION : capi_v2_voicewakeup_get_kpps
	DESCRIPTION: Function to calculate KPPS
	    Compute peak cycles per frame
	    Calculate cps per 10ms frame
===========================================================================*/
void capi_v2_voicewakeup_get_kpps(capi_v2_voicewakeup_t* me_ptr, uint32_t *kpps )
{
	uint32_t cycles_per_frame = 1990;  // hardcoded number number derived module executing on Windows

	/* Dividing by a factor here as we get the peak cycles_per_frame for 10ms frame
              get the value in kcps
     */
	*kpps  = (cycles_per_frame * SCALING_FRAME_DURATION_TO_SECONDS) / SCALING_KILO;
	return;
}

/*===========================================================================
	FUNCTION : capi_v2_voicewakeup_get_bandwidth
	DESCRIPTION: Function to calculate code and data BandWidth
			For 'data' memory, add up the sizes of:
				module data structure
				internal data used by module code
				dynamic memory allocated
				size of soundmodel data copied by module
===========================================================================*/
void capi_v2_voicewakeup_get_bandwidth(capi_v2_voicewakeup_t* me_ptr,
		uint32_t * code_bandwidth, uint32_t * data_bandwidth)
{
	uint16_t mem_dynamic        = 0;  // this example module does not alloc any memory
	uint32_t mem_model_size     = me_ptr->params_config.sound_model_size;
	uint32_t internal_data_size = 1000; // approximated
	uint32_t static_mem_size    = 1000;    // approximated
	uint16_t mem_vw_lib         = sizeof(capi_v2_voicewakeup_t) + internal_data_size;
		   /* Total data memory req. = Lib instance + + Dynamic Mem
		    * As it runs for every 10 msec, it runs for 100 times in a second
		    * ( total mem req. in bytes *100) Bytes/sec
		    * For converting Bytes/sec to bits per sec we multiply it by 8
		    * Algorithm may need to factor in thread memory and/or
            *specific cache miss factor
		    */
		   *data_bandwidth = ( (mem_vw_lib + mem_dynamic + mem_model_size) *
		           SCALING_BYTE_TO_BIT * SCALING_FRAME_DURATION_TO_SECONDS) /
		           SCALING_BITS_TO_KILO_BITS;

		   // memory required for code
		   // should include all Static memory required for text, data,...
		   *code_bandwidth = (static_mem_size *
		           SCALING_BYTE_TO_BIT * SCALING_FRAME_DURATION_TO_SECONDS) /
		           SCALING_BITS_TO_KILO_BITS;
		   *code_bandwidth = 6000; // Kludge of roughly calculated size

	   return;
}

/*===========================================================================
	FUNCTION : capi_v2_voicewakeup_get_kw_end_max_delay
	DESCRIPTION: Function to calculate the maximum number samples in
	   delay between Discovery and KeywordEnd Detection.

       Data structure used when returning the maximum number of samples
       that the Look Ahead Buffer needs to perform circular buffering.

       A hardcoded value is supplied for is example.
       The actual required size of this buffer will be dependent on algorithm
       and maybe related to in soundmodel.
       This parameter indicates the maximum delay between the keyword end position
       and the keyword detection in the stream.
       This delay is calculated in terms of samples starting from the frame
       for which a keyword is detected.
  ===========================================================================*/
void capi_v2_voicewakeup_get_kw_end_max_delay(capi_v2_voicewakeup_t* me_ptr,
		            uint32_t *kw_end_max_delay_samples )
{
	uint32_t max_kw_end_frame_delay = 10;  // approximated millisecond latency
	*kw_end_max_delay_samples  = max_kw_end_frame_delay * SAMPLES_PER_MS;
	return;
}


/*===========================================================================
	FUNCTION : capi_v2_voicewakeup_raise_event
	DESCRIPTION: Function to send event using the callback function
===========================================================================*/
capi_v2_err_t capi_v2_voicewakeup_raise_event(capi_v2_voicewakeup_t *me_ptr,
		                             capi_v2_voicewakeup_events_config_t *event_config)

{
	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;
	if (NULL == me_ptr->cb_info.event_cb)
	{
		FARF(ERROR,"CAPIv2 VoiceWakeup: Event callback is not set. Unable to raise process event!");
		return capi_v2_result;
	}
	capi_v2_event_info_t event_info;

    FARF(HIGH,"CAPIv2 VoiceWakeup: Event callback called with %lu", event_config->type);
	switch(event_config->type) {
	   case CAPI_V2_EVENT_KPPS:
	   {
	    	event_info.payload.actual_data_len = sizeof(capi_v2_event_KPPS_t);
	    	event_info.payload.max_data_len = sizeof(capi_v2_event_KPPS_t);
	    	event_info.payload.data_ptr = (int8_t*)event_config->payload_ptr;
	    	event_info.port_info.is_valid = FALSE;
	   }
	   break;

	   case CAPI_V2_EVENT_BANDWIDTH:
	   {
	    	event_info.payload.actual_data_len = sizeof(capi_v2_event_bandwidth_t);
	    	event_info.payload.max_data_len = sizeof(capi_v2_event_bandwidth_t);
	    	event_info.payload.data_ptr = (int8_t*)event_config->payload_ptr;
	    	event_info.port_info.is_valid = FALSE;
	   }
	   break;

	   case CAPI_V2_EVENT_DATA_TO_DSP_SERVICE:
	   {
	        capi_v2_event_data_to_dsp_service_t event_payload;
	        event_payload.param_id = event_config->param_id;  // sub-type
	        event_payload.token = 0;  // not used
	        event_payload.payload.actual_data_len = event_config->payload_size;
	        event_payload.payload.data_ptr = (int8_t *)event_config->payload_ptr;
	        event_payload.payload.max_data_len = event_config->payload_size  ;

	    	event_info.payload.actual_data_len = sizeof(capi_v2_event_data_to_dsp_service_t);
	    	event_info.payload.max_data_len = sizeof(capi_v2_event_data_to_dsp_service_t);
	    	event_info.payload.data_ptr = (int8_t*)&event_payload;
	    	event_info.port_info.is_valid = FALSE;
	   }
	   break;

	   case CAPI_V2_EVENT_PROCESS_STATE:   /* set flag denoting module is in state to begin processing */
	   {
	    	event_info.payload.actual_data_len = sizeof(capi_v2_event_process_state_t);
	    	event_info.payload.max_data_len = sizeof(capi_v2_event_process_state_t);
	    	event_info.payload.data_ptr = (int8_t*)event_config->payload_ptr;
	    	event_info.port_info.is_valid = FALSE;
	   }
	   break;

	   default:
	   {
		   FARF(ERROR,"CAPIv2 VoiceWakeup: Callback can not be called with event %lu!", event_config->type);
		   CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EUNSUPPORTED);
		   return capi_v2_result;
	   }


	}
	// send event
	capi_v2_result = me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context,
				event_config->type,
				&event_info);
	if (CAPI_V2_FAILED(capi_v2_result))
    {
	   FARF(ERROR, "CAPIv2 VoiceWakeup: raise event type %lu filed with %lu",
					event_config->type, capi_v2_result);
	}
    return capi_v2_result;
}

/*===========================================================================
	FUNCTION : capi_v2_voicewakeup_is_ready_to_process
	DESCRIPTION: Function to update is enabled module flag
	   	 Send all appropriate events to client that signify that
	   	 module is ready for processing
    This should be called whenever any of the following parameters change:
         SoundModel (size)
         Operation Mode
         number of Confidence levels are set
         input Media Format (currently does not change - only on format support)
===========================================================================*/
bool_t capi_v2_voicewakeup_is_ready_to_process(capi_v2_voicewakeup_t* me_ptr)
{
	bool_t process_state = FALSE;
	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;
    capi_v2_voicewakeup_events_config_t event_data;
	/* Test that all parameters are set to allow processing to continue
	 * These would be dependent on the algorithm implementation
	 * For this example module, soundModel must have been registered and
	 * one minimum confidence level (for a single keyword) must be set.
	 * Operation mode include Keyword detection
	 * Custom implementations need NOT require SoundModel being provided.
	 */
	if (me_ptr->params_config.num_conf_levels == 1 &&
		me_ptr->params_config.sound_model_ptr != NULL &&
		me_ptr->params_config.sound_model_size > 0 &&
		(me_ptr->params_config.operation_mode == 0x01 ||
		 me_ptr->params_config.operation_mode == 0x03) )
    {
		process_state = TRUE;  // set FALSE later if error occurs

	    // send generic PROCESS_EVENT to enable processing
	    capi_v2_event_process_state_t process_event;
	    process_event.is_enabled = process_state;
	  	event_data.type = CAPI_V2_EVENT_PROCESS_STATE;
	    event_data.param_id = 0;
	  	event_data.payload_ptr = (void *)&process_event;
	  	event_data.payload_size = (uint8_t)sizeof(capi_v2_event_process_state_t);
	  	capi_v2_result = capi_v2_voicewakeup_raise_event(me_ptr, &event_data);
        if (CAPI_V2_FAILED(capi_v2_result))
        	process_state = FALSE;

    	// send CAPI_V2_EVENT_KPPS event
  		capi_v2_event_KPPS_t kpps_event;
  		capi_v2_voicewakeup_get_kpps(me_ptr, &kpps_event.KPPS);
  	  	event_data.type = CAPI_V2_EVENT_KPPS;
  	    event_data.param_id = 0;
  	  	event_data.payload_ptr = (void *)&kpps_event;
  	  	event_data.payload_size = (uint8_t)sizeof(capi_v2_event_KPPS_t);
  	  	capi_v2_result = capi_v2_voicewakeup_raise_event(me_ptr, &event_data);
        if (CAPI_V2_FAILED(capi_v2_result))
        	process_state = FALSE;

    	// send CAPI_V2_EVENT_BANDWIDTH event
  	  	// The sizes can be calculated after input format and soundModel data are known.
		capi_v2_event_bandwidth_t bandwidth_event;
  		capi_v2_voicewakeup_get_bandwidth(me_ptr,
  				&bandwidth_event.code_bandwidth , &bandwidth_event.data_bandwidth);
  	  	event_data.type = CAPI_V2_EVENT_BANDWIDTH;
  	    event_data.param_id  = 0;
  	  	event_data.payload_ptr = (void *)&bandwidth_event;
  	  	event_data.payload_size = (uint8_t)sizeof(capi_v2_event_bandwidth_t);
  	  	capi_v2_result = capi_v2_voicewakeup_raise_event(me_ptr, &event_data);
        if (CAPI_V2_FAILED(capi_v2_result))
        	process_state = FALSE;

        // Send CAPI_V2_EVENT_DATA_TO_DSP_SERVICE: PARAM_ID_KW_END_MAX_BUFFER_REQ event to framework.
  	  	// This buffer size can be calculated after input format is known.
        kw_end_max_delay_t kw_end_max_delay;
        memset(&kw_end_max_delay, 0, sizeof(kw_end_max_delay_t));
  		capi_v2_voicewakeup_get_kw_end_max_delay(me_ptr, &kw_end_max_delay.kw_end_max_delay_samples);
      	event_data.type = CAPI_V2_EVENT_DATA_TO_DSP_SERVICE;
        event_data.param_id = PARAM_ID_KW_END_MAX_BUFFER_REQ;
        event_data.payload_ptr = (void *)&kw_end_max_delay;
       	event_data.payload_size = (uint8_t)sizeof(kw_end_max_delay_t);
      	FARF(HIGH, "detect_keyword : send CB CAPI_V2_EVENT_DATA_TO_DSP_SERVICE: PARAM_ID_KW_END_MAX_BUFFER_REQ event - max delay %d   ",
      			kw_end_max_delay.kw_end_max_delay_samples);
      	capi_v2_result = capi_v2_voicewakeup_raise_event(me_ptr, &event_data);
        if (CAPI_V2_FAILED(capi_v2_result))
        	process_state = FALSE;
    }
  	return process_state;
}
