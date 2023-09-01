
/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef IMAGE_ASYNCDSPQ
#define IMAGE_ASYNCDSPQ

#include <image_dspq_common.h>

/* Queue Handle*/
typedef void* image_dspq_t;

/*
 * Message callback function pointer which user can register for response message
 * @param length  - Length of message in bytes
 * @param context - Data pointer*/
typedef void (*image_dspq_message_callback)(uint32_t length, void *context);

/*
 * Error callback function pointer which user can register for any error in queue
 * This callback would be called by library's error callback
 * @param err - error number*/
typedef void (*image_dspq_error_callback)(int err);

/*
 * Sync callback to enqueue sync operation in queue.
 * @param void pointer which can contain user define data structure*/
typedef void (*image_dspq_sync_callback)(void*);

/*
 * Data structure used internally to enqueue sync before destroying the queue.*/
typedef struct {
    image_dspq_sync_callback sync_callback;
    void *context;
}imgq_sync_t;

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * Creates 2 instances of asyncdspq type queues. One is request and the other is response queue (from CPU's
     * point of view).Currently CPU is master and only he can command DSP to perform certain work and respond
     * back if necessary. From User's perspective, these queue creation, maintenance and deletion are hidden and
     * he does not need to worry about that(library internally takes care of it).
     * @param queue Output- image_dspq_t type of handle
     * @param error_callback_ - image_dspq_error_callback type of function pointer
     * @param message_callback_ - image_dspq_message_callback type of function pointer*/
    AEEResult image_dspq_init(image_dspq_t *queue, image_dspq_error_callback error_callback_, image_dspq_message_callback message_callback_);

    /*
     * Destroys the image_queue. Before destroying enqueues sync operation to make sure that all enqueued
     * operations are executed.Also, destroys any data structures and allocated memory related to buffer.
     * @param queue - queue handle*/
    AEEResult image_dspq_destroy(image_dspq_t queue);

    /*
     * For any buffer to be used with this library, it is to be wrapped in imgq_buf_info type of structure. For any
     * user defined data, User is expected to wrap it in private data structure. Since this private data is to be
     * shared with DSP, the rpcmem allocated memory(for private data) is allocated by library.
     * User is expected to pass size of his private data structure. User is expected to fill data structures with
     * buffer specific information after calling this api.
     * All the buffers are mapped on DSP side to access them on DSP.
     * @param queue              - queue handle
     * @param buf_priv_data_size - size of user defined data structures*/
    imgq_buf_info* image_dspq_allocate_buf_info(image_dspq_t queue, uint32_t buf_priv_data_size);

    /*
     * Allocates rpcmem type of image buffer of size imgq_buf_info->data_size and maps it on DSP side.
     * @param queue - queue handle
     * @param buf   - imgq_buf_info type of pointer*/
    AEEResult image_dspq_allocate_buffer(image_dspq_t queue, imgq_buf_info *buf, uint32_t size);

    /*
     * Imports any ION allocated image buffer to be passed and mapped on DSP side
     * @param queue             - queue handle
     * @param buf               - imgq_buf_info type of pointer
     * @param imported_buf_ptr  - pointer to buffer to be imported
     * @param imported_buf_size - imported buffer size
     * @param imported_buf_fd   - imported buffer fd*/
    AEEResult image_dspq_import_buffer(image_dspq_t queue, imgq_buf_info *buf, void *imported_buf_ptr, uint32_t imported_buf_size, int imported_buf_fd);

    /*
     * Destroys all buffer data structures and DSP side mappings. If image buffer is not imported then destroy and unmap
     * it on DSP side also
     * @param queue - queue handle
     * @param buf   - imgq_buf_info type of pointer*/
    AEEResult image_dspq_deregister_buffer(image_dspq_t queue, imgq_buf_info *buf);
    /*
     * Enqueues operation to be performed on DSP side.
     * @param queue     - queue handle
     * @param library   - library name(e.g. "libexample.so")
     * @param operation - function name to be executed(e.g. "imgq_wrapper")
     * @param num_bufs  - number of imgq_buf_info type of buffers to be passed
     * @param bufs      - pointer to array of imgq_buf_info type of pointers*/
    AEEResult image_dspq_enqueue_operation(image_dspq_t queue, const char *library, const char *operation, uint32_t num_bufs, intptr_t *bufs);
    /*
     * Enqueues sync operation. This api can be used to stop further execution on CPU till all the prior requests are done.
     * Once DSP is done with sync request it responds back with IMGQ_DSP_MSG_SYNC type of message which in turn invoke
     * callback registered by user while calling this api.
     * @param queue - queue handle
     * @param callback - image_dspq_sync_callback type of function pointer
     * @param context - data sent by DSP.*/
    AEEResult image_dspq_sync(image_dspq_t queue, image_dspq_sync_callback callback, void *context);

#ifdef __cplusplus
}
#endif

#endif //IMAGE_ASYNCDSPQ
