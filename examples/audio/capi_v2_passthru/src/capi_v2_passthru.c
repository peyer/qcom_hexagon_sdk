/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "HAP_farf.h"

#include "Elite_CAPI_V2.h"
#include "Elite_pcm_ch_defs.h"
#include "capi_v2_utils_props.h"
#include "capi_v2_passthru.h"

#include <stdlib.h>
#include <string.h>

#define THROW(exception, errno) \
		exception = errno; \
		goto exception##bail;

#define CATCH(exception) exception##bail: if (exception != TEST_SUCCESS)
#define CORRECT_DYNAMIC_PROP_PTR (capi_v2_proplist_t*)0x55555555

// -----------------------------------------------------------------------
//  Macros
// -----------------------------------------------------------------------
#define CAPI_V2_PASSTHRU_STACK_SIZE  (2048)
#define CAPI_V2_PASSTHRU_MEDIA_FORMAT_SIZE (sizeof(capi_v2_set_get_media_format_t) + \
		sizeof(capi_v2_standard_data_format_t))

// -----------------------------------------------------------------------
//  Data Types
// -----------------------------------------------------------------------

const char data_format_name[][32] = {
		{ "FIXED_POINT" },
		{ "FLOATING_POINT" },
		{ "RAW_COMPRESSED" },
		{ "IEC61937_PACKETIZED" }
};

typedef struct media_fmt_t
{
	capi_v2_data_format_header_t header;
	capi_v2_standard_data_format_t std;
} media_fmt_t;

// CAPI V2 PASSTHRU
typedef struct _capi_v2_passthru_t
{
	// CAPI_V2 interface
	const capi_v2_vtbl_t* vtbl_ptr;
	// CAPI_V2 Properties
	int heap_id;
	capi_v2_event_callback_info_t event_callback;
	capi_v2_port_num_info_t ports;
	// Media formats:
	media_fmt_t input_media_format;
	media_fmt_t output_media_format;
	// Module calibration parameters
	uint32_t passthru_enable;

} capi_v2_passthru_t;


/*===========================================================================
	FUNCTION : capi_v2_passthru_output_media_format_event
	DESCRIPTION: Function to send the output media format using the
	callback function
===========================================================================*/
static void capi_v2_passthru_raise_output_media_format_event(capi_v2_passthru_t *me_ptr)
{
	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;

	// Raise an event
	capi_v2_event_info_t    event_info;

	event_info.port_info.is_valid = TRUE;
	event_info.port_info.is_input_port = FALSE;
	event_info.port_info.port_index = 0;
	event_info.payload.actual_data_len = event_info.payload.max_data_len = sizeof(me_ptr->output_media_format);
	event_info.payload.data_ptr = (int8_t*)(&me_ptr->output_media_format);

	capi_v2_result = me_ptr->event_callback.event_cb(me_ptr->event_callback.event_context,
			CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED, &event_info);
	if (CAPI_V2_FAILED(capi_v2_result))
	{
		FARF(ERROR,"CAPI V2 PASSTHRU: Failed to send output media format updated event with %lu",
				capi_v2_result);
	}
    else
    {
        FARF(HIGH,"CAPI V2 PASSTHRU: Success: send output media format updated event with %lu", capi_v2_result);
    }
}

