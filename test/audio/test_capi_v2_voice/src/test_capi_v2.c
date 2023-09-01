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

#include "capi_v2_utils_props.h"

#include "capi_v2_test.h"

#include "inter_module_comm_server.h"

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

    // Set event callback information
    capi_v2_event_callback_info_t event_cb_info1;
    event_cb_info1 = capi_v2_tst_get_cb_info(&module);

    capi_v2_prop_t props1[2];
    init_set_properties->props_num = 2;
    init_set_properties->prop_ptr = props1;
    props1[0].id = CAPI_V2_EVENT_CALLBACK_INFO;
    props1[0].payload.data_ptr = (int8_t*)(&event_cb_info1);
    props1[0].payload.actual_data_len = sizeof(event_cb_info1);
    props1[0].payload.max_data_len = sizeof(event_cb_info1);

    capi_v2_session_identifier_t session_identifier;
    session_identifier.service_id = 1;
    session_identifier.session_id = 2;
    props1[1].id = CAPI_V2_SESSION_IDENTIFIER;
    props1[1].payload.data_ptr = (int8_t*)(&session_identifier);
    props1[1].payload.actual_data_len = sizeof(session_identifier);
    props1[1].payload.max_data_len = sizeof(session_identifier);

  result = init_f((capi_v2_t*)(ptr), init_set_properties);
  if (CAPI_V2_FAILED(result)) {
      FARF(ERROR, "MAIN: Initialization error                                          ");
  THROW(result, result);
  }


/*
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
*/
/*
    capi_v2_session_identifier_t session_identifier;
    capi_v2_proplist_t proplist;
    capi_v2_prop_t props[1];
    proplist.props_num = 1;
    proplist.prop_ptr = props;
    session_identifier.service_id = 1;
    session_identifier.session_id = 2;
    props[0].id = CAPI_V2_SESSION_IDENTIFIER;
    props[0].payload.data_ptr = (int8_t*)(&session_identifier);
    props[0].payload.actual_data_len = sizeof(session_identifier);
    props[0].payload.max_data_len = sizeof(session_identifier);

  module.module_ptr->vtbl_ptr->set_properties(module.module_ptr, &proplist);
*/

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


