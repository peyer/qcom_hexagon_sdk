/* ======================================================================== */
/**
 @file capi_v2_sp_rx_utils.cpp

 C source file to implement the utility functions for
 CAPIv2 for DECIMATE example.
 */

/* =========================================================================
 Copyright (c) 2015 QUALCOMM Technologies Incorporated.
 All rights reserved. Qualcomm Technologies Proprietary and Confidential.
 ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#include "Elite_CAPI_V2_properties.h"

#include "capi_v2_sp_rx_utils.h"
#include "AEEstd.h"

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_lib_destroy
 DESCRIPTION: Function to free the library
 ===========================================================================*/
void capi_v2_sp_rx_lib_destroy(capi_v2_sp_rx_t* me_ptr)
{
   if(me_ptr->lib_mem.lib_ptr)
   {
      // Free the allocated memory
      free(me_ptr->lib_mem.lib_ptr);
      me_ptr->lib_mem.lib_ptr = NULL;
   }
}

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_update_event_states
 DESCRIPTION: Function to update the event states for KPPS and BW events
 ===========================================================================*/
static void capi_v2_sp_rx_update_event_states(capi_v2_sp_rx_t* me_ptr)
{
   /* Do KPPS and BW voting now */
   capi_v2_event_info_t event_info;
   capi_v2_event_KPPS_t event_kpps;
   capi_v2_event_bandwidth_t event_bw;

   event_kpps.KPPS = 5000; /* Add profiled value here */
   event_info.port_info.is_valid = FALSE;
   event_info.payload.actual_data_len = sizeof(capi_v2_event_KPPS_t);
   event_info.payload.max_data_len = sizeof(capi_v2_event_KPPS_t);
   event_info.payload.data_ptr = (int8_t *)&event_kpps;

   me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_V2_EVENT_KPPS, &event_info);

   event_bw.code_bandwidth = 0;
   event_bw.data_bandwidth = 1050000; /* Add profiled value here */
   event_info.port_info.is_valid = FALSE;
   event_info.payload.actual_data_len = sizeof(capi_v2_event_bandwidth_t);
   event_info.payload.max_data_len = sizeof(capi_v2_event_bandwidth_t);
   event_info.payload.data_ptr = (int8_t *)&event_bw;

   me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_V2_EVENT_BANDWIDTH, &event_info);

   FARF(HIGH, "SP Rx module voted for KPPS and BW successfully!");
}

