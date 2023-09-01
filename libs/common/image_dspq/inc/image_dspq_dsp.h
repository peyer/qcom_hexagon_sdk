/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef IMAGE_DSP_ASYNCDSPQ
#define IMAGE_DSP_ASYNCDSPQ

#include <stdint.h>
#include <AEEStdErr.h>
#include <AEEStdDef.h>

#include <image_dspq_common.h>

/* Function signature, User is expected to pack his data in
 * @param num_bufs - Nubmer of buffers to be passed
 * @param bufs - buffer pointer array
 * @param message_type - message of imgq_dsp_msg_response type
 * @param result_size - size of result to be send back through response queue to CPU in bytes
 * @param res - result to be sent back. Note that first 4 bytes are of imgq_dsp_msg_response type response*/
typedef int (*imgq_dsp_function)(int num_bufs, intptr_t *bufs, int *result_size, void **res);

#endif //IMAGE_DSP_ASYNCDSPQ