capi_v2_err_t test_capi_v2_main_refport(capi_v2_get_static_properties_f get_static_properties_f,
                             capi_v2_init_f init_f,
                             capi_v2_proplist_t* init_set_properties,
                             capi_v2_proplist_t* static_properties,
                             const char* filename_in, const char* filename_inref,
                             const char* filename_out,
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

  if (filename_inref != NULL) {
    if ((module.finpref = fopen(filename_inref, "rb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open input ref file                          ");
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
  if (module.finpref) {
    fclose(module.finpref);
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


/*
 * Input and Output media format is same for both the TX and RX module.
 * */
capi_v2_err_t test_capi_v2_process_imc_TxRx(module_info_t* module_tx, module_info_t* module_rx)
{
   FARF(HIGH, "CAPI V2 TEST: Executing Process Data command.");
   uint32_t frame = 1;
///   int8_t i = 0;
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

      if(CAPI_V2_FAILED(result = FillInputBuffer(input_str_data_rx, module_rx, temp_in_buffer_rx)))
      {
         FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
         break;
      }

    bool_t is_file_over = (input_str_data_tx[0].buf_ptr[0].actual_data_len < input_str_data_tx[0].buf_ptr[0].max_data_len) && (input_str_data_rx[0].buf_ptr[0].actual_data_len < input_str_data_rx[0].buf_ptr[0].max_data_len);
    if (is_file_over) {
      break;
    }

      while((input_str_data_tx[0].buf_ptr[0].actual_data_len > 0)
            && (input_str_data_rx[0].buf_ptr[0].actual_data_len > 0))
      {

         //FARF(HIGH, "CAPI V2 TEST: Processing frame: %ld", frame);
         frame++;

         result = CAPI_V2_EOK;
         uint32_t input_bytes_given_tx[] = { input_str_data_tx[0].buf_ptr[0].actual_data_len };
         uint32_t input_bytes_given_rx[] =    { input_str_data_rx[0].buf_ptr[0].actual_data_len };


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
         for (ch = 0; ch < input_str_data_tx[0].bufs_num; ch++) {
            capi_v2_buf_t* bufref_ptr = &(input_str_data_tx[0].buf_ptr[ch]);
            uint32_t ref_num_bytes_consumed = bufref_ptr->actual_data_len;
            memmove(bufref_ptr->data_ptr, bufref_ptr->data_ptr + ref_num_bytes_consumed,
                    input_bytes_given_tx[0] - ref_num_bytes_consumed);
            bufref_ptr->actual_data_len = input_bytes_given_tx[0] - ref_num_bytes_consumed;
         }

      }
      if(CAPI_V2_FAILED(result))
      {
         break;
      }
   } while(frame<15); //while(1);


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


capi_v2_err_t test_capi_v2_main_imc(capi_v2_get_static_properties_f get_static_properties_tx_f,
                             capi_v2_init_f init_tx_f,
                             capi_v2_get_static_properties_f get_static_properties_rx_f,
                             capi_v2_init_f init_rx_f,
                             capi_v2_proplist_t* init_set_properties,
                             capi_v2_proplist_t* static_properties,
                             const char* filename_in_tx, const char* filename_out_tx, const char* filename_config_tx,
                             const char* filename_in_rx, const char* filename_out_rx, const char* filename_config_rx)
{
  module_info_t module_rx, module_tx;
  uint32_t malloc_size = 0;
  uint8_t* ptr = 0;
  uint8_t* ptr1 = 0;
  capi_v2_err_t result = CAPI_V2_EOK;

  // ------------------------
  // Initializations
  // ------------------------
  // TODO init_profiling();
  memset(&module_rx, 0, sizeof(module_rx));
  memset(&module_tx, 0, sizeof(module_tx));

  if (filename_in_rx != NULL) {
    if ((module_rx.finp = fopen(filename_in_rx, "rb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open input file                          ");
      THROW(result, CAPI_V2_EFAILED);
    }
  }
  if (filename_out_rx != NULL) {
    if ((module_rx.fout = fopen(filename_out_rx, "wb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open output file                          ");
      THROW(result, CAPI_V2_EFAILED);
    }
  }
  if ((module_rx.fCfg = fopen(filename_config_rx, "rb")) == NULL) {
    FARF(ERROR, "test_capi_v2: Cannot open config file                                ");
    THROW(result, CAPI_V2_EFAILED);
  }


  if (filename_in_tx != NULL) {
    if ((module_tx.finp = fopen(filename_in_tx, "rb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open input file                          ");
      THROW(result, CAPI_V2_EFAILED);
    }
  }
  if (filename_out_tx != NULL) {
    if ((module_tx.fout = fopen(filename_out_tx, "wb")) == NULL) {
      FARF(ERROR, "test_capi_v2: Cannot open output file                          ");
      THROW(result, CAPI_V2_EFAILED);
    }
  }
  if ((module_tx.fCfg = fopen(filename_config_tx, "rb")) == NULL) {
    FARF(ERROR, "test_capi_v2: Cannot open config file                                ");
    THROW(result, CAPI_V2_EFAILED);
  }

    //intialize IMC service used for inter module communication.
    result = imc_init();
    if (CAPI_V2_FAILED(result))
    {
       FARF(ERROR, "Failed to initialize intermodule communication server!");
       THROW(result, result);
    }

  // ------------------------
  // STEP 1: Get size requirements of CAPI_V2
  // ------------------------
  FARF(HIGH, "MAIN: ------------------------                                          ");
  FARF(HIGH, "MAIN: Initialize module for Rx                                          ");
  FARF(HIGH, "MAIN: ------------------------                                          ");

  result = get_static_properties_rx_f(init_set_properties, static_properties);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: get_static_properties error                                                 ");
    THROW(result, result);
  }

  capi_v2_utils_props_process_properties(static_properties,
                                         test_capi_v2_get_props, &malloc_size);
  malloc_size = align_to_8_byte(malloc_size);


  FARF(HIGH, "MAIN: ------------------------                                            ");
  FARF(HIGH, "MAIN: Initialize module for Tx                                            ");
  FARF(HIGH, "MAIN: ------------------------                                            ");

  result = get_static_properties_tx_f(init_set_properties, static_properties);
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
    FARF(ERROR, "MAIN: Memory allocation error for Rx                                       ");
    THROW(result, CAPI_V2_ENOMEMORY);
  }
  module_rx.module_ptr = (capi_v2_t*)ptr;
  FARF(HIGH, "allocated %6lu bytes of memory at location 0x%08p for Rx             ",
       malloc_size, ptr);
  module_rx.out_format.max_data_len = sizeof(module_rx.out_format_buf);
  module_rx.out_format.data_ptr = module_rx.out_format_buf;

  ptr1 = (uint8_t*)malloc(malloc_size);
  if (!ptr1) {
    FARF(ERROR, "MAIN: Memory allocation error for Tx                                       ");
    THROW(result, CAPI_V2_ENOMEMORY);
  }
  module_tx.module_ptr = (capi_v2_t*)ptr1;
  FARF(HIGH, "allocated %6lu bytes of memory at location 0x%08p for Tx             ",
       malloc_size, ptr1);
  module_tx.out_format.max_data_len = sizeof(module_tx.out_format_buf);
  module_tx.out_format.data_ptr = module_tx.out_format_buf;


  // ------------------------
  // STEP 3: Initialize module
  // ------------------------

    // Set event callback information for Rx
    capi_v2_event_callback_info_t event_cb_info;
    event_cb_info = capi_v2_tst_get_cb_info(&module_rx);

    capi_v2_prop_t props[2];
    init_set_properties->props_num = 2;
    init_set_properties->prop_ptr = props;
    props[0].id = CAPI_V2_EVENT_CALLBACK_INFO;
    props[0].payload.data_ptr = (int8_t*)(&event_cb_info);
    props[0].payload.actual_data_len = sizeof(event_cb_info);
    props[0].payload.max_data_len = sizeof(event_cb_info);

    capi_v2_session_identifier_t session_identifier;
    session_identifier.service_id = 9;
    session_identifier.session_id = 256;
    props[1].id = CAPI_V2_SESSION_IDENTIFIER;
    props[1].payload.data_ptr = (int8_t*)(&session_identifier);
    props[1].payload.actual_data_len = sizeof(session_identifier);
    props[1].payload.max_data_len = sizeof(session_identifier);

  result = init_rx_f((capi_v2_t*)(ptr), init_set_properties);
  if (CAPI_V2_FAILED(result)) {
      FARF(ERROR, "MAIN: Initialization error for Rx                                          ");
  THROW(result, result);
  }
  module_rx.requires_data_buffering = (bool_t)*(static_properties->prop_ptr[CAPI_V2_REQUIRES_DATA_BUFFERING].payload.data_ptr);


    // Set event callback information for Tx
    capi_v2_event_callback_info_t event_cb_info1;
    event_cb_info1 = capi_v2_tst_get_cb_info(&module_tx);

    capi_v2_prop_t props1[2];
    init_set_properties->props_num = 2;
    init_set_properties->prop_ptr = props1;
    props1[0].id = CAPI_V2_EVENT_CALLBACK_INFO;
    props1[0].payload.data_ptr = (int8_t*)(&event_cb_info1);
    props1[0].payload.actual_data_len = sizeof(event_cb_info1);
    props1[0].payload.max_data_len = sizeof(event_cb_info1);

    capi_v2_session_identifier_t session_identifier1;
    session_identifier1.service_id = 8;
    session_identifier1.session_id = 256;
    props1[1].id = CAPI_V2_SESSION_IDENTIFIER;
    props1[1].payload.data_ptr = (int8_t*)(&session_identifier1);
    props1[1].payload.actual_data_len = sizeof(session_identifier1);
    props1[1].payload.max_data_len = sizeof(session_identifier1);

  event_cb_info = capi_v2_tst_get_cb_info(&module_tx);

  result = init_tx_f((capi_v2_t*)(ptr1), init_set_properties);
  if (CAPI_V2_FAILED(result)) {
      FARF(ERROR, "MAIN: Initialization error for Tx                                          ");
  THROW(result, result);
  }
  module_tx.requires_data_buffering = (bool_t)*(static_properties->prop_ptr[CAPI_V2_REQUIRES_DATA_BUFFERING].payload.data_ptr);


  // ------------------------
  // Run config file for Rx
  // ------------------------
  FARF(HIGH, "MAIN: ----------------------                                             ");
  FARF(HIGH, "MAIN: Run config file for Rx                                             ");
  FARF(HIGH, "MAIN: ----------------------                                             ");

  // Added new Input/Output commands for test script support
  testCommand inputCmd[2];
  strncpy(inputCmd[0].opCode, "Input", 6);
  strncpy(inputCmd[1].opCode, "Output", 7);
  inputCmd[0].pFunction = &Inputfile;
  inputCmd[1].pFunction = &Outputfile;

  result = RunTest(&module_rx, inputCmd, 2);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: Error in RunTest                                                ");
    THROW(result, result);
  }




  // ------------------------
  // Run config file for Tx
  // ------------------------
  FARF(HIGH, "MAIN: ----------------------                                             ");
  FARF(HIGH, "MAIN: Run config file for Tx                                             ");
  FARF(HIGH, "MAIN: ----------------------                                             ");

  result = RunTest(&module_tx, inputCmd, 2);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: Error in RunTest for Tx                                         ");
    THROW(result, result);
  }


   // ------------------------
   // Process TX && RX
   // ------------------------
   test_capi_v2_process_imc_TxRx(&module_tx, &module_rx);

  // ------------------------
  // Destroy CAPI V2 and free memory
  // ------------------------
  result = module_rx.module_ptr->vtbl_ptr->end(module_rx.module_ptr);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: Error in call to end                                          ");
    THROW(result, result);
  }


  // ------------------------
  // Destroy CAPI V2 and free memory
  // ------------------------
  result = module_tx.module_ptr->vtbl_ptr->end(module_tx.module_ptr);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "MAIN: Error in call to end                                          ");
    THROW(result, result);
  }

    //inter module communication deinit
    imc_deinit();

  CATCH(result){};

  if (ptr) {
    free(ptr);
  }
  if (ptr1) {
    free(ptr1);
  }
  // TODO deinit_profiling();

  if (module_rx.finp) {
    fclose(module_rx.finp);
  }
  if (module_tx.finp) {
      fclose(module_tx.finp);
  }

  if (module_rx.fout) {
    fclose(module_rx.fout);
  }
  if (module_tx.fout) {
    fclose(module_tx.fout);
  }

  if (module_rx.fCfg) {
    fclose(module_rx.fCfg);
  }
  if (module_tx.fCfg) {
    fclose(module_tx.fCfg);
  }


  FARF(HIGH, "MAIN: Done                                                          ");
  return result;


}


