/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "HAP_farf.h"
#include "capi_v2_utils_props.h"
#include "capi_v2_gain.h"

#include <stdlib.h>
#include <string.h>

#define THROW(exception, errno) \
		exception = errno; \
		goto exception##bail;

#define CATCH(exception) exception##bail: if (exception != TEST_SUCCESS)
#define CORRECT_DYNAMIC_PROP_PTR (capi_v2_proplist_t*)0x55555555

// JV-TODO: Profile this. Assume 2k for now.
#define CAPI_V2_GAIN_STACK_SIZE  (2048)
#define CAPI_V2_GAIN_MEDIA_FORMAT_SIZE (sizeof(capi_v2_set_get_media_format_t) + \
		sizeof(capi_v2_standard_data_format_t))


typedef struct media_fmt_t
{
	capi_v2_data_format_header_t format_header;
	capi_v2_standard_data_format_t std;
} media_fmt_t;

typedef struct _capi_v2_gain_t
{
	// CAPI_V2 interface
	const capi_v2_vtbl_t* vtbl_ptr;

	// capi_v2 properties
	int heap_id;
	capi_v2_event_callback_info_t event_callback;
	capi_v2_port_num_info_t ports;
	//Media format
	media_fmt_t in_fmt;
	media_fmt_t out_fmt;
	// Module calibration parameters
	uint32_t gain_enable;
	uint32_t gain_val;
} capi_v2_gain_t;

static bool_t is_supported_media_type(const media_fmt_t* format_ptr)
{
	if (format_ptr->format_header.data_format != CAPI_V2_FIXED_POINT) {
		FARF(ERROR, "<<is_supported_media_type>> Only fixed point data supported. Received %lu",
				format_ptr->format_header.data_format);
		return FALSE;
	}

	if (format_ptr->std.bits_per_sample != 16) {
		FARF(ERROR, "<<is_supported_media_type>> Only 16 bit data supported. Received %lu",
				format_ptr->std.bits_per_sample);
		return FALSE;
	}

	if (format_ptr->std.data_interleaving != CAPI_V2_DEINTERLEAVED_UNPACKED) {
		FARF(ERROR, "<<is_supported_media_type>> Interleaved data not supported");
		return FALSE;
	}

	if (!format_ptr->std.data_is_signed) {
		FARF(ERROR, "<<is_supported_media_type>> Unsigned data not supported");
		return FALSE;
	}

	switch(format_ptr->std.num_channels) {
	case 0:
		FARF(ERROR, "<<is_supported_media_type>> Zero channels passed");
		return FALSE;
	case 1:
		FARF(HIGH, "<<is_supported_media_type>> Single Mic data passed");
		return TRUE;
		break;
	case 2:
		FARF(HIGH, "<<is_supported_media_type>> Dual Mic data passed.");
		return TRUE;
		break;
	case 4:
		FARF(HIGH, "<<is_supported_media_type>> Quad Mic data passed.");
		return TRUE;
		break;
	default:
		FARF(ERROR, "<<is_supported_media_type>> Only 1, 2 and 4 channels supported. Received %lu.",
				format_ptr->std.num_channels);
		return FALSE;
	}

	return TRUE;
}

/*===========================================================================
	FUNCTION : capi_v2_gain_output_media_format_event
	DESCRIPTION: Function to send the output media format using the
	callback function
===========================================================================*/
static void capi_v2_gain_raise_output_media_format_event(capi_v2_gain_t *me_ptr)
{
	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;

	// Raise an event
	capi_v2_event_info_t    event_info;

	event_info.port_info.is_valid = TRUE;
	event_info.port_info.is_input_port = FALSE;
	event_info.port_info.port_index = 0;
	event_info.payload.actual_data_len = event_info.payload.max_data_len = sizeof(me_ptr->out_fmt);
	event_info.payload.data_ptr = (int8_t*)(&me_ptr->out_fmt);

	capi_v2_result = me_ptr->event_callback.event_cb(me_ptr->event_callback.event_context,
			CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED, &event_info);
	if (CAPI_V2_FAILED(capi_v2_result))
	{
		FARF(ERROR,"CAPIv2 gain: phani Failed to send output media format updated event with %lu",
				capi_v2_result);
	}
    else
    {
        FARF(HIGH,"CAPI V2 GAIN: Sucess : phani send output media format updated event with %lu", capi_v2_result);
    }
}


