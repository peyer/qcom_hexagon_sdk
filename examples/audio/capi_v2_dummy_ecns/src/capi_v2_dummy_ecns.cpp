/**

 @brief CAPI V2 API wrapper for Dummy ECNS example
 Currently the example, supports single, dual and quad mic use case based on
 number of channels from Voice framwork.

 A single CAPIv2 example overloads SingleMic(SM), DualMic(DM) and QuadMic(QM) dummy ECNS modules.
 Each mic configuration will have separate module Id and hence will have separate
 exposed entry functions which distinguish the dummy ECNS modules for SM/DM/QM.

 For SM Dummy ECNS: Eg: VOICE_MODULE_DUMMY_SM_ECNS 0x10027060
   capi_v2_custom_dummy_sm_ecns_get_static_properties
   capi_v2_custom_dummy_sm_ecns_init

 For DM Dummy ECNS: Eg: VOICE_MODULE_DUMMY_DM_ECNS 0x10027061
   capi_v2_custom_dummy_dm_ecns_get_static_properties
   capi_v2_custom_dummy_dm_ecns_init

 For QM Dummy ECNS: Eg: VOICE_MODULE_DUMMY_QM_ECNS 0x10027062
   capi_v2_custom_dummy_qm_ecns_get_static_properties
   capi_v2_custom_dummy_qm_ecns_init

 As it is dummy example, definitions are same for all. So, we are internally calling single function.
 User can update as different functions based on thier need.

 */

/*-----------------------------------------------------------------------
 Copyright (c) 2012-2015 Qualcomm  Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 -----------------------------------------------------------------------*/

#ifndef _DEBUG  // To enable FARF messages
#define _DEBUG
#endif

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "HAP_farf.h"

#include "string.h"
#include "Elite_CAPI_V2_properties.h"
#include "Elite_fwk_extns_ecns.h"
#include "Elite_lib_get_capi_v2_module.h"
#include "capi_v2_custom_dummy_ecns.h"
#include "adsp_vparams_api.h"
#include "adsp_vparams_internal_api.h"
#include "adsp_vpm_api.h"
#include "adsp_vcmn_api.h"
#include "dummy_ecns_lib.h" // Core library header file

#include "AEEstd.h"

/*==========================================================================
  Globals
  ========================================================================== */
#define ROUNDTO8(x) ((((uint32_t)(x) + 7) >> 3) << 3)
#define NB_SAMPLING_RATE      (8000)
#define WB_SAMPLING_RATE      (16000)
#define FB_SAMPLING_RATE      (48000)

#define SIZE_OF_ARRAY(a) (sizeof(a)/sizeof((a)[0]))
#define DUMMY_ECNS_CFG_SIZE 4000

#define OUT_BUF_SAMP_48K 960
/* KPPS values */
/* worst case profiled values */
#define DUMMY_ECNS_NB_KPPS (10000)
#define DUMMY_ECNS_WB_KPPS (15000)
#define DUMMY_ECNS_FB_KPPS (20000)

/* Delay values */
/* worst case profiled values */
#define DUMMY_ECNS_NB_DELAY   (10000)
#define DUMMY_ECNS_WB_DELAY   (15000)
#define DUMMY_ECNS_FB_DELAY   (20000)

#define SINGLE_MIC 1
#define DUAL_MIC 2
#define QUAD_MIC 4

#define SM_NEAR_PRI_NUM_CHANNELS (SINGLE_MIC)
#define DM_PRI_NUM_CHANNELS (DUAL_MIC)
#define QM_PRI_NUM_CHANNELS (QUAD_MIC)
#define FAR_NUM_CHANNELS (1)

#define NUM_INPUT_PORTS  (2)
#define NUM_OUTPUT_PORTS (1)

#define PRI_PORT_INDEX 0 
#define SEC_PORT_INDEX 1

#define INVALID    ((int32_t)(-1))


#ifndef VOICE_LOG_TAP_POINT_VPTX_RTM
#define VOICE_LOG_TAP_POINT_VPTX_RTM   0x00010F67
#endif

// Qurt Elite APIs
typedef enum
{
	QURT_ELITE_HEAP_DEFAULT=0,
	/**< Default heap value. */

	QURT_ELITE_HEAP_OUT_OF_RANGE
	/**< Heap value is out of range. */ //keep at end

}  QURT_ELITE_HEAP_ID;

typedef struct{
	capi_v2_set_get_media_format_t media_format;
	capi_v2_standard_data_format_t data_format;
} capi_data_format_struct_t;

typedef struct capi_v2_dummy_ecns_t {
	const capi_v2_vtbl_t *vtbl;
	/**< Pointer to the virtual table functions */
	void *static_buf_ptr;
	void *param_struct_ptr;
	int32_t enable;
	uint32_t module_id;
	uint32_t num_mic;
	uint32_t sampling_rate;
	uint32_t output_framesize;  //20msec frame size
	int8_t params_modified;             //flag to indicate if interface paramters are changed.
	int32_t *mem_ptr;         // memory fragment parameters
	capi_v2_event_cb_f event_cb; /**< Callback function to issue to framework to report delay and kpps */
	void* event_context; /**< Event context that must be issued in the callback */
	uint32_t delay;
	uint32_t kpps;
	capi_v2_port_num_info_t num_port_info;
	capi_data_format_struct_t input_media_type[NUM_INPUT_PORTS];
	QURT_ELITE_HEAP_ID heap_id;
	capi_data_format_struct_t output_media_type[NUM_OUTPUT_PORTS];
	int8_t primary_input_port_idx;
	int8_t primary_mic_idx;
	int8_t secondary_mic1_idx;
	int8_t secondary_mic2_idx;
	int8_t secondary_mic3_idx;
	int8_t far_input_port_idx;
	int8_t primary_output_port_idx;
	int16_t pri_num_channels;  // used to track number of input channels for primary port based on topology.
	uint32_t param_id_1;
} capi_v2_dummy_ecns_t;

static capi_v2_err_t capi_v2_custom_dummy_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr);
///static capi_v2_err_t capi_v2_custom_dummy_ecns_init(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties);
static capi_v2_err_t capi_v2_dummy_ecns_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties);

static capi_v2_err_t capi_v2_dummy_ecns_process(capi_v2_t* _pif, capi_v2_stream_data_t* input[], capi_v2_stream_data_t* output[]);
static capi_v2_err_t capi_v2_dummy_ecns_end(capi_v2_t* _pif);
static capi_v2_err_t capi_v2_dummy_ecns_set_param(capi_v2_t* _pif, uint32_t param_id, const capi_v2_port_info_t *port_info_ptr, capi_v2_buf_t *params_ptr);
static capi_v2_err_t capi_v2_dummy_ecns_get_param(capi_v2_t* _pif, uint32_t param_id, const capi_v2_port_info_t *port_info_ptr, capi_v2_buf_t *params_ptr);
///static capi_v2_err_t capi_v2_dummy_ecns_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr);
static capi_v2_err_t capi_v2_dummy_ecns_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr);


