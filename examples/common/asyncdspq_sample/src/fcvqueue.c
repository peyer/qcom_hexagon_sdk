/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Example asynchronous FastCV-based image processing operation queue - CPU-side implementation */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <rpcmem.h>
#include <semaphore.h>
#include "fcvqueue.h"
#include "fcvqueue_dsp.h"
#include "fcvqueue_common.h"
#include "asyncdspq.h"


//#define DEBUGPRINT(x, ...) printf(x, ##__VA_ARGS__)
#define DEBUGPRINT(x, ...) 


/* Configuration */
#define REQ_QUEUE_LENGTH 4096
#define RESP_QUEUE_LENGTH 4096
#define MAX_MSG_LEN 64


/* Internal data structures */

typedef enum {
    BUFFER_OWNER_APPS = 1,
    BUFFER_OWNER_DSP
} fcvqueue_buffer_owner;

typedef struct {
    asyncdspq_t req_queue;
    uint32_t req_attach_handle;
    asyncdspq_t resp_queue;
    uint32_t resp_attach_handle;
    uint32_t dsp_handle;
    fcvqueue_error_callback_t error_callback;
    void *callback_context;
} fcvqueue_internal_t;

typedef struct {
    void *buffer;
    int fd;
    uint32_t width, stride, height, size;
    fcvqueue_buffer_owner owner;
} fcvqueue_buffer_internal_t;

typedef struct {
    fcvqueue_sync_callback_t callback;
    void *context;
} fcvqueue_sync_context_t;


/* Helper macro - pass error */
#define PASSERR(x) { if ( (err = x) != 0 ) { goto fail; } }

/* Helper macro - pass error if expression is false */
#define FAILFALSE(x, e) { if ( !(x) ) { err = e; goto fail; } }


static void fcvqueue_error_callback(asyncdspq_t queue, void *context, AEEResult error);
static void fcvqueue_message_callback(asyncdspq_t queue, void *context);


AEEResult fcvqueue_create(fcvqueue_t *queue,
                          fcvqueue_error_callback_t error_callback, void *callback_context) {

    int err = 0;
    fcvqueue_internal_t *q = NULL;

    *queue = NULL;
    FAILFALSE(((q = malloc(sizeof(fcvqueue_internal_t))) != NULL), AEE_ENOMEMORY);
    memset(q, 0, sizeof(fcvqueue_internal_t));
    q->error_callback = error_callback;
    q->callback_context = callback_context;

    /* Create request and response queues. We use synchronous writes to the DSP and a message
       callback for responses. This is a common pattern for asyncdspq clients. */
    PASSERR(asyncdspq_create(&q->req_queue, &q->req_attach_handle,
                            ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP,
                            REQ_QUEUE_LENGTH,
                            fcvqueue_error_callback, NULL, NULL, q, 0));
    PASSERR(asyncdspq_create(&q->resp_queue, &q->resp_attach_handle,
                            ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                            RESP_QUEUE_LENGTH,
                            fcvqueue_error_callback, fcvqueue_message_callback, NULL, q, 0));

    /* Attach queues on the DSP side and initialize */
    PASSERR(fcvqueue_dsp_init(q->req_attach_handle, q->resp_attach_handle, &q->dsp_handle));

    *queue = (fcvqueue_t) q;
    return AEE_SUCCESS;

fail:
    if ( q->req_queue ) {
        asyncdspq_destroy(q->req_queue);
    }
    if ( q->resp_queue ) {
        asyncdspq_destroy(q->resp_queue);
    }
    free(q);
    return err;
}


AEEResult fcvqueue_destroy(fcvqueue_t queue) {

    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;

    assert(q != NULL);
    assert(q->req_queue);
    assert(q->resp_queue);

    /* Synchronize to ensure the queues are empty */
    PASSERR(fcvqueue_sync(queue));

    /* Shut down DSP-side processing and detach queues */
    PASSERR(fcvqueue_dsp_destroy(q->dsp_handle));

    /* Destroy queues */
    PASSERR(asyncdspq_destroy(q->req_queue));
    q->req_queue = NULL;
    PASSERR(asyncdspq_destroy(q->resp_queue));
    q->resp_queue = NULL;

    return AEE_SUCCESS;

fail:
    return err;
}


