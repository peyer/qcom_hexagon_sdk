/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Example asynchronous FastCV-based image processing operation queue - DSP-side implementation */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <qurt.h>
#include <fastcv.h>
#include <HAP_mem.h>
#ifndef _DEBUG
#define _DEBUG
#endif
#include <HAP_farf.h>
#include "fcvqueue.h"
#include "fcvqueue_common.h"
#include "asyncdspq.h"


#define MAX_BUFFERS 32
#define MAX_MSG_LEN 64
#define FULL_CACHE_OP_THRESHOLD 524288
/* Note: Benchmark threshold for best performance */


/* Internal data structures */

typedef enum {
    BUFFER_OWNER_APPS = 1,
    BUFFER_OWNER_DSP
} fcvqueue_buffer_owner;

typedef struct {
    void *buffer;
    int fd;
    uint32_t width, stride, height, size;
    fcvqueue_buffer_owner owner;
} fcvqueue_buffer_internal_t;

typedef struct {
    asyncdspq_t req_queue;
    asyncdspq_t resp_queue;
    fcvqueue_buffer_internal_t buffers[MAX_BUFFERS];
} fcvqueue_internal_t;


/* Helper macro - pass error */
#define PASSERR(x) { if ( (err = x) != 0 ) { goto fail; } }

/* Helper macro - pass error if expression is false */
#define FAILFALSE(x, e) { if ( !(x) ) { err = e; goto fail; } }


static void fcvqueue_error_callback(asyncdspq_t queue, void *context, AEEResult error);
static void fcvqueue_message_callback(asyncdspq_t queue, void *context);


AEEResult fcvqueue_dsp_init(uint32 req_queue_handle, uint32 resp_queue_handle, uint32 *queue_handle) {

    int err = 0;
    fcvqueue_internal_t *q = NULL;

    HAP_setFARFRuntimeLoggingParams(0x1f, NULL, 0);
    
    *queue_handle = 0;

    /* Allocate and initialize DSP-side queue structure */
    FAILFALSE(((q = malloc(sizeof(fcvqueue_internal_t))) != NULL), AEE_ENOMEMORY);
    memset(q, 0, sizeof(fcvqueue_internal_t));

    /* Attach to message queues. We'll use a message callback to receive messages and synchronous
       writes to write responses. */
    PASSERR(asyncdspq_attach(&q->req_queue, req_queue_handle,
                            fcvqueue_error_callback, fcvqueue_message_callback, NULL, q));
    PASSERR(asyncdspq_attach(&q->resp_queue, resp_queue_handle,
                            fcvqueue_error_callback, NULL, NULL, q));
    
    *queue_handle = (uint32) q;
    return AEE_SUCCESS;

fail:
    free(q);
    return err;
}


AEEResult fcvqueue_dsp_destroy(uint32 queue_handle) {

    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue_handle;
    int i;

    assert(q != NULL);

    /* Check that no buffers remain mapped - this would cause a memory leak */
    for ( i = 0; i < MAX_BUFFERS; i++ ) {
        if ( q->buffers[i].buffer != NULL ) {
            return AEE_EALLOCATED;
        }
    }

    /* Cancel any outstanding requests - not that there should be any */
    PASSERR(asyncdspq_cancel(q->req_queue));
    PASSERR(asyncdspq_cancel(q->resp_queue));

    /* Detach queues and free the structre */
    PASSERR(asyncdspq_detach(q->req_queue));
    PASSERR(asyncdspq_detach(q->resp_queue));
    free(q);

fail:
    return err;
}