static void capi_v2_dummy_ecns_send_kpps(capi_v2_dummy_ecns_t* me);
static void capi_v2_dummy_ecns_send_delay(capi_v2_dummy_ecns_t* me);
static void capi_v2_dummy_ecns_get_kpps_delay(capi_v2_dummy_ecns_t* me, uint32_t *delay, uint32_t *kpps);
static capi_v2_err_t capi_v2_dummy_ecns_send_output_mediatype(capi_v2_dummy_ecns_t *me);
capi_v2_err_t capi_v2_voice_dummy_ecns_process_set_properties(capi_v2_dummy_ecns_t *me, capi_v2_proplist_t *init_set_properties);
capi_v2_err_t capi_v2_voice_dummy_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr);

static capi_v2_vtbl_t vtbl =
{
		capi_v2_dummy_ecns_process,
		capi_v2_dummy_ecns_end,
		capi_v2_dummy_ecns_set_param,
		capi_v2_dummy_ecns_get_param,
		capi_v2_dummy_ecns_set_properties,
		capi_v2_dummy_ecns_get_properties,
};



/*----------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * This function is used to query the static properties to create the CAPIv2.
 *
 * @param[in] init_props_ptr, pointer to the initializing prop list
 * @param[in, out] out_props_ptr, pointer to the output pro list
 * @return capi_v2_err_t, result code
 *----------------------------------------------------------------------------*/

static capi_v2_err_t capi_v2_ecns_raise_port_threshold_event(
      capi_v2_dummy_ecns_t *me, uint32_t port_id, bool_t is_input_port)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

   uint32_t bytes_per_samples = 2;

   capi_v2_port_data_threshold_change_t event = {
         160 * bytes_per_samples
   };

   capi_v2_event_info_t event_info =   {
         {
               TRUE,
               is_input_port,
               port_id
         },
         {
               reinterpret_cast<int8_t*>(&event),
               sizeof(event),
               sizeof(event)
         }
   };

   capi_v2_result |= me->event_cb( me->event_context,
         CAPI_V2_EVENT_PORT_DATA_THRESHOLD_CHANGE, &event_info);

   if( CAPI_V2_FAILED(capi_v2_result) )
   {
	   FARF(HIGH, "ecns Failed to send port_threshold event with %lu", capi_v2_result);
   }

   FARF(HIGH, "ecns raised port threshold. is_input_port(%ld) port_id(%ld) threshold (%lu)", is_input_port, port_id,
         event.new_threshold_in_bytes);
   return capi_v2_result;
}


capi_v2_err_t capi_v2_custom_dummy_sm_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr)
{

	capi_v2_err_t result;

	result = capi_v2_custom_dummy_ecns_get_static_properties(init_props_ptr, out_props_ptr);

	return result;
}


capi_v2_err_t capi_v2_custom_dummy_dm_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr)
{

	capi_v2_err_t result;

	result = capi_v2_custom_dummy_ecns_get_static_properties(init_props_ptr, out_props_ptr);

	return result;
}



capi_v2_err_t capi_v2_custom_dummy_qm_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr)
{

	capi_v2_err_t result;

	result = capi_v2_custom_dummy_ecns_get_static_properties(init_props_ptr, out_props_ptr);

	return result;
}



capi_v2_err_t capi_v2_custom_dummy_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr)
{

	capi_v2_err_t result;

	// Implement different static properties for different mic use cases in the case.
	result = capi_v2_voice_dummy_ecns_get_static_properties(init_props_ptr, out_props_ptr);

	return result;
}


/*------------------------------------------------------------------------------
 * This function updates the static property information needed by module
 * Mandatory properties to be set
	- CAPI_V2_INIT_MEMORY_REQUIREMENT
    - CAPI_V2_IS_INPLACE:
	- CAPI_V2_REQUIRES_DATA_BUFFERING
	- CAPI_V2_STACK_SIZE
	- CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS
	- CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS
	- CAPI_V2_CUSTOM_PROPERTY

 *----------------------------------------------------------------------------*/
