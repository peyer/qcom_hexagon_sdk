/* ======================================================================== */
/**
 @file capi_v2_sp_tx_utils.cpp

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

#include "capi_v2_sp_tx_utils.h"

void capi_v2_sp_tx_lib_destroy(capi_v2_sp_tx_t* me_ptr)
{
   if(me_ptr->lib_mem.lib_ptr)
   {
      // Free the allocated memory
      free(me_ptr->lib_mem.lib_ptr);
      me_ptr->lib_mem.lib_ptr = NULL;
   }
}

/*===========================================================================
 FUNCTION : capi_v2_sp_tx_update_event_states
 DESCRIPTION: Function to update the event states for KPPS and BW events
 ===========================================================================*/
static void capi_v2_sp_tx_update_event_states(capi_v2_sp_tx_t* me_ptr)
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

   FARF(HIGH, "SP TX module voted for KPPS and BW successfully!");
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
 * This utility function creates the feedback path and buffers. It then adds the buffers into the buffer Q for usage.
 */
capi_v2_err_t capi_v2_sp_tx_fb_path_init(capi_v2_sp_tx_t* me_ptr)
{
   sp_lib_err_t result = SP_LIB_EOK;

   uint32_t fb_data_size_per_spkr = 0;
   uint32_t fb_data_total_size = 0;
   uint8_t index = 0;
   uint8_t spkr = 0;
   sp_v2_tx_fb_data_payload_t *fb_data_payload_ptr = NULL;
   void *fb_buff_ptr = NULL;
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

   /*Creating and initializing CX data buffers */

   /* Query Cxdata buffer size */
   result = sp_tx_get_fb_data_per_spkr_mem_req(&me_ptr->lib_config, &fb_data_size_per_spkr);

   if(SP_LIB_EOK != result)
   {
      FARF(ERROR, "SP Tx: failed to get cxdata memory req, result: %x !!", result);
      return CAPI_V2_EFAILED ;
   }

   /* determining the spkr config */
   if(1 == me_ptr->num_spkr)
   {
      if((AFE_FBSP_VSENS_RIGHT_CHAN == me_ptr->feedback_path_info.feedback_cfg.chan_info[0])
            || (AFE_FBSP_VSENS_RIGHT_CHAN == me_ptr->feedback_path_info.feedback_cfg.chan_info[1]))
      {
         me_ptr->spkr_cfg = SPKR_R;
      }
   }
   else /* 2 speakers */
   {
      me_ptr->spkr_cfg = SPKR_LR;
   }

   /* Feedback path is already configured in FWK, we just need to allocate the buffers that can be passed */
   fb_data_total_size = me_ptr->num_spkr * fb_data_size_per_spkr
         + sizeof(sp_v2_tx_fb_data_payload_t);

   /* Allocate the buffers, initialize them for Tx fb data and push the same to bufQ */
   for(index = 0; index < NUM_CXDATA_BUFFERS; index++)
   {

      fb_buff_ptr = malloc(fb_data_total_size);
      if(!fb_buff_ptr)
      {
         FARF(ERROR, "Memory allocation error                                       ");
         return CAPI_V2_ENOMEMORY;
      }

      memset(fb_buff_ptr, 0, fb_data_total_size);

      fb_data_payload_ptr = (sp_v2_tx_fb_data_payload_t *)fb_buff_ptr;

      fb_data_payload_ptr->spkr_cfg = me_ptr->spkr_cfg;

      for(spkr = 0; spkr < me_ptr->num_spkr; spkr++)
      {
         fb_data_payload_ptr->fb_data_ptr[spkr] = (int8_t *)fb_data_payload_ptr
               + sizeof(sp_v2_tx_fb_data_payload_t) + spkr * fb_data_size_per_spkr;

         if(SP_LIB_EOK
               != (result = sp_tx_init_fb_data_per_spkr(&me_ptr->lib_config,
                                                        fb_data_payload_ptr->fb_data_ptr[spkr],
                                                        fb_data_size_per_spkr)))
         {
            FARF(ERROR, "SP V2 TH VI: Failed to init cx data, result: %d", result);

            free(fb_buff_ptr);
            return CAPI_V2_EFAILED ;
         }
      }

      capi_v2_event_info_t event_info;
      capi_v2_event_data_to_dsp_service_t data_event;
      capi_v2_fb_src_push_buf_t push_buf;

      push_buf.buf_ptr = fb_buff_ptr;
      push_buf.queue_ptr = me_ptr->feedback_path_info.bufQ_ptr;

      data_event.param_id = CAPI_V2_PARAM_ID_FB_SRC_PUSH_BUF;
      data_event.payload.actual_data_len = sizeof(capi_v2_fb_src_push_buf_t);
      data_event.payload.max_data_len = sizeof(capi_v2_fb_src_push_buf_t);
      data_event.payload.data_ptr = (int8_t *)&push_buf;

      event_info.port_info.is_valid = FALSE;
      event_info.payload.actual_data_len = sizeof(data_event);
      event_info.payload.max_data_len = sizeof(data_event);
      event_info.payload.data_ptr = (int8_t *)&data_event;

      capi_v2_result = me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context,
                                                CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, &event_info);
      if(CAPI_V2_FAILED(capi_v2_result))
      {
         FARF(ERROR,
              "CAPIv2 SP_TX: Failed to send CAPI_V2_EVENT_DATA_TO_DSP_SERVICE event with %lu",
              capi_v2_result);
         free(fb_buff_ptr);
      }

      /* update the number of buffers created to the client queue info, so that when we deinit the Q,
       * Proper number of buffs are deallocated
       */
      (*(me_ptr->feedback_path_info.num_buffers_ptr))++;
   }

   return CAPI_V2_EOK;
}