AEEResult fcvqueue_dsp_map(uint32 queue_handle,
                           int bufferfd, uint32 bufferoffset, uint32 bufferlen,
                           uint32 width, uint32 height, uint32 stride) {

    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue_handle;
    int i;

    assert(q != NULL);
    if ( (height * stride) != bufferlen ) {
        return AEE_EBADPARM;
    }
    
    /* Note that each dmahandle object in the IDL interface is translated into a
       fd/offset/len triplet */

    /* Find an empty buffer slot */
    for ( i = 0; i < MAX_BUFFERS; i++ ) {
        if ( q->buffers[i].buffer == NULL ) {
            fcvqueue_buffer_internal_t *b = &q->buffers[i];

            /* Map buffer to the DSP MMU. This ensures FastCV keeps an MMU and SMMU mapping
               in place for the buffer until we unmap it. */
            FAILFALSE(((b->buffer = HAP_mmap(NULL, bufferlen, HAP_PROT_READ|HAP_PROT_WRITE,
                                             0, bufferfd, 0)) != NULL), AEE_ENOMEMORY);
            FARF(RUNTIME_HIGH, "Mapped fd %d to 0x%08x, len %u\n", bufferfd, b->buffer, bufferlen);

            /* Initialize buffer bookkeeping. Buffers belong to the apps CPU by default */
            b->width = width;
            b->height = height;
            b->stride = stride;
            b->size = bufferlen;
            b->fd = bufferfd;
            b->owner = BUFFER_OWNER_APPS;
            break;
        }
    }
    if ( i == MAX_BUFFERS ) {
        return AEE_EOUTOFHANDLES;
    }
    return AEE_SUCCESS;

fail:
    return err;
}




AEEResult fcvqueue_dsp_unmap(uint32 queue_handle,
                             int32 buffer_fd) {
    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue_handle;
    int i;

    assert(q != NULL);
    
    /* Find the buffer from our bookkeeping list */
    for ( i = 0; i < MAX_BUFFERS; i++ ) {
        if ( q->buffers[i].fd == buffer_fd ) {
            fcvqueue_buffer_internal_t *b = &q->buffers[i];

            /* Buffers can't be unmapped if they're used by the DSP */
            if ( b->owner != BUFFER_OWNER_APPS ) {
                return AEE_EBADSTATE;
            }

            /* Unmap the buffer from the DSP MMU. This will cause FastCV to release
               MMU and SMMU mappings. */
            PASSERR(HAP_munmap(b->buffer, b->size));
            FARF(RUNTIME_HIGH, "Unmapped fd %d from 0x%08x, len %u\n", b->fd, b->buffer, b->size);

            /* The buffer slot can now be reused */
            memset(b, 0, sizeof(fcvqueue_buffer_internal_t));
            break;
        }
    }
    if ( i == MAX_BUFFERS ) {
        return AEE_EBADITEM;
    }
    return AEE_SUCCESS;

fail:
    return err;
}


void fcvqueue_error_callback(asyncdspq_t queue, void *context, AEEResult error) {

    /* asyncdspq error callback. Errors shouldn't happen on the DSP side, and there
       isn't much we can do here */
    FARF(RUNTIME_ERROR, "fcqueue error callback: %d", error);
    assert(0);
}


static fcvqueue_buffer_internal_t *find_buffer(fcvqueue_internal_t *q, int fd) {

    int i;
    
    /* Find the buffer from our bookkeeping list */
    for ( i = 0; i < MAX_BUFFERS; i++ ) {
        if ( q->buffers[i].fd == fd ) {
            return &q->buffers[i];
        }
    }
    return NULL;
}