capi_v2_err_t capi_v2_voice_dummy_ecns_get_static_properties(capi_v2_proplist_t *init_props_ptr, capi_v2_proplist_t *out_props_ptr)
{
	// out_props_ptr cannot be NULL as we need to know the prop requested
	if (NULL == out_props_ptr)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 get_static_properties() FAILED received bad property pointer !!, %p", out_props_ptr);
		return CAPI_V2_EFAILED;
	}

	uint16_t i;
	capi_v2_prop_t *prop_ptr = out_props_ptr->prop_ptr;

	for (i = 0; i < out_props_ptr->props_num; i++)
	{
		capi_v2_buf_t *payload = &prop_ptr[i].payload;

		switch (prop_ptr[i].id)
		{
		case CAPI_V2_INIT_MEMORY_REQUIREMENT:
		{
			if (payload->max_data_len >= sizeof(capi_v2_init_memory_requirement_t))
			{
				capi_v2_init_memory_requirement_t *data_ptr = (capi_v2_init_memory_requirement_t*) payload->data_ptr;
				data_ptr->size_in_bytes = sizeof(capi_v2_dummy_ecns_t);  // Specify the module memory requirement here. This memory is allocated by framework
				payload->actual_data_len = sizeof(capi_v2_init_memory_requirement_t);
			} else
			{
				FARF(ERROR, "DUMMY ECNS: CAPI V2 get_static_properties() FAILED Get Property id 0x%lx Bad param size %lu", (uint32_t) prop_ptr[i].id,
						payload->max_data_len);
				return CAPI_V2_ENEEDMORE;
			}

			break;
		}
		case CAPI_V2_IS_INPLACE:
		{
			if (payload->max_data_len >= sizeof(capi_v2_is_inplace_t))
			{
				capi_v2_is_inplace_t *data_ptr = (capi_v2_is_inplace_t*) payload->data_ptr;
				data_ptr->is_inplace = FALSE;		// No need to change inplace computing for ECNS module because input and output buffers are different
				payload->actual_data_len = sizeof(capi_v2_is_inplace_t);
			} else
			{
				FARF(ERROR, "DUMMY ECNS: Error! CAPI V2 Get Property id 0x%lx Bad param size %lu", (uint32_t) prop_ptr[i].id, payload->max_data_len);
				return CAPI_V2_ENEEDMORE;
			}
			break;
		}
		case CAPI_V2_REQUIRES_DATA_BUFFERING:
		{
			if (payload->max_data_len >= sizeof(capi_v2_requires_data_buffering_t))
			{
				capi_v2_requires_data_buffering_t *data_ptr = (capi_v2_requires_data_buffering_t*) payload->data_ptr;
				data_ptr->requires_data_buffering = FALSE; // Depending on whether the module needs data buffering to be handled by framework, set this to TRUE/FALSE
				payload->actual_data_len = sizeof(capi_v2_requires_data_buffering_t);
			} else
			{
				FARF(ERROR, "DUMMY ECNS: Error! CAPI V2 Get Property id 0x%lx Bad param size %lu", (uint32_t) prop_ptr[i].id, payload->max_data_len);
				return CAPI_V2_ENEEDMORE;
			}
			break;
		}
		case CAPI_V2_STACK_SIZE:
		{
			if (payload->max_data_len >= sizeof(capi_v2_stack_size_t))
			{
				capi_v2_stack_size_t *data_ptr = (capi_v2_stack_size_t*) payload->data_ptr;
				data_ptr->size_in_bytes = 1024; // Worst case stack size
				payload->actual_data_len = sizeof(capi_v2_stack_size_t);
			} else
			{
				FARF(ERROR, "DUMMY ECNS: Error! CAPI V2 Get Property id 0x%lx Bad param size %lu", (uint32_t) prop_ptr[i].id, payload->max_data_len);
				return CAPI_V2_ENEEDMORE;
			}

			break;
		}
		case CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS:
		{
			if (payload->max_data_len >= sizeof(capi_v2_num_needed_framework_extensions_t))
			{
				capi_v2_num_needed_framework_extensions_t *data_ptr = (capi_v2_num_needed_framework_extensions_t *) payload->data_ptr;
				data_ptr->num_extensions = 1; // Specify the number of framework extensions needed by ECNS
				payload->actual_data_len = sizeof(capi_v2_num_needed_framework_extensions_t);
			} else
			{
				FARF(ERROR, "DUMMY ECNS: Error! CAPI V2 Get Property id 0x%lx Bad param size %lu", (uint32_t) prop_ptr[i].id, payload->max_data_len);
				return CAPI_V2_ENEEDMORE;
			}
			break;
		}

		case CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS:
		{
			if (payload->max_data_len >= (sizeof(capi_v2_framework_extension_id_t)))
			{
				capi_v2_framework_extension_id_t *data_ptr = (capi_v2_framework_extension_id_t *) payload->data_ptr;
				data_ptr[0].id = FWK_EXTN_ECNS; // Define the ID of framework extensions needed by module.
				payload->actual_data_len = sizeof(capi_v2_framework_extension_id_t);
			} else
			{
				FARF(ERROR,"DUMMY ECNS: Error! CAPI V2 Get Property id 0x%lx Bad param size %lu", (uint32_t) prop_ptr[i].id, payload->max_data_len);
				return CAPI_V2_ENEEDMORE;
			}
			break;
		}
		case CAPI_V2_CUSTOM_PROPERTY:
		{
			capi_v2_get_ecns_property_t *ecns_custom_prop = (capi_v2_get_ecns_property_t *) payload->data_ptr;

			switch (ecns_custom_prop->sec_property_id)
			{
			case CAPI_V2_PROPERTY_ID_ECNS_OUTPUT_CAPABILITIES:
			{
				ecns_output_capabilities_t *ecns_output_info = (ecns_output_capabilities_t*) ecns_custom_prop->ecns_info;
				ecns_output_info->output_lec = FALSE;
				// Set this to TRUE to update framework that module provides Linear Echo Canceller output.
				// NUM_OUTPUT_PORTS need to be incremented by 1, if it is true.
				ecns_output_info->output_nr = FALSE;
				// Set this to TRUE to update framework that module provides Noise Reference output.
				// NUM_OUTPUT_PORTS need to be incremented by 1 if it is true.
				payload->actual_data_len = sizeof(capi_v2_get_ecns_property_t);
				break;
			}
			case CAPI_V2_PROPERTY_ID_ECNS_MONITORING_CAPABILITIES:
			{
				ecns_monitoring_capabilities_t *ecns_monitoring_info =  (ecns_monitoring_capabilities_t*)ecns_custom_prop->ecns_info;
				ecns_monitoring_info->is_rtm_supported=FALSE; // Set TRUE if module has real time monitoring capabilities
				payload->actual_data_len =  sizeof(capi_v2_get_ecns_property_t);
				break;
			}
			case CAPI_V2_PROPERTY_ID_ECNS_VP3_CAPABILITIES:
			{
				ecns_vp3_capabilities_t *ecns_vp3_info =  (ecns_vp3_capabilities_t*)ecns_custom_prop->ecns_info;
				ecns_vp3_info->is_vp3_supported=FALSE; // Set to TRUE if module supports VP3
				payload->actual_data_len =  sizeof(capi_v2_get_ecns_property_t);
				break;
			}
			case CAPI_V2_PROPERTY_ID_ECNS_RATE_MATCHING_CAPABILITIES:
			{
				ecns_rate_matching_capabilities_t *ecns_rate_matching_info =  (ecns_rate_matching_capabilities_t*)ecns_custom_prop->ecns_info;
				ecns_rate_matching_info->is_rate_matching_supported=FALSE; // Set to TRUE if module supports rate matching/sample slip

				payload->actual_data_len =  sizeof(capi_v2_get_ecns_property_t);
				break;
			}
			case CAPI_V2_PROPERTY_ID_ECNS_STT_CAPABILITIES:
			{
				ecns_stt_capabilities_t *ecns_stt_info =  (ecns_stt_capabilities_t*)ecns_custom_prop->ecns_info;
				ecns_stt_info->is_stt_supported=FALSE;
				payload->actual_data_len =  sizeof(capi_v2_get_ecns_property_t);
				break;
			}
			case CAPI_V2_PROPERTY_ID_MULTICHANNEL_REF_CAPABILITIES:
			{
				ecns_multichannel_ref_capabilities_t *ecns_multichannel_ref_info =  (ecns_multichannel_ref_capabilities_t*)ecns_custom_prop->ecns_info;
				ecns_multichannel_ref_info->is_multichannel_ref_supported = FALSE;
				break;
			}
			default:
			{
				FARF(HIGH, "DUMMY ECNS: ERROR !! CAPI V2 Invalid CUSTOM property id %lx", ecns_custom_prop->sec_property_id);
				break;
			}
			}
			break;
		}

		default:
		{
			FARF(ERROR, "DUMMY ECNS: CAPI V2 get_static_properties() FAILED Get Property for 0x%x. Not supported.", prop_ptr[i].id);
			return CAPI_V2_EBADPARAM;
		}
		}
		FARF(HIGH, "DUMMY ECNS: CAPI V2 get_static_properties() Get Property for 0x%x done", prop_ptr[i].id);
	}
	return CAPI_V2_EOK;
}

/*------------------------------------------------------------------------------
 * This function is used init the CAPIv2 lib.
 *
 * @param[in] _pif, pointer to the CAPIv2 lib.
 * @param[in] init_set_properties, pointer to the prop list that needs to be init'ed.
 *
 * @return capi_v2_err_t, result code
 *----------------------------------------------------------------------------*/