capi_v2_err_t capi_v2_sp_tx_lib_init(capi_v2_sp_tx_t *me_ptr)
{
   void *lib_ptr = NULL;

   /* This is the size of the library that will be allocated */
   uint32_t mem_req = 30;

   /* Creating and initializing Lib */

   lib_ptr = malloc(mem_req);

   if(NULL == lib_ptr)
   {
      FARF(ERROR, "CAPI V2 SP TX out of memory failed !!");
      return CAPI_V2_ENOMEMORY;
   }

   me_ptr->lib_mem.lib_ptr = (int8_t *)lib_ptr;

   return CAPI_V2_EOK;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_tx_process_get_properties
 DESCRIPTION: Utility function resp_txonsible for getting the properties from SP_TX
 example.
 ===========================================================================*/
capi_v2_err_t capi_v2_sp_tx_process_get_properties(capi_v2_sp_tx_t *me_ptr,
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
               memory_size = align_to_8_byte(sizeof(capi_v2_sp_tx_t));

               capi_v2_init_memory_requirement_t *data_ptr =
                     (capi_v2_init_memory_requirement_t*)(payload_ptr->data_ptr);
               data_ptr->size_in_bytes = memory_size;
               payload_ptr->actual_data_len = sizeof(capi_v2_init_memory_requirement_t);

               FARF(HIGH, "CAPIv2 SP_TX: get property for initialization memory "
                    "requirements %lu bytes",
                    data_ptr->size_in_bytes);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: get property id 0x%lx Bad param size %lu",
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
               FARF(ERROR, "CAPIv2 SP_TX: get property id 0x%lx Bad param size %lu",
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
             buffers have enough sp_txace.
             3. The module should be able to handle any number of samples.

             If this value is true, the module must behave as follows:
             1. The module must define a threshold in terms of number of bytes for each
             input port and each output port.
             2. The module must consume data on its inputs and fill data on its outputs
             till the amount of remaining data on each buffer of at least one input
             port is less than its threshold or the amount of free sp_txace on each buffer
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
               FARF(ERROR, "CAPIv2 SP_TX: get property id 0x%lx Bad param size %lu",
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
               data_ptr->size_in_bytes = CAPI_V2_SP_TX_STACK_SIZE;
               payload_ptr->actual_data_len = sizeof(capi_v2_stack_size_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
            break;
         }

         /**< The number of framework extensions needed by this module.
             Payload Structure: capi_v2_num_needed_framework_extensions_t */
         case CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS:
         {
            if(payload_ptr->max_data_len >= sizeof(capi_v2_num_needed_framework_extensions_t))
            {
               capi_v2_num_needed_framework_extensions_t *data_ptr =
                     (capi_v2_num_needed_framework_extensions_t*)payload_ptr->data_ptr;

               data_ptr->num_extensions = 1;
               payload_ptr->actual_data_len = sizeof(capi_v2_num_needed_framework_extensions_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: get property id 0x%lx Bad param size %lu",
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

               data_ptr->id = FWK_EXTN_FEEDBACK;
               payload_ptr->actual_data_len = sizeof(capi_v2_framework_extension_id_t);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: get property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->max_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         default:
         {
            FARF(HIGH, "CAPIv2 SP_TX: get property id for 0x%x is not supported.",
                 prop_array[i].id);
            CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EUNSUPPORTED);
         }
         break;
      }

      if(CAPI_V2_FAILED(capi_v2_result))
      {
         FARF(ERROR, "CAPIv2 SP_TX: get property id for 0x%x failed with opcode %lu",
              prop_array[i].id, capi_v2_result);
      }
      else
      {

         FARF(HIGH, "CAPIv2 SP_TX: get property id for 0x%x done", prop_array[i].id);
      }
   }
   return capi_v2_result;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_tx_process_set_properties
 DESCRIPTION: Function to set properties for decimate example
 ===========================================================================*/
capi_v2_err_t capi_v2_sp_tx_process_set_properties(capi_v2_sp_tx_t* me_ptr,
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
         case CAPI_V2_HEAP_ID:
         {
            if(payload_ptr->actual_data_len >= sizeof(capi_v2_heap_id_t))
            {
               //capi_v2_heap_id_t *data_ptr = (capi_v2_heap_id_t*)payload->data_ptr;
               FARF(HIGH, "CAPIv2 SP_TX: Set property id for heap is ignored.");
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: Set property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->actual_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

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
               FARF(HIGH, "CAPIv2 SP_TX: Set property id for Event CallBack done.");
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: Set property id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->actual_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         case CAPI_V2_INPUT_MEDIA_FORMAT:
         {
            if(payload_ptr->actual_data_len >= sizeof(capi_v2_sp_tx_media_fmt_t))
            {
               capi_v2_err_t result_init;

               FARF(HIGH, "CAPIv2 SP_TX: received input media fmt");

               capi_v2_sp_tx_media_fmt_t *in_data_ptr =
                     (capi_v2_sp_tx_media_fmt_t *)(payload_ptr->data_ptr);

               //copy and save the input media fmt
               me_ptr->input_media_fmt[0].std_fmt = in_data_ptr->std_fmt;

               me_ptr->output_media_fmt[0].std_fmt = in_data_ptr->std_fmt;

               uint32_t output_sampling_rate = me_ptr->input_media_fmt[0].std_fmt.sampling_rate;
               me_ptr->output_media_fmt[0].std_fmt.sampling_rate = output_sampling_rate;
               me_ptr->num_spkr = me_ptr->input_media_fmt[0].std_fmt.num_channels / 2;

               /* Feedback path should be rcvd till now */
               if(NULL == me_ptr->feedback_path_info.bufQ_ptr)
               {
                  FARF(ERROR, "SP TX: failed Feedback info not yet rcvd!");
                  return CAPI_V2_EFAILED ;
               }

               if(me_ptr->feedback_path_info.feedback_cfg.num_channels
                     != (int32_t)me_ptr->input_media_fmt[0].std_fmt.num_channels)
               {
                  FARF(ERROR,
                       "CAPI V2 SP TX failed! Invalid Media fmt, num ch(%lu) doesnt match that of feedback cfg(%ld)!!",
                       me_ptr->input_media_fmt[0].std_fmt.num_channels,
                       me_ptr->feedback_path_info.feedback_cfg.num_channels);
                  return CAPI_V2_EFAILED ;
               }

               //Create the feedback path
               capi_v2_sp_tx_fb_path_init(me_ptr);

               /* Now we have all the information for mallocing the lib
                * If no set param was received we would run in with default params
                */
               result_init = capi_v2_sp_tx_lib_init(me_ptr);
               if(CAPI_V2_EOK != result_init)
               {
                  capi_v2_sp_tx_lib_destroy(me_ptr);
                  FARF(ERROR, "CAPI V2 SP TX failed to init lib, result: %lu !!", result_init);
                  return CAPI_V2_EFAILED ;
               }

               /* We can do a set param to the library here if any other config exists */

               /* Now our library is ready, we would raise the KPPS voting events */
               capi_v2_sp_tx_update_event_states(me_ptr);
            }
            else
            {
               FARF(ERROR, "CAPIv2 SP_TX: Set Param id 0x%lx Bad param size %lu",
                    (uint32_t )prop_array[i].id, payload_ptr->actual_data_len);
               CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_ENEEDMORE);
            }
         }
         break;

         /**< Can be used to query the media format for a particular output port.
             * This property can also be used to set the output media format for modules at support control
             * of the output media format. If a module only supports controlling some aspects like say the
             * sample rate only, all other fields can be set to CAPI_V2_DATA_FORMAT_INVALID_VAL.
             * The port id must be set in the payload by the caller.
             */
            case CAPI_V2_OUTPUT_MEDIA_FORMAT:
            {
               if (payload_ptr->max_data_len >= sizeof(capi_v2_sp_tx_media_fmt_t))
               {
                  capi_v2_sp_tx_media_fmt_t *data_ptr =
                        (capi_v2_sp_tx_media_fmt_t*)payload_ptr->data_ptr;

                  if ( (NULL == me_ptr) ||
                        (prop_array[i].port_info.is_valid && prop_array[i].port_info.port_index != 0 ))
                  {
                     FARF(ERROR,"CAPIv2 SP_tx: get property id 0x%lx due to invalid/unexpected values",
                           (uint32_t)prop_array[i].id);
                     CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_EFAILED);
                  }
                  else
                  {
                     *data_ptr = me_ptr->output_media_fmt[0];
                     payload_ptr->actual_data_len = sizeof(capi_v2_sp_tx_media_fmt_t);
                  }
               }
               else
               {
                  FARF(ERROR,"CAPIv2 SP_tx: get property id 0x%lx Bad param size %lu",
                        (uint32_t)prop_array[i].id, payload_ptr->max_data_len);
                  CAPI_V2_SET_ERROR(capi_v2_result,CAPI_V2_ENEEDMORE);
               }
            }
            break;

         default:
         {
            FARF(ERROR, "CAPIv2 SP_TX: Set property for 0x%x. Not supported.", prop_array[i].id);
            CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EUNSUPPORTED);
         }
         break;

      }

      if(CAPI_V2_FAILED(capi_v2_result))
      {
         FARF(HIGH, "CAPIv2 SP_TX: Set property for 0x%x failed with opcode %lu", prop_array[i].id,
              capi_v2_result);
      }
      else
      {
         FARF(HIGH, "CAPIv2 SP_TX: Set property for 0x%x done", prop_array[i].id);
      }
   }
   return capi_v2_result;
}

/*===========================================================================
 FUNCTION : capi_v2_sp_tx_release_memory
 DESCRIPTION: Function to release allocated memory
 ===========================================================================*/
void capi_v2_sp_tx_release_memory(capi_v2_sp_tx_t *me_ptr)
{
   if(me_ptr->lib_mem.lib_ptr)
   {
      free(me_ptr->lib_mem.lib_ptr);
   }

   memset(me_ptr, 0, sizeof(capi_v2_sp_tx_t));
}

/*===========================================================================
 FUNCTION : capi_v2_sp_tx_pop_fb_data_from_buffer_q
 DESCRIPTION: This utility function pops the buffer from the Buffer Q.
 ===========================================================================*/
void capi_v2_sp_tx_pop_fb_data_from_buffer_q(capi_v2_sp_tx_t *me_ptr, void **fb_buf_pptr)
{
   capi_v2_event_info_t event_info;
   capi_v2_fb_src_pop_buf_t pop_buf;
   capi_v2_event_data_to_dsp_service_t data_event;

   pop_buf.buf_ptr_ptr = fb_buf_pptr;
   pop_buf.queue_ptr = me_ptr->feedback_path_info.bufQ_ptr;

   data_event.param_id = CAPI_V2_PARAM_ID_FB_SRC_POP_BUF;
   data_event.payload.actual_data_len = sizeof(capi_v2_fb_src_pop_buf_t);
   data_event.payload.max_data_len = sizeof(capi_v2_fb_src_pop_buf_t);
   data_event.payload.data_ptr = (int8_t *)&pop_buf;

   event_info.port_info.is_valid = FALSE;
   event_info.payload.actual_data_len = sizeof(data_event);
   event_info.payload.max_data_len = sizeof(data_event);
   event_info.payload.data_ptr = (int8_t *)&data_event;

   /* Get the fb data buffer */
   me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_V2_EVENT_DATA_TO_DSP_SERVICE,
                            &event_info);
}

