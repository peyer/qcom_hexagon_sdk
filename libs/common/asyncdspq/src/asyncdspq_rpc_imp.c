/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <stdlib.h>
#include <qurt.h>
#include <HAP_mem.h>
#include "asyncdspq_rpc.h"
#include "asyncdspq_internal.h"


AEEResult asyncdspq_rpc_create(int queuefd, uint32 queueoffset, uint32 queuelen, uint32 *queue_handle) {

    asyncdspq_local_t *q;

    *queue_handle = 0;
    if ( queueoffset != 0 ) {
        return AEE_EBADPARM;
    }

    // Allocate local queue structure; this will also be the attach handle
    if ( (q = malloc(sizeof(asyncdspq_local_t))) == NULL ) {
        return AEE_ENOMEMORY;
    }
    memset(q, 0, sizeof(asyncdspq_local_t));
    q->fd = queuefd;
    qurt_signal_init(&q->message_signal);
    qurt_signal_init(&q->space_signal);
    qurt_signal_init(&q->callback_signal);

    // Map the shared queue to DSP memory
    q->header = (asyncdspq_header_t*) HAP_mmap(NULL, queuelen, HAP_PROT_READ|HAP_PROT_WRITE, 0, queuefd, 0);
    if ( (q->header == NULL) || (q->header == (void*)-1) ) {
        free(q);
        return AEE_EBADPARM;
    }
    q->alloc_size = queuelen;

    *queue_handle = (uint32_t) q;
    return AEE_SUCCESS;
}



AEEResult asyncdspq_rpc_destroy(uint32 queue_handle) {

    int err = AEE_SUCCESS;
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;

    if ( q->attach_handle != 0 ) {
        // Still attached
        return AEE_EBADSTATE;
    }
    
    qurt_signal_destroy(&q->message_signal);
    qurt_signal_destroy(&q->space_signal);
    qurt_signal_destroy(&q->callback_signal);

    // Unmap and free queue
    if ( (err = HAP_munmap(q->header, q->alloc_size)) != 0 ) {
        goto fail;
    }
    free(q);
    
fail:
    return err;
}


AEEResult asyncdspq_rpc_wait_message(uint32 queue_handle, int32 *messages) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;
    uint32_t s;

    s = qurt_signal_wait(&q->message_signal, 0x03, QURT_SIGNAL_ATTR_WAIT_ANY);
    if ( s & 1 ) {
        // Have a message
        *messages = 1;
        qurt_signal_clear(&q->message_signal, 1);
    } else if ( s & 2 ) {
        // Exit
        *messages = -1;
        qurt_signal_clear(&q->message_signal, 2);
    } else {
        // Shouldn't happen
        return AEE_EFAILED;
    }

    return AEE_SUCCESS;
}


AEEResult asyncdspq_rpc_cancel_wait_message(uint32 queue_handle) {

    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;
    qurt_signal_set(&q->message_signal, 2);
    return AEE_SUCCESS;
}



AEEResult asyncdspq_rpc_signal_message(uint32 queue_handle) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;

    qurt_signal_set(&q->message_signal, 1);
    qurt_signal_set(&q->callback_signal, 1);

    return AEE_SUCCESS;
}


AEEResult asyncdspq_rpc_wait_space(uint32 queue_handle, int32 *space) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;
    uint32_t s;

    s = qurt_signal_wait(&q->space_signal, 0x03, QURT_SIGNAL_ATTR_WAIT_ANY);
    if ( s & 1 ) {
        // Have space
        *space = 1;
        qurt_signal_clear(&q->space_signal, 1);
    } else if ( s & 2 ) {
        // Exit
        *space = -1;
        qurt_signal_clear(&q->space_signal, 2);
    } else {
        // Shouldn't happen
        return AEE_EFAILED;
    }

    return AEE_SUCCESS;
}



AEEResult asyncdspq_rpc_cancel_wait_space(uint32 queue_handle) {

    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;
    qurt_signal_set(&q->space_signal, 2);
    return AEE_SUCCESS;
}


AEEResult asyncdspq_rpc_signal_space(uint32 queue_handle) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue_handle;

    qurt_signal_set(&q->space_signal, 1);
    qurt_signal_set(&q->callback_signal, 1);

    return AEE_SUCCESS;
}