capi_v2_err_t capi_v2_custom_dummy_sm_ecns_init(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties)
{
	capi_v2_err_t result;

	if (NULL == _pif)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 Init FAILED received bad pointer");
		return CAPI_V2_EFAILED;
	}
	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t *) _pif;

	// Clean the structure first
	memset(me, 0, sizeof(capi_v2_dummy_ecns_t));
	me->enable = 0 ; // Disable module by default
	//me->module_id = VOICE_MODULE_DUMMY_SM_ECNS;
	me->param_id_1 = 0;

	me->num_mic = SINGLE_MIC; // set the number of mics/channels required by the module.
	// Here, for ECNS, number of channels are assumed as equal to number of mics.
	// Later you can extract the number of channels from voice framework
	// and compare with required channels by module.

	FARF(HIGH, "DUMMY ECNS:init Number of Mics/Channels on primary port: %d", me->num_mic);


	/* Set the vtable to allow the processing */
	me->vtbl = &vtbl;

	result=capi_v2_dummy_ecns_set_properties(_pif, init_set_properties);

	dummy_ecns_lib_init(); //initialize core library accordingly, here it is just single dummy call for all mic scenarios

	return result;

}

capi_v2_err_t capi_v2_custom_dummy_dm_ecns_init(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties)
{
	capi_v2_err_t result;

	if (NULL == _pif)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 Init FAILED received bad pointer");
		return CAPI_V2_EFAILED;
	}
	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t *) _pif;

	// Clean the structure first
	memset(me, 0, sizeof(capi_v2_dummy_ecns_t));
	me->enable = 0 ; // Disable module by default
	//me->module_id = VOICE_MODULE_DUMMY_DM_ECNS;
	me->param_id_1 = 0;

	me->num_mic = DUAL_MIC;   // set the number of mics/channels required by the module.
	// Here, for ECNS, number of channels are assumed as equal to number of mics.
	// Later you can extract the number of channels from voice framework
	// and compare with required channels by module.
	FARF(HIGH, "DUMMY ECNS:init Number of Mics/Channels on primary port: %d", me->num_mic);

	/* Set the vtable to allow the processing */
	me->vtbl = &vtbl;

	result=capi_v2_dummy_ecns_set_properties(_pif, init_set_properties);

	dummy_ecns_lib_init(); //initialize core library accordingly, here it is just single dummy call for all mic scenarios

	return result;

}

capi_v2_err_t capi_v2_custom_dummy_qm_ecns_init(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties)
{
	capi_v2_err_t result;

	if (NULL == _pif)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 Init FAILED received bad pointer");
		return CAPI_V2_EFAILED;
	}
	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t *) _pif;

	// Clean the structure first
	memset(me, 0, sizeof(capi_v2_dummy_ecns_t));
	me->enable = 0 ; // Disable module by default
	//me->module_id = VOICE_MODULE_DUMMY_QM_ECNS;
	me->param_id_1 = 0;

	me->num_mic = QUAD_MIC;   // set the number of mics/channels required by the module.
	// Here, for ECNS, number of channels are assumed as equal to number of mics.
	// Later you can extract the number of channels from voice framework
	// and compare with required channels by module.

	FARF(HIGH, "DUMMY ECNS:init Number of Mics/Channels on primary port: %d", me->num_mic);

	/* Set the vtable to allow the processing */
	me->vtbl = &vtbl;


	me->primary_mic_idx = me->secondary_mic1_idx = INVALID;
	me->secondary_mic2_idx = me->secondary_mic3_idx = INVALID;

	result=capi_v2_dummy_ecns_set_properties(_pif, init_set_properties);

	dummy_ecns_lib_init(); //initialize core library accordingly, here it is just single dummy call for all mic scenarios

	return result;

}

capi_v2_err_t capi_v2_dummy_ecns_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties)
{
	capi_v2_err_t result;

	if (NULL == _pif)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 Init FAILED received bad pointer");
		return CAPI_V2_EFAILED;
	}

	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t *) _pif;

	result=capi_v2_voice_dummy_ecns_process_set_properties(me, init_set_properties);

	return result;

}

/*------------------------------------------------------------------------------
 * This function initializes the static property information for the module
 * Following properties are set by framework and should be handled by module.
	- CAPI_V2_INPUT_MEDIA_FORMAT
    - CAPI_V2_EVENT_CALLBACK_INFO
	- CAPI_V2_PORT_NUM_INFO
	- CAPI_V2_HEAP_ID
 *----------------------------------------------------------------------------*/