capi_v2_err_t capi_v2_sp_rx_lib_init(capi_v2_sp_rx_t *me_ptr)
{
   int8_t *lib_ptr = NULL;

   /* This is the size of the library that will be allocated */
   uint32_t mem_req = 30;

   /* Creating and initializing Lib */
   /* Here, according to the algorithm memory requirements allocate the memory accordingly */
   lib_ptr = (int8_t *)malloc(mem_req);

   if(NULL == lib_ptr)
   {
      FARF(ERROR, "CAPI V2 SP RX out of memory failed !!");
      return CAPI_V2_ENOMEMORY;
   }

   me_ptr->lib_mem.lib_ptr = lib_ptr;

   return CAPI_V2_EOK;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_process_get_properties
 DESCRIPTION: Utility function resp_rxonsible for getting the properties from SP_RX
 example.
 ===========================================================================*/
capi_v2_err_t capi_v2_sp_rx_process_get_properties(capi_v2_sp_rx_t *me_ptr,
                                                   capi_v2_proplist_t *proplist_ptr)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_prop_t *prop_array = proplist_ptr->prop_ptr;
   uint32_t i;
   for(i = 0; i < proplist_ptr->props_num; i++)
   {
      capi_v2_buf_t *payload_ptr = &(prop_array[i].payload);

      switch(prop_array[i].id)
      {
         /*The amount of memory in bytes to be passed into the capi_v2_init function.
          Payload structure: capi_v2_init_memory_requirement_t.
          */
         case CAPI_V2_INIT_MEMORY_REQUIREMENT:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_init_memory_requirement_t))
            {
               uint32_t memory_size = 0;
               // Each block must begin with 8-byte aligned memory.
               memory_size = align_to_8_byte(sizeof(capi_v2_sp_rx_t));

               capi_v2_init_memory_requirement_t *data_ptr =
                     (capi_v2_init_memory_requirement_t*)(payload_ptr->data_ptr);

               data_ptr->size_in_bytes = memory_size;
               payload_ptr->actual_data_len = sizeof(capi_v2_init_memory_requirement_t);

               FARF(HIGH, "CAPIv2 SP_RX: get property for initialization memory "
                    "requirements %lu bytes",
                    data_ptr->size_in_bytes);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
            break;
         }

         /**< Indicates whether the module can perform in-place computation. If this value
             is true, the caller may provide the same pointers for input and output (but this
             is not guaranteed). This requires that the input and output data format be the
             same and the requires_data_buffering property be false.
             Payload Structure: capi_v2_is_inplace_t */

         case CAPI_V2_IS_INPLACE:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_is_inplace_t))
            {
               capi_v2_is_inplace_t *data_ptr = (capi_v2_is_inplace_t*)payload_ptr->data_ptr;
               data_ptr->is_inplace = TRUE;
               payload_ptr->actual_data_len = sizeof(capi_v2_is_inplace_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
            break;
         }

         /**< Inform the caller service whether the module needs data buffering or not.
             If this value is false, the module must behave as follows:
             1. The number of output samples should always be the same as the number
             of input samples on all output ports. The caller must ensure that the
             number of input samples is the same on all input ports.
             2. All the input must be consumed. The caller must ensure that the output
             buffers have enough sp_rxace.
             3. The module should be able to handle any number of samples.

             If this value is true, the module must behave as follows:
             1. The module must define a threshold in terms of number of bytes for each
             input port and each output port.
             2. The module must consume data on its inputs and fill data on its outputs
             till the amount of remaining data on each buffer of at least one input
             port is less than its threshold or the amount of free sp_rxace on each buffer
             of at least one output port is less than its threshold.

             Note: Setting this value to true adds significant overhead, so it should
             only be used if:
             1. The module performs encoding/decoding of data.
             2. The module performs rate conversion between the input and output.

             Payload Structure: capi_v2_requires_data_buffering_t
          */

         case CAPI_V2_REQUIRES_DATA_BUFFERING:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_requires_data_buffering_t))
            {
               capi_v2_requires_data_buffering_t *data_ptr =
                     (capi_v2_requires_data_buffering_t*)payload_ptr->data_ptr;
               data_ptr->requires_data_buffering = FALSE;
               payload_ptr->actual_data_len = sizeof(capi_v2_requires_data_buffering_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
            break;
         }

         /**< The amount of stack size in bytes needed by this module
             Payload Structure: capi_v2_stack_size_t */
         case CAPI_V2_STACK_SIZE:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_stack_size_t))
            {
               capi_v2_stack_size_t *data_ptr = (capi_v2_stack_size_t*)payload_ptr->data_ptr;
               data_ptr->size_in_bytes = CAPI_V2_SP_RX_STACK_SIZE;
               payload_ptr->actual_data_len = sizeof(capi_v2_stack_size_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
            break;
         }

         /**< The number of framework extensions needed by this module.
             Payload Structure: capi_v2_num_needed_framework_extensions_t
             Two framework extensions are needed in RX:
             1) Feedback: It is for setting up the path for feedback mechanism from the TX module to the
                RX module.
             2) Codec Interrupts: It is for obtaining the list of codec interrupts supported by the module.
                The list of valid interrupts can be found at Elite_fwk_extns_codec_interrupt.h
              */
         case CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_num_needed_framework_extensions_t))
            {
               capi_v2_num_needed_framework_extensions_t *data_ptr =
                     (capi_v2_num_needed_framework_extensions_t*)payload_ptr->data_ptr;

               data_ptr->num_extensions = 2;
               payload_ptr->actual_data_len = sizeof(capi_v2_num_needed_framework_extensions_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         case CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_framework_extension_id_t))
            {
               capi_v2_framework_extension_id_t *data_ptr =
                     (capi_v2_framework_extension_id_t *)payload_ptr->data_ptr;

               data_ptr[0].id = FWK_EXTN_FEEDBACK;
               data_ptr[1].id = FWK_EXTN_CDC_INTERRUPT;
               payload_ptr->actual_data_len = sizeof(capi_v2_framework_extension_id_t) * 2;
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         default:
         {
            FARF(HIGH, "CAPIv2 SP_RX: get property id for 0x%x is not supported.",
                 prop_array[i].id);
            CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EUNSUPPORTED);
         }
         break;
      }

      if(CAPI_V2_FAILED(capi_v2_result))
      {
         FARF(ERROR, "CAPIv2 SP_RX: get property id for 0x%x failed with opcode %lu",
              prop_array[i].id, capi_v2_result);
      }
      else
      {

         FARF(HIGH, "CAPIv2 SP_RX: get property id for 0x%x done", prop_array[i].id);
      }
   }
   return capi_v2_result;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_process_set_properties
 DESCRIPTION: Function to set properties for decimate example
 ===========================================================================*/