void pcm_gain(int16_t* primary_mic_ptr,
		int16_t* out_ptr,
		uint32_t gain_val,
		uint32_t num_samples)
{
	uint32_t i;
	uint32_t out;

	for (i = 0; i < num_samples; i++) {
		out = gain_val * primary_mic_ptr[i];
		out_ptr[i] = (out >> 13);
	}
}

static capi_v2_err_t capi_v2_gain_process(capi_v2_t* _pif,
		capi_v2_stream_data_t* input[],
		capi_v2_stream_data_t* output[])
{
	capi_v2_err_t result = CAPI_V2_EOK;
	uint32_t i, k;
	int16_t* in_pcm_ch_ptr;
	int16_t* out_pcm_ch_ptr;
	capi_v2_buf_t * in_buf_ptr,*out_buf_ptr;
	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;
	FARF(ERROR, "CAPI V2 gain_process: Process");
	// Assuming number of output ports are same as input ports
	for (k = 0; k < me->ports.num_output_ports; k++) {
		in_buf_ptr = input[k]->buf_ptr;
		out_buf_ptr = output[k]->buf_ptr;
		if (me->gain_enable) {
			// process each channel
			for (i = 0; i < input[k]->bufs_num; i++) {
				in_pcm_ch_ptr = (int16_t*)in_buf_ptr->data_ptr;
				out_pcm_ch_ptr = (int16_t*)out_buf_ptr->data_ptr;
				//apply gain per channel
				pcm_gain(in_pcm_ch_ptr, out_pcm_ch_ptr, me->gain_val, (in_buf_ptr->actual_data_len) / sizeof(int16_t));
				// update the number of samples prepared
				out_buf_ptr->actual_data_len = in_buf_ptr->actual_data_len;
				//move on to next channel
				in_buf_ptr++;
				out_buf_ptr++;
			}
		} else {
			// Gain disabled. Just passthrough the PCM samples
			for (i = 0; i < input[k]->bufs_num; i++) {
				memmove(out_buf_ptr->data_ptr, in_buf_ptr->data_ptr, in_buf_ptr->actual_data_len);
				out_buf_ptr->actual_data_len = in_buf_ptr->actual_data_len;
				in_buf_ptr++;
				out_buf_ptr++;
			}
		}
	}

	// This is an example piece of code for understanding the far end reference signal
	// We can enable it by sending the enable parameter value as 2.
	if(me->gain_enable == 2) {
		// Far end reference signal, Modules receives if NEEDED_FAR_END_REFERENCE extension raised
		// Reference signal will be received in last port
		if(me->ports.num_input_ports > me->ports.num_output_ports) {
			k = me->ports.num_input_ports-1;
			in_buf_ptr = input[k]->buf_ptr;
			out_buf_ptr = output[0]->buf_ptr;
			in_pcm_ch_ptr = (int16_t*)in_buf_ptr->data_ptr;
			out_pcm_ch_ptr = (int16_t*)out_buf_ptr->data_ptr;
			FARF(HIGH, "Voice far end reference signal: ");
			for (i=0;i<(in_buf_ptr->actual_data_len/2);i++) {
				out_pcm_ch_ptr[i] = in_pcm_ch_ptr[i]; // replacing output of the module with reference signal
				//FARF(HIGH, "%x",in_pcm_ch_ptr[i]);
			}

		}
		else
			FARF(HIGH, "Reference signal check failed");
	}
	return result;
}

static capi_v2_err_t capi_v2_gain_end(capi_v2_t* _pif)
{
	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;
	me->vtbl_ptr = NULL;
	return CAPI_V2_EOK;
}