capi_v2_err_t capi_v2_voice_dummy_ecns_process_set_properties(capi_v2_dummy_ecns_t *me, capi_v2_proplist_t *init_set_properties)
{

	uint32_t i, result = 0;
	uint32_t event_cb_idx = -1;
	capi_v2_event_cb_f cb_func =NULL;
	void* cb_context = NULL;


	if (NULL == me)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 Init FAILED received bad pointer");
		return CAPI_V2_EFAILED;
	}

	if (NULL == init_set_properties || NULL == init_set_properties->prop_ptr)
	{
		FARF(ERROR, "DUMMY ECNS: CAPI V2 Init FAILED received bad pointer for init props");
		return CAPI_V2_EFAILED;
	}

	capi_v2_prop_t *prop_ptr = init_set_properties->prop_ptr;

	for (i = 0; i < init_set_properties->props_num; i++)
	{
		capi_v2_buf_t *payload = &prop_ptr[i].payload;
		switch (uint32_t(prop_ptr[i].id))
		{
		case CAPI_V2_INPUT_MEDIA_FORMAT:
		{
			capi_v2_port_info_t input_port_info = prop_ptr[i].port_info;
			capi_data_format_struct_t *dummy_ecns_data_format_ptr = (capi_data_format_struct_t*) payload->data_ptr;
			//for (uint32_t i = 0; i < me->num_port_info.num_input_ports; i++)
			for (uint32_t i = 0; i < payload->actual_data_len/sizeof(capi_data_format_struct_t); i++)
			{
				// Payload can be no smaller than the header for media format
				if (payload->actual_data_len < sizeof(capi_data_format_struct_t))                  {
					FARF(ERROR, "DUMMY ECNS: Error! Size mismatch for input format property, got %lu", payload->actual_data_len);
					//bail out
					return CAPI_V2_EBADPARAM;
				}

				if (CAPI_V2_FIXED_POINT != dummy_ecns_data_format_ptr->media_format.format_header.data_format)
				{
					FARF(ERROR, "DUMMY ECNS: Error! unsupported media format %d",
							(int) dummy_ecns_data_format_ptr[i].media_format.format_header.data_format);
					return CAPI_V2_EBADPARAM;
				}
				capi_v2_standard_data_format_t *pcm_format_ptr = &dummy_ecns_data_format_ptr->data_format;
				// We only care about number of channels and sampling rate
				if (((NB_SAMPLING_RATE != pcm_format_ptr->sampling_rate) && (WB_SAMPLING_RATE != pcm_format_ptr->sampling_rate)
						&& (FB_SAMPLING_RATE != pcm_format_ptr->sampling_rate)) || (16 != pcm_format_ptr->bits_per_sample))
				{
					FARF(ERROR, "DUMMY ECNS: Error! invalid format chan %lu, SR %lu, bits per samp %lu", pcm_format_ptr->num_channels,
							pcm_format_ptr->sampling_rate, pcm_format_ptr->bits_per_sample);
					return CAPI_V2_EBADPARAM;
				}



				FARF(HIGH, "DUMMY ECNS: Number of Mics/Channels on primary port: %d", me->num_mic);


				FARF(HIGH, "Meida format port_index=%d,sampling_rate=%d,bits_per_sample=%d,num_channels=%d,",input_port_info.port_index,pcm_format_ptr->sampling_rate,pcm_format_ptr->bits_per_sample,pcm_format_ptr->num_channels);
				if(input_port_info.port_index == PRI_PORT_INDEX)// FWK will pass the information when input MIC port is running
				{
					// Extract the number of channels from voice framework and compare with required channels by module.
					// Here, for ECNS, number of channels are assumed as equal to number of mics.
					// Fail if, num channels supported by module is different from num channels sent by framework
					//if (me->num_mic != dummy_ecns_data_format_ptr[i].data_format.num_channels)
					//{
					//      FARF(ERROR, "DUMMY ECNS: Error! Invalid number of channels (%ld) for primary port required by the module (%ld)",
					//                          dummy_ecns_data_format_ptr[i].data_format.num_channels, me->num_mic);
					//      return CAPI_V2_EBADPARAM;
					//}
					FARF(HIGH, "DUMMY ECNS: number of channels (%ld) for primary port required by the module (%ld)",
							dummy_ecns_data_format_ptr[i].data_format.num_channels, me->num_mic);

					me->primary_input_port_idx = PRI_PORT_INDEX;
					me->input_media_type[PRI_PORT_INDEX]= dummy_ecns_data_format_ptr[i];
					me->output_media_type[0] = me->input_media_type[PRI_PORT_INDEX];
					me->primary_output_port_idx = 0;
					switch (me->num_mic)
					{
					case SINGLE_MIC:
					{
						FARF(HIGH, "DUMMY ECNS: SINGLE_MIC: Number of Channels (%d) on Primary Port with Channel Type:%d",
								me->num_mic, dummy_ecns_data_format_ptr[i].data_format.channel_type[0]);
						// Set the primary mic index to 0 for Single mic topology
						// Use index i for secondary mic
						me->primary_mic_idx = 0;
						me->pri_num_channels = SM_NEAR_PRI_NUM_CHANNELS;
						break;
					}
					case DUAL_MIC:
					{
						FARF(HIGH, "DUMMY ECNS: DUAL_MIC: Number of Channels (%d) on Primary Port with Channel Types:%d, %d",
								me->num_mic, dummy_ecns_data_format_ptr[i].data_format.channel_type[0], dummy_ecns_data_format_ptr[i].data_format.channel_type[1]);
						me->pri_num_channels = DM_PRI_NUM_CHANNELS;
						me->primary_mic_idx = 0;
						me->secondary_mic1_idx =1;
						break;
					}
					case QUAD_MIC:
					{
						FARF(HIGH, "DUMMY ECNS: QUAD_MIC: Number of Channels (%d) on Primary Port", me->num_mic);
						me->pri_num_channels = QM_PRI_NUM_CHANNELS;
						me->primary_mic_idx = 0;
						me->secondary_mic1_idx = 1;
						me->secondary_mic2_idx = 2;
						me->secondary_mic3_idx = 3;
						break;
					}

					}

					capi_v2_ecns_raise_port_threshold_event(me, PRI_PORT_INDEX, TRUE);
				}

				if(input_port_info.port_index == 1)	// FWK will pass the information only when the reference data port is running
				{
					me->far_input_port_idx = SEC_PORT_INDEX;
					me->input_media_type[SEC_PORT_INDEX]= dummy_ecns_data_format_ptr[i];

					// Fail if, num channels supported by module is different from num channels sent by framework
					if (FAR_NUM_CHANNELS != dummy_ecns_data_format_ptr[i].data_format.num_channels)
					{
						FARF(ERROR, "DUMMY ECNS: Error! invalid number of channels (%ld) for secondary port ",
								dummy_ecns_data_format_ptr[i].data_format.num_channels);
						return CAPI_V2_EBADPARAM;
					}
					FARF(HIGH, "DUMMY ECNS: Secondary Port with Channel Type: %d", dummy_ecns_data_format_ptr[i].data_format.channel_type[0]);
					FARF(HIGH, "DUMMY ECNS: Far Port at index %d, number of channels (%ld) for secondary port", i, dummy_ecns_data_format_ptr[i].data_format.num_channels);
					capi_v2_ecns_raise_port_threshold_event(me, SEC_PORT_INDEX, TRUE);
					break;
				}

			}

			me->sampling_rate = dummy_ecns_data_format_ptr[0].data_format.sampling_rate;
			me->output_framesize = (me->sampling_rate/1000)*20; // 20 msec frame size
			FARF(HIGH, "DUMMY ECNS: CAPI V2 Sampling rate (%d), Output Framesize=(%ld), number of channels (%ld) on primary port", me->sampling_rate,me->output_framesize, me->num_mic);

			// Here, ECNS might have different output media format than the input media format.
			// So, output media format is provided to the framework by the module
			// using an event call back during CAPI_V2_INPUT_MEDIA_FORMAT
			capi_v2_dummy_ecns_send_output_mediatype(me);
			break;
		}
		case CAPI_V2_OUTPUT_MEDIA_FORMAT:
		{
			// In CAPIv2 Interface, CAPI_V2_OUTPUT_MEDIA_FORMAT is upadted as an event callback of CAPI_V2_INPUT_MEDIA_FORMAT
			// So, nothing to be done here.
			FARF(HIGH,"DUMMY ECNS: CAPIv2 dummy ECNS received output media fmt");
			break;
		}
		case CAPI_V2_EVENT_CALLBACK_INFO:
		{
			// Set the event callback pointer
			// This is required so that module can raise events to framework later.
			capi_v2_event_callback_info_t *cb_info_ptr = (capi_v2_event_callback_info_t*) payload->data_ptr;
			event_cb_idx = i;
			//FARF (HIGH, "DUMMY ECNS: Debug: In Init function case CAPI_V2_EVENT_CALLBACK_INFO, cb_info_ptr->event_cb = %x", cb_info_ptr->event_cb );
			cb_func = me->event_cb = cb_info_ptr->event_cb;
			//function pointer for event callback
			cb_context = me->event_context = cb_info_ptr->event_context;
			// state required for event callback
			//FARF (HIGH, "DUMMY ECNS: Debug: In Init function case CAPI_V2_EVENT_CALLBACK_INFO, me->event_cb = %x", me->event_cb );


			break;
		}

		case CAPI_V2_ALGORITHMIC_RESET:
		{
			dummy_ecns_lib_reset(); // reset core processing library

			FARF(HIGH,  "DUMMY ECNS: Algorithm Reset done");
			break;
		}
		case CAPI_V2_PORT_NUM_INFO:
		{
			capi_v2_port_num_info_t *num_port_info = (capi_v2_port_num_info_t*) payload->data_ptr;
			if ((num_port_info->num_input_ports != NUM_INPUT_PORTS) || (num_port_info->num_output_ports != NUM_OUTPUT_PORTS))
			{
				FARF(ERROR, "DUMMY ECNS: Error! incorrect number of input ( %lu) or output ( %lu) ports", num_port_info->num_input_ports,
						num_port_info->num_output_ports);
				//bail out
				return CAPI_V2_EBADPARAM;
			}
			// Update module about port information
			me->num_port_info = *num_port_info;
			FARF(HIGH, "DUMMY ECNS: CAPI V2 received port info: Number of input (%lu) ports and  Number of output (%lu) ports",
					num_port_info->num_input_ports, num_port_info->num_output_ports);

			break;

		}

		case CAPI_V2_HEAP_ID:
		{
			if (sizeof(capi_v2_heap_id_t) != payload->actual_data_len)
			{
				FARF(ERROR, "DUMMY ECNS: CAPI V2 received bad size(%ld) for heap id property", payload->actual_data_len);
				return CAPI_V2_EFAILED;
			}
			// Update heap ID to module.
			// This heap id is used to allocate memory using Qurt Elite API 'qurt_elite_memory_malloc' subsequently
			capi_v2_heap_id_t* heap_id_ptr = (capi_v2_heap_id_t*) payload->data_ptr;
			//memscpy(&me->heap_id, sizeof(me->heap_id), heap_id_ptr, payload->actual_data_len);
			std_memscpy(&me->heap_id, sizeof(me->heap_id), heap_id_ptr, payload->actual_data_len);
			FARF(HIGH,"DUMMY ECNS: Heap id is set.");
			break;
		}

		default:
		{
			FARF(HIGH, "DUMMY ECNS: Unsupported prop_id 0x%lx", (uint32_t) prop_ptr[i].id);
			return CAPI_V2_EFAILED;
		}
		}
	}
	return result;

}