capi_v2_err_t capi_v2_sp_rx_process_set_properties(capi_v2_sp_rx_t* me_ptr,
                                                   capi_v2_proplist_t *proplist_ptr)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_prop_t *prop_array = proplist_ptr->prop_ptr;
   uint8_t i;
   for(i = 0; i < proplist_ptr->props_num; i++)
   {
      capi_v2_buf_t *payload_ptr = &(prop_array[i].payload);

      switch(prop_array[i].id)
      {
         /* This heap id will be used by mallocs */
         case CAPI_V2_HEAP_ID:
         {
            if(payload_ptr->actual_data_len >= sizeof(capi_v2_heap_id_t))
            {
               //capi_v2_heap_id_t *data_ptr = (capi_v2_heap_id_t*)payload->data_ptr;
               FARF(HIGH, "CAPIv2 SP_RX: Set property id for heap is ignored.");
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: Set property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->actual_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         /* This is needed to raise any event info to the service. Here, it will be used for
          * raising KPPS and BW events as well as for feedback communication between Tx and Rx module
          * (explained later)
          * */
         case CAPI_V2_EVENT_CALLBACK_INFO:
         {
            if(payload_ptr->actual_data_len >= sizeof(capi_v2_event_callback_info_t))
            {
               capi_v2_event_callback_info_t *data_ptr =
                     (capi_v2_event_callback_info_t*)payload_ptr->data_ptr;
               if(NULL == data_ptr)
               {
                  CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EBADPARAM);
               }
               else
               {
                  me_ptr->cb_info = *data_ptr;
               }
               FARF(HIGH, "CAPIv2 SP_RX: Set property id for Event CallBack done.");
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: Set property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->actual_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         /* Please read the description of the call flow in the beginning of capi_v2_sp_rx.c */
         case CAPI_V2_INPUT_MEDIA_FORMAT:
         {
            if(payload_ptr->actual_data_len >= sizeof(capi_v2_sp_rx_media_fmt_t))
            {
               capi_v2_err_t result_init;

               FARF(HIGH, "CAPIv2 SP_RX: received input media fmt");

               capi_v2_sp_rx_media_fmt_t *in_data_ptr =
                     (capi_v2_sp_rx_media_fmt_t *)(payload_ptr->data_ptr);

               //copy and save the input media fmt
               me_ptr->input_media_fmt[0].std_fmt = in_data_ptr->std_fmt;

               me_ptr->output_media_fmt[0].std_fmt = in_data_ptr->std_fmt;

               uint32_t output_sampling_rate = me_ptr->input_media_fmt[0].std_fmt.sampling_rate;
               me_ptr->output_media_fmt[0].std_fmt.sampling_rate = output_sampling_rate;
               me_ptr->num_spkr = me_ptr->input_media_fmt[0].std_fmt.num_channels;

               /* Now we have all the information for mallocing the lib
                * If no set param was received we would run in with default params
                */
               result_init = capi_v2_sp_rx_lib_init(me_ptr);
               if(CAPI_V2_EOK != result_init)
               {
                  capi_v2_sp_rx_lib_destroy(me_ptr);
                  FARF(ERROR, "CAPI V2 SP RX failed to init lib, result: %lu !!", result_init);
                  return CAPI_V2_EFAILED ;
               }

               /* We can do a set param to the library here if any other config exists */

               /* Now our library is ready, we would raise the KPPS and BW voting events */
               capi_v2_sp_rx_update_event_states(me_ptr);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_RX: Set Param id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->actual_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         /**< Can be used to query the media format for a particular output port.
             * This property can also be used to set the output media format for modules at support control
             * of the output media format. If a module only supports controlling some asp_rxects like say the
             * sample rate only, all other fields can be set to CAPI_V2_DATA_FORMAT_INVALID_VAL.
             * The port id must be set in the payload by the caller.
             */
            case CAPI_V2_OUTPUT_MEDIA_FORMAT:
            {
               if (payload_ptr->max_data_len >= sizeof(capi_v2_sp_rx_media_fmt_t))
               {
                  capi_v2_sp_rx_media_fmt_t *data_ptr =
                        (capi_v2_sp_rx_media_fmt_t*)payload_ptr->data_ptr;

                  if ( (NULL == me_ptr) ||
                        (prop_array[i].port_info.is_valid && prop_array[i].port_info.port_index != 0 ))
                  {
                     FARF(ERROR,"CAPIv2 SP_RX: get property id 0x%lx due to invalid/unexpected values",
                           (uint32_t)prop_array[i].id);
                     CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EFAILED);
                  }
                  else
                  {
                     *data_ptr = me_ptr->output_media_fmt[0];
                     payload_ptr->actual_data_len = sizeof(capi_v2_sp_rx_media_fmt_t);
                  }
               }
               else
               {
                  FARF(ERROR,"CAPIv2 SP_RX: get property id 0x%lx Bad param size %lu",
                        (uint32_t)prop_array[i].id, payload_ptr->max_data_len);
                  CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
               }
            }
            break;

         default:
         {
            FARF(ERROR, "CAPIv2 SP_RX: Set property for 0x%x. Not supported.", prop_array[i].id);
            CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EUNSUPPORTED);
         }
         break;
      }

      if(CAPI_V2_FAILED(capi_v2_result))
      {
         FARF(HIGH, "CAPIv2 SP_RX: Set property for 0x%x failed with opcode %lu", prop_array[i].id,
              capi_v2_result);
      }
      else
      {
         FARF(HIGH, "CAPIv2 SP_RX: Set property for 0x%x done", prop_array[i].id);
      }
   }
   return capi_v2_result;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_release_memory
 DESCRIPTION: Function to release allocated memory
 ===========================================================================*/
void capi_v2_sp_rx_release_memory(capi_v2_sp_rx_t *me_ptr)
{
   if(me_ptr->lib_mem.lib_ptr)
   {
      free(me_ptr->lib_mem.lib_ptr);
   }

   memset(me_ptr, 0, sizeof(capi_v2_sp_rx_t));
}

/* Here, we will explain the feedback(fb) communication between
 * Tx path module(operating on Vsens and Isens data from speakers) and the Rx path processing(which operate on
 * actual music data) using Queues(Q).
 *
 *       TX(Buffer Queue)            <=>                RX(Data Queue)
 *
 * It is a bi-directional communication. The following steps highlight the procedure:
 * -> The Tx module creates buffers and adds them to the buffer queue.
 * -> When Tx module wants to communication buffers to Rx module, it pops a buffer from Buffer Q, writes the
 *    fb data on this buffer and pushes it onto the Data Q.
 * -> Then the Rx module pops the buffer from its Data Q, uses it and then returns it back to the Buffer Q.
 *
 * The Queue's creation and destruction is responsibility of the service code. In CAPIv2 layer,
 * we only expect the buffers to be created in the Tx module and pushed onto the Buffer Q. Rest communication
 * can be achieved by callback mechanisms highlighted in Elite_fwk_extns_feedback.h.
 *
 * This feedback framework is a generalized implementation i.e. multiple Tx modules can feed to one Rx module.
 * The client list is maintained in the client_list_ptr_ptr, we can iterate over this and receive data from
 * multiple Tx modules.
 *
 */

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_get_fb_data
 DESCRIPTION: This utility function retrieves fb data from dataQ.
 ===========================================================================*/
void capi_v2_sp_rx_get_fb_data(capi_v2_sp_rx_t *me_ptr, int8_t *fb_data_ptr[MAX_SP_SPKRS],
                               void *fb_data_buf_rcvd_ptr[MAX_SP_SPKRS],
                               void *fb_data_buf_client_ptr[MAX_SP_SPKRS])
{

   feedback_client_list_t *client_list_ptr = NULL;
   void *psFbDataBuf = NULL;

   sp_v2_tx_fb_data_payload_t *fb_data_payload_ptr;

   /* Loop through client list and get the fb data buffers
    * If multiple Tx modules feeds into this module then client_list_ptr_ptr would
    * contain a linked list of  (feedback_client_list_t *) denoting multiple feedback paths.
    * */
   if(*me_ptr->client_list_ptr_ptr)
   {
      client_list_ptr = (feedback_client_list_t *)*me_ptr->client_list_ptr_ptr;
      while(client_list_ptr != NULL)
      {
         psFbDataBuf = NULL;

         capi_v2_event_info_t event_info;
         capi_v2_fb_dst_get_buf_from_src_t pop_buf;
         capi_v2_event_data_to_dsp_service_t data_event;

         data_event.param_id = CAPI_V2_PARAM_ID_FB_DST_GET_BUF_FROM_SRC;
         data_event.payload.actual_data_len = sizeof(capi_v2_fb_dst_get_buf_from_src_t);
         data_event.payload.max_data_len = sizeof(capi_v2_fb_dst_get_buf_from_src_t);
         data_event.payload.data_ptr = (int8_t *)&pop_buf;

         event_info.port_info.is_valid = FALSE;
         event_info.payload.actual_data_len = sizeof(data_event);
         event_info.payload.max_data_len = sizeof(data_event);
         event_info.payload.data_ptr = (int8_t *)&data_event;

         pop_buf.buf_ptr_ptr = &psFbDataBuf;
         pop_buf.client_ptr = client_list_ptr->element;

         /* Get the fb data buffer */
         me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_V2_EVENT_DATA_TO_DSP_SERVICE,
                                  &event_info);

         if(NULL != psFbDataBuf)
         {
            FARF(HIGH, "CAPIv2 SP_RX: Popped FB buffer from data Q!");

            fb_data_payload_ptr = (sp_v2_tx_fb_data_payload_t *)psFbDataBuf;
            if(SPKR_L == fb_data_payload_ptr->spkr_cfg)
            {
               fb_data_ptr[SPKR_L] = (int8_t *)fb_data_payload_ptr->fb_data_ptr[0];
               fb_data_buf_rcvd_ptr[SPKR_L] = psFbDataBuf;
               fb_data_buf_client_ptr[SPKR_L] = client_list_ptr->element;
            }
            else if(SPKR_R == fb_data_payload_ptr->spkr_cfg)
            {
               fb_data_ptr[SPKR_R] = (int8_t *)fb_data_payload_ptr->fb_data_ptr[0];
               fb_data_buf_rcvd_ptr[SPKR_R] = psFbDataBuf;
               fb_data_buf_client_ptr[SPKR_R] = client_list_ptr->element;
            }
            else/*SPKR_LR*/
            {
               fb_data_ptr[SPKR_L] = (int8_t *)fb_data_payload_ptr->fb_data_ptr[0];
               fb_data_ptr[SPKR_R] = (int8_t *)fb_data_payload_ptr->fb_data_ptr[1];
               fb_data_buf_rcvd_ptr[SPKR_L] = psFbDataBuf;
               fb_data_buf_client_ptr[SPKR_L] = client_list_ptr->element;
            }
         }

         /* Iterate to the next fb client if it exists */
         client_list_ptr = client_list_ptr->next;
      }
   }
}

/*===========================================================================
 FUNCTION : capi_v2_sp_rx_return_fb_data
 DESCRIPTION: This utility function returns fb data to the buffer Q.
 ===========================================================================*/
void capi_v2_sp_rx_return_fb_data(capi_v2_sp_rx_t *me_ptr, void *fb_data_buf_rcvd_ptr[MAX_SP_SPKRS],
                                  void *fb_data_buf_client_ptr[MAX_SP_SPKRS])
{
   int32_t i = 0;

   capi_v2_event_info_t event_info;
   capi_v2_fb_dst_rtn_buf_to_src_t push_buf;
   capi_v2_event_data_to_dsp_service_t data_event;

   data_event.param_id = CAPI_V2_PARAM_ID_FB_DST_RTN_BUF_TO_SRC;
   data_event.payload.actual_data_len = sizeof(capi_v2_fb_dst_rtn_buf_to_src_t);
   data_event.payload.max_data_len = sizeof(capi_v2_fb_dst_rtn_buf_to_src_t);
   data_event.payload.data_ptr = (int8_t *)&push_buf;

   event_info.port_info.is_valid = FALSE;
   event_info.payload.actual_data_len = sizeof(data_event);
   event_info.payload.max_data_len = sizeof(data_event);
   event_info.payload.data_ptr = (int8_t *)&data_event;

   for(i = 0; i < MAX_SP_SPKRS; i++)
   {
      if(fb_data_buf_rcvd_ptr[i])
      {
         push_buf.buf_ptr = fb_data_buf_rcvd_ptr[i];
         push_buf.client_ptr = fb_data_buf_client_ptr[i];

         me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_V2_EVENT_DATA_TO_DSP_SERVICE,
                                  &event_info);

         FARF(HIGH, "CAPIv2 SP_RX: Returned FB buffer to buffer Q!");
      }
   }
}

/*===========================================================================
 FUNCTION : capi_v2_sp_v2_rx_intr_handler
 DESCRIPTION: This utility function is a prototype of how interrupts raised from the Codec may be handled. We can save
 the interrupts raised and do corresponding processing in this module when process function is called.
 ===========================================================================*/
capi_v2_err_t capi_v2_sp_v2_rx_intr_handler(capi_v2_sp_rx_t *me_ptr, uint32_t int_id)
{
   if(NULL == me_ptr->lib_mem.lib_ptr)
   {
      FARF(ERROR, "SP RX : Intr handler, Library ptr is NULL");
      return CAPI_V2_EFAILED;
   }

   switch(int_id)
   {
      case CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP:
      {
         //me_ptr->rx_intr[SPKR_L] |= ;
      }
      break;

      case CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP:
      {
         //me_ptr->rx_intr[SPKR_R] |= ;
      }
      break;
      case CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK:
      {
         //me_ptr->rx_intr[SPKR_L] |= ;
         //me_ptr->rx_intr[SPKR_R] |= ;
      }
      break;
      case CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE:
      {
         //me_ptr->rx_intr[SPKR_L] |= ;
         //me_ptr->rx_intr[SPKR_R] |= ;
      }
      break;

      default:
         break;
   }
#ifdef CLIP_INTR_DEBUG
   FARF(HIGH, "int_index %ld rx_intr[SPKR_L] %ld rx_intr[SPKR_R] %ld",
        int_id, me_ptr->rx_intr[SPKR_L], me_ptr->rx_intr[SPKR_R]);
#endif

   return CAPI_V2_EOK;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_v2_rx_copy_pcmlevels_to_libbuf
 DESCRIPTION:  This utility function is for copying the PCM levels received from the Codec along with the speaker Clip
 interrupts.
 ===========================================================================*/
capi_v2_err_t capi_v2_sp_v2_rx_copy_pcmlevels_to_libbuf(capi_v2_sp_rx_t *me_ptr, uint32_t int_id,
                                                        int8_t *cdc_clip_pcm_level_buf)
{
   int16_t *clip_pcm_buf_ptr;

   capi_v2_cdc_clip_pcm_info_t *clip_pcm_info_ptr =
         (capi_v2_cdc_clip_pcm_info_t *)cdc_clip_pcm_level_buf;

   if(CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP == int_id)
   {
      clip_pcm_buf_ptr = &me_ptr->clip_pcm_buf[SPKR_L][0];
   }
   else if(CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP == int_id)
   {
      clip_pcm_buf_ptr = &me_ptr->clip_pcm_buf[SPKR_R][0];
   }
   else
   {
      return CAPI_V2_EFAILED ;
   }

   std_memscpy((void *)clip_pcm_buf_ptr, sizeof(uint16_t) * MAX_PCM_BUF_LEN,
           (const void *)clip_pcm_info_ptr->cdc_clip_pcm_level_buf.data_ptr,
           clip_pcm_info_ptr->cdc_clip_pcm_level_buf.actual_data_len);

#ifdef CLIP_INTR_DEBUG
   int8_t index;
   for(index = 0; index < MAX_PCM_BUF_LEN; index++)
   {
      FARF(HIGH, "index %d extr pcm level 0x%x", index, clip_pcm_buf_ptr[index]);
   }
#endif

   return CAPI_V2_EOK;
}