/*===========================================================================
 FUNCTION : capi_v2_sp_tx_push_fb_data_to_data_q
 DESCRIPTION: This utility function pushes the buffer to the Data Q in the Rx path module.
 ===========================================================================*/
void capi_v2_sp_tx_push_fb_data_to_data_q(capi_v2_sp_tx_t *me_ptr, void *fb_buff_ptr)
{
   capi_v2_event_info_t event_info;
   capi_v2_fb_src_push_buf_t push_buf;
   capi_v2_event_data_to_dsp_service_t data_event;

   push_buf.buf_ptr = fb_buff_ptr;

   push_buf.queue_ptr = me_ptr->feedback_path_info.dataQ_ptr;

   data_event.param_id = CAPI_V2_PARAM_ID_FB_SRC_PUSH_BUF;
   data_event.payload.actual_data_len = sizeof(capi_v2_fb_src_push_buf_t);
   data_event.payload.max_data_len = sizeof(capi_v2_fb_src_push_buf_t);
   data_event.payload.data_ptr = (int8_t *)&push_buf;

   event_info.port_info.is_valid = FALSE;

   event_info.payload.actual_data_len = sizeof(data_event);
   event_info.payload.max_data_len = sizeof(data_event);
   event_info.payload.data_ptr = (int8_t *)&data_event;

   me_ptr->cb_info.event_cb(me_ptr->cb_info.event_context, CAPI_V2_EVENT_DATA_TO_DSP_SERVICE,
                            &event_info);

   FARF(HIGH, "CAPIv2 SP_TX: Sent FB buffer to dataQ!");
}