/*------------------------------------------------------------------------------
 * This function is responsible for processing of data.
 *
 * @param[in] _pif, pointer to the CAPI lib.
 * @param[in] input, pointer to input data buffer.
 * @param[in] output, pointer to output data buffer.
 *
 * @return capi_v2_err_t, result code
 * It does passthrough of primary input port data to output port.
 *----------------------------------------------------------------------------*/

static capi_v2_err_t capi_v2_dummy_ecns_process(capi_v2_t* _pif, capi_v2_stream_data_t* input[], capi_v2_stream_data_t* output[])
{
	int16 *ec_out_ptr;
	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t *) _pif;


	int16 *ptr_Tx_Input[4] = {(int16*) (input[me->primary_input_port_idx]->buf_ptr[me->primary_mic_idx].data_ptr), NULL, NULL, NULL};
	int16 *ptr_Rx_Input[4] = {(int16*) (input[me->far_input_port_idx]->buf_ptr[0].data_ptr), NULL, NULL, NULL};
	uint32_t nSamples_near = input[me->primary_input_port_idx]->buf_ptr[me->primary_mic_idx].actual_data_len >> 1;
	uint32_t nSamples_far = input[me->far_input_port_idx]->buf_ptr[0].actual_data_len>>1;

	FARF(HIGH, "DUMMY ECNS: me->primary_input_port_idx %d, me->far_input_port_idx %d",
			me->primary_input_port_idx, me->far_input_port_idx);


	if(nSamples_near != 0 || nSamples_far != 0)
		FARF(HIGH, "DUMMY ECNS: Invoking Process Samples %d, actual_size %d, ref actual_size %d",nSamples_near,input[0]->buf_ptr[0].actual_data_len, input[0]->buf_ptr[1].actual_data_len);
	//FARF(ERROR, "capi_v2_dummy_ecns_process: Process");
	if ( SM_NEAR_PRI_NUM_CHANNELS != me->pri_num_channels)
	{
		ptr_Tx_Input[1] = (int16*) (input[me->primary_input_port_idx]->buf_ptr[me->secondary_mic1_idx].data_ptr);
		if(QM_PRI_NUM_CHANNELS == me->pri_num_channels)
		{
			ptr_Tx_Input[2] = (int16*) (input[me->primary_input_port_idx]->buf_ptr[me->secondary_mic2_idx].data_ptr);
			ptr_Tx_Input[3] = (int16*) (input[me->primary_input_port_idx]->buf_ptr[me->secondary_mic3_idx].data_ptr);
		}
	}

	ec_out_ptr = (int16 *) output[me->primary_output_port_idx]->buf_ptr[0].data_ptr;
	//output[me->primary_output_port_idx]->buf_ptr[0].actual_data_len = me->output_framesize<<1;  //setting up primary output size 160<<1 = >320
	output[me->primary_output_port_idx]->buf_ptr[0].actual_data_len = input[me->primary_input_port_idx]->buf_ptr[0].actual_data_len;
	output[me->primary_output_port_idx]->buf_ptr[0].max_data_len = input[me->primary_input_port_idx]->buf_ptr[0].max_data_len;
	uint32_t near_ch0_bufsize = input[me->primary_input_port_idx]->buf_ptr[0].max_data_len;    // it is from the input buffer size (framework)

	if((me->enable))
	{
		if(ec_out_ptr!=ptr_Tx_Input[0])
		{
			dummy_ecns_lib_process(ec_out_ptr, near_ch0_bufsize, ptr_Tx_Input[0], nSamples_near<<1, ptr_Rx_Input[0], nSamples_far);

			// Just to check if recieving proper reference input
			//dummy_ecns_lib_process(ec_out_ptr, near_ch0_bufsize, ptr_Rx_Input[0], nSamples_near<<1, ptr_Rx_Input[0], nSamples_far);
		}
	}

	return CAPI_V2_EOK;
}

/*------------------------------------------------------------------------------
 * This function returns the module to the uninitialized state and can be used
 * to free the memory that was allocated by module.
 * This function also frees the virtual function table.
 *----------------------------------------------------------------------------*/
capi_v2_err_t capi_v2_dummy_ecns_end(capi_v2_t* _pif)
{
	if (NULL == _pif)
	{
		FARF(ERROR, "DUMMY ECNS: FAILED received bad property pointer");
		return CAPI_V2_EFAILED;
	}

	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t*) _pif;

	me->static_buf_ptr = NULL;
	me->param_struct_ptr = NULL;
	me->vtbl = NULL;

	FARF(HIGH, "DUMMY ECNS: CAPI_V2 End done");
	return CAPI_V2_EOK;
}

/*------------------------------------------------------------------------------
 * This function is responsible for setting parameters in module.
 *
 * @param[in] _pif, pointer to the CAPIv2 lib.
 * @param[in] param_id, parameter id sent from client processor.
 * @param[in] port_info_ptr, pointer to port.
 * @param[in] params_ptr, data pointer to parameters.
 *
 * @return capi_v2_err_t, result code
 *----------------------------------------------------------------------------*/

