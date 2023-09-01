/* ======================================================================== */
/**
   @file capi_v2_voicewakeup.cpp

   C source file to implement the Audio Post Processor Interface for
   voicewakeup example
 */

/* =========================================================================
   Copyright (c) 2015 QUALCOMM Technologies Incorporated.
   All rights reserved. Qualcomm Technologies Proprietary and Confidential.
   ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef _DEBUG  // To enable FARF messages
#define _DEBUG
#endif
#include "capi_v2_voicewakeup.h"
#include "capi_v2_voicewakeup_utils.h"
#include <assert.h>
#include <hexagon_protos.h>
#include "detect_keyword.h"

#include "Elite_fwk_extns_lab.h"
#include "Elite_fwk_extns_detection_engine.h"

/*------------------------------------------------------------------------
 * Static declarations
 * -----------------------------------------------------------------------*/

static capi_v2_err_t capi_v2_voicewakeup_process(capi_v2_t* _pif,
		capi_v2_stream_data_t* input[],
		capi_v2_stream_data_t* output[]);

static capi_v2_err_t capi_v2_voicewakeup_end(capi_v2_t* _pif);

static capi_v2_err_t capi_v2_voicewakeup_set_param(capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t *port_info_ptr,
		capi_v2_buf_t *params_ptr);

static capi_v2_err_t capi_v2_voicewakeup_get_param(capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t *port_info_ptr,
		capi_v2_buf_t *params_ptr);

static capi_v2_err_t capi_v2_voicewakeup_set_properties(capi_v2_t* _pif,
		capi_v2_proplist_t *props_ptr);

static capi_v2_err_t capi_v2_voicewakeup_get_properties(capi_v2_t* _pif,
		capi_v2_proplist_t *props_ptr);

/** Virtual table for capi_v2_voicewakeup
 *  */
static capi_v2_vtbl_t vtbl =
{
		capi_v2_voicewakeup_process,
		capi_v2_voicewakeup_end,
		capi_v2_voicewakeup_set_param,
		capi_v2_voicewakeup_get_param,
		capi_v2_voicewakeup_set_properties,
		capi_v2_voicewakeup_get_properties
};

/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_get_static_properties
  DESCRIPTION: Function to get static properties of voicewakeup module.

  Each property is identified by a property_id and it's corresponding
  data type is predefined.
  ListenStreamManager framework queries for the properties from module on
  as-needed basis.
  The cases supported below are included for supporting HAP SDK Unit Testing.
  -----------------------------------------------------------------------*/
capi_v2_err_t capi_v2_voicewakeup_get_static_properties (
		capi_v2_proplist_t *init_set_properties,
		capi_v2_proplist_t *proplist_ptr)