static capi_v2_err_t capi_v2_gain_set_param(capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t* port_info_ptr,
		capi_v2_buf_t* params_ptr)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;

	//Validate pointers
	if (NULL == _pif || NULL == params_ptr) {
		FARF(ERROR, "CAPI V2 GAIN: set param received NULL pointer");
		return CAPI_V2_EBADPARAM;
	}

	switch(param_id) {
	case CAPI_V2_GAIN_PARAM_ID_ENABLE:
	{
		// This parameter is applicable for all ports
		if (params_ptr->actual_data_len >= sizeof(uint32_t)) {
			uint32_t* enable = (uint32_t*)(params_ptr->data_ptr);
			me->gain_enable = *enable;
			FARF(HIGH, "CAPI V2 GAIN: enable/disable <0x%x>", me->gain_enable);

			// Raise an event
			{
				capi_v2_event_process_state_t process_state;
				capi_v2_event_info_t event_info;
				//event_info.port_info = 0; //TODO
				event_info.payload.data_ptr = (int8_t*)&(process_state);
				event_info.payload.actual_data_len = sizeof(capi_v2_event_process_state_t);
				event_info.payload.max_data_len = sizeof(capi_v2_event_process_state_t);

				process_state.is_enabled = (bool_t)(me->gain_enable);

				if (me->event_callback.event_cb) {
					result = (me->event_callback.event_cb)(me->event_callback.event_context, CAPI_V2_EVENT_PROCESS_STATE, &event_info);
					FARF(LOW, "CAPI V2 GAIN: PROCESS STATE(%d) updated with event callback", proces_state.is_enabled);
				}
			}
		} else {
			FARF(ERROR, "CAPI V2 GAIN: set_param received bad payload size <%d> for parameter <0x%x>", params_ptr->max_data_len, param_id);
		}
	}
	break;

	case CAPI_V2_GAIN_PARAM_ID_MASTER_GAIN:
	{
		//This parameter is applicable for all ports
		gain_ctrl_master_gain_t *gain_ptr;
		gain_ptr = (gain_ctrl_master_gain_t *)params_ptr->data_ptr;
		me->gain_val = (uint32_t)gain_ptr->master_gain;
		FARF(HIGH, "CAPI V2 GAIN: set master gain <0x%x>", me->gain_val);

		//Update the KPPS based on the gain -- This is just dummy update for an example
		if (gain_ptr->master_gain > 0x1000) {
			capi_v2_event_KPPS_t kpps;
			capi_v2_event_info_t event_info;
			//event_info.port_info = 0; //TODO
			event_info.payload.data_ptr = (int8_t*)&kpps;
			event_info.payload.actual_data_len = sizeof(capi_v2_event_KPPS_t);
			event_info.payload.max_data_len = sizeof(capi_v2_event_KPPS_t);

			kpps.KPPS = 5000;

			if (me->event_callback.event_cb) {
				result = (me->event_callback.event_cb)(me->event_callback.event_context, CAPI_V2_EVENT_KPPS, &event_info);
				FARF(LOW, "CAPI V2 GAIN: KPPS(%d) updated with event callback", kpps.KPPS);
			}
		}
	}
	break;

	default:
		FARF(HIGH, "CAPI V2 GAIN: set_param received unsupported parameter(0x%x)", param_id);
		result = CAPI_V2_EBADPARAM;
		break;
	}

	return result;
}

