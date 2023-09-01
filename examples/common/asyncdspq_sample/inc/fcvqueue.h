/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Example asynchronous FastCV-based image processing operation queue.
   Main API, available on apps CPU only. */

#ifndef FCVQUEUE_H
#define FCVQUEUE_H

#include <stdint.h>
#include <AEEStdErr.h>
#include <AEEStdDef.h>

typedef void* fcvqueue_t;
typedef void* fcvqueue_buffer_t;

typedef void (*fcvqueue_sync_callback_t)(fcvqueue_t, void*);

typedef enum {
    FCVQUEUE_OP_COPY = 1,
    FCVQUEUE_OP_DILATE3X3,
    FCVQUEUE_OP_GAUSSIAN3X3,
    FCVQUEUE_OP_MEDIAN3X3
} fcvqueue_op_t;

typedef void (*fcvqueue_error_callback_t)(fcvqueue_t, void*, AEEResult);


AEEResult fcvqueue_create(fcvqueue_t *queue,
                          fcvqueue_error_callback_t error_callback, void *callback_context);
AEEResult fcvqueue_destroy(fcvqueue_t queue);

AEEResult fcvqueue_alloc_buffer(fcvqueue_t queue, uint32_t width, uint32_t height, uint32_t stride,
                                fcvqueue_buffer_t *buffer);
AEEResult fcvqueue_free_buffer(fcvqueue_t queue, fcvqueue_buffer_t buffer);

AEEResult fcvqueue_buffer_ptr(fcvqueue_t queue, fcvqueue_buffer_t buffer, void **ptr);

AEEResult fcvqueue_enqueue_buffer_in(fcvqueue_t queue, fcvqueue_buffer_t buffer);
AEEResult fcvqueue_enqueue_buffer_out(fcvqueue_t queue, fcvqueue_buffer_t buffer);

AEEResult fcvqueue_enqueue_op(fcvqueue_t queue, fcvqueue_op_t op,
                              fcvqueue_buffer_t input, fcvqueue_buffer_t output);

AEEResult fcvqueue_enqueue_sync(fcvqueue_t queue, fcvqueue_sync_callback_t callback, void *context);

AEEResult fcvqueue_sync(fcvqueue_t queue);


#endif //FCVQUEUE_H