AEEResult fcvqueue_alloc_buffer(fcvqueue_t queue,
                                uint32_t width, uint32_t height, uint32_t stride,
                                fcvqueue_buffer_t *buffer) {

    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    fcvqueue_buffer_internal_t *b = NULL;
    uint32_t size;

    *buffer = NULL;

    if ( ((stride & 127) != 0) || (stride < width) || (width < 128) || (height < 128) ) {
        return AEE_EBADPARM;
    }                                                                      
    
    /* Allocate and populate buffer structure */
    FAILFALSE(((b = malloc(sizeof(fcvqueue_buffer_internal_t))) != NULL), AEE_ENOMEMORY);
    memset(b, 0, sizeof(fcvqueue_buffer_internal_t));
    b->width = width;
    b->height = height;
    b->stride = stride;
    size = stride * height;
    b->size = size;
    b->owner = BUFFER_OWNER_APPS;

    /* Allocate shareable ION buffer and get its FD */
    /* Note that the client must have called rpcmem_init() */
    FAILFALSE(((b->buffer = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL),
              AEE_ENOMEMORY);
    FAILFALSE(((b->fd = rpcmem_to_fd(b->buffer)) > 0), AEE_ENOMEMORY);
    DEBUGPRINT("fcvqueue_alloc_buffer: Got buffer at %p, fd %d, struct %p\n", b->buffer, b->fd, b);

    /* Map the buffer to the SMMU and DSP MMU. The DSP code keeps a mapping open until
       we call fcvqueue_dsp_unmap() and maintains an internal list of mapped buffers.
       Note that the buffer must be made available to the DSP with
       fcvqueue_enqueue_buffer_in() before using it with any of the processing functions
       to ensure correct cache operations are done. */
    PASSERR(fcvqueue_dsp_map(q->dsp_handle, b->fd, 0, size,
                             width, height, stride));

    *buffer = (fcvqueue_buffer_t) b;
    return AEE_SUCCESS;

fail:
    if ( b && (b->buffer) ) {
        rpcmem_free(b->buffer);
        b->buffer = NULL;
    }
    free(b);
    return err;
}


AEEResult fcvqueue_free_buffer(fcvqueue_t queue, fcvqueue_buffer_t buffer) {

    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    fcvqueue_buffer_internal_t *b = (fcvqueue_buffer_internal_t*) buffer;

    /* Take the buffer back to apps CPU ownership before unmapping and freeing it */
    assert(b);
    if ( b->owner != BUFFER_OWNER_APPS ) {
        PASSERR(fcvqueue_enqueue_buffer_out(queue, buffer));
    }

    /* We'll synchronize the queue first to ensure the buffer is no longer in use.
       (fcvqueue_enqueue_buffer_out() is asynchronous)
       Note that the current fcvqueue implementation is not thread safe. */
    PASSERR(fcvqueue_sync(queue));
    assert(b->owner == BUFFER_OWNER_APPS);

    /* Unmap the buffer from the SMMU and DSP MMU */
    PASSERR(fcvqueue_dsp_unmap(q->dsp_handle, b->fd));

    /* Free buffer memory */
    DEBUGPRINT("fcvqueue_free_buffer: Freeing at %p, fd %d, struct %p\n", b->buffer, b->fd, b);
    rpcmem_free(b->buffer);
    free(b);
    
    return AEE_SUCCESS;

fail:
    return err;
}


AEEResult fcvqueue_buffer_ptr(fcvqueue_t queue, fcvqueue_buffer_t buffer, void **ptr) {

    fcvqueue_buffer_internal_t *b = (fcvqueue_buffer_internal_t*) buffer;
    assert(b);
    *ptr = b->buffer;
    return AEE_SUCCESS;
}



void fcvqueue_error_callback(asyncdspq_t queue, void *context, AEEResult error) {

    fcvqueue_internal_t *q = (fcvqueue_internal_t*) context;
    q->error_callback((fcvqueue_t)q, q->callback_context, error);
}


void fcvqueue_message_callback(asyncdspq_t queue, void *context) {

    uint32_t msg[MAX_MSG_LEN / 4];
    unsigned msg_len;
    int err = 0;
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) context;

    assert(q);
    assert(queue == q->resp_queue);
        
    /* asyncdspq calls the message callback when there may be one or more messages available.
       We must drain the whole queue before exiting the callback, otherwise we risk not getting
       a callback for messages that were sent while the notification was on its way. */
    while ( 1 ) {
        err = asyncdspq_read_noblock(q->resp_queue, (uint8_t*)msg, sizeof(msg), &msg_len);
        if ( err == AEE_ENOMORE ) {
            /* The queue is empty */
            return;
        }
        if ( err != AEE_SUCCESS ) {
            goto fail;
        }
        FAILFALSE((msg_len > 4), AEE_EBADITEM);
        
        switch ( msg[0] ) {
            
            case FCVQUEUE_RESP_SYNC: {
                /* Sync response. Call the client's callback and free the context object. */
                fcvqueue_sync_context_t *sync;
                FAILFALSE((msg_len == 12), AEE_EBADITEM);
                uint64_t msg64 = (uintptr_t)msg[2];
                sync = (fcvqueue_sync_context_t*) (((uintptr_t)msg[1]) | (uintptr_t)(msg64<<32));
                sync->callback((fcvqueue_t)q, sync->context);
                free(sync);
                break;
            }
            
            default:
                err = AEE_EUNSUPPORTED;
                goto fail;
        }
    }

fail:
    q->error_callback((fcvqueue_t)q, q->callback_context, err);
}