capi_v2_err_t capi_v2_dummy_ecns_set_param(capi_v2_t* _pif, uint32_t param_id, const capi_v2_port_info_t *port_info_ptr, capi_v2_buf_t *params_ptr)
{
	if (NULL == _pif || NULL == params_ptr)
	{
		FARF(ERROR, "DUMMY ECNS: FAILED received bad property pointer for param_id property, 0x%lx", param_id);
		return CAPI_V2_EFAILED;
	}

	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t*) _pif;
	capi_v2_err_t nResult = CAPI_V2_EOK;
	switch (param_id)
	{
	case VOICE_PARAM_MOD_ENABLE:
	case DUMMY_ECNS_PARAM_MOD_ENABLE:
	{
		int16_t nEnableFlag = *((int16_t*) params_ptr->data_ptr);
		if (sizeof(int32_t) != params_ptr->actual_data_len)
		{
			FARF(ERROR, "DUMMY ECNS: voice DUMMY ECNS set_param: Bad Param Size");
			nResult = CAPI_V2_EBADPARAM;
			break;
		}
		if (me->enable != nEnableFlag)
		{

			me->enable = (int32_t) nEnableFlag;
			if (TRUE == nEnableFlag)
			{
				me->params_modified = TRUE;
			}
			FARF(HIGH, "DUMMY ECNS: enable(%ld)",me->enable);
		}
		FARF(HIGH, "david DUMMY ECNS: enable(%ld)",me->enable);
		if (NULL == me->event_cb)
		{
			FARF(ERROR,"DUMMY ECNS: Event callback is not set. Unable to raise process event!");
			nResult = CAPI_V2_EBADPARAM;
			break;
		}

		capi_v2_event_process_state_t event;
		event.is_enabled = (bool_t)((me->enable ==0) ? 0 : 1);
		capi_v2_event_info_t event_info;
		event_info.port_info.is_valid = FALSE;
		event_info.payload.actual_data_len = sizeof(event);
		event_info.payload.max_data_len = sizeof(event);
		event_info.payload.data_ptr = (int8_t*)(&event);

		FARF (HIGH, "DUMMY Debug : Event callback at CAPI_V2_EVENT_PROCESS_STATE = %x", me->event_cb);
		nResult = me->event_cb(me->event_context,CAPI_V2_EVENT_PROCESS_STATE,
				&event_info);
		if (CAPI_V2_FAILED(nResult))
		{
			FARF(ERROR,
					"DUMMY ECNS: Failed to send process_check update event with %lu", nResult);
		}

		break;
	}
	case DUMMY_ECNS_PARAM_ID_1:
	{
		int16_t param_1 = *((int16_t*) params_ptr->data_ptr);
		if (sizeof(int32_t) != params_ptr->actual_data_len)
		{
			FARF(ERROR, "DUMMY ECNS: voice DUMMY ECNS set_param :: Bad Param Size for param id = %x ", DUMMY_ECNS_PARAM_ID_1);
			nResult = CAPI_V2_EBADPARAM;
			break;
		}

		me->param_id_1 = (uint32_t) param_1;
	}
	default:
	{
		FARF(ERROR, "DUMMY ECNS: voice DUMMY ECNS set_param :: Param ID not supported");
		nResult = CAPI_V2_EBADPARAM;
		break;
	}
	}
	uint32_t new_kpps = 0, new_delay = 0;

	if (me->enable)
	{
		capi_v2_dummy_ecns_get_kpps_delay(me,&new_delay,&new_kpps);
	}

	if (new_kpps != me->kpps)
	{
		me->kpps = new_kpps;
		capi_v2_dummy_ecns_send_kpps(me);
	}

	if (new_delay != me->delay)
	{
		me->delay = new_delay;
		capi_v2_dummy_ecns_send_delay(me);
	}

	return nResult;
}

/*------------------------------------------------------------------------------
 * This function is responsible for getting parameters of module.
 *
 * @param[in] _pif, pointer to the CAPIv2 lib.
 * @param[in] param_id, parameter id sent from client processor.
 * @param[in] port_info_ptr, pointer to port.
 * @param[in] params_ptr, data pointer to parameters to be written by module.
 *
 * @return capi_v2_err_t, result code
 *----------------------------------------------------------------------------*/

capi_v2_err_t capi_v2_dummy_ecns_get_param(capi_v2_t* _pif, uint32_t param_id, const capi_v2_port_info_t *port_info_ptr, capi_v2_buf_t *params_ptr)
{
	if (NULL == _pif || NULL == params_ptr || NULL == params_ptr->data_ptr)
	{
		FARF(ERROR, "DUMMY ECNS: FAILED received bad property pointer for param_id property, 0x%lx", param_id);
		return CAPI_V2_EFAILED;
	}
	uint32_t nBufferSize = params_ptr->max_data_len;
	uint16_t* nParamSize = (uint16_t*) &(params_ptr->actual_data_len);
	int8_t* pParamsBuffer = params_ptr->data_ptr;
	capi_v2_err_t nResult = CAPI_V2_EOK;

	capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t*) _pif;

	switch (param_id)
	{
	case VOICE_PARAM_MOD_ENABLE:
	{
		*nParamSize = sizeof(int32_t);
		if (*nParamSize > nBufferSize)
		{
			FARF(ERROR, "DUMMY ECNS: voice DUMMYECNS get_param :: on/off required size = %u, given size = %ld", *nParamSize, nBufferSize);
			return CAPI_V2_ENOMEMORY;
		}
		*((int32_t*) pParamsBuffer) = 0;  // Clearing the whole buffer
		*((int16_t*) pParamsBuffer) = (int16_t) me->enable;
		break;
	}
	case  DUMMY_ECNS_PARAM_ID_1:
	{
		*nParamSize = sizeof(int32_t);
		if (*nParamSize > nBufferSize)
		{
			FARF(ERROR, "DUMMY ECNS: DUMMY_ECNS_PARAM_ID_1 get_param :: on/off required size = %u, given size = %ld", *nParamSize, nBufferSize);
			return CAPI_V2_ENOMEMORY;
		}
		*((int16_t*) pParamsBuffer) = (int16_t) me->param_id_1;
		break;
	}
	default:
	{
		FARF(ERROR, "DUMMY ECNS: voice DUMMY ECNS get_param :: Param ID not supported");
		return CAPI_V2_EUNSUPPORTED;
		break;
	}
	}
	return nResult;
}

/*------------------------------------------------------------------------------
 * This function is responsible for setting properties to module.
 *
 * @param[in] _pif, pointer to the CAPIv2 lib
 * @param[in] props_ptr, pointer to various properties
 *
 * @return capi_v2_err_t, result code
 *----------------------------------------------------------------------------*/