static capi_v2_err_t capi_v2_gain_get_param(capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t* port_info_ptr,
		capi_v2_buf_t* params_ptr)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;

	//Validate pointers
	if (NULL == _pif || NULL == params_ptr) {
		FARF(ERROR, "CAPI V2 GAIN: get param received NULL pointer");
		return CAPI_V2_EBADPARAM;
	}

	// All parameters of gain module are applicable for all ports
	switch(param_id) {
	case CAPI_V2_GAIN_PARAM_ID_ENABLE:
		if (params_ptr->max_data_len >= sizeof(uint32_t)) {
			// Fill the current gain enable value
			uint32_t* enable = (uint32_t*)(params_ptr->data_ptr);
			*enable = me->gain_enable;
			// Update the data length
			params_ptr->actual_data_len = sizeof(uint32_t);
			FARF(LOW, "CAPI V2 GAIN: Received get param Enable/Disable %d", *enable);
		} else {
			result = CAPI_V2_EFAILED;
			FARF(ERROR, "CAPI V2 GAIN: get_param received bad payload size <%d> for parameter <0x%x>", params_ptr->max_data_len, param_id);
		}
		break;
	case CAPI_V2_GAIN_PARAM_ID_MASTER_GAIN:
		if (params_ptr->max_data_len >= sizeof(uint32_t)) {
			//Fill the current gain value
			gain_ctrl_master_gain_t *gain_ptr = (gain_ctrl_master_gain_t *)params_ptr->data_ptr;
			gain_ptr->master_gain = (uint16_t)me->gain_val;
			gain_ptr->reserved = 0;
			//Update the data length
			params_ptr->actual_data_len = sizeof(gain_ctrl_master_gain_t);
			FARF(LOW, "CAPI V2 GAIN: Received get param for gain %d", *master_gain);
		} else {
			result = CAPI_V2_EFAILED;
			FARF(ERROR, "CAPI V2 GAIN: get_param received bad payload size <%d> for parameter <0x%x>", params_ptr->max_data_len, param_id);
		}
		break;
	default:
		FARF(ERROR, "CAPI V2 GAIN Get Param: Unsupported param id <0x%x> received", param_id);
		result = CAPI_V2_EBADPARAM;
		break;
	}
	return result;
}

static capi_v2_err_t capi_v2_gain_set_properties(
		capi_v2_t* _pif,
		capi_v2_proplist_t* props_list)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	uint32_t i;
	capi_v2_prop_t* prop;

	if (NULL == props_list) {
		FARF(ERROR, "CAPI V2 GAIN: set properties received NULL props pointer");
		return CAPI_V2_EFAILED;
	}

	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;
	prop = props_list->prop_ptr;

	for (i = 0; i < props_list->props_num; i++) {
		capi_v2_property_id_t id = prop->id;
		capi_v2_buf_t payload = prop->payload;

		switch(id) {
		case CAPI_V2_INPUT_MEDIA_FORMAT:
		{
			media_fmt_t* mfmt_ptr = (media_fmt_t*)(payload.data_ptr);
			if (mfmt_ptr->format_header.data_format == CAPI_V2_FIXED_POINT) {
				me->in_fmt = *mfmt_ptr;
				me->out_fmt = me->in_fmt; // output media format is same as input for Gain Module

				FARF(HIGH, "CAPI V2 GAIN: phani media format update, sampling freq %d, num channels %d, bits per sample %d", me->in_fmt.std.sampling_rate, me->in_fmt.std.num_channels,
						me->in_fmt.std.bits_per_sample);

				if(!is_supported_media_type(mfmt_ptr)) {
					FARF(ERROR, "CAPI V2 GAIN : phani Unsupported input media format");
					result |= CAPI_V2_EUNSUPPORTED;
				}
				else
				{
					capi_v2_gain_raise_output_media_format_event(me);
				}
			}
			else {
				FARF(HIGH, "CAPI V2 GAIN: unsupported media data format %d, supports only FIXED_POINT", mfmt_ptr->format_header.data_format);
				result |= CAPI_V2_EUNSUPPORTED;
			}
			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT:
		{
			media_fmt_t* mfmt_ptr = (media_fmt_t*)(payload.data_ptr);
			if (mfmt_ptr->format_header.data_format == CAPI_V2_FIXED_POINT) {
				me->out_fmt = *mfmt_ptr;
			} else {
				FARF(HIGH, "CAPI V2 GAIN: Unsupported output media format %d", mfmt_ptr->format_header.data_format);
				result |= CAPI_V2_EUNSUPPORTED;
			}
			break;
		}
		case CAPI_V2_ALGORITHMIC_RESET:
		{
			// Nothing to reset for gain module. No buffering. So we are good to go now.
			result |= CAPI_V2_EOK;
			break;
		}
		case CAPI_V2_HEAP_ID:
		{
			capi_v2_heap_id_t* heap_id = (capi_v2_heap_id_t*)payload.data_ptr;
			me->heap_id = heap_id->heap_id;
			break;
		}
		case CAPI_V2_EVENT_CALLBACK_INFO:
		{
			FARF(HIGH, "CAPI V2 GAIN: Recevied event callback pointer");
			capi_v2_event_callback_info_t* event_callback =
					(capi_v2_event_callback_info_t*)payload.data_ptr;
			me->event_callback.event_cb = event_callback->event_cb;
			me->event_callback.event_context = event_callback->event_context;
			break;
		}
		case CAPI_V2_PORT_NUM_INFO:
		{
			capi_v2_port_num_info_t* ports = (capi_v2_port_num_info_t*)(payload.data_ptr);
			me->ports.num_input_ports = ports->num_input_ports;
			me->ports.num_output_ports = ports->num_output_ports;
			FARF(HIGH, "CAPI V2 GAIN: Number of ports set, input: %d, output: %d", ports->num_input_ports, ports->num_output_ports);
			result |= CAPI_V2_EOK;
			break;
		}
		case CAPI_V2_CUSTOM_INIT_DATA:
		{
			FARF(HIGH, "CAPI V2 GAIN: Set property id custom Init data unsupported! ");
			break;
		}
		case CAPI_V2_SESSION_IDENTIFIER:
		{
			FARF(HIGH, "CAPI V2 GAIN: Session Identifier property unsupported! ");
			result |= CAPI_V2_EOK;
			break;
		}
		case CAPI_V2_EXTERNAL_SERVICE_ID:
		{
			FARF(ERROR, "CAPI V2 GAIN: does not support comm with external services");
			result |= CAPI_V2_EUNSUPPORTED;
			break;
		}
		default:
		{
			FARF(ERROR, "CAPI V2 GAIN: received unknown property 0x%x", id);
			result |= CAPI_V2_EUNSUPPORTED;
			break;
		}
		}
		prop++;
	}

	return result;
}

