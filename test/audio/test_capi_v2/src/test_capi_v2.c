/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "test_capi_v2.h"

#include "adsp_error_codes.h"
#include "Elite_CAPI_V2.h"
#include "Elite_pcm_ch_defs.h"

#include "HAP_farf.h"
#include "HAP_mem.h"
#include "AEEstd.h"
#include "capi_v2_utils_props.h"

#include "capi_v2_test.h"
#include "Elite_fwk_extns_feedback.h"
#include "Elite_fwk_extns_codec_interrupt.h"

#include "feedback_fwk.h"

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

#define TRY(exception, func) \
   if (TEST_SUCCESS != (exception = func)) {\
      goto exception##bail; \
   }

#define THROW(exception, errno) \
   exception = errno; \
   goto exception##bail;

#define CATCH(exception) exception##bail: if (exception != TEST_SUCCESS)

#define CHANNELS_FOR_MONO     1
#define CHANNELS_FOR_STEREO   2

#define MAX_PCM_BUF_LEN 16

typedef struct cdc_info_t
{
   capi_v2_num_needed_cdc_int_t num_cdc_int;
   capi_v2_cdc_int_t *cdc_int_ptr;
}cdc_info_t;
capi_v2_err_t test_capi_v2_get_props(void* ctx, capi_v2_property_id_t id,
                                  capi_v2_buf_t* payload)
{
  switch(id) {
    case CAPI_V2_INIT_MEMORY_REQUIREMENT:
      capi_v2_utils_props_get_init_memory_requirement(payload, (uint32_t*)ctx);
      return CAPI_V2_EOK;
    default:
      break;
  }
  return CAPI_V2_EFAILED;;
}
/*
 * Input and Output media format is same for both the TX and RX module.
 * */