/*------------------------------------------------------------------------------
 CAPI V2 Interface Implementation
------------------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_passthru_process(
		capi_v2_t* _pif,
		capi_v2_stream_data_t* input[],
		capi_v2_stream_data_t* output[])
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_passthru_t* me = (capi_v2_passthru_t*)_pif;
	uint32_t i, k;
	capi_v2_buf_t *in_buf_ptr, *out_buf_ptr;

	FARF(ERROR, "CAPI V2 PASSTHRU: Process");
	//Here input prts are equal to output ports
	for(k=0; k<me->ports.num_input_ports; k++)
	{
		in_buf_ptr = input[k]->buf_ptr;
		out_buf_ptr = output[k]->buf_ptr;
		if(me->passthru_enable)
		{
			for(i=0; i<input[k]->bufs_num; i++)
			{
				memcpy(out_buf_ptr->data_ptr, in_buf_ptr->data_ptr, in_buf_ptr->actual_data_len);
				out_buf_ptr->actual_data_len = in_buf_ptr->actual_data_len;
				in_buf_ptr++;
				out_buf_ptr++;
			}
		}
	}

	return result;
}

static capi_v2_err_t capi_v2_passthru_end(capi_v2_t* _pif)
{
	capi_v2_passthru_t* me = (capi_v2_passthru_t*)_pif;
	me->vtbl_ptr = NULL;
	return CAPI_V2_EOK;
}

static capi_v2_err_t capi_v2_passthru_set_param(
		capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t* port_info_ptr,
		capi_v2_buf_t* params_ptr)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_passthru_t* me = (capi_v2_passthru_t*)_pif;

	//Validate pointers
	if (NULL == _pif || NULL == params_ptr) {
		FARF(ERROR, "CAPI V2 PASSTHRU: set param received NULL pointer");
		return CAPI_V2_EBADPARAM;
	}

	switch(param_id)
	{
	case CAPI_V2_PARAM_PASSTHRU_MODULE_ENABLE:
		// This parameter is applicable for all ports
		if(params_ptr->actual_data_len >= sizeof(uint32_t))
		{
			uint32_t *enable = (uint32_t*)(params_ptr->data_ptr);
			me->passthru_enable = *enable;
			FARF(HIGH, "CAPI V2 PASSTHRU enabled <0x%x>", me->passthru_enable);

			{
				capi_v2_event_process_state_t process_state;
				capi_v2_event_info_t event_info;
				//event_info.port_info = 0;
				event_info.payload.data_ptr = (int8_t*)&(process_state);
				event_info.payload.actual_data_len = sizeof(capi_v2_event_process_state_t);
				event_info.payload.max_data_len = sizeof(capi_v2_event_process_state_t);

				process_state.is_enabled = (bool_t)(me->passthru_enable);

				if(me->event_callback.event_cb)
				{
					result = (me->event_callback.event_cb)(me->event_callback.event_context, CAPI_V2_EVENT_PROCESS_STATE, &event_info);
					FARF(LOW, "CAPI V2 PASSTHRU: PROCESS STATE(%d) updated with event callback", proces_state.is_enabled);
				}
			}

		}
		else
		{
			FARF(ERROR, "CAPI V2 PASSTHRU: set_param received bad payload size <%d> for parameter <0x%x>", params_ptr->max_data_len, param_id);
		}
		break;

	default:
		FARF(HIGH, "CAPI V2 PASSTHRU: set_param received unsupported parameter(0x%x)", param_id);
		result = CAPI_V2_EBADPARAM;
		break;
	}

	return result;
}

static capi_v2_err_t capi_v2_passthru_get_param(
		capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t* port_info_ptr,
		capi_v2_buf_t* params_ptr)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_passthru_t* me = (capi_v2_passthru_t*)_pif;

	//Validate pointers
	if (NULL == _pif || NULL == params_ptr) {
		FARF(ERROR, "CAPI V2 PASSTHRU: set param received NULL pointer");
		return CAPI_V2_EBADPARAM;
	}

	switch(param_id)
	{
	case CAPI_V2_PARAM_PASSTHRU_MODULE_ENABLE:
		if(params_ptr->max_data_len >= sizeof(uint32_t))
		{
			// Fill the current gain enable value
			uint32_t *enable = (uint32_t*)(params_ptr->data_ptr);
			*enable = me->passthru_enable;
			// Update the data length
			params_ptr->actual_data_len = sizeof(uint32_t);
			FARF(LOW, "CAPI V2 PASSTHRU: Received get param Enable/Disable %d", *enable);
		}
		else
		{
			result = CAPI_V2_EFAILED;
			FARF(ERROR, "CAPI V2 PASSTHRU: get_param received bad payload size <%d> for parameter <0x%x>", params_ptr->max_data_len, param_id);
		}
		break;
	default:
		FARF(ERROR,"CAPI V2 PASSTHRU Get Param: Unsupported param id <0x%x> received", param_id);
		result = CAPI_V2_EBADPARAM;
		break;
	}
	return result;
}

static capi_v2_err_t capi_v2_passthru_set_properties(
		capi_v2_t* _pif,
		capi_v2_proplist_t* props_ptr)
{
	uint32_t i = 0;
	if (NULL == props_ptr) {
		FARF(ERROR, "Passthru set properties received NULL props pointer");
		return CAPI_V2_EFAILED;
	}

	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_passthru_t* me = (capi_v2_passthru_t*)_pif;

	for (i = 0; i < props_ptr->props_num; i++) {
		// Extract property details
		capi_v2_property_id_t id = props_ptr->prop_ptr[i].id;
		capi_v2_buf_t payload = props_ptr->prop_ptr[i].payload;

		switch(id) {
		case CAPI_V2_INPUT_MEDIA_FORMAT:
		{
			media_fmt_t *mfmt_ptr = (media_fmt_t*)(payload.data_ptr);
			if(mfmt_ptr->header.data_format == CAPI_V2_FIXED_POINT) {
				me->input_media_format = *mfmt_ptr;
				me->output_media_format = me->input_media_format; // output media format is same as input for CAPI V2 PASSTHRU
				FARF(MEDIUM, "CAPI V2 PASSTHRU: media format update, sampling freq %d, num channels %d, bits per sample %d", me->input_media_format.std.sampling_rate, me->input_media_format.std.num_channels,
						me->input_media_format.std.bits_per_sample);

				capi_v2_passthru_raise_output_media_format_event(me);

			}
			else {
				FARF(HIGH, "CAPI V2 PASSTHRU: unsupported media data format %d, supports only FIXED_POINT", mfmt_ptr->header.data_format);
				result |= CAPI_V2_EUNSUPPORTED;
			}

			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT:
		{
			media_fmt_t *mfmt_ptr = (media_fmt_t*)(payload.data_ptr);
			if(mfmt_ptr->header.data_format == CAPI_V2_FIXED_POINT) {
				me->output_media_format = *mfmt_ptr;
			}
			else {
				FARF(HIGH, "CAPI V2 PASSTHRU: Unsupported output media format %d", mfmt_ptr->header.data_format);
				result |= CAPI_V2_EUNSUPPORTED;
			}
			break;
		}
		case CAPI_V2_ALGORITHMIC_RESET:
		{
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
			FARF(HIGH, "CAPI V2 PASSTHRU: Recevied event callback pointer");
			capi_v2_event_callback_info_t* event_callback =
					(capi_v2_event_callback_info_t*)payload.data_ptr;
			me->event_callback.event_cb = event_callback->event_cb;
			me->event_callback.event_context = event_callback->event_context;
			break;
		}
		case CAPI_V2_PORT_NUM_INFO:
		{
			capi_v2_port_num_info_t *ports = (capi_v2_port_num_info_t*)(payload.data_ptr);
			me->ports.num_input_ports = ports->num_input_ports;
			me->ports.num_output_ports = ports->num_output_ports;
			if(me->ports.num_input_ports != me->ports.num_output_ports) {
				FARF(ERROR, "Passthru module: Number of output and input ports are not same. Supports only same number of output and input ports %d %d",
						me->ports.num_input_ports, me->ports.num_output_ports);

				result |= CAPI_V2_EFAILED;
			}
			else{
				FARF(HIGH, "CAPI V2 PASSTHRU: Number of ports set, input: %d, output: %d", ports->num_input_ports, ports->num_output_ports);
				result |= CAPI_V2_EOK;
			}
			break;
		}
		// TODO
		/*
	        case CAPI_V2_PORT_INFO:
	        {
	          FARF(ERROR, "can only support one input/output");
	          result |= ADSP_EUNSUPPORTED;
	          break;
	        }*/
		case CAPI_V2_EXTERNAL_SERVICE_ID:
		{
			FARF(ERROR, "does not support comm with external services");
			result |= CAPI_V2_EUNSUPPORTED;
			break;
		}
		case CAPI_V2_CUSTOM_INIT_DATA:
		{
			FARF(HIGH, "CAPI V2 PASSTHRU: Set property id custom Init data unsupported! ");
			result |= CAPI_V2_EOK;
			break;
		}
		case CAPI_V2_SESSION_IDENTIFIER:
		{
			FARF(HIGH, "CAPI V2 PASSTHRU: Session Identifier property unsupported! ");
			result |= CAPI_V2_EOK;
			break;
		}
		default:
		{
			FARF(ERROR, "received unknown or unsupported property 0x%08x", id);
			result |= CAPI_V2_EUNSUPPORTED;
			break;
		}
		}
	}

	return result;
}