static capi_v2_err_t capi_v2_gain_get_properties(
		capi_v2_t* _pif,
		capi_v2_proplist_t* props_list)
{
	uint32_t i = 0;
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;

	for (i = 0; i < props_list->props_num; i++) {
		capi_v2_property_id_t id = props_list[i].prop_ptr->id;
		capi_v2_buf_t* payload = &props_list[i].prop_ptr->payload;

		switch(id) {
		// CAPI_V2 queryable non-static properties
		case CAPI_V2_PORT_DATA_THRESHOLD:
		{
			payload->actual_data_len = 0;
			result = CAPI_V2_EOK;
			FARF(ERROR, "CAPI V2 GAIN: No buffering required and threshold request not expected");
			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT_SIZE:
		{
			FARF(LOW, "CAPI V2 GAIN: get_properties for output media format size");
			if (payload->max_data_len >= sizeof(capi_v2_output_media_format_size_t)) {
				capi_v2_output_media_format_size_t* psize = (capi_v2_output_media_format_size_t*)payload->data_ptr;
				psize->size_in_bytes = sizeof(me->out_fmt) - sizeof(me->out_fmt.format_header);
				payload->actual_data_len = sizeof(capi_v2_output_media_format_size_t);
			}
			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT:
		{
			FARF(LOW, "CAPI V2 GAIN: get_properties for output media format ");
			if (payload->max_data_len >= sizeof(media_fmt_t)) {
				media_fmt_t* pfmt = (media_fmt_t*)payload->data_ptr;
				*pfmt = me->out_fmt;
				payload->actual_data_len = sizeof(media_fmt_t);
			}
			break;
		}
		case CAPI_V2_METADATA:
		{
			payload->actual_data_len = 0;
			FARF(ERROR, "CAPI V2 GAIN: meadata does not apply");
			break;
		}
		default:
		{
			FARF(ERROR, "CAPI V2 GAIN: received request to get unknown/unsupported property 0x%x", id);
			result |= CAPI_V2_EFAILED;
			break;
		}
		}
	}

	return result;
}

static const capi_v2_vtbl_t vtbl =
{
		capi_v2_gain_process,
		capi_v2_gain_end,
		capi_v2_gain_set_param,
		capi_v2_gain_get_param,
		capi_v2_gain_set_properties,
		capi_v2_gain_get_properties,
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


capi_v2_err_t capi_v2_gain_set_props(void* ctx, capi_v2_property_id_t id,
		capi_v2_buf_t* payload)
{
	capi_v2_err_t result = CAPI_V2_EOK;

	switch(id) {
	case CAPI_V2_INIT_MEMORY_REQUIREMENT:
	{
		result = capi_v2_utils_props_set_init_memory_requirement(payload,
				sizeof(capi_v2_gain_t));
		break;
	}
	case CAPI_V2_STACK_SIZE:
	{
		result = capi_v2_utils_props_set_stack_size(payload, CAPI_V2_GAIN_STACK_SIZE);
		break;
	}
	case CAPI_V2_MAX_METADATA_SIZE:
	{
		// This module doesn't support metadata - return 0
				result = capi_v2_utils_props_set_max_metadata_size(payload, 0, 0);
				break;
	}
	case CAPI_V2_IS_INPLACE:
	{
		result = capi_v2_utils_props_set_is_inplace(payload, TRUE);
		break;
	}
	case CAPI_V2_REQUIRES_DATA_BUFFERING:
	{
		result = capi_v2_utils_props_set_requires_data_buffering(payload, FALSE);
		break;
	}
	case CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS:
	{
		//Gain module requesting for 0 extension
		result = capi_v2_utils_props_set_num_framework_extensions(payload, 0);
		break;
	}
	case CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS:
	{
		break;
	}
	default:
	{
		FARF(ERROR, "unknown static property 0x%08x", id);
		result = CAPI_V2_EFAILED;
		break;
	}
	}
	return result;
}

capi_v2_err_t capi_v2_gain_get_static_properties(
		capi_v2_proplist_t* init_set_properties,
		capi_v2_proplist_t* static_properties)
{

	if (NULL == static_properties) {
		FARF(ERROR, "static_properties is NULL");
		return CAPI_V2_EFAILED;
	}

	return capi_v2_utils_props_process_properties(static_properties,
			capi_v2_gain_set_props, 0);
}

capi_v2_err_t capi_v2_gain_init(capi_v2_t* _pif,
		capi_v2_proplist_t* init_set_properties)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_gain_t* me = (capi_v2_gain_t*)_pif;
	capi_v2_prop_t* props;
	uint32_t i;

	FARF(ERROR, "capi_v2_gain_init");

	memset((void*)me, 0, sizeof(capi_v2_gain_t));

	// Initialize virtual function pointer table
	me->vtbl_ptr = &vtbl;

	// module is disabled by default
	me->gain_enable = 0;
	me->event_callback.event_cb = NULL;
	me->event_callback.event_context = NULL;

	// Assuming default number of ports as 1
	me->ports.num_input_ports = 1;
	me->ports.num_output_ports = 1;

	// validate properties requested are ok to set at initialization
	// We are just printing message if not valid at init time and proceeding further
	props = init_set_properties->prop_ptr;
	for (i = 0; i < init_set_properties->props_num; i++) {
		switch(props->id) {
		case CAPI_V2_INPUT_MEDIA_FORMAT:
		case CAPI_V2_EVENT_CALLBACK_INFO:
		case CAPI_V2_HEAP_ID:
		case CAPI_V2_PORT_NUM_INFO:
			break;
		default:
			FARF(HIGH, "This property with id <%d> is not supported at initialization of module", props->id);
			break;
		}
		props++;
	}

	// Setting parameters now
	if (init_set_properties->props_num) {
		result = capi_v2_gain_set_properties(_pif, init_set_properties);
	}

	return result;
}

#ifdef __cplusplus
}
#endif //__cplusplus