static AEEResult do_buf_in(fcvqueue_internal_t *q, int fd) {

    fcvqueue_buffer_internal_t *b;
    int err = 0;
    
    /* Transfer buffer to DSP ownership, with cache maintenance. The buffer must currently
       be with the application CPU.
       After this the buffer can be used for processing operations. */
    FAILFALSE(((b = find_buffer(q, fd)) != NULL), AEE_EBADPARM);
    FAILFALSE((b->owner == BUFFER_OWNER_APPS), AEE_EBADSTATE);

    /* Invalidate caches for the buffer. If the buffer is larger than a threshold,
       we'll simply invalidate the entire cache rather than walk through the buffer
       by virtual address. */
    if ( b->size < FULL_CACHE_OP_THRESHOLD ) {
        FARF(RUNTIME_HIGH, "Invalidate buffer at 0x%08x, len %u\n", b->buffer, b->size);
        qurt_mem_cache_clean((qurt_addr_t)b->buffer, b->size, QURT_MEM_CACHE_INVALIDATE, QURT_MEM_DCACHE);
    } else {
        FARF(RUNTIME_HIGH, "Invalidate all. Buffer at 0x%08x, len %u\n", b->buffer, b->size);
        qurt_mem_cache_clean((qurt_addr_t)b->buffer, b->size, QURT_MEM_CACHE_FLUSH_INVALIDATE_ALL, QURT_MEM_DCACHE);
    }

    b->owner = BUFFER_OWNER_DSP;
    return AEE_SUCCESS;

fail:
    return err;
}


static AEEResult do_buf_out(fcvqueue_internal_t *q, int fd) {

    fcvqueue_buffer_internal_t *b;
    int err = 0;
    
    /* Transfer buffer to CPU ownership, with cache maintenance. The buffer must currently
       be with the DSP.
       After this the buffer can no longer be used on the DSP. */
    FAILFALSE(((b = find_buffer(q, fd)) != NULL), AEE_EBADPARM);
    FAILFALSE((b->owner == BUFFER_OWNER_DSP), AEE_EBADSTATE);

    /* Flush caches for the buffer. If the buffer is larger than a threshold,
       we'll simply flush the entire cache rather than walk through the buffer
       by virtual address. */
    if ( b->size < FULL_CACHE_OP_THRESHOLD ) {
        FARF(RUNTIME_HIGH, "Flush buffer at 0x%08x, len %u\n", b->buffer, b->size);
        qurt_mem_cache_clean((qurt_addr_t)b->buffer, b->size, QURT_MEM_CACHE_FLUSH, QURT_MEM_DCACHE);
    } else {
        FARF(RUNTIME_HIGH, "Flush all. Buffer at 0x%08x, len %u\n", b->buffer, b->size);
        qurt_mem_cache_clean((qurt_addr_t)b->buffer, b->size, QURT_MEM_CACHE_FLUSH_ALL, QURT_MEM_DCACHE);
    }

    b->owner = BUFFER_OWNER_APPS;
    return AEE_SUCCESS;

fail:
    return err;
}


static AEEResult do_process(fcvqueue_internal_t *q, fcvqueue_op_t op, int input_fd, int output_fd) {

    fcvqueue_buffer_internal_t *in_buf, *out_buf;
    int err = 0;

    /* Perform a processing operation on buffers. Both buffers must be in DSP ownership. */
    FAILFALSE(((in_buf = find_buffer(q, input_fd)) != NULL), AEE_EBADPARM);
    FAILFALSE((in_buf->owner == BUFFER_OWNER_DSP), AEE_EBADSTATE);
    FAILFALSE(((out_buf = find_buffer(q, output_fd)) != NULL), AEE_EBADPARM);
    FAILFALSE((out_buf->owner == BUFFER_OWNER_DSP), AEE_EBADSTATE);

    /* Input and output buffer sizes must currently be identical for all buffers */
    FAILFALSE(((in_buf->size == out_buf->size) &&
               (in_buf->width == out_buf->width) &&
               (in_buf->stride == out_buf->stride) &&
               (in_buf->stride == in_buf->width) &&
               (in_buf->height == out_buf->height)), AEE_EINVALIDFORMAT);
               
    switch ( op ) {
        case FCVQUEUE_OP_COPY: {
            FARF(RUNTIME_HIGH, "Op: copy 0x%08x to 0x%08x, len %u\n", in_buf->buffer, out_buf->buffer, out_buf->size);
            memcpy(out_buf->buffer, in_buf->buffer, out_buf->size);
            break;
        }

        case FCVQUEUE_OP_DILATE3X3: {
            FARF(RUNTIME_HIGH, "Op: dilate3x3 0x%08x to 0x%08x, len %u\n", in_buf->buffer, out_buf->buffer, out_buf->size);
            fcvFilterDilate3x3u8(in_buf->buffer, in_buf->width, in_buf->height, out_buf->buffer);
            break;
        }
            
        case FCVQUEUE_OP_GAUSSIAN3X3: {
            FARF(RUNTIME_HIGH, "Op: gaussian3x3 0x%08x to 0x%08x, len %u\n", in_buf->buffer, out_buf->buffer, out_buf->size);
            fcvFilterGaussian3x3u8(in_buf->buffer, in_buf->width, in_buf->height, out_buf->buffer, 1);
            break;
        }
            
        case FCVQUEUE_OP_MEDIAN3X3: {
            FARF(RUNTIME_HIGH, "Op: median3x3 0x%08x to 0x%08x, len %u\n", in_buf->buffer, out_buf->buffer, out_buf->size);
            fcvFilterMedian3x3u8(in_buf->buffer, in_buf->width, in_buf->height, out_buf->buffer);
            break;
        }
            
        default:
            err = AEE_EUNSUPPORTED;
            goto fail;
    }
              
    return AEE_SUCCESS;

fail:
    return err;
}