/*
static capi_v2_err_t capi_v2_dummy_ecns_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
   if (!_pif || !props_ptr)
   {
      FARF(ERROR, "DUMMY ECNS: Error! Received bad pointer in set_properties");
      return CAPI_V2_EFAILED;
   }

   capi_v2_dummy_ecns_t *me = (capi_v2_dummy_ecns_t*) _pif;
   uint32_t i;
   for (i = 0; i < props_ptr->props_num; i++)
   {
      capi_v2_prop_t* current_prop_ptr = &(props_ptr->prop_ptr[i]);
      capi_v2_buf_t* payload_ptr = &(current_prop_ptr->payload);
      switch (current_prop_ptr->id)
      {
         case CAPI_V2_EVENT_CALLBACK_INFO:
            {
              // Set the event callback pointer
              // This is required so that module can raise events to framework later.
               capi_v2_event_callback_info_t *cb_info_ptr = (capi_v2_event_callback_info_t*) payload_ptr->data_ptr;
               me->event_cb = cb_info_ptr->event_cb;
                              //function pointer for event callback
               me->event_context = cb_info_ptr->event_context;
                              // state required for event callback
               break;
            }
         case CAPI_V2_ALGORITHMIC_RESET:
            {
			   dummy_ecns_lib_reset(); // reset core processing library

               FARF(HIGH,  "DUMMY ECNS: Algorithm Reset done");
               break;
            }
         default:
            {
               FARF(HIGH, "DUMMY ECNS: Unsupported prop ID %lx", current_prop_ptr->id);
               return CAPI_V2_EUNSUPPORTED;
            }
      }
   }
   return CAPI_V2_EOK;
}
 */

static capi_v2_err_t capi_v2_dummy_ecns_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
	return CAPI_V2_EUNSUPPORTED;
}


/*------------------------------------------------------------------------------
 * This function is responsible for updating the kpps to caller service using
 * event callback functionality.
 *
 * @param[in] me, Module pointer.
 *
 *----------------------------------------------------------------------------*/

void capi_v2_dummy_ecns_send_kpps(capi_v2_dummy_ecns_t* me)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_event_info_t kpps_event;
	capi_v2_event_KPPS_t kpps_payload;

	kpps_payload.KPPS = me->kpps;
	kpps_event.payload.data_ptr = (int8_t*) &kpps_payload;
	kpps_event.payload.actual_data_len = kpps_event.payload.max_data_len = sizeof(kpps_payload);

	kpps_event.port_info.is_valid = FALSE;

	//FARF (HIGH, "DUMMY ECNS: Debug: Calling event callback for CAPI_V2_EVENT_KPPS");
	result = me->event_cb(me->event_context, CAPI_V2_EVENT_KPPS, &kpps_event);

	if (result)
	{
		FARF(ERROR, "DUMMY ECNS: Error! kpps reporting failed, error %ld", result);
	}
	//FARF (HIGH, "DUMMY ECNS: Debug: After calling event callback for CAPI_V2_EVENT_KPPS");

}

/*------------------------------------------------------------------------------
 * This function is responsible for updating the algorithm delay to caller
 * service using event callback functionality.
 *
 * @param[in] me, Module pointer.
 *
 *----------------------------------------------------------------------------*/

void capi_v2_dummy_ecns_send_delay(capi_v2_dummy_ecns_t* me)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_event_info_t algorithmic_delay_event;
	capi_v2_event_algorithmic_delay_t algorithmic_delay_payload;

	algorithmic_delay_payload.delay_in_us = me->delay;
	algorithmic_delay_event.payload.data_ptr = (int8_t*) &algorithmic_delay_payload;
	algorithmic_delay_event.payload.actual_data_len = algorithmic_delay_event.payload.max_data_len = sizeof(algorithmic_delay_payload);

	algorithmic_delay_event.port_info.is_valid = FALSE;

	result = me->event_cb(me->event_context, CAPI_V2_EVENT_ALGORITHMIC_DELAY, &algorithmic_delay_event);

	if (result)
	{
		FARF(ERROR, "DUMMY ECNS: Error! algorithmic_delay reporting failed, error %ld", result);
	}
}

/*------------------------------------------------------------------------------
 * This function is responsible for updating the module about algorithm delay
 * and kpps depending on topology id and sampling rate
 *
 * @param[in] me, Module pointer.
 * @param[in] delay_value, pointer to store delay.
 * @param[in] kpps_value, pointer to store kpps.
 *
 *----------------------------------------------------------------------------*/

void capi_v2_dummy_ecns_get_kpps_delay(capi_v2_dummy_ecns_t* me, uint32_t *delay_value, uint32_t *kpps_value)
{
	uint32_t kpps = 0,delay =0;

	if (me->enable)
	{
		switch (me->num_mic)
		{
		// Add proper kpps and delay values based on single, dual and quad mic requirements
		// Here, it is clubbed in only one, as it is dummy ECNS example.
		case SINGLE_MIC:
		case DUAL_MIC:
		case QUAD_MIC:
		{
			switch (me->sampling_rate)
			{
			case NB_SAMPLING_RATE:
				kpps = DUMMY_ECNS_NB_KPPS;
				delay = DUMMY_ECNS_NB_DELAY;
				break;
			case WB_SAMPLING_RATE:
				kpps = DUMMY_ECNS_WB_KPPS;
				delay = DUMMY_ECNS_WB_DELAY;
				break;
			case FB_SAMPLING_RATE:
				kpps = DUMMY_ECNS_FB_KPPS;
				delay = DUMMY_ECNS_FB_DELAY;
				break;
			default:
				FARF(ERROR, "DUMMY ECNS: sampling rate  %ld not supported by DUMMY FV5", me->sampling_rate);
				break;
			}
			break;
		}
		}
	}
	*delay_value = delay;
	*kpps_value = kpps;
	return;
}

/*------------------------------------------------------------------------------
 * This function is responsible for updating the output media format
 * The output media type is provided to the framework by the module using
 * an event callback which may be raised at any time and depends on the module.
 *
 * @param[in] me, Module pointer.
 *
 * @return capi_v2_err_t, result code
 *----------------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_dummy_ecns_send_output_mediatype(capi_v2_dummy_ecns_t *me)
{
	capi_v2_err_t result = CAPI_V2_EOK;
	capi_v2_event_info_t process_event_info;

	if (NULL == me->event_context)
	{
		return CAPI_V2_EBADPARAM;
	}

	process_event_info.port_info.is_valid = TRUE;
	process_event_info.port_info.is_input_port = FALSE;
	process_event_info.port_info.port_index = 0;

	process_event_info.payload.actual_data_len = sizeof(me->output_media_type[0]);
	process_event_info.payload.data_ptr = (int8_t *) &me->output_media_type;
	process_event_info.payload.max_data_len = sizeof(me->output_media_type[0]);
	//FARF (HIGH, "DUMMY ECNS: Debug: Event callback at capi_v2_dummy_ecns_send_output_mediatype = %x", me->event_cb);
	result = me->event_cb(me->event_context, CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED, &process_event_info);

	if (CAPI_V2_EOK != result)
	{
		FARF(ERROR, "DUMMY ECNS: david Failed to raise event for output media type");
	} else
	{
		FARF(HIGH, "DUMMY ECNS: Raised event for output media type");
	}

	return result;
}