capi_v2_err_t test_capi_v2_process(module_info_t* module_tx, module_info_t* module_rx, cdc_info_t *cdc_info_ptr)
{
   FARF(HIGH, "CAPI V2 TEST: Executing Process Data command.");
   uint32_t frame = 1;
   int8_t i = 0;
   capi_v2_err_t result = CAPI_V2_EOK;

   // ------------------------
   // Buffer pointers Tx
   // ------------------------

   int8_t* input_buffer_tx = NULL;
   int8_t* temp_in_buffer_tx = NULL;
   int8_t* output_buffer_tx = NULL;
   int8_t* temp_out_buffer_tx = NULL;

   // ------------------------
   // Buffer pointers Rx
   // ------------------------

   int8_t* input_buffer_rx = NULL;
   int8_t* temp_in_buffer_rx = NULL;
   int8_t* output_buffer_rx = NULL;
   int8_t* temp_out_buffer_rx = NULL;

   int8_t* ptr = NULL;
   uint32_t increment = 0;
   uint32_t ch = 0;

   // ------------------------
   // Buffer setup Tx
   // ------------------------
   capi_v2_stream_data_t input_str_data_tx[1], output_str_data_tx[1];
   capi_v2_stream_data_t* input_tx[] =
   { &input_str_data_tx[0] };
   capi_v2_stream_data_t* output_tx[] =
   { &output_str_data_tx[0] };

   capi_v2_buf_t in_tx[CAPI_V2_MAX_CHANNELS], out_tx[CAPI_V2_MAX_CHANNELS];
   input_str_data_tx[0].buf_ptr = &in_tx[0];
   output_str_data_tx[0].buf_ptr = &out_tx[0];
   input_str_data_tx[0].bufs_num = calculateNumBuffers(&module_tx->in_format);
   output_str_data_tx[0].bufs_num = calculateNumBuffers(&module_tx->out_format);

   uint32_t in_buf_size_tx = module_tx->in_buffer_len * input_str_data_tx[0].bufs_num;
   uint32_t out_buf_size_tx = module_tx->out_buffer_len * output_str_data_tx[0].bufs_num;

   uint32_t in_buf_size_rx = module_rx->in_buffer_len * input_str_data_tx[0].bufs_num;
   uint32_t out_buf_size_rx = module_rx->out_buffer_len * output_str_data_tx[0].bufs_num;

   // Allocate input buffer
   input_buffer_tx = (int8_t *)malloc(in_buf_size_tx);
   if(!input_buffer_tx)
   {
      FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for input buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   temp_in_buffer_tx = (int8_t *)malloc(in_buf_size_tx);
   if(!temp_in_buffer_tx)
   {
      FARF(ERROR,
           "CAPI V2 TEST: Process Buffers command Memory allocation error for temp input buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   output_buffer_tx = input_buffer_tx;
   if(!output_buffer_tx)
   {
      FARF(ERROR,
           "CAPI V2 TEST: Process Buffers command Memory allocation error for output_str_data buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   temp_out_buffer_tx = (int8_t *)malloc(out_buf_size_tx);
   if(!temp_out_buffer_tx)
   {
      FARF(ERROR,
           "CAPI V2 TEST: Process Buffers command Memory allocation error for temp output_str_data buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   // ------------------------
   // Set input and output_str_data buffers for Tx
   // ------------------------

   {
      
      ptr = input_buffer_tx;
      increment = 0;

      for(ch = 0; ch < input_str_data_tx[0].bufs_num; ch++)
      {
         in_tx[ch].data_ptr = ptr + increment;
         in_tx[ch].actual_data_len = 0;
         in_tx[ch].max_data_len = module_tx->in_buffer_len;
         increment += module_tx->in_buffer_len;
      }

      ptr = output_buffer_tx;
      increment = 0;
      for(ch = 0; ch < output_str_data_tx[0].bufs_num; ch++)
      {
         out_tx[ch].data_ptr = ptr + increment;
         out_tx[ch].actual_data_len = 0;
         out_tx[ch].max_data_len = module_tx->out_buffer_len;
         increment += module_tx->out_buffer_len;
      }
   }


   // ------------------------
   // Buffer setup Rx
   // ------------------------
   capi_v2_stream_data_t input_str_data_rx[1], output_str_data_rx[1];
   capi_v2_stream_data_t* input_rx[] =
   { &input_str_data_rx[0] }, *output_rx[] =
   { &output_str_data_rx[0] };
   capi_v2_buf_t in_rx[CAPI_V2_MAX_CHANNELS], out_rx[CAPI_V2_MAX_CHANNELS];
   input_str_data_rx[0].buf_ptr = &in_rx[0];
   output_str_data_rx[0].buf_ptr = &out_rx[0];
   input_str_data_rx[0].bufs_num = calculateNumBuffers(&module_rx->in_format);
   output_str_data_rx[0].bufs_num = calculateNumBuffers(&module_rx->out_format);


   // Allocate input buffer
   input_buffer_rx = (int8_t *)malloc(in_buf_size_rx);
   if( !input_buffer_rx)
   {
      FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for input buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   temp_in_buffer_rx = (int8_t *)malloc(in_buf_size_rx);
   if(!temp_in_buffer_rx)
   {
      FARF(ERROR,
           "CAPI V2 TEST: Process Buffers command Memory allocation error for temp input buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   output_buffer_rx = input_buffer_rx;
   if(!output_buffer_rx)
   {
      FARF(ERROR,
           "CAPI V2 TEST: Process Buffers command Memory allocation error for output_str_data buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   temp_out_buffer_rx = (int8_t *)malloc(out_buf_size_rx);
   if(!temp_out_buffer_rx)
   {
      FARF(ERROR,
           "CAPI V2 TEST: Process Buffers command Memory allocation error for temp output_str_data buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
   }

   // ------------------------
   // Set input and output_str_data buffers for Rx
   // ------------------------

   {

      ptr = input_buffer_rx;
      increment = 0;

      for(ch = 0; ch < input_str_data_rx[0].bufs_num; ch++)
      {
         in_rx[ch].data_ptr = ptr + increment;
         in_rx[ch].actual_data_len = 0;
         in_rx[ch].max_data_len = module_rx->in_buffer_len;
         increment += module_rx->in_buffer_len;
      }

      ptr = output_buffer_rx;
      increment = 0;
      for(ch = 0; ch < output_str_data_rx[0].bufs_num; ch++)
      {
         out_rx[ch].data_ptr = ptr + increment;
         out_rx[ch].actual_data_len = 0;
         out_rx[ch].max_data_len = module_rx->out_buffer_len;
         increment += module_rx->out_buffer_len;
      }
   }

   // ------------------------
   // Process buffers
   // ------------------------

   do
   {
      // Fill input buffer
      // while (input buffer not consumed)
      // {
      //    process()
      //    dumpOutput()
      // }

      // ------------------------
      // Read input
      // ------------------------

      if(CAPI_V2_FAILED(result = FillInputBuffer(input_str_data_tx, module_tx, temp_in_buffer_tx)))
      {
         FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
         break;
      }
      bool_t is_file_over = (input_str_data_tx[0].buf_ptr[0].actual_data_len
            < input_str_data_tx[0].buf_ptr[0].max_data_len);

      if(is_file_over)
      {
         break;
      }

      if(CAPI_V2_FAILED(result = FillInputBuffer(input_str_data_rx, module_rx, temp_in_buffer_rx)))
      {
         FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
         break;
      }

      is_file_over = (input_str_data_rx[0].buf_ptr[0].actual_data_len
            < input_str_data_rx[0].buf_ptr[0].max_data_len);

      if(is_file_over)
      {
         break;
      }

      while((input_str_data_tx[0].buf_ptr[0].actual_data_len > 0)
            && (input_str_data_rx[0].buf_ptr[0].actual_data_len > 0))
      {

         //FARF(HIGH, "CAPI V2 TEST: Processing frame: %ld", frame);
         frame++;

         result = CAPI_V2_EOK;
         uint32_t input_bytes_given_tx[] =
         { input_str_data_tx[0].buf_ptr[0].actual_data_len };
         uint32_t input_bytes_given_rx[] =
         { input_str_data_rx[0].buf_ptr[0].actual_data_len };

         // ------------------------
         // Call Processing function for Tx
         // ------------------------
         result = module_tx->module_ptr->vtbl_ptr->process(module_tx->module_ptr, input_tx,
                                                           output_tx);

         // ------------------------
         // Call Processing function for Rx
         // ------------------------
         result = module_rx->module_ptr->vtbl_ptr->process(module_rx->module_ptr, input_rx,
                                                           output_rx);

         // Since, input and output samples are same
         for(ch = 0; ch < output_str_data_tx[0].bufs_num; ch++)
         {
            out_tx[ch].actual_data_len = module_tx->out_buffer_len;
         }

         for(ch = 0; ch < output_str_data_rx[0].bufs_num; ch++)
         {
            out_rx[ch].actual_data_len = module_rx->out_buffer_len;
         }

         // Now check if the process function went through fine Tx
         if(CAPI_V2_FAILED(result))
         {
            FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
            break;
         }
         result = CheckInputOutputSizes(input_tx, input_bytes_given_tx, output_tx,
                                        module_tx->requires_data_buffering, 1lu, 1lu);

         if(CAPI_V2_FAILED(result))
         {
            break;
         }

         result = DumpOutputToFile(output_tx[0], module_tx, temp_out_buffer_tx);
         if(CAPI_V2_FAILED(result))
         {
            break;
         }

         // Now check if the process function went through fine Rx
         result = CheckInputOutputSizes(input_rx, input_bytes_given_rx, output_rx,
                                        module_rx->requires_data_buffering, 1lu, 1lu);
         if(CAPI_V2_FAILED(result))
         {
            break;
         }

         result = DumpOutputToFile(output_rx[0], module_rx, temp_out_buffer_rx);
         if(CAPI_V2_FAILED(result))
         {
            break;
         }

         // Adjust the input buffers if they are not completely consumed.
         for(ch = 0; ch < input_str_data_rx[0].bufs_num; ch++)
         {
            capi_v2_buf_t* buf_ptr = &(input_str_data_rx[0].buf_ptr[ch]);
            uint32_t num_bytes_consumed = buf_ptr->actual_data_len;
            memmove(buf_ptr->data_ptr, buf_ptr->data_ptr + num_bytes_consumed,
                    input_bytes_given_rx[0] - num_bytes_consumed);
            buf_ptr->actual_data_len = input_bytes_given_rx[0] - num_bytes_consumed;
         }

         // ------------------------
         // Now we will excersice Codec interrupts, we will raise codec events at some frame numbers
         // which will mimic the codec behaviour
         // ------------------------
         if(0 == (frame%117))
         {
            for(i = 0; i<cdc_info_ptr->num_cdc_int.num_cdc_int; i++)
            {
               switch(cdc_info_ptr->cdc_int_ptr[i].cdc_int)
               {
                  /* Interrupts for FBSP Rx processing */
                  case CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP:
                  case CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP:
                  {
                     capi_v2_port_info_t port_info;
                     capi_v2_buf_t buf;

                     int16_t cdc_clip_pcm_level[MAX_PCM_BUF_LEN] = {
                           0xD,0xE, 0xA, 0xD,
                           0xB,0xA, 0xB, 0xE,
                           0xD,0xE, 0xA, 0xD,
                           0xB,0xA, 0xB, 0xE,
                     };

                     capi_v2_cdc_clip_pcm_info_t clip_pcm_info;

                     clip_pcm_info.cdc_clip_pcm_level_buf.actual_data_len = sizeof(uint16_t) * MAX_PCM_BUF_LEN;
                     clip_pcm_info.cdc_clip_pcm_level_buf.max_data_len = sizeof(uint16_t) * MAX_PCM_BUF_LEN;
                     clip_pcm_info.cdc_clip_pcm_level_buf.data_ptr = (int8_t *)&cdc_clip_pcm_level;

                     buf.data_ptr = (int8_t *)&clip_pcm_info;
                     buf.max_data_len = sizeof(capi_v2_cdc_clip_pcm_info_t);
                     buf.actual_data_len = sizeof(capi_v2_cdc_clip_pcm_info_t);
                     port_info.is_valid = FALSE;

                     module_rx->module_ptr->vtbl_ptr->set_param(module_rx->module_ptr,
                                                               cdc_info_ptr->cdc_int_ptr[i].cdc_int,
                                                               &port_info,
                                                               &buf);

                     FARF(HIGH,
                          "CAPI V2 TEST: Sent codec interrupt:0x%lx for frame: %d.",
                          cdc_info_ptr->cdc_int_ptr[i].cdc_int,
                          frame);
                     break;
                  }

                  case CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK:
                  case CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE:
                  {
                     capi_v2_port_info_t port_info;
                     capi_v2_buf_t buf;

                     buf.data_ptr = NULL;
                     buf.max_data_len = 0;
                     buf.actual_data_len = 0;
                     port_info.is_valid = FALSE;

                     module_rx->module_ptr->vtbl_ptr->set_param(module_rx->module_ptr,
                                                               cdc_info_ptr->cdc_int_ptr[i].cdc_int,
                                                               &port_info,
                                                               &buf);
                     FARF(HIGH,
                          "CAPI V2 TEST: Sent codec interrupt:0x%lx for frame: %d.",
                          cdc_info_ptr->cdc_int_ptr[i].cdc_int,
                          frame);
                     break;
                  }
               }
            }
         }

      }
      if(CAPI_V2_FAILED(result))
      {
         break;
      }
   } while(1);

   // ------------------------
   // Clear processing buffers
   // ------------------------
   done: 
   if(NULL != input_buffer_tx)
   {
      free(input_buffer_tx);
      input_buffer_tx = NULL;
   }
   if(NULL!=temp_in_buffer_tx)
   {
      free(temp_in_buffer_tx);
      temp_in_buffer_tx = NULL;
   }
   if(NULL!=output_buffer_tx)
   {
      free(output_buffer_tx);
      output_buffer_tx = NULL;
   }
   if(NULL!=temp_out_buffer_tx)
   {
      free(temp_out_buffer_tx);
      temp_out_buffer_tx = NULL;
   }

   if(NULL!=input_buffer_rx)
   {
      free(input_buffer_rx);
      input_buffer_rx = NULL;
   }
   if(NULL!=temp_in_buffer_rx)
   {
      free(temp_in_buffer_rx);
      temp_in_buffer_rx = NULL;
   }
   if(NULL!=output_buffer_rx)
   {
      free(output_buffer_rx);
      output_buffer_rx = NULL;
   }
   if(NULL!=temp_out_buffer_rx)
   {
      free(temp_out_buffer_rx);
      temp_out_buffer_rx = NULL;
   }

   return result;
}

capi_v2_err_t test_capi_v2_main_feedback(capi_v2_get_static_properties_f get_static_properties_rx_f,
                                capi_v2_init_f init_rx_f,
                                capi_v2_get_static_properties_f get_static_properties_tx_f,
                                capi_v2_init_f init_tx_f,
                                capi_v2_proplist_t* init_set_properties,
                                capi_v2_proplist_t* static_properties,
                                const char* filename_rx_in,
                                const char* filename_rx_out,
                                const char* filename_rx_config,
                                const char* filename_tx_in,
                                const char* filename_tx_out,
                                const char* filename_tx_config)
{

   module_info_t module_rx;
   module_info_t module_tx;

   FARF (HIGH, "Beginning of test_capi_v2_main_feedback");
   uint32_t malloc_size = 0;
   uint8_t* ptr = 0;
   capi_v2_err_t result = CAPI_V2_EOK;
   int8_t curr_fwk_extns;
   int8_t curr_prop;
   capi_v2_buf_t cfg_buf;
   capi_v2_port_info_t port_info;
   port_info.is_valid = FALSE;

   cdc_info_t cdc_info;
   feedback_client_list_t *feedback_client_list_ptr = NULL;

   // ------------------------
   // Initializations RX files
   // ------------------------
   memset(&module_rx, 0, sizeof(module_rx));
   memset(&module_tx, 0, sizeof(module_tx));

   if(filename_rx_in != NULL)
   {
      if((module_rx.finp = fopen(filename_rx_in, "rb")) == NULL)
      {
         FARF(ERROR, "test_capi_v2: Cannot open input file                          ");
         THROW(result, CAPI_V2_EFAILED);
      }
   }
   if(filename_rx_out != NULL)
   {
      if((module_rx.fout = fopen(filename_rx_out, "wb")) == NULL)
      {
         FARF(ERROR, "test_capi_v2: Cannot open output file                          ");
         THROW(result, CAPI_V2_EFAILED);
      }
   }
   if((module_rx.fCfg = fopen(filename_rx_config, "rb")) == NULL)
   {
      FARF(ERROR, "test_capi_v2: Cannot open config file                                ");
      THROW(result, CAPI_V2_EFAILED);
   }

   // ------------------------
   // Get size requirements of CAPI_V2_RX
   // ------------------------
   FARF(HIGH, "MAIN: ----------------                                              ");
   FARF(HIGH, "MAIN: Initialize RX module                                          ");
   FARF(HIGH, "MAIN: ----------------                                              ");

   result = get_static_properties_rx_f(init_set_properties, static_properties);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR,
           "MAIN: get_static_properties error                                                 ");
      THROW(result, result);
   }

   capi_v2_utils_props_process_properties(static_properties, test_capi_v2_get_props, &malloc_size);
   malloc_size = align_to_8_byte(malloc_size);

   // ------------------------
   // Allocate memory CAPI_V2_RX
   // ------------------------
   ptr = (uint8_t*)malloc(malloc_size);
   if(!ptr)
   {
      FARF(ERROR, "MAIN: Memory allocation error                                       ");
      THROW(result, CAPI_V2_ENOMEMORY);
   }

   module_rx.module_ptr = (capi_v2_t*)ptr;
   memset(ptr, 0, malloc_size);
   FARF(HIGH, "allocated %6lu bytes of memory at location 0x%08p             ", malloc_size, ptr);

   module_rx.out_format.max_data_len = sizeof(module_rx.out_format_buf);
   module_rx.out_format.data_ptr = module_rx.out_format_buf;

   for(curr_prop = 0; curr_prop < static_properties->props_num; curr_prop++)
   {
      if(CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS == static_properties->prop_ptr[curr_prop].id)
      {
         std_memscpy(&module_rx.num_extensions, sizeof(capi_v2_num_needed_framework_extensions_t),
                 static_properties->prop_ptr[curr_prop].payload.data_ptr,
                 static_properties->prop_ptr[curr_prop].payload.actual_data_len);
         break;
      }
   }

   module_rx.fwk_extn_ptr = (capi_v2_framework_extension_id_t *)malloc(
         module_rx.num_extensions.num_extensions * sizeof(capi_v2_framework_extension_id_t));
   if(!module_rx.fwk_extn_ptr)
   {
      FARF(ERROR, "MAIN: Memory allocation error                                       ");
      THROW(result, CAPI_V2_ENOMEMORY);
   }

   capi_v2_prop_t static_props_extn_rx[] =
   {
         { CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS,
               { (int8_t *)module_rx.fwk_extn_ptr, 0, sizeof(capi_v2_framework_extension_id_t)
                     * module_rx.num_extensions.num_extensions },
                     { FALSE, FALSE, 0 } }, };

   capi_v2_proplist_t static_proplist_extn_rx =
   { 1, static_props_extn_rx };

   result = get_static_properties_rx_f(init_set_properties, &static_proplist_extn_rx);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR,
           "MAIN: get_static_properties error                                                 ");
      THROW(result, result);
   }

   // ------------------------
   // Initialize module CAPI_V2_RX
   // ------------------------
   result = init_rx_f((capi_v2_t*)(ptr), init_set_properties);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR, "MAIN: Initialization error                                          ");
      THROW(result, result);
   }

   {
      // Set event callback information
      capi_v2_event_callback_info_t event_cb_info;
      event_cb_info = capi_v2_tst_get_cb_info(&module_rx);

      capi_v2_proplist_t proplist;
      capi_v2_prop_t props[1];
      proplist.props_num = 1;
      proplist.prop_ptr = props;
      props[0].id = CAPI_V2_EVENT_CALLBACK_INFO;
      props[0].payload.data_ptr = (int8_t*)(&event_cb_info);
      props[0].payload.actual_data_len = sizeof(event_cb_info);
      props[0].payload.max_data_len = sizeof(event_cb_info);

      module_rx.module_ptr->vtbl_ptr->set_properties(module_rx.module_ptr, &proplist);

   }
   module_rx.requires_data_buffering =
         (bool_t)*(static_properties->prop_ptr[CAPI_V2_REQUIRES_DATA_BUFFERING].payload.data_ptr);

   // ------------------------
   // Initializations Tx files
   // ------------------------
   if(filename_tx_in != NULL)
   {
      if((module_tx.finp = fopen(filename_tx_in, "rb")) == NULL)
      {
         FARF(ERROR, "test_capi_v2: Cannot open input file                          ");
         THROW(result, CAPI_V2_EFAILED);
      }
   }
   if(filename_tx_out != NULL)
   {
      if((module_tx.fout = fopen(filename_tx_out, "wb")) == NULL)
      {
         FARF(ERROR, "test_capi_v2: Cannot open output file                          ");
         THROW(result, CAPI_V2_EFAILED);
      }
   }
   if((module_tx.fCfg = fopen(filename_tx_config, "rb")) == NULL)
   {
      FARF(ERROR, "test_capi_v2: Cannot open config file                                ");
      THROW(result, CAPI_V2_EFAILED);
   }

   // ------------------------
   // Get size requirements of CAPI_V2_TX
   // ------------------------
   FARF(HIGH, "MAIN: ----------------                                              ");
   FARF(HIGH, "MAIN: Initialize TX module                                          ");
   FARF(HIGH, "MAIN: ----------------                                              ");

   result = get_static_properties_tx_f(init_set_properties, static_properties);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR,
           "MAIN: get_static_properties error                                                 ");
      THROW(result, result);
   }

   capi_v2_utils_props_process_properties(static_properties, test_capi_v2_get_props, &malloc_size);
   malloc_size = align_to_8_byte(malloc_size);

   // ------------------------
   // Allocate memory CAPI_V2_TX
   // ------------------------
   ptr = (uint8_t*)malloc(malloc_size);
   if(!ptr)
   {
      FARF(ERROR, "MAIN: Memory allocation error                                       ");
      THROW(result, CAPI_V2_ENOMEMORY);
   }

   module_tx.module_ptr = (capi_v2_t*)ptr;
   FARF(HIGH, "allocated %6lu bytes of memory at location 0x%08p             ", malloc_size, ptr);

   module_tx.out_format.max_data_len = sizeof(module_tx.out_format_buf);
   module_tx.out_format.data_ptr = module_tx.out_format_buf;

   for(curr_prop = 0; curr_prop < static_properties->props_num; curr_prop++)
   {
      if(CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS == static_properties->prop_ptr[curr_prop].id)
      {
         std_memscpy(&module_tx.num_extensions, sizeof(capi_v2_num_needed_framework_extensions_t),
                 static_properties->prop_ptr[curr_prop].payload.data_ptr,
                 static_properties->prop_ptr[curr_prop].payload.actual_data_len);
         break;
      }
   }

   module_tx.fwk_extn_ptr = (capi_v2_framework_extension_id_t *)malloc(
         module_tx.num_extensions.num_extensions * sizeof(capi_v2_framework_extension_id_t));
   if(!module_tx.fwk_extn_ptr)
   {
      FARF(ERROR, "MAIN: Memory allocation error                                       ");
      THROW(result, CAPI_V2_ENOMEMORY);
   }

   capi_v2_prop_t static_props_extn_tx[] =
   {
         { CAPI_V2_NEEDED_FRAMEWORK_EXTENSIONS,
               { (int8_t *)module_tx.fwk_extn_ptr, 0, sizeof(capi_v2_framework_extension_id_t)
                     * module_tx.num_extensions.num_extensions },
                     { FALSE, FALSE, 0 } }, };

   capi_v2_proplist_t static_proplist_extn_tx =
   { 1, static_props_extn_tx };

   result = get_static_properties_tx_f(init_set_properties, &static_proplist_extn_tx);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR,
           "MAIN: get_static_properties error                                                 ");
      THROW(result, result);
   }

   // ------------------------
   // Initialize module CAPI_V2_TX
   // ------------------------
   result = init_tx_f((capi_v2_t*)(ptr), init_set_properties);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR, "MAIN: Initialization error                                          ");
      THROW(result, result);
   }

   {
      // Set event callback information
      capi_v2_event_callback_info_t event_cb_info;
      event_cb_info = capi_v2_tst_get_cb_info(&module_tx);

      capi_v2_proplist_t proplist;
      capi_v2_prop_t props[1];
      proplist.props_num = 1;
      proplist.prop_ptr = props;
      props[0].id = CAPI_V2_EVENT_CALLBACK_INFO;
      props[0].payload.data_ptr = (int8_t*)(&event_cb_info);
      props[0].payload.actual_data_len = sizeof(event_cb_info);
      props[0].payload.max_data_len = sizeof(event_cb_info);

      module_tx.module_ptr->vtbl_ptr->set_properties(module_tx.module_ptr, &proplist);

   }
   module_tx.requires_data_buffering =
         (bool_t)*(static_properties->prop_ptr[CAPI_V2_REQUIRES_DATA_BUFFERING].payload.data_ptr);

   // ------------------------
   // Setup the feedback path config
   // ------------------------
   feedback_info_t feedback_path_info_v2;
   feedback_path_info_v2.feedback_cfg.num_channels = 4; // V I V Isens for two spkrs

   feedback_path_info_v2.feedback_cfg.chan_info[0] = 1;
   feedback_path_info_v2.feedback_cfg.chan_info[1] = 2;
   feedback_path_info_v2.feedback_cfg.chan_info[2] = 3;
   feedback_path_info_v2.feedback_cfg.chan_info[3] = 4;

   // ------------------------
   // Create the Buffer Q(in Tx module) and Data Q(in Rx module) for feedback path
   // ------------------------
   create_queue(&module_tx.queue_handle);
   create_queue(&module_rx.queue_handle);

   // ------------------------
   // Setup the feedback Client list --> List of Queues feeding into the Rx module.
   // Here, we have one client Tx module feeding into the Rx module
   // ------------------------

   feedback_client_list_ptr = (feedback_client_list_t*)malloc(sizeof(feedback_client_list_t));
   if(!feedback_client_list_ptr)
   {
      FARF(ERROR, "MAIN: Memory allocation error                                       ");
      THROW(result, CAPI_V2_ENOMEMORY);
   }

   fb_client_info_t fb_client_info;
   fb_client_info.buf_q_handle = module_tx.queue_handle;
   fb_client_info.data_q_handle = module_rx.queue_handle;

   feedback_client_list_ptr->element = &fb_client_info;
   feedback_client_list_ptr->next = NULL;

   // ------------------------
   // Setup the feedback path in CAPI_V2_TX
   // ------------------------
   for(curr_fwk_extns = 0; curr_fwk_extns < module_tx.num_extensions.num_extensions;
         curr_fwk_extns++)
   {
      if(FWK_EXTN_FEEDBACK == module_tx.fwk_extn_ptr[curr_fwk_extns].id)
      {
         feedback_path_info_v2.bufQ_ptr = module_tx.queue_handle;
         feedback_path_info_v2.dataQ_ptr = module_rx.queue_handle;
         feedback_path_info_v2.num_buffers_ptr = &module_tx.queue_num_buffers;

         cfg_buf.data_ptr = (int8_t *)&feedback_path_info_v2;
         cfg_buf.max_data_len = sizeof(feedback_info_t);
         cfg_buf.actual_data_len = sizeof(feedback_info_t);
         module_tx.module_ptr->vtbl_ptr->set_param(module_tx.module_ptr,
                                                   CAPI_V2_PARAM_ID_FB_PATH_INFO, &port_info,
                                                   &cfg_buf);
      }
   }

   // ------------------------
   // Setup the feedback path client info to CAPI_V2_RX
   // ------------------------
   for(curr_fwk_extns = 0; curr_fwk_extns < module_rx.num_extensions.num_extensions;
         curr_fwk_extns++)
   {
      if(FWK_EXTN_FEEDBACK == module_rx.fwk_extn_ptr[curr_fwk_extns].id)
      {
         cfg_buf.data_ptr = (int8_t *)&feedback_client_list_ptr;
         cfg_buf.max_data_len = sizeof(void *);
         cfg_buf.actual_data_len = sizeof(void *);
         module_rx.module_ptr->vtbl_ptr->set_param(module_rx.module_ptr,
                                                   CAPI_V2_PARAM_ID_CLIENT_INFO, &port_info,
                                                   &cfg_buf);
      }
   }

   // ------------------------
   // Run config file for RX
   // ------------------------
   FARF(HIGH, "MAIN: ----------------                                              ");
   FARF(HIGH, "MAIN: Run config file for RX                                        ");
   FARF(HIGH, "MAIN: ----------------                                              ");

   // Added new Input/Output commands for test script support
   testCommand inputCmd[2];
   strncpy(inputCmd[0].opCode, "Input", 6);
   strncpy(inputCmd[1].opCode, "Output", 7);
   inputCmd[0].pFunction = &Inputfile;
   inputCmd[1].pFunction = &Outputfile;

   result = RunTest(&module_rx, inputCmd, 2);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR, "MAIN: Error in RunTest                                              ");
      THROW(result, result);
   }

   // ------------------------
   // Run config file for TX
   // ------------------------
   FARF(HIGH, "MAIN: ----------------                                              ");
   FARF(HIGH, "MAIN: Run config file for TX                                        ");
   FARF(HIGH, "MAIN: ----------------                                              ");

   // Added new Input/Output commands for test script support
   result = RunTest(&module_tx, inputCmd, 2);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR, "MAIN: Error in RunTest                                              ");
      THROW(result, result);
   }

   // ------------------------
   // Setup the codec interrupt path to CAPI_V2_RX
   // ------------------------
   for(curr_fwk_extns = 0; curr_fwk_extns < module_rx.num_extensions.num_extensions;
         curr_fwk_extns++)
   {
      if(FWK_EXTN_CDC_INTERRUPT == module_rx.fwk_extn_ptr[curr_fwk_extns].id)
      {
         capi_v2_buf_t buf;
         port_info.is_valid = FALSE;

         buf.data_ptr = (int8_t *)&cdc_info.num_cdc_int.num_cdc_int;
         buf.max_data_len = sizeof(capi_v2_num_needed_cdc_int_t);

         module_rx.module_ptr->vtbl_ptr->get_param(module_rx.module_ptr, CAPI_V2_PARAM_ID_NUM_NEEDED_CDC_INT,
                                         &port_info, &buf);

         if(0 != cdc_info.num_cdc_int.num_cdc_int)
         {
            if(NULL
                  == (cdc_info.cdc_int_ptr = (capi_v2_cdc_int_t *)malloc(
                        sizeof(capi_v2_cdc_int_t) * cdc_info.num_cdc_int.num_cdc_int)))
            {
               FARF(ERROR, "MAIN: Memory allocation error");
               THROW(result, CAPI_V2_ENOMEMORY);
            }

            buf.data_ptr = (int8_t *)cdc_info.cdc_int_ptr;
            buf.max_data_len = sizeof(capi_v2_cdc_int_t) * cdc_info.num_cdc_int.num_cdc_int;
            module_rx.module_ptr->vtbl_ptr->get_param(module_rx.module_ptr, CAPI_V2_PARAM_ID_CDC_INT_LIST,
                                            &port_info, &buf);

            FARF(HIGH, "MAIN: Successfully registered for Codec interrupt!");

         }
      }
   }

   // ------------------------
   // Process TX && RX
   // ------------------------
   test_capi_v2_process(&module_tx, &module_rx, &cdc_info);

   // ------------------------
   // Destroy CAPI V2 RX and free memory
   // ------------------------
   result = module_rx.module_ptr->vtbl_ptr->end(module_rx.module_ptr);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR, "MAIN: Error in call to end                                          ");
      THROW(result, result);
   }

   // ------------------------
   // Destroy CAPI V2 Tx and free memory
   // ------------------------
   result = module_tx.module_ptr->vtbl_ptr->end(module_tx.module_ptr);
   if(CAPI_V2_FAILED(result))
   {
      FARF(ERROR, "MAIN: Error in call to end                                          ");
      THROW(result, result);
   }

   // ------------------------
   // Destroy the Buffer Q(in Tx module) and Data Q(in Rx module) for feedback path
   // ------------------------
   destroy_queue(&module_rx.queue_handle);
   destroy_queue(&module_tx.queue_handle);

   CATCH(result){};

   if(feedback_client_list_ptr)
   {
      free(feedback_client_list_ptr);
   }

   if(module_rx.fwk_extn_ptr)
   {
      free(module_rx.fwk_extn_ptr);
   }

   if(module_tx.fwk_extn_ptr)
   {
      free(module_tx.fwk_extn_ptr);
   }

   if(module_rx.module_ptr)
   {
      free(module_rx.module_ptr);
   }

   if(module_tx.module_ptr)
   {
      free(module_tx.module_ptr);
   }

   if(module_rx.finp)
   {
      fclose(module_rx.finp);
   }
   if(module_rx.fout)
   {
      fclose(module_rx.fout);
   }
   if(module_rx.fCfg)
   {
      fclose(module_rx.fCfg);
   }
   FARF(HIGH, "MAIN: Done                                                          ");
   return result;
}