{
	capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
	capi_v2_prop_t *prop_array = NULL;
	uint32_t i;
	if (NULL == init_set_properties)
	{
		FARF(HIGH,"CAPIv2 VoiceWakeup: get static properties ignoring init_set_properties!");
		return CAPI_V2_EOK;
	}
	if (NULL == proplist_ptr)
	{
		FARF(HIGH,"CAPIv2 VoiceWakeup: get static properties ignoring init_set_properties!");
		return CAPI_V2_EBADPARAM;
	}

	prop_array = proplist_ptr->prop_ptr;
	for (i = 0; i < proplist_ptr->props_num; i++)
	{
		capi_v2_buf_t *payload_ptr = &(prop_array[i].payload);

		switch(prop_array[i].id)
		{
		/*The amount of memory in bytes to be passed into the capi_v2_init function.
		    Payload structure: capi_v2_init_memory_requirement_t.
		 */
		case CAPI_V2_INIT_MEMORY_REQUIREMENT:
		{
			if (payload_ptr->max_data_len >= sizeof(capi_v2_init_memory_requirement_t))
			{
	          capi_v2_init_memory_requirement_t *data_ptr = (capi_v2_init_memory_requirement_t*)payload_ptr->data_ptr;
	          /* TODO: Define Module object size.
	           *
	           * - Library object must start with pointer to vtbl. vtbl is Virtual Table
	           *   containing CAPIv2 function pointers. See capi_v2_sample_module_t defined
	           *   above.
	           * - Any addition memory that the framework needs to allocate for the module
	           *   can be specified here.
	           * - This is the first capi_v2 call wrt to the module. All subsequent capi_v2 calls
	           *   will have the reference to the space allocated with this size. This space
	           *   is free for Module's use and only mandate is that the first 32bit address
	           *   should point to the vtbl
	           */
	          /* Size in byte should include all memory module needs for internal usage.
               * This extra size should also be align_to_8_byte.
               */
	          data_ptr->size_in_bytes = align_to_8_byte(sizeof(capi_v2_voicewakeup_t)); // + extra space


	          payload_ptr->actual_data_len = sizeof(capi_v2_init_memory_requirement_t);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
			break;
		}

		/**< Indicates whether the module can perform in-place computation.
		    There are no output buffers for voicewakeup modules so this is always true.

		    Payload Structure: capi_v2_is_inplace_t */

		case CAPI_V2_IS_INPLACE:
		{
			if (payload_ptr->max_data_len >= sizeof(capi_v2_is_inplace_t))
			{
				capi_v2_is_inplace_t *data_ptr = (capi_v2_is_inplace_t*)payload_ptr->data_ptr;
				data_ptr->is_inplace = TRUE;
				payload_ptr->actual_data_len = sizeof(capi_v2_is_inplace_t);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
			break;
		}


		/**< Inform the caller service whether the module needs data buffering or not.
		    VoiceWakeup modules must be designed with following considerations:
		       1. All the input must be consumed.
		       2. The module should be able to handle any number of samples.
		    Since VoiceWakeup modules do not have output buffer and only use a
		    single input port, they do not perform encoding/decoding, nor
		    do they perform rate conversion between the input and output buffers,
		    so this property should be set false.

		    Payload Structure: capi_v2_requires_data_buffering_t
		 */

		case CAPI_V2_REQUIRES_DATA_BUFFERING:
		{
			if (payload_ptr->max_data_len >= sizeof(capi_v2_requires_data_buffering_t))
			{
				capi_v2_requires_data_buffering_t *data_ptr =
						(capi_v2_requires_data_buffering_t*)payload_ptr->data_ptr;
				data_ptr->requires_data_buffering = FALSE;
				payload_ptr->actual_data_len = sizeof(capi_v2_requires_data_buffering_t);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
			break;
		}

		/**< The amount of stack size in bytes needed by this module
		    Payload Structure: capi_v2_stack_size_t */
		case CAPI_V2_STACK_SIZE:
		{
			if (payload_ptr->max_data_len >= sizeof(capi_v2_stack_size_t))
			{
				capi_v2_stack_size_t *data_ptr = (capi_v2_stack_size_t*)payload_ptr->data_ptr;
				data_ptr->size_in_bytes = VW_STACK_SIZE;
				payload_ptr->actual_data_len = sizeof(capi_v2_stack_size_t);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
			break;
		}

		/**< The maximum size of metadata generated by this module after each call
		    to process(). If this value is zero, the module does not generate any
		    metadata. It includes size of different structures used to pack
		    metadata (See property CAPI_V2_METADATA).
		    Payload Structure: capi_v2_max_metadata_size_t */
		case CAPI_V2_MAX_METADATA_SIZE:
		{
			if (payload_ptr->max_data_len >= sizeof(capi_v2_max_metadata_size_t))
			{
				capi_v2_max_metadata_size_t *data_ptr =
						(capi_v2_max_metadata_size_t*)payload_ptr->data_ptr;
				data_ptr->size_in_bytes = 0;
				payload_ptr->actual_data_len = sizeof(capi_v2_max_metadata_size_t);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
		}
		break;

		/**< Currently the LSM framework is not performing any specific action based on
		    voice wakeup framework extensions. However, this behavior may change in the future,
		    then modules that do not return the framework extension may not work correctly
		    in the future.
		    So this example modules returns %%%  */
		/**< The number of framework extensions supported by this module.
		    Payload Structure: capi_v2_num_needed_framework_extensions_t */
		case CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS:
		{
			if (payload_ptr->max_data_len >= sizeof(capi_v2_num_needed_framework_extensions_t))
			{
				capi_v2_num_needed_framework_extensions_t *data_ptr =
						(capi_v2_num_needed_framework_extensions_t*)payload_ptr->data_ptr;

				data_ptr->num_extensions = 2;
				payload_ptr->actual_data_len = sizeof(capi_v2_num_needed_framework_extensions_t);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
		}
		break;
		/**< The number of framework extensions needed by this module.
		    %%% LSM framework do something with number return > 0??
		    Payload Structure: capi_v2_num_needed_framework_extensions_t */
		case CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS:
		{
			if (payload_ptr->max_data_len >= (sizeof(capi_v2_framework_extension_id_t)*2))
			{
				capi_v2_framework_extension_id_t *data_ptr =
						(capi_v2_framework_extension_id_t*)payload_ptr->data_ptr;

				data_ptr[0].id = FWK_EXTN_LAB;
				data_ptr[1].id = FWK_EXTN_DETECTION_ENGINE;
				payload_ptr->actual_data_len = (sizeof(capi_v2_framework_extension_id_t)*2);
			}
			else
			{
				FARF(ERROR,"CAPIv2 VoiceWakeup: get property id 0x%lx Bad param size %lu",
						(uint32_t)prop_array[i].id, payload_ptr->max_data_len);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
			}
		}
		break;

		default:
		{
			// return Unsupported for test properties
			FARF(HIGH,"CAPIv2 VoiceWakeup: get property id for 0x%x is not supported.",prop_array[i].id);
			CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EUNSUPPORTED);
		}
		break;
		}

		capi_v2_result ^= (capi_v2_result & CAPI_V2_EUNSUPPORTED); // Ignoring non-fatal error code.
		if (CAPI_V2_FAILED(capi_v2_result))
		{
			FARF(ERROR,"CAPIv2 VoiceWakeup: get property id for 0x%x failed with opcode %lu",
					prop_array[i].id, capi_v2_result);
		}
		else
		{
			FARF(HIGH,"CAPIv2 VoiceWakeup: get property id for 0x%x done",prop_array[i].id);
		}
	}
	return capi_v2_result;

}


/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_init
  DESCRIPTION: Initialize the CAPIv2 VoiceWakeup module and library.
  Any memory allocated by this function should be released by end().
  -----------------------------------------------------------------------*/
capi_v2_err_t capi_v2_voicewakeup_init (capi_v2_t* _pif,
		capi_v2_proplist_t      *init_set_properties)
{
	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;

	if (NULL == _pif || NULL == init_set_properties )
	{
		FARF(ERROR,"CAPIv2 VoiceWakeup: "
				"Init received bad pointer, 0x%lx, 0x%lx",
				(uint32_t)_pif, (uint32_t)init_set_properties);

		return CAPI_V2_EBADPARAM;
	}

	int8_t* ptr = (int8_t*)_pif;
	capi_v2_voicewakeup_t *me_ptr = (capi_v2_voicewakeup_t *)ptr;

	capi_v2_voicewakeup_init_media_fmt(me_ptr);

	me_ptr->vtbl.vtbl_ptr = &vtbl;

	memset(&me_ptr->detection_config, 0, sizeof(capi_v2_voicewakeup_detection_config_t));

	memset(&me_ptr->params_config, 0, sizeof(capi_v2_voicewakeup_params_config_t));
	me_ptr->params_config.operation_mode =  0x01; // default is MODE_KEYWORD_DETECTION
	me_ptr->params_config.gain = 0x0100; // L16Q8
	me_ptr->params_config.epd_begin_threshold = 0x02710000; // L32Q2
	me_ptr->params_config.epd_end_threshold = 0xFA9B62B7; // L32Q2

	capi_v2_result = capi_v2_voicewakeup_set_properties(_pif, init_set_properties);

	//Ignoring non-fatal error code.
	capi_v2_result ^= (capi_v2_result & CAPI_V2_EUNSUPPORTED);
	if (CAPI_V2_FAILED(capi_v2_result))
	{
		return capi_v2_result;
	}

	FARF(HIGH, "CAPIv2 VoiceWakeup: Init done!");
	return capi_v2_result;
}

/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_process
  DESCRIPTION: Process the input data looking detecting a "match" defined by the
    algorithm and the algorithm-specific input SoundModel data.
   	Loop thru list of input buffers until all data is consumed or error occurs.
   	  input[] is an array of capi_v2_stream_data_t type streams 1 or more input buffers
      Each input buffer contains 10 millisecond of samples - 320 bytes
    Output buffer is ignored - it is expected that this ptr is NULL
    Calls function that determines if 'detection' criteria is met or not.
  -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_voicewakeup_process(capi_v2_t* _pif,
		capi_v2_stream_data_t* input[],
		capi_v2_stream_data_t* output[])
{

	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;
	capi_v2_voicewakeup_t* me_ptr = (capi_v2_voicewakeup_t*)(_pif);
	uint32_t i;
	capi_v2_stream_data_t *inp_buf_list = input[0];
	capi_v2_buf_t *inp_buf_ptr = NULL;

	assert(me_ptr);
	assert(input);

	for(i=0;i<inp_buf_list->bufs_num;i++)
	{
	    inp_buf_ptr = inp_buf_list->buf_ptr+i;
	    capi_v2_result = detect_keyword(me_ptr, inp_buf_ptr);
	    if (CAPI_V2_FAILED(capi_v2_result)) {
	    	FARF(HIGH, "CAPIv2 VoiceWakeup : send CB event");
	    	break;
	    }

	}
	return capi_v2_result;
}

/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_end
  DESCRIPTION: Returns the library to the uninitialized state and frees the
  memory that was allocated by init(). This function also frees the virtual
  function table.
  -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_voicewakeup_end(capi_v2_t* _pif)
{
	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;
	if (NULL == _pif)
	{
		FARF(ERROR,"CAPIv2 VoiceWakeup: End received bad pointer, 0x%lx",(uint32_t)_pif);
		return CAPI_V2_EBADPARAM;
	}

	capi_v2_voicewakeup_t* me_ptr = (capi_v2_voicewakeup_t*)(_pif);

	capi_v2_voicewakeup_release_memory(me_ptr);

	me_ptr->vtbl.vtbl_ptr = NULL;

	FARF(HIGH,"CAPIv2 VoiceWakeup: End done");
	return capi_v2_result;
}


/*----------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_set_param
  DESCRIPTION: Sets either a parameter value or a parameter structure containing
  multiple parameters. In the event of a failure, the appropriate error code is
  returned.
  Each parameter is identified by a param_id and it's corresponding data type
  is predefined.
  Module can choose to honor the parameter or choose to ignore it and
  return CAPI_V2_EUNSUPPORTED.
  Implementation of all set parameters below are optional.
  --------------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_voicewakeup_set_param(capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t *port_info_ptr,
		capi_v2_buf_t *params_ptr)

{
	if (NULL == _pif || NULL == params_ptr)
	{
		FARF(ERROR,"CAPIv2 VoiceWakeup: Set param received bad pointer, 0x%lx, 0x%lx",
				(uint32_t)_pif, (uint32_t)params_ptr);
		return CAPI_V2_EBADPARAM;
	}

	capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;
	capi_v2_voicewakeup_t* me_ptr = (capi_v2_voicewakeup_t*)(_pif);
    bool_t is_ready_to_process = FALSE;

	switch (param_id)
	{
	   case LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD:
	   {
	      uint32_t epd_threshd_minor_version =  *((uint32_t *)params_ptr->data_ptr);
	      if (params_ptr->actual_data_len < sizeof(lsm_param_id_epd_threshold_t))
	      {
	          FARF(ERROR, "Set LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD fail: Invalid payload size: %lu",
	                params_ptr->actual_data_len);
	          return CAPI_V2_EBADPARAM;
	      }

	      if (epd_threshd_minor_version > LSM_API_VERSION_EPD_THRESHOLD)
	      {
			  /* Choose the appropriate API version
			   * That this time only LSM_API_VERSION_EPD_THRESHOLD is supported
			   */
		    FARF(ERROR, "Set LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD: version: %lu forced to %lu",
		    		epd_threshd_minor_version, LSM_API_VERSION_EPD_THRESHOLD);
	        epd_threshd_minor_version = LSM_API_VERSION_EPD_THRESHOLD;
	      }

	      if (LSM_API_VERSION_EPD_THRESHOLD == epd_threshd_minor_version)
	      {

	        lsm_param_id_epd_threshold_t *epd_threshd_ptr = (lsm_param_id_epd_threshold_t *)params_ptr->data_ptr;
	        me_ptr->params_config.epd_begin_threshold = epd_threshd_ptr->epd_begin_threshold;
	        me_ptr->params_config.epd_end_threshold   = epd_threshd_ptr->epd_end_threshold;
	        params_ptr->actual_data_len = sizeof(lsm_param_id_epd_threshold_t);
	        FARF(HIGH, "Start threshold:0x%lx, End threshold:0x%lx",
	              epd_threshd_ptr->epd_begin_threshold,
	              epd_threshd_ptr->epd_end_threshold);
	      }
	   }
       break;

	   case LSM_PARAM_ID_OPERATION_MODE:
	   {
	      lsm_param_id_operation_mode_t *op_mode_ptr = (lsm_param_id_operation_mode_t *)params_ptr->data_ptr;
	      if(params_ptr->actual_data_len < sizeof(lsm_param_id_operation_mode_t))
	      {
	        FARF(ERROR, "Set LSM_PARAM_ID_OPERATION_MODE fail: Invalid payload size: %lu",
	                params_ptr->actual_data_len);
	        return CAPI_V2_EBADPARAM;
	      }
	      uint32_t opmode_minor_version = op_mode_ptr->minor_version;
          // Verify version of operationMode
	      if (opmode_minor_version > LSM_API_VERSION_OPERATION_MODE)
	      {
		    /* Choose the appropriate API version
		     * That this time only LSM_API_VERSION_OPERATION_MODE is supported
		     */
	        FARF(ERROR, "Set LSM_PARAM_ID_OPERATION_MODE: version: %lu forced to %lu",
	        		opmode_minor_version, LSM_API_VERSION_OPERATION_MODE);
	        opmode_minor_version = LSM_API_VERSION_OPERATION_MODE;
	      }

	      if(LSM_API_VERSION_OPERATION_MODE == opmode_minor_version)
	      {

	        // This example implementation only detects keywords.  For this example,
	        // either stand-alone keyword detection or keyword + user detection modes are allowed.
	        if ( !(0x01 == op_mode_ptr->mode || 0x03 == op_mode_ptr->mode))
	        {
	          FARF(ERROR, "LSM_PARAM_ID_OPERATION_MODE fail: unsupported mode : %d", op_mode_ptr->mode);
	          return CAPI_V2_EFAILED;
	        }

	        me_ptr->params_config.operation_mode = (op_mode_ptr->mode);
	        params_ptr->actual_data_len = sizeof(lsm_param_id_operation_mode_t);
	        FARF(HIGH, "Operation mode:0x%x",op_mode_ptr->mode);
	      }
	      is_ready_to_process = capi_v2_voicewakeup_is_ready_to_process(me_ptr);
	   }
       break;

	   case LSM_PARAM_ID_GAIN:
	   {
	     uint32_t gain_minor_version =  *((uint32_t *)params_ptr->data_ptr);
	     if (params_ptr->actual_data_len < sizeof(lsm_param_id_gain_t))
	     {
			 /* Choose the appropriate API version
			  * That this time only LSM_API_VERSION_GAIN is supported
			  */
	         FARF(ERROR, "Set LSM_PARAM_ID_GAIN fail: Invalid payload size: %lu",
	                params_ptr->actual_data_len);
	         return CAPI_V2_EBADPARAM;
	     }
	     if (gain_minor_version > LSM_API_VERSION_GAIN)
	     {
			 /* Choose the appropriate API version
			  * That this time only LSM_API_VERSION_GAIN is supported
			  */
		     FARF(ERROR, "Set LSM_API_VERSION_GAIN: version: %lu forced to %lu",
		    		  gain_minor_version, LSM_API_VERSION_GAIN);
	         gain_minor_version = LSM_API_VERSION_GAIN;
	     }

	     if (LSM_API_VERSION_GAIN == gain_minor_version)
	     {

	       lsm_param_id_gain_t *gain_ptr = (lsm_param_id_gain_t *)params_ptr->data_ptr;
	       me_ptr->params_config.gain = gain_ptr->gain;
	       params_ptr->actual_data_len = sizeof(lsm_param_id_gain_t);
	       FARF(HIGH, "Gain:0x%x",gain_ptr->gain);
	     }
	 }
	 break;

	 case LSM_PARAM_ID_MIN_CONFIDENCE_LEVELS:
     {
	    FARF(HIGH,"CAPIv2 VoiceWakeup: set param MIN_CONFIDENCE_LEVELS                          ");
	    /* payload contains number of confidence levels (1 byte) and
	     * array of bytes for each minimum confidence level value
	     */
	    lsm_param_id_min_confidence_levels_t *conf_levels_ptr = (lsm_param_id_min_confidence_levels_t *)params_ptr->data_ptr;
        if (params_ptr->max_data_len < sizeof(lsm_param_id_min_confidence_levels_t)) {
		  FARF(ERROR, "<<set_param>> param data buffer not set up correctly, max data %lu len too small ",
				params_ptr->max_data_len);
		  return CAPI_V2_EBADPARAM;
        }
        // Get the number of confidence levels in payload
	    me_ptr->params_config.num_conf_levels = conf_levels_ptr->num_active_models;
        if (0 == me_ptr->params_config.num_conf_levels) {
		    FARF(ERROR, "<<set_param>> number of confidence level zero");
		    return CAPI_V2_EBADPARAM;
        }
        // Make sure that we can set/store all the confidence levels into module structure
        if (me_ptr->params_config.num_conf_levels > MAX_CONFIDENCE_LEVELS) {
		    FARF(ERROR, "<<set_param>> number of confidence level %lu exceeds max %lu ",
				me_ptr->params_config.num_conf_levels, MAX_CONFIDENCE_LEVELS);
	       	me_ptr->params_config.num_conf_levels = 0;
		    return CAPI_V2_EBADPARAM;
        }
        // Copy the confidence levels into module structure, ignore padding
	    memscpy(me_ptr->params_config.conf_levels,
			    me_ptr->params_config.num_conf_levels,
				params_ptr->data_ptr + sizeof(lsm_param_id_min_confidence_levels_t),
				me_ptr->params_config.num_conf_levels);
	    params_ptr->actual_data_len = me_ptr->params_config.num_conf_levels +
					sizeof(lsm_param_id_min_confidence_levels_t);
	    FARF(HIGH, "<<get_param>> MinConfidenceLevels got %lu values, %lu bytes                              ",
			  me_ptr->params_config.num_conf_levels, params_ptr->actual_data_len);

	    is_ready_to_process = capi_v2_voicewakeup_is_ready_to_process(me_ptr);
    }
	 break;

	 case LSM_PARAM_ID_REGISTER_SOUND_MODEL:
	 {
		FARF(HIGH, "CAPIv2 VoiceWakeup : set_param Id REGISTER_SOUND_MODEL   ");
		/* copy soundmodel info to param config module struct
		 *    xx xx xx xx    # numbers of bytes read from file
		 *    yy yy yy yy    # address of SoundModel memory <msw>
		 *    yy yy yy yy    # address of SoundModel memory <lsw>
         */
		memscpy(&me_ptr->params_config.sound_model_ptr,
				sizeof(me_ptr->params_config.sound_model_ptr),
				&params_ptr->data_ptr,
				params_ptr->actual_data_len);
		me_ptr->params_config.sound_model_size = params_ptr->actual_data_len;
		FARF(HIGH, "CAPIv2 VoiceWakeup : SM ptr %p, size %d                    ",
				me_ptr->params_config.sound_model_ptr,
				me_ptr->params_config.sound_model_size);

		// Extract metadata from soundmodel used by detection algorithm
		capi_v2_voicewakeup_sound_model_header_t * soundModelHeader = (capi_v2_voicewakeup_sound_model_header_t *)me_ptr->params_config.sound_model_ptr;
		FARF(HIGH, "CAPIv2 VoiceWakeup : SM threshold 0x%x, time over threshold %d (ms)  ",
				soundModelHeader->threshold, soundModelHeader->duration_over_threshold );

		is_ready_to_process = capi_v2_voicewakeup_is_ready_to_process(me_ptr);
	  }
	  break;

	  case LSM_PARAM_ID_DEREGISTER_SOUND_MODEL:
	  {
		FARF(HIGH, "CAPIv2 VoiceWakeup : set_param Id 0X%x ", (int)param_id);
		capi_v2_voicewakeup_release_memory(me_ptr);
		is_ready_to_process = capi_v2_voicewakeup_is_ready_to_process(me_ptr);
	  }
      break;

	  default:
	  {
		FARF(ERROR,"CAPIv2 VoiceWakeup: Set unsupported param ID 0x%x",(int)param_id);
		return CAPI_V2_EUNSUPPORTED;
	  }
	}

	FARF(HIGH,"CAPIv2 VoiceWakeup: Set param done, ready to process %d", (int)is_ready_to_process);
	return capi_v2_result;
}



/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_get_param
  DESCRIPTION: Gets either a parameter value or a parameter structure
  containing multiple parameters. In the event of a failure, the appropriate
  error code is returned.
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_voicewakeup_get_param(capi_v2_t* _pif,
		uint32_t param_id,
		const capi_v2_port_info_t *port_info_ptr,
		capi_v2_buf_t *params_ptr)
{
	if (NULL == _pif || NULL == params_ptr)
	{
		FARF(ERROR,"CAPIv2 VoiceWakeup: Get param received bad pointer, 0x%lx, 0x%lx",
				(uint32_t)_pif, (uint32_t)params_ptr);
		return CAPI_V2_EBADPARAM;
	}

	capi_v2_err_t  capi_v2_result  = CAPI_V2_EOK;
	capi_v2_voicewakeup_t* me_ptr = (capi_v2_voicewakeup_t*)(_pif);
	//void *param_payload_ptr  = (void *)(params_ptr->data_ptr);

	switch (param_id)
	{
	  case LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD:
	  {
		/*check if available size is large enough*/
		if(params_ptr->max_data_len < sizeof(lsm_param_id_epd_threshold_t))
		{
			FARF(ERROR, "Get LSM_PARAM_ID_ENDPOINT_DETECT_THRESHOLD failed: Insufficient container size: %lu",
					params_ptr->max_data_len);
			return CAPI_V2_EBADPARAM;
		}

		lsm_param_id_epd_threshold_t *param_payload_ptr = (lsm_param_id_epd_threshold_t *)params_ptr->data_ptr;
		param_payload_ptr->minor_version = LSM_API_VERSION_EPD_THRESHOLD;
		param_payload_ptr->epd_begin_threshold = me_ptr->params_config.epd_begin_threshold;
		param_payload_ptr->epd_end_threshold = me_ptr->params_config.epd_end_threshold;
 		params_ptr->actual_data_len = sizeof(lsm_param_id_epd_threshold_t);
	  }
	  break;

	  case LSM_PARAM_ID_OPERATION_MODE:
	  {
		/*check if available size is large enough*/
		if(params_ptr->max_data_len < sizeof(lsm_param_id_operation_mode_t))
		{
			FARF(ERROR, "Get LSM_PARAM_ID_OPERATION_MODE failed: Insufficient container size: %lu",
					params_ptr->max_data_len);
			return CAPI_V2_EBADPARAM;
		}

		lsm_param_id_operation_mode_t *param_payload_ptr = (lsm_param_id_operation_mode_t *)params_ptr->data_ptr;
		param_payload_ptr->minor_version = LSM_API_VERSION_OPERATION_MODE;
		param_payload_ptr->mode = me_ptr->params_config.operation_mode;
		param_payload_ptr->reserved = 0;
		params_ptr->actual_data_len = sizeof(lsm_param_id_operation_mode_t);
	  }
	  break;
	  case LSM_PARAM_ID_GAIN:
	  {
		if(params_ptr->max_data_len < sizeof(lsm_param_id_gain_t))
		{
			FARF(ERROR, "Get LSM_PARAM_ID_GAIN failed: Insufficient container size: %lu",
						params_ptr->max_data_len);
			return CAPI_V2_EBADPARAM;
		}

		lsm_param_id_gain_t *param_payload_ptr = (lsm_param_id_gain_t *)params_ptr->data_ptr;
		param_payload_ptr->minor_version = LSM_API_VERSION_GAIN;
		param_payload_ptr->gain = me_ptr->params_config.gain;
		param_payload_ptr->reserved = 0;
		params_ptr->actual_data_len = sizeof(lsm_param_id_gain_t);
	  }
	  break;

	  case LSM_PARAM_ID_MIN_CONFIDENCE_LEVELS:
	  {
		FARF(HIGH,"CAPIv2 VoiceWakeup: get param MinConfidenceLevels");
		uint32_t req_data_len = sizeof(lsm_param_id_min_confidence_levels_t) +
				(me_ptr->params_config.num_conf_levels * sizeof(uint8_t));
		// PAD so data is multiple of 4
		req_data_len += (req_data_len % 4);

		if (params_ptr->max_data_len >= req_data_len) {
			memscpy(params_ptr->data_ptr,
                    sizeof(lsm_param_id_min_confidence_levels_t),
					&(me_ptr->params_config.num_conf_levels),
                    sizeof(lsm_param_id_min_confidence_levels_t) );
			memscpy(params_ptr->data_ptr + sizeof(lsm_param_id_min_confidence_levels_t),
					(size_t)me_ptr->params_config.num_conf_levels,
					me_ptr->params_config.conf_levels,
					(size_t)me_ptr->params_config.num_conf_levels);
			params_ptr->actual_data_len = req_data_len;
			FARF(HIGH, "<<get_param>> MinConfidenceLevels got %lu values, %lu bytes                              ",
					me_ptr->params_config.num_conf_levels, req_data_len);
		} else {
			FARF(ERROR, "<<get_param>> Data size %lu is not big enough to hold %lu padded to 4 bytes ",
					params_ptr->max_data_len, me_ptr->params_config.num_conf_levels);
			return CAPI_V2_ENEEDMORE;
		}
	  }
	  break;

	  default :
	  {
		FARF(ERROR,"CAPIv2 VoiceWakeup: Get unsupported param ID 0x%x",(int)param_id);
		return CAPI_V2_EUNSUPPORTED;
	  }
	}

	FARF(HIGH,"CAPIv2 VoiceWakeup: Get param ID 0x%x done", (int)param_id);
	return capi_v2_result;
}

/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_set_properties
  DESCRIPTION: Function to set the properties for the VoiceWakeup module

  CAPI_V2_INPUT_MEDIA_FORMAT must be implemented by the module

  LSM Service framework calls set_properties with CAPI_V2_INPUT_MEDIA_FORMAT
  HAP SDK Unit Test framework initializes the properties:
      CAPI_V2_PORT_NUM_INFO
      CAPI_V2_EVENT_CALLBACK_INFO
  	  CAPI_V2_INPUT_MEDIA_FORMAT

 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_voicewakeup_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *proplist_ptr)
{
    capi_v2_err_t  capi_v2_result = CAPI_V2_EOK;
    capi_v2_voicewakeup_t* me_ptr = (capi_v2_voicewakeup_t*)(_pif);
    capi_v2_prop_t *prop_array = NULL;
    uint8_t i;
    if (NULL == _pif || NULL == proplist_ptr)
    {
			FARF(ERROR,"CAPIv2 VoiceWakeup: Set properties received bad input pointer");
			return CAPI_V2_EBADPARAM;
    }
    prop_array = proplist_ptr->prop_ptr;
    for ( i=0; i < proplist_ptr->props_num; i++)
    {
        capi_v2_buf_t *payload_ptr = &(prop_array[i].payload);
        switch(prop_array[i].id)
        {
			case CAPI_V2_PORT_NUM_INFO:
			{
				if (payload_ptr->actual_data_len >= sizeof(capi_v2_port_num_info_t))
				{
					capi_v2_port_num_info_t *data_ptr = (capi_v2_port_num_info_t*)payload_ptr->data_ptr;

					if (data_ptr->num_input_ports > CAPI_V2_VOICEWAKEUP_MAX_IN_PORTS)
					{
						FARF(ERROR,"CAPIv2 VoiceWakeup: Set property id 0x%lx number of input ports "
								"cannot be more than %d", (uint32_t)prop_array[i].id,
								CAPI_V2_VOICEWAKEUP_MAX_IN_PORTS);
						CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EBADPARAM);
					}
					if (data_ptr->num_output_ports > CAPI_V2_VOICEWAKEUP_MAX_OUT_PORTS)
					{
						FARF(ERROR,"CAPIv2 VoiceWakeup: Set property id 0x%lx number of output ports "
								"cannot be greater 0", (uint32_t)prop_array[i].id);
						CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EBADPARAM);
					}
				}
				else
				{
					FARF(ERROR,"CAPIv2 VoiceWakeup: Set property id  0x%lx Bad param size %lu",
							(uint32_t)prop_array[i].id, payload_ptr->actual_data_len);
					CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
				}
			}
			break;

			case CAPI_V2_EVENT_CALLBACK_INFO:
			{
				if (payload_ptr->actual_data_len >= sizeof(capi_v2_event_callback_info_t))
				{
					capi_v2_event_callback_info_t *data_ptr =
							(capi_v2_event_callback_info_t*)payload_ptr->data_ptr;
					if (NULL == data_ptr)
					{
						CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EBADPARAM);
					}
					else
					{
						me_ptr->cb_info = *data_ptr;
					}
					FARF(HIGH,"CAPIv2 VoiceWakeup: Set property id for Event CallBack done.");
				}
				else
				{
					FARF(ERROR,"CAPIv2 VoiceWakeup: Set property id 0x%lx Bad param size %lu",
							(uint32_t)prop_array[i].id, payload_ptr->actual_data_len);
					CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
				}
			}
			break;

			case CAPI_V2_INPUT_MEDIA_FORMAT:
			{
				if (payload_ptr->actual_data_len >= sizeof(capi_v2_voicewakeup_media_fmt_t))
				{
					FARF(HIGH,"CAPIv2 VoiceWakeup: received input media fmt");
					capi_v2_voicewakeup_media_fmt_t *in_data_ptr =
							(capi_v2_voicewakeup_media_fmt_t *) (payload_ptr->data_ptr);
					capi_v2_standard_data_format_t *media_fmt_data = NULL;
			        media_fmt_data = (capi_v2_standard_data_format_t *)&in_data_ptr->std_fmt;

			        if(BITS_PER_SAMPLE != media_fmt_data->bits_per_sample)
			        {
			          FARF(ERROR, "CAPI V2 SVA2.0 received unsupported bit_per_sample = %lu; %lu is supported",
			                media_fmt_data->bits_per_sample, BITS_PER_SAMPLE);
			          return CAPI_V2_EBADPARAM;
			        }
			        if(SAMPLING_RATE != media_fmt_data->sampling_rate)
			        {
			          FARF(ERROR, "CAPI V2 SVA2.0 received unsupported sampling rate = %lu; %lu is supported",
			                media_fmt_data->sampling_rate, SAMPLING_RATE);
			          return CAPI_V2_EBADPARAM;
			        }
			        if(CAPI_V2_VOICEWAKEUP_MAX_CHANNELS != media_fmt_data->num_channels )
			        {
			          FARF(ERROR, "CAPI V2 SVA2.0 received unsupported number of channels = %lu; %lu is supported",
			                media_fmt_data->num_channels, CAPI_V2_VOICEWAKEUP_MAX_CHANNELS);
			          return CAPI_V2_EBADPARAM;
			        }

					//copy input media fmt to module structure

					memscpy(&me_ptr->input_media_fmt[0].std_fmt,
						sizeof(me_ptr->input_media_fmt[0].std_fmt),
						&in_data_ptr->std_fmt,
						sizeof(capi_v2_standard_data_format_t) );
				}
				else
				{
					FARF(ERROR,"CAPIv2 VoiceWakeup: Set Param id 0x%lx Bad param size %lu",
							(uint32_t)prop_array[i].id, payload_ptr->actual_data_len);
					CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
				}
			}
			break;

			default:
			{
				FARF(HIGH,"CAPIv2 VoiceWakeup: Set property for 0x%x. Not supported.",prop_array[i].id);
				CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EUNSUPPORTED);
			}
			break;
		}

    	capi_v2_result ^= (capi_v2_result & CAPI_V2_EUNSUPPORTED); // Ignoring non-fatal error code.
		if (CAPI_V2_FAILED(capi_v2_result))
		{
			FARF(HIGH,"CAPIv2 VoiceWakeup: Set property for 0x%x failed with opcode %lu",
						prop_array[i].id, capi_v2_result);
		}
		else
		{
			FARF(HIGH,"CAPIv2 VoiceWakeup: Set property for 0x%x done", prop_array[i].id);
		}
    }
	return capi_v2_result;
}


/*------------------------------------------------------------------------
  Function name: capi_v2_voicewakeup_get_properties
  DESCRIPTION: Function to get the properties for the VoiceWakeup module.

  Currently get property function is NOT called by LSM nor UnitTest
  frameworks.
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_voicewakeup_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
	  return CAPI_V2_EUNSUPPORTED;
}