void fcvqueue_message_callback(asyncdspq_t queue, void *context) {
    uint32_t msg[MAX_MSG_LEN / 4];
    unsigned msg_len;
    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) context;

    assert(q);
    assert(queue == q->req_queue);
    
    /* asyncdspq calls the message callback when there may be one or more messages available.
       We must drain the whole queue before exiting the callback, otherwise we risk not getting
       a callback for messages that were sent while the notification was on its way. */
    while ( 1 ) {
        err = asyncdspq_read_noblock(q->req_queue, (uint8_t*)msg, sizeof(msg), &msg_len);
        if ( err == AEE_ENOMORE ) {
            /* The queue is empty */
            return;
        }
        if ( err != AEE_SUCCESS ) {
            goto fail;
        }
        FAILFALSE((msg_len > 4), AEE_EBADITEM);
        
        switch ( msg[0] ) {

            case FCVQUEUE_REQ_BUF_IN: {
                FAILFALSE((msg_len == 8), AEE_EBADITEM);
                PASSERR(do_buf_in(q, (int) msg[1]));
                break;
            }
            
            case FCVQUEUE_REQ_BUF_OUT: {
                FAILFALSE((msg_len == 8), AEE_EBADITEM);
                PASSERR(do_buf_out(q, (int) msg[1]));
                break;
            }

            case FCVQUEUE_REQ_PROCESS: {
                FAILFALSE((msg_len == 16), AEE_EBADITEM);
                PASSERR(do_process(q, (fcvqueue_op_t) msg[1], (int) msg[2], (int) msg[3]));
                break;
            }
            
            case FCVQUEUE_REQ_SYNC: {
                /* Sync request. We'll just send a response back - this signals to the client
                   that all messages up to this point in the queue have been processed. */                
                FAILFALSE((msg_len == 12), AEE_EBADITEM);
                FARF(RUNTIME_HIGH, "Sync: 0x%08x%08x", msg[2], msg[1]);
                msg[0] = FCVQUEUE_RESP_SYNC;
                /* msg[1] and msg[2] already contain the context */
                PASSERR(asyncdspq_write(q->resp_queue, (uint8_t*)msg, 12));
                break;
            }
            
            default:
                err = AEE_EUNSUPPORTED;
                goto fail;
        }
    }

    
fail:
    /* Error - send an error response to the client */
    msg[0] = FCVQUEUE_RESP_ERROR;
    msg[1] = err;
    asyncdspq_write(q->resp_queue, (uint8_t*)msg, 8);
}
