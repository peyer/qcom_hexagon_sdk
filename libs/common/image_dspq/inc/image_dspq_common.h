/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef IMAGE_ASYNCDSPQ_COMMON
#define IMAGE_ASYNCDSPQ_COMMON

#include <stdint.h>
#include <AEEStdErr.h>
#include <AEEStdDef.h>

/* Buffer type.
 * Following operation would be performed on DSP side on buffers
 * IMGQ_BUF_IN - cache invalidation
 * IMGQ_BUF_INTERMIDIATE - None
 * IMGQ_BUF_OUT - Cache Flush*/
typedef enum {
    IMGQ_BUF_IN = 0,
    IMGQ_INTERMIDIATE,
    IMGQ_BUF_OUT
}imgq_buf_type;

/* buffer owner.
 * Ownership is passed to DSP while performing operation and it is
 * given back to CPU when operation is done.
 * */
typedef enum {
    IMGQ_CPU = 0,
    IMGQ_DSP
}imgq_buf_owner;

/* Type of message CPU can enqueue in request queue.
 * This is used internally by library*/
typedef enum {
    IMGQ_CPU_MSG_OPERATION = 0,
    IMGQ_CPU_MSG_SYNC
}imgq_cpu_msg_type;

/* Type of message DSP can send to CPU.
 * This is used internally*/
typedef enum {
    IMGQ_DSP_MSG_SYNC = 0,
    IMGQ_DSP_MSG_ERR,
    IMGQ_DSP_MSG_USR
}imgq_dsp_msg_response;

/* Image type*/
typedef enum {
    IMGQ_IMAGE_RGB = 0,
    IMGQ_IMAGE_RGBX,
    IMGQ_IMAGE_NV12,
    IMGQ_IMAGE_NV21,
    IMGQ_IMAGE_UYVY,
    IMGQ_IMAGE_YUYV,
    IMGQ_IMAGE_IYUV,
    IMGQ_IMAGE_YUV4,
    IMGQ_IMAGE_U8,
    IMGQ_IMAGE_U16,
    IMGQ_IMAGE_S16,
    IMGQ_IMAGE_U32,
    IMGQ_IMAGE_S32,
    IMGQ_IMAGE_UNDEFINED,
}imgq_buf_format;

/* Metadata buffer structure which mapped on DSP side. Once mapped DSP can directly read this memory.*/
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t planes;
    uint32_t identifier;
    imgq_buf_format format;
    uint32_t size;
    uint32_t alignment;
    imgq_buf_type type;
    imgq_buf_owner owner;
}imgq_buf_metadata;

/* buffer information data structure to hold metadata, user private data and image buffer on both CPU and DSP side.*/
typedef struct {
    imgq_buf_metadata *metadata; //User must not assign this memory. It will be assigned when calling image_allocate_buffer_info.
    void *data_ptr;
    uint32_t data_size;
    uint8_t dsp_mapped; //For future use. One more field is required for DSP mapped pointer
    uint8_t imported;
    int imported_fd; //FD for imported buffer to share on DSP side to map.
    void *buf_priv_data;
    uint32_t buf_priv_data_size;
}imgq_buf_info;

#endif