static capi_v2_err_t capi_v2_passthru_get_properties(
		capi_v2_t* _pif,
		capi_v2_proplist_t* props_ptr)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_passthru_t* me = (capi_v2_passthru_t*)_pif;
	uint32_t i = 0;

	for (i = 0; i < props_ptr->props_num; i++) {
		capi_v2_property_id_t id = props_ptr[i].prop_ptr->id;
		capi_v2_buf_t* payload = &props_ptr[i].prop_ptr->payload;

		switch(id) {
		// CAPI_V2 queryable non-static properties
		case CAPI_V2_PORT_DATA_THRESHOLD:
		{
			payload->actual_data_len = 0;
			result = CAPI_V2_EOK;
			FARF(ERROR, "CAPI V2 PASSTHRU: No buffering required and threshold request not expected");
			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT:
		{
			FARF(LOW, "CAPI V2 PASSTHRU: get_properties for output media format size");
			if(payload->max_data_len >= sizeof(capi_v2_output_media_format_size_t)) {
				capi_v2_output_media_format_size_t *psize = (capi_v2_output_media_format_size_t*) payload->data_ptr;
				psize->size_in_bytes = sizeof(me->output_media_format) - sizeof(me->output_media_format.header);
				payload->actual_data_len = sizeof(capi_v2_output_media_format_size_t);
			}
			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT_SIZE:
		{
			FARF(LOW, "CAPI V2 PASSTHRU: get_properties for output media format ");
			if(payload->max_data_len >= sizeof(media_fmt_t)) {
				media_fmt_t *pfmt = (media_fmt_t*)payload->data_ptr;
				*pfmt = me->output_media_format;
				payload->actual_data_len = sizeof(media_fmt_t);
			}
			break;
		}
		case CAPI_V2_METADATA:
		{
			payload->actual_data_len = 0;
			FARF(ERROR, "metadata does not apply");
			break;
		}
		default:
		{
			FARF(ERROR, "received request to get unknown or unsupported property 0x%08x", id);
			result |= CAPI_V2_EFAILED;
			break;
		}
		}
	}
	return result;
}

static const capi_v2_vtbl_t vtbl =
{
		// add function pointers to CAPI_V2 functions
		capi_v2_passthru_process,
		capi_v2_passthru_end,
		capi_v2_passthru_set_param,
		capi_v2_passthru_get_param,
		capi_v2_passthru_set_properties,
		capi_v2_passthru_get_properties
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

capi_v2_err_t capi_v2_passthru_set_props(void* ctx, capi_v2_property_id_t id,
		capi_v2_buf_t* payload)
{
	capi_v2_err_t result = CAPI_V2_EOK;

	switch(id) {
	case CAPI_V2_INIT_MEMORY_REQUIREMENT:
	{
		capi_v2_utils_props_set_init_memory_requirement(payload,
				sizeof(capi_v2_passthru_t));
		break;
	}
	case CAPI_V2_STACK_SIZE:
	{
		capi_v2_utils_props_set_stack_size(payload, CAPI_V2_PASSTHRU_STACK_SIZE);
		break;
	}
	case CAPI_V2_MAX_METADATA_SIZE:
	{
		// This module doesn't support metadata - return 0
		capi_v2_utils_props_set_max_metadata_size(payload, 0, 0);
		break;
	}
	case CAPI_V2_IS_INPLACE:
	{
		capi_v2_utils_props_set_is_inplace(payload, TRUE);
		break;
	}
	case CAPI_V2_REQUIRES_DATA_BUFFERING:
	{
		capi_v2_utils_props_set_requires_data_buffering(payload, FALSE);
		break;
	}
	default:
	{
		//FARF(ERROR, "unknown static property 0x%08x", id);
		//result = CAPI_V2_EFAILED;
		break;
	}
	}
	return result;
}


capi_v2_err_t capi_v2_passthru_get_static_properties(
		capi_v2_proplist_t* init_set_properties,
		capi_v2_proplist_t* static_properties)
{

	if (NULL == static_properties) {
		FARF(ERROR, "static_properties is NULL");
		return CAPI_V2_EFAILED;
	}

	return capi_v2_utils_props_process_properties(static_properties,
			capi_v2_passthru_set_props, 0);
}

capi_v2_err_t capi_v2_passthru_init(capi_v2_t* _pif,
		capi_v2_proplist_t* init_set_properties)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_prop_t *props;
	uint32_t i;

	if (NULL == _pif) {
		FARF(ERROR, "_pif is NULL.");
		return CAPI_V2_EBADPARAM;
	}

	// *** CONFIGURE DEFAULTS ***
	// Initialize memory
	capi_v2_passthru_t* me_ptr = (capi_v2_passthru_t*)_pif;
	memset((void*)me_ptr, 0, sizeof(capi_v2_passthru_t));

	// Set the v-table
	me_ptr->vtbl_ptr = &vtbl;

	// module is disabled by default
	me_ptr->passthru_enable = 0;
	me_ptr->event_callback.event_cb = NULL;
	me_ptr->event_callback.event_context = NULL;

	// Assuming default number of ports as 1
	me_ptr->ports.num_input_ports = 1;
	me_ptr->ports.num_output_ports = 1;

	// validate properties requested are ok to set at initialization
	// We are just printing message if not valid at init time and proceeding further
	props = init_set_properties->prop_ptr;
	for(i=0; i < init_set_properties->props_num; i++) {
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

	FARF(HIGH, "num of init set props %ld", init_set_properties->props_num);
	// Setting parameters now
	if(init_set_properties->props_num) {
		result = capi_v2_passthru_set_properties(_pif, init_set_properties);
	}

	return result;
}

#ifdef __cplusplus
}
#endif //__cplusplus