AEEResult fcvqueue_enqueue_sync(fcvqueue_t queue, fcvqueue_sync_callback_t callback, void *context) {

    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    fcvqueue_sync_context_t *sync = NULL;
    int err = 0;
    uint32_t req[3];
    assert(q);

    /* Allocate a sync context structure. Making this more efficient by using a pool
       is left as an exercise... */
    FAILFALSE(((sync = malloc(sizeof(fcvqueue_sync_context_t))) != NULL), AEE_ENOMEMORY);
    memset(sync, 0, sizeof(fcvqueue_sync_context_t));
    sync->callback = callback;
    sync->context = context;

    /* Build and send sync request */
    req[0] = FCVQUEUE_REQ_SYNC;
    req[1] = (uint32_t) (((uintptr_t)sync));    
    uint64_t sync64 = (uintptr_t)sync;
    req[2] = (uint32_t) (sync64 >> 32);
    PASSERR(asyncdspq_write(q->req_queue, (uint8_t*)req, sizeof(req)));

    return AEE_SUCCESS;
    
fail:
    free(sync);
    return err;
}


static void sync_callback_signal_sem(fcvqueue_t queue, void *context) {
    sem_post((sem_t*)context);
}

AEEResult fcvqueue_sync(fcvqueue_t queue) {

    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    int err = 0;
    sem_t sem;
    assert(q);

    /* Synchronous sync. Enqueue a sync operation with a callback that signals a semaphore and
       wait on the semaphore to synchronize. */
    PASSERR(sem_init(&sem, 0, 0));
    PASSERR(fcvqueue_enqueue_sync(queue, sync_callback_signal_sem, (void*)&sem));
    PASSERR(sem_wait(&sem));
    PASSERR(sem_destroy(&sem));
    
    return AEE_SUCCESS;

fail:
    return err;
}


AEEResult fcvqueue_enqueue_buffer_in(fcvqueue_t queue, fcvqueue_buffer_t buffer) {
    
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    fcvqueue_buffer_internal_t *b = (fcvqueue_buffer_internal_t*) buffer;
    int err = 0;
    uint32_t msg[4];
    assert(q);
    assert(b);

    DEBUGPRINT("fcvqueue_enqueue_buffer_in: fd %d, buffer %p\n", b->fd, b->buffer);
    /* Transfer a buffer to DSP ownership, performing the necessary cache ops on the DSP size.
       The buffer must currently be in apps CPU ownership. */
    FAILFALSE((b->owner == BUFFER_OWNER_APPS), AEE_EBADSTATE);
    msg[0] = FCVQUEUE_REQ_BUF_IN;
    msg[1] = b->fd;
    PASSERR(asyncdspq_write(q->req_queue, (uint8_t*)msg, 8));
    b->owner = BUFFER_OWNER_DSP;

    return AEE_SUCCESS;

fail:
    return err;
}


AEEResult fcvqueue_enqueue_buffer_out(fcvqueue_t queue, fcvqueue_buffer_t buffer) {
    
    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    fcvqueue_buffer_internal_t *b = (fcvqueue_buffer_internal_t*) buffer;
    int err = 0;
    uint32_t msg[4];
    assert(q);
    assert(b);

    /* Transfer a buffer to Apps CPU ownership, performing the necessary cache ops on the DSP sizd.
       The buffer must currently be in DSP ownership. */
    DEBUGPRINT("fcvqueue_enqueue_buffer_out: fd %d, buffer %p\n", b->fd, b->buffer);
    FAILFALSE((b->owner == BUFFER_OWNER_DSP), AEE_EBADSTATE);
    msg[0] = FCVQUEUE_REQ_BUF_OUT;
    msg[1] = b->fd;
    PASSERR(asyncdspq_write(q->req_queue, (uint8_t*)msg, 8));
    b->owner = BUFFER_OWNER_APPS;

    return AEE_SUCCESS;

fail:
    return err;
}


AEEResult fcvqueue_enqueue_op(fcvqueue_t queue, fcvqueue_op_t op,
                              fcvqueue_buffer_t input, fcvqueue_buffer_t output) {

    fcvqueue_internal_t *q = (fcvqueue_internal_t*) queue;
    fcvqueue_buffer_internal_t *in_buf = (fcvqueue_buffer_internal_t*) input;
    fcvqueue_buffer_internal_t *out_buf = (fcvqueue_buffer_internal_t*) output;
    int err = 0;
    uint32_t msg[4];
    
    assert(q);
    assert(in_buf);
    assert(out_buf);

    /* Perform a processing operation on buffers. Both buffers must currently be in DSP ownership. */
    FAILFALSE(((in_buf->owner == BUFFER_OWNER_DSP) &&
               (out_buf->owner == BUFFER_OWNER_DSP)), AEE_EBADSTATE);
    DEBUGPRINT("fcvqueue_free_buffer: Op %d, %p(%d) -> %p(%d)\n", (int)op,
               in_buf->buffer, in_buf->fd, out_buf->buffer, out_buf->fd);
    msg[0] = FCVQUEUE_REQ_PROCESS;
    msg[1] = op;
    msg[2] = in_buf->fd;
    msg[3] = out_buf->fd;
    PASSERR(asyncdspq_write(q->req_queue, (uint8_t*)msg, 16));

    return AEE_SUCCESS;

fail:
    return err;
}
