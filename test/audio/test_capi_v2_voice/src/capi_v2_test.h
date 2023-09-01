#ifndef CAPI_V2_TEST_H
#define CAPI_V2_TEST_H
/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "adsp_error_codes.h"
#include "Elite_CAPI_V2.h"
#include "capi_v2_profile.h"

#define WORDSIZE 30

#define FORMAT_BUF_SIZE 128

typedef struct _module_info_t
{
  capi_v2_t* module_ptr;
  bool_t is_enabled;
  uint32_t alg_delay;
  uint32_t kpps;
  uint32_t id;
  void *ptr;
  bool_t is_in_place;
  uint32_t in_buffer_len;
  uint32_t out_buffer_len;
  FILE* finp;
  FILE* finpref;
  FILE* fout;
  FILE* fCfg;
  bool_t requires_data_buffering;
  capi_v2_buf_t in_format;
  capi_v2_buf_t out_format;
  // Preallocating buffers to avoid the need for handling malloc failures.
  int8_t in_format_buf[FORMAT_BUF_SIZE];
  int8_t out_format_buf[FORMAT_BUF_SIZE];
} module_info_t;

typedef struct testCommand testCommand;
struct testCommand {
  char opCode[WORDSIZE];
  capi_v2_err_t(* pFunction)(module_info_t* module);
};

uint32_t calculateNumBuffers(const capi_v2_buf_t* format);
capi_v2_err_t FillInputBuffer(capi_v2_stream_data_t* input_bufs_ptr, module_info_t* module, void* temp_in_buffer);
capi_v2_err_t RunTest(module_info_t* module, const testCommand* pExtendedCmdSet, const uint32_t extendedCmdSetSize);
capi_v2_err_t GetWord(FILE* fCfg, char word[]);
capi_v2_err_t GetUIntParameter(FILE* fCfg, const char parameterName[], uint32_t* pValues);
capi_v2_err_t GetIntParameter(FILE* fCfg, const char parameterName[], int32_t* pValue);
capi_v2_err_t Inputfile(module_info_t* module);
capi_v2_err_t Outputfile(module_info_t* module);

capi_v2_err_t CheckInputOutputSizes(capi_v2_stream_data_t* input[],
                                 uint32_t input_bytes_given[],
                                 capi_v2_stream_data_t* output[],
                                 bool_t requires_data_buffering,
                                 uint32_t num_input_ports,
                                 uint32_t num_output_ports);
capi_v2_err_t DumpOutputToFile(capi_v2_stream_data_t* output_bufs_ptr, module_info_t* module, void* temp_out_buffer);

capi_v2_event_callback_info_t capi_v2_tst_get_cb_info(module_info_t* module);

#endif // CAPI_V2_TEST_H