capi_v2_err_t test_capi_v2_main(capi_v2_get_static_properties_f get_static_properties_f,
                             capi_v2_init_f init_f,
                             capi_v2_proplist_t* init_set_properties,
                             capi_v2_proplist_t* static_properties,
                             const char* filename_in, const char* filename_out,
                             const char* filename_config)
{
  module_info_t module;
  uint32_t malloc_size = 0;
  uint8_t* ptr = 0;
  capi_v2_err_t result = CAPI_V2_EOK;

  // ------------------------
  // Initializations
  // ------------------------
  // TODO init_profiling();
  memset(&module, 0, sizeof(module));

  if (filename_in != NULL) {
    if ((module.finp = fopen(filename_in, "rb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open input file                          ");
      THROW(result, CAPI_V2_EFAILED);
    }
  }
  if (filename_out != NULL) {
    if ((module.fout = fopen(filename_out, "wb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open output file                          ");
      THROW(result, CAPI_V2_EFAILED);
    }
  }
  if ((module.fCfg = fopen(filename_config, "rb")) == NULL) {
    FARF(ERROR, "test_capi_v2: Cannot open config file                                ");
    THROW(result, CAPI_V2_EFAILED);
  }

  // ------------------------
  // STEP 1: Get size requirements of CAPI_V2
  // ------------------------
  FARF(HIGH, "MAIN: ----------------                                              ");
  FARF(HIGH, "MAIN: Initialize module                                             ");
  FARF(HIGH, "MAIN: ----------------                                              ");

  result = get_static_properties_f(init_set_properties, static_properties);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: get_static_properties error                                                 ");
    THROW(result, result);
  }

  capi_v2_utils_props_process_properties(static_properties,
                                         test_capi_v2_get_props, &malloc_size);
  malloc_size = align_to_8_byte(malloc_size);

  // ------------------------
  // STEP 2: Allocate memory
  // ------------------------
  ptr = (uint8_t*)malloc(malloc_size);
  if (!ptr) {
    FARF(ERROR, "MAIN: Memory allocation error                                       ");
    THROW(result, CAPI_V2_ENOMEMORY);
  }

  module.module_ptr = (capi_v2_t*)ptr;
  FARF(HIGH, "allocated %6lu bytes of memory at location 0x%08p             ",
       malloc_size, ptr);

  module.out_format.max_data_len = sizeof(module.out_format_buf);
  module.out_format.data_ptr = module.out_format_buf;
  // ------------------------
  // STEP 3: Initialize module
  // ------------------------
  result = init_f((capi_v2_t*)(ptr), init_set_properties);
  if (CAPI_V2_FAILED(result)) {
      FARF(ERROR, "MAIN: Initialization error                                          ");
  THROW(result, result);
  }

  {
  // Set event callback information
  capi_v2_event_callback_info_t event_cb_info;
  event_cb_info = capi_v2_tst_get_cb_info(&module);

  capi_v2_proplist_t proplist;
  capi_v2_prop_t props[1];
  proplist.props_num = 1;
  proplist.prop_ptr = props;
  props[0].id = CAPI_V2_EVENT_CALLBACK_INFO;
  props[0].payload.data_ptr = (int8_t*)(&event_cb_info);
  props[0].payload.actual_data_len = sizeof(event_cb_info);
  props[0].payload.max_data_len = sizeof(event_cb_info);

  module.module_ptr->vtbl_ptr->set_properties(module.module_ptr, &proplist);

  }
  module.requires_data_buffering = (bool_t)*(static_properties->prop_ptr[CAPI_V2_REQUIRES_DATA_BUFFERING].payload.data_ptr);

  // ------------------------
  // Run config file
  // ------------------------
  FARF(HIGH, "MAIN: ----------------                                              ");
  FARF(HIGH, "MAIN: Run config file                                               ");
  FARF(HIGH, "MAIN: ----------------                                              ");

  // Added new Input/Output commands for test script support
  testCommand inputCmd[2];
  strncpy(inputCmd[0].opCode, "Input", 6);
  strncpy(inputCmd[1].opCode, "Output", 7);
  inputCmd[0].pFunction = &Inputfile;
  inputCmd[1].pFunction = &Outputfile;

  result = RunTest(&module, inputCmd, 2);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: Error in RunTest                                              ");
    THROW(result, result);
  }

  // ------------------------
  // Destroy CAPI V2 and free memory
  // ------------------------
  result = module.module_ptr->vtbl_ptr->end(module.module_ptr);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: Error in call to end                                          ");
    THROW(result, result);
  }

  CATCH(result){};

  if (ptr) {
    free(ptr);
  }

  // TODO deinit_profiling();

  if (module.finp) {
    fclose(module.finp);
  }
  if (module.fout) {
    fclose(module.fout);
  }
  if (module.fCfg) {
    fclose(module.fCfg);
  }
  FARF(HIGH, "MAIN: Done                                                          ");
  return result;
}


