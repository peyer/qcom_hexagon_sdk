/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Low-level shared memory userspace queue */

#include <asyncdspq.h>
#include <asyncdspq_internal.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#  include <stdio.h>

#if defined(ASYNCDSPQ_APP_CPU)
#  include <rpcmem.h>
#  include <stdio.h>
#  include <unistd.h>
#  include "asyncdspq_rpc.h"
#endif

#define QUEUE_MIN_LENGTH 256
#define QUEUE_ALIGN 256
#define MAX_RESP_LENGTH 256

#define ALIGN_SIZE(x) ((x + (QUEUE_ALIGN-1)) & (~(QUEUE_ALIGN-1)))
#define PASSERR(x) { if ( (err = x) != 0 ) { goto fail; } }
#define CALLBACK_THREAD_STACK_SIZE 16384



#ifdef ASYNCDSPQ_SUPPORT_CREATE
static void *asyncdspq_reader_message_thread(void *arg);
static void *asyncdspq_writer_message_thread(void *arg);
static void *asyncdspq_reader_space_thread(void *arg);
static void *asyncdspq_writer_space_thread(void *arg);
static void *asyncdspq_reader_callback_thread(void *arg);
static void *asyncdspq_writer_callback_thread(void *arg);

AEEResult asyncdspq_create(asyncdspq_t *queue, asyncdspq_attach_handle_t *attach_handle,
                          asyncdspq_endpoint_t writer, asyncdspq_endpoint_t reader,
                          unsigned queue_length, asyncdspq_error_callback_t error_callback,
                          asyncdspq_callback_t message_callback, asyncdspq_callback_t space_callback,
                          void *callback_context, uint32_t flags) {

    unsigned alloc_size;
    asyncdspq_local_t *q = NULL;
    uint32 o;
    int err = AEE_SUCCESS;

    *queue = NULL;
    *attach_handle = 0;
    if ( (queue_length < QUEUE_MIN_LENGTH) || ((queue_length & 3) != 0) ) {
        return AEE_EBADPARM;
    }
    
    if ( (q = malloc(sizeof(asyncdspq_local_t))) == NULL ) {
        return AEE_ENOMEMORY;
    }

    if ( (writer == reader) ||
         ((writer != ASYNCDSPQ_ENDPOINT_APP_CPU) && (writer != ASYNCDSPQ_ENDPOINT_CDSP)) ||
         ((reader != ASYNCDSPQ_ENDPOINT_APP_CPU) && (reader != ASYNCDSPQ_ENDPOINT_CDSP)) ) {
        return AEE_EBADPARM;
    }

    // Allocate ION buffer for the shared queue and its headers
    alloc_size = ALIGN_SIZE(sizeof(asyncdspq_header_t)) +
        ALIGN_SIZE(sizeof(uint32_t)) +
        ALIGN_SIZE(sizeof(uint32_t)) +
        ALIGN_SIZE(queue_length);
    assert(alloc_size >= 4 * QUEUE_ALIGN);
    q->alloc_size = alloc_size;
    if ( (q->header = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, alloc_size)) == NULL ) {
        err = AEE_ENOMEMORY;
        goto fail;
    }
    assert((((uintptr_t)q->header) & 4095) == 0);
    if ( (q->fd = rpcmem_to_fd(q->header)) < 1 ) {
        err = AEE_ENOMEMORY;
        goto fail;
    }

    // Set up queue/status pointers/offsets and otherwise initialize the header
    memset(q->header, 0, alloc_size);
    q->header->version = 1;
    q->header->error = 0;
    q->header->writer = writer;
    q->header->reader = reader;
    o = ALIGN_SIZE(sizeof(asyncdspq_header_t));
    q->header->queue_offset = o;
    q->header->queue_length = queue_length;
    o += ALIGN_SIZE(queue_length);
    q->header->read_offset_offset = o;
    o += ALIGN_SIZE(sizeof(uint32_t));
    q->header->write_offset_offset = o;
    o += ALIGN_SIZE(sizeof(uint32_t));
    assert(o == alloc_size);
    q->header->flags = flags;

    q->error_callback = error_callback;
    q->message_callback = message_callback;
    q->space_callback = space_callback;
    q->callback_context = callback_context;

    // Create a mutex to protect read/write_noblock()
    PASSERR(pthread_mutex_init(&q->access_mutex, NULL));
    
    // Create queue on the DSP side
#ifdef ASYNCDSPQ_APP_CPU
    PASSERR(asyncdspq_rpc_create(q->fd, 0, alloc_size, &q->attach_handle));
    *attach_handle = q->attach_handle;
#else
#  error asyncdspq_create() not supported on this platform
#endif

    // Create threads to handle incoming message notifications and outgoing space available messages
    // on the reader side
    if ( reader == ASYNCDSPQ_ENDPOINT_APP_CPU ) {
        pthread_attr_t attr;

        if ( q->message_callback ) {
            q->callback_mask = 0;
            PASSERR(pthread_cond_init(&q->callback_cond, NULL));
            PASSERR(pthread_mutex_init(&q->callback_mutex, NULL));
            PASSERR(pthread_attr_init(&attr));
            PASSERR(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
            PASSERR(pthread_create(&q->callback_thread, &attr, asyncdspq_reader_callback_thread, q));
        }
        
        q->message_mask = 0;
        PASSERR(pthread_cond_init(&q->message_cond, NULL));
        PASSERR(pthread_mutex_init(&q->message_mutex, NULL));
        PASSERR(pthread_attr_init(&attr));
        PASSERR(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        PASSERR(pthread_create(&q->message_thread, &attr, asyncdspq_reader_message_thread, q));

        q->space_mask = 0;
        PASSERR(pthread_cond_init(&q->space_cond, NULL));
        PASSERR(pthread_mutex_init(&q->space_mutex, NULL));
        PASSERR(pthread_attr_init(&attr));
        PASSERR(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        PASSERR(pthread_create(&q->space_thread, &attr, asyncdspq_reader_space_thread, q));
    }

    // Create a thread to handle outgoing message notifications and incoming space available notifications
    // on the writer side
    if ( writer == ASYNCDSPQ_ENDPOINT_APP_CPU ) {
        pthread_attr_t attr;
        
        if ( q->space_callback ) {
            q->callback_mask = 0;
            PASSERR(pthread_cond_init(&q->callback_cond, NULL));
            PASSERR(pthread_mutex_init(&q->callback_mutex, NULL));
            PASSERR(pthread_attr_init(&attr));
            PASSERR(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
            PASSERR(pthread_create(&q->callback_thread, &attr, asyncdspq_writer_callback_thread, q));
        }
        
        q->message_mask = 0;
        PASSERR(pthread_cond_init(&q->message_cond, NULL));
        PASSERR(pthread_mutex_init(&q->message_mutex, NULL));
        PASSERR(pthread_attr_init(&attr));
        PASSERR(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        PASSERR(pthread_create(&q->message_thread, &attr, asyncdspq_writer_message_thread, q));

        q->space_mask = 0;
        PASSERR(pthread_cond_init(&q->space_cond, NULL));
        PASSERR(pthread_mutex_init(&q->space_mutex, NULL));
        PASSERR(pthread_attr_init(&attr));
        PASSERR(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        PASSERR(pthread_create(&q->space_thread, &attr, asyncdspq_writer_space_thread, q));
        // Note: Queue must be ready to use before the writer space thread starts
    }

    *queue = (asyncdspq_t) q;
    return AEE_SUCCESS;

fail:
    if ( q && q->header ) {
        rpcmem_free(q->header);
    }
    // FIXME: Undo other initialization? Nothing should really fail here...
    free(q);
    *queue = NULL;
    return err;
}


AEEResult asyncdspq_destroy(asyncdspq_t queue) {

    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    int err = AEE_SUCCESS;

    // FIXME: Check for detach?

    // Signal threads to exit and wait for them to finish.
    // Any outstanding requests on the DSP side should already have been terminated
    // in asyncdspq_detach()
    if ( q->header->reader == ASYNCDSPQ_ENDPOINT_APP_CPU ) {
        void *ret;
        PASSERR(asyncdspq_rpc_cancel_wait_message(q->attach_handle));
        PASSERR(pthread_join(q->message_thread, &ret));
        
        pthread_mutex_lock(&q->space_mutex);
        q->space_mask |= 2;
        pthread_cond_signal(&q->space_cond);
        pthread_mutex_unlock(&q->space_mutex);
        PASSERR(pthread_join(q->space_thread, &ret));
        
        if ( q->message_callback ) {
            pthread_mutex_lock(&q->callback_mutex);
            q->callback_mask |= 2;
            pthread_cond_signal(&q->callback_cond);
            pthread_mutex_unlock(&q->callback_mutex);
            PASSERR(pthread_join(q->callback_thread, &ret));            
        }
    }

    if ( q->header->writer == ASYNCDSPQ_ENDPOINT_APP_CPU ) {
        void *ret;
        PASSERR(asyncdspq_rpc_cancel_wait_space(q->attach_handle));
        PASSERR(pthread_join(q->space_thread, &ret));
        pthread_mutex_lock(&q->message_mutex);
        q->message_mask |= 2;
        pthread_cond_signal(&q->message_cond);
        pthread_mutex_unlock(&q->message_mutex);
        PASSERR(pthread_join(q->message_thread, &ret));
        
        if ( q->space_callback ) {
            pthread_mutex_lock(&q->callback_mutex);
            q->callback_mask |= 2;
            pthread_cond_signal(&q->callback_cond);
            pthread_mutex_unlock(&q->callback_mutex);
            PASSERR(pthread_join(q->callback_thread, &ret));            
        }
    }

    PASSERR(pthread_mutex_destroy(&q->access_mutex));
    
    // Destroy queue on the DSP side
    PASSERR(asyncdspq_rpc_destroy(q->attach_handle));

fail:
    rpcmem_free(q->header);
    free(q);

    return err;
}


AEEResult asyncdspq_cancel(asyncdspq_t queue) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    int err = 0;
    
    if ( q->header->reader == ASYNCDSPQ_ENDPOINT_APP_CPU ) {
        PASSERR(pthread_mutex_lock(&q->message_mutex));
        q->message_mask |= 2;
        PASSERR(pthread_cond_broadcast(&q->message_cond));
        PASSERR(pthread_mutex_unlock(&q->message_mutex));
    }
    if ( q->header->writer == ASYNCDSPQ_ENDPOINT_APP_CPU ) {
        PASSERR(pthread_mutex_lock(&q->space_mutex));
        q->space_mask |= 2;
        PASSERR(pthread_cond_broadcast(&q->space_cond));
        PASSERR(pthread_mutex_unlock(&q->space_mutex));
    }

fail:
    return err;
}

#endif // ASYNCDSPQ_SUPPORT_CREATE



#ifdef ASYNCDSPQ_SUPPORT_ATTACH

static void asyncdspq_reader_callback_thread(void *arg);
static void asyncdspq_writer_callback_thread(void *arg);


AEEResult asyncdspq_attach(asyncdspq_t *queue, asyncdspq_attach_handle_t attach_handle,
                          asyncdspq_error_callback_t error_callback,
                          asyncdspq_callback_t message_callback, asyncdspq_callback_t space_callback,
                          void *callback_context) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) attach_handle;
    int err = 0;
    FARF(RUNTIME_LOW, "asyncdspq_attach\n");

    *queue = NULL;
    if ( q == NULL ) {
        return AEE_EBADPARM;
    }
    q->attach_handle = attach_handle;
    q->error_callback = error_callback;
    q->message_callback = message_callback;
    q->space_callback = space_callback;
    q->callback_context = callback_context;
    qurt_mutex_init(&q->access_mutex);

    if ( q->header->reader == ASYNCDSPQ_ENDPOINT_CDSP ) {
        qurt_mutex_init(&q->message_mutex);
        if ( q->message_callback ) {
            qurt_thread_attr_t attr;
            qurt_thread_attr_init(&attr);
            qurt_thread_attr_set_stack_size(&attr, CALLBACK_THREAD_STACK_SIZE);
            if ( (q->callback_thread_stack = malloc(CALLBACK_THREAD_STACK_SIZE)) == NULL ) {
                return AEE_ENOMEMORY;
            }
            qurt_thread_attr_set_stack_addr(&attr, q->callback_thread_stack);
            int prio = qurt_thread_get_priority(qurt_thread_get_id());
            qurt_thread_attr_set_priority(&attr, prio);
            PASSERR(qurt_thread_create(&q->callback_thread, &attr, asyncdspq_reader_callback_thread, q));
        }
    }

    if ( q->header->writer == ASYNCDSPQ_ENDPOINT_CDSP ) {
        qurt_mutex_init(&q->space_mutex);
        if ( q->space_callback ) {
            qurt_thread_attr_t attr;
            qurt_thread_attr_init(&attr);
            qurt_thread_attr_set_stack_size(&attr, CALLBACK_THREAD_STACK_SIZE);
            if ( (q->callback_thread_stack = malloc(CALLBACK_THREAD_STACK_SIZE)) == NULL ) {
                return AEE_ENOMEMORY;
            }
            qurt_thread_attr_set_stack_addr(&attr, q->callback_thread_stack);
            int prio = qurt_thread_get_priority(qurt_thread_get_id());
            qurt_thread_attr_set_priority(&attr, prio);
            PASSERR(qurt_thread_create(&q->callback_thread, &attr, asyncdspq_writer_callback_thread, q));
            qurt_signal_set(&q->callback_signal, 1);
        }
    }

    *queue = (asyncdspq_t) q;
    return err;
    
fail:
    free(q->callback_thread_stack);
    q->callback_thread_stack = NULL;
    return err;
}


AEEResult asyncdspq_detach(asyncdspq_t queue) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    int err = 0;
    FARF(RUNTIME_LOW, "asyncdspq_detach\n");
    
    if ( q == NULL ) {
        return AEE_EBADPARM;
    }
    qurt_signal_set(&q->message_signal, 0x02);
    qurt_signal_set(&q->space_signal, 0x02);
    
    if ( ((q->header->reader == ASYNCDSPQ_ENDPOINT_CDSP) && (q->message_callback)) ||
         ((q->header->writer == ASYNCDSPQ_ENDPOINT_CDSP) && (q->space_callback)) ) {
        int status;
        qurt_signal_set(&q->callback_signal, 0x02);
        if ( (err = qurt_thread_join(q->callback_thread, &status)) != 0 ) {
            return err;
        }
        free(q->callback_thread_stack);
    }
    if ( q->header->reader == ASYNCDSPQ_ENDPOINT_CDSP ) {
        qurt_mutex_destroy(&q->message_mutex);
    }
    if ( q->header->writer == ASYNCDSPQ_ENDPOINT_CDSP ) {
        qurt_mutex_destroy(&q->space_mutex);
    }
    qurt_mutex_destroy(&q->access_mutex);
    
    q->attach_handle = 0;
    
    return AEE_SUCCESS;
}


AEEResult asyncdspq_cancel(asyncdspq_t queue) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    
    if ( q == NULL ) {
        return AEE_EBADPARM;
    }
    qurt_signal_set(&q->message_signal, 0x02);
    qurt_signal_set(&q->space_signal, 0x02);

    return AEE_SUCCESS;
}


#endif /* ASYNCDSPQ_SUPPORT_ATTACH */


#if defined(ASYNCDSPQ_ARM)

#define cache_invalidate_word(x)
#define cache_flush_word(x) 
#define barrier_full() __asm__ __volatile__ ("dmb sy" : : : "memory")
#define barrier_store() __asm__ __volatile__ ("dmb st" : : : "memory");
#define cache_flush(a, l)
#define cache_invalidate(a, l)

#elif defined(ASYNCDSPQ_HEXAGON)

#define cache_invalidate_word(x) { __asm__ __volatile__(" dcinva (%0)\n"::"r"(x)); \
                                   __asm__ __volatile__(" syncht\n"::); }
#define cache_flush_word(x) { __asm__ __volatile__(" dccleana (%0)\n"::"r"(x)); \
                              __asm__ __volatile__(" syncht\n"::); }
#define barrier_full(x) __asm__ __volatile__(" barrier\n"::)
#define barrier_store(x) __asm__ __volatile__(" barrier\n"::)

static inline void cache_flush(void *addr, size_t length) {
    uint8_t *a = (uint8_t*) (((uintptr_t) addr) & (~0x1f));
    uint32_t len = ((((uintptr_t) addr) & 0x1f) + length + 0x1f) & (~0x1f);
    while ( len ) {
        __asm__ __volatile__ (" dccleana (%0)\n"::"r"(a));
        len -= 0x20;
        a += 0x20;
    }
    __asm__ __volatile__(" syncht\n"::);    
}

static inline void cache_invalidate(void *addr, size_t length) {
    uint8_t *a = (uint8_t*) (((uintptr_t) addr) & (~0x1f));
    uint32_t len = ((((uintptr_t) addr) & 0x1f) + length + 0x1f) & (~0x1f);
    while ( len ) {
        __asm__ __volatile__ (" dcinva (%0)\n"::"r"(a));
        len -= 0x20;
        a += 0x20;
    }
    __asm__ __volatile__(" syncht\n"::);    
}

#endif



AEEResult asyncdspq_write_noblock(asyncdspq_t queue, const uint8_t *msg, unsigned length) {

    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    volatile uint8_t *qd = (volatile uint8_t*) (((uintptr_t)q->header) + q->header->queue_offset);
    volatile uint32_t *ro = (volatile uint32_t*) (((uintptr_t)q->header) + q->header->read_offset_offset);
    volatile uint32_t *wo = (volatile uint32_t*) (((uintptr_t)q->header) + q->header->write_offset_offset);
    uint32_t qsize = q->header->queue_length;
    uint32_t r, w;
    uint32_t qleft;

    assert(q->header->version == 1);
    if ( (length+4) > (qsize - 4) ) {
        return AEE_EBUFFERTOOSMALL;
    }
    if ( ((length & 3) != 0) || (length < 4) ) {
        return AEE_EBADPARM;
    }

    ASYNCDSPQ_MUTEX_LOCK_RETURN_ERR(q->access_mutex);

    // Check how much space we have left and if the response fits
    cache_invalidate_word(ro);
    r = *ro;
    barrier_full();
    w = *wo;
    if ( r == w ) {
        qleft = qsize - 4;
    } else if ( w > r ) {
        qleft = qsize - w + r - 4;
    } else {
        qleft = r - w - 4;
    }
    assert(qleft <= (qsize - 4));
    if ( (length+4) > qleft ) {
        ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(q->access_mutex);
        return AEE_ENOMORE;
    }

    // Write message to the queue
    // First 32-bit word is message length
    assert(w <= (qsize - 4));
    *((uint32_t*)&qd[w]) = length;
    cache_flush_word(&qd[w]);
    w += 4;
    if ( w >= qsize ) {
        w = 0;
    }
    if ( (w + length) <= qsize ) {
        memcpy((void*)&qd[w], msg, length);
        cache_flush((void*)&qd[w], length);
        w += length;
        if ( w >= qsize ) {
            w = 0;
        }
    } else {
        unsigned l1 = qsize - w;
        assert(l1 < length);
        memcpy((void*)&qd[w], msg, l1);
        cache_flush((void*)&qd[w], l1);
        memcpy((void*)qd, &msg[l1], length - l1);
        cache_flush((void*)qd, length - l1);
        w = length - l1;
    }

    // Update write pointer
    barrier_store();
    *wo = w;
    cache_flush_word(wo);

    // Signal a new message is available    
#if defined(ASYNCDSPQ_PTHREADS)
    pthread_mutex_lock(&q->message_mutex);
    q->message_mask |= 1;
    pthread_cond_signal(&q->message_cond);
    pthread_mutex_unlock(&q->message_mutex);
#elif defined(ASYNCDSPQ_QURT_THREADS)
    qurt_signal_set(&q->message_signal, 0x01);
#endif

    ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(q->access_mutex);
    return 0;
}



AEEResult asyncdspq_read_noblock(asyncdspq_t queue, uint8_t *buf, unsigned buf_length, unsigned *msg_length) {

    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    volatile uint8_t *qd = (volatile uint8_t*) (((uintptr_t)q->header) + q->header->queue_offset);
    volatile uint32_t *ro = (volatile uint32_t*) (((uintptr_t)q->header) + q->header->read_offset_offset);
    volatile uint32_t *wo = (volatile uint32_t*) (((uintptr_t)q->header) + q->header->write_offset_offset);
    uint32_t qsize = q->header->queue_length;
    uint32_t r, w;
    uint32_t qleft;
    uint32_t len;

    assert(q->header->version == 1);

    ASYNCDSPQ_MUTEX_LOCK_RETURN_ERR(q->access_mutex);

    // Check if we have a request to handle (read pointer != write pointer)
    cache_invalidate_word(wo);
    w = *wo;
    barrier_full();
    r = *ro;
    if ( r == w ) {
        ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(q->access_mutex);
        return AEE_ENOMORE;
    }
    assert(((r & 3) == 0) && ((w & 3) == 0));
    
    if ( w > r ) {
        qleft = w-r;
    } else {
        qleft = qsize - r + w;
    }
    assert(qleft >= 4);
    assert((qleft & 3) == 0);

    // Get message length (first word)
    cache_invalidate_word(&qd[r]);
    len = *((uint32_t*)&qd[r]);
    if ( (len > qleft) || ((len & 3) != 0) ) {
        ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(q->access_mutex);
        return AEE_EBADITEM;
    }
    if ( len > buf_length ) {
        ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(q->access_mutex);
        return AEE_ENOMEMORY; // need a bigger buffer
    }
    *msg_length = len;
    r += 4;
    if ( r >= qsize ) {
        r = 0;
    }

    // Copy message to client buffer
    if ( (r + len) <= qsize ) {
        cache_invalidate((void*)&qd[r], len);
        memcpy(buf, (void*)&qd[r], len);
        r += len;
        if ( r >= qsize ) {
            r = 0;
        }
    } else {
        uint32_t l1 = qsize - r;
        assert(l1 < len);
        cache_invalidate((void*)&qd[r], l1);
        memcpy(buf, (void*)&qd[r], l1);
        cache_invalidate((void*)&qd[0], len - l1);
        memcpy(&buf[l1], (void*)&qd[0], len - l1);
        r = len - l1;
    }

    // Update read pointer
    barrier_full();
    *ro = r;
    cache_invalidate_word(ro);

    // Signal there's space available in the queue
#if defined(ASYNCDSPQ_PTHREADS)
    pthread_mutex_lock(&q->space_mutex);
    q->space_mask |= 1;
    pthread_cond_signal(&q->space_cond);
    pthread_mutex_unlock(&q->space_mutex);
#elif defined(ASYNCDSPQ_QURT_THREADS)
    qurt_signal_set(&q->space_signal, 0x01);
#endif

    ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(q->access_mutex);
    return AEE_SUCCESS;
}



int asyncdspq_read_waits;

AEEResult asyncdspq_read(asyncdspq_t queue, uint8_t *buf, unsigned buf_length, unsigned *msg_length) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    AEEResult err = AEE_SUCCESS;

    if ( q->message_callback ) {
        return AEE_EUNSUPPORTED;
    }

    while ( 1 ) {
        ASYNCDSPQ_MUTEX_LOCK(q->message_mutex);
        err = asyncdspq_read_noblock(queue, buf, buf_length, msg_length);
        if ( err != AEE_ENOMORE ) {
            // Have a message or got an error
            ASYNCDSPQ_MUTEX_UNLOCK(q->message_mutex);
            return err;
        }

#if defined(ASYNCDSPQ_PTHREADS)
        if ( q->message_mask & 2 ) {
            ASYNCDSPQ_MUTEX_UNLOCK(q->message_mutex);
            return AEE_EEXPIRED;
        }
        asyncdspq_read_waits++;
        pthread_cond_wait(&q->message_cond, &q->message_mutex);
        q->message_mask = q->message_mask & (~1);
        pthread_mutex_unlock(&q->message_mutex);
#elif defined (ASYNCDSPQ_QURT_THREADS)
        if ( qurt_signal_get(&q->message_signal) & 2 ) {
            ASYNCDSPQ_MUTEX_UNLOCK(q->message_mutex);
            return AEE_EEXPIRED;
        }
        asyncdspq_read_waits++;
        qurt_signal_wait(&q->message_signal, 0x03, QURT_SIGNAL_ATTR_WAIT_ANY);
        qurt_signal_clear(&q->message_signal, 1);
        qurt_mutex_unlock(&q->message_mutex);
#endif
    }
    
    return err;
}



int asyncdspq_write_waits;

AEEResult asyncdspq_write(asyncdspq_t queue, const uint8_t *msg, unsigned length) {
    
    asyncdspq_local_t *q = (asyncdspq_local_t*) queue;
    AEEResult err = AEE_SUCCESS;

    if ( q->space_callback ) {
        return AEE_EUNSUPPORTED;
    }

    while ( 1 ) {
        err = asyncdspq_write_noblock(queue, msg, length);
        if ( err != AEE_ENOMORE ) {
            return err;
        }

        asyncdspq_write_waits++;
#if defined(ASYNCDSPQ_PTHREADS)
        pthread_mutex_lock(&q->space_mutex);
        while ( q->space_mask == 0 ) {
            pthread_cond_wait(&q->space_cond, &q->space_mutex);
        }
        if ( q->space_mask & 2 ) {
            pthread_mutex_unlock(&q->space_mutex);
            return AEE_EEXPIRED;
        } else if ( q->space_mask & 1 ) {
            q->space_mask = q->space_mask & (~1);
        }
        pthread_mutex_unlock(&q->space_mutex);
#elif defined(ASYNCDSPQ_QURT_THREADS)
        qurt_mutex_lock(&q->space_mutex);
        uint32_t s = qurt_signal_wait(&q->space_signal, 0x03, QURT_SIGNAL_ATTR_WAIT_ANY);
        if ( s & 2 ) {
            qurt_mutex_unlock(&q->space_mutex);
            return AEE_EEXPIRED;
        } else if ( s & 1 ) {
            qurt_signal_clear(&q->space_signal, 1);
        }
        qurt_mutex_unlock(&q->space_mutex);
#endif
    }
    
    return err;    
}



static ASYNCDSPQ_THREAD_FUNC_TYPE asyncdspq_reader_callback_thread(void *arg) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) arg;

    while ( 1 ) {
#if defined(ASYNCDSPQ_PTHREADS)
        pthread_mutex_lock(&q->callback_mutex);
        while ( q->callback_mask == 0 ) {
            pthread_cond_wait(&q->callback_cond, &q->callback_mutex);
        }
        if ( q->callback_mask & 2 ) {
            q->callback_mask = q->callback_mask & (~2);
            pthread_mutex_unlock(&q->callback_mutex);
            return NULL;
        } else if ( q->callback_mask & 1 ) {
            q->callback_mask = q->callback_mask & (~1);
        }
        pthread_mutex_unlock(&q->callback_mutex);
#elif defined(ASYNCDSPQ_QURT_THREADS)
        uint32_t s = qurt_signal_wait(&q->callback_signal, 0x3, QURT_SIGNAL_ATTR_WAIT_ANY);
        if ( s & 2 ) {
            qurt_signal_clear(&q->callback_signal, 2);
            qurt_thread_exit(0);
        } else if ( s & 1 ) {
            qurt_signal_clear(&q->callback_signal, 1);
        }
#endif
        
        assert(q->message_callback);
        q->message_callback((asyncdspq_t)q, q->callback_context);
    }
}


static ASYNCDSPQ_THREAD_FUNC_TYPE asyncdspq_writer_callback_thread(void *arg) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) arg;

    while ( 1 ) {
#if defined(ASYNCDSPQ_PTHREADS)
        pthread_mutex_lock(&q->callback_mutex);
        while ( q->callback_mask == 0 ) {
            pthread_cond_wait(&q->callback_cond, &q->callback_mutex);
        }
        if ( q->callback_mask & 2 ) {
            q->callback_mask = q->callback_mask & (~2);
            pthread_mutex_unlock(&q->callback_mutex);
            return NULL;
        } else if ( q->callback_mask & 1 ) {
            q->callback_mask = q->callback_mask & (~1);
        }
        pthread_mutex_unlock(&q->callback_mutex);
#elif defined(ASYNCDSPQ_QURT_THREADS)
        uint32_t s = qurt_signal_wait(&q->callback_signal, 0x3, QURT_SIGNAL_ATTR_WAIT_ANY);
        if ( s & 2 ) {
            qurt_signal_clear(&q->callback_signal, 2);
            qurt_thread_exit(0);
        } else if ( s & 1 ) {
            qurt_signal_clear(&q->callback_signal, 1);
        }
#endif
        
        assert(q->space_callback);
        q->space_callback((asyncdspq_t)q, q->callback_context);
    }
}




#if defined(ASYNCDSPQ_APP_CPU)


static void *asyncdspq_reader_message_thread(void *arg) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) arg;
    int err = AEE_SUCCESS;
    int numwaits = 0;

    while ( 1 ) {
        int32_t messages;
        if ( (err = asyncdspq_rpc_wait_message(q->attach_handle, &messages)) != 0 ) {
            if ( q->error_callback ) {
                q->error_callback((asyncdspq_t) q, q->callback_context, err);
                return NULL;
            }
        }
        numwaits++;
        pthread_mutex_lock(&q->message_mutex);
        if ( messages == 1 ) {
            q->message_mask |= 1;
        } else if ( messages == -1 ) {
            q->message_mask |= 2;
        }
        pthread_cond_signal(&q->message_cond);
        pthread_mutex_unlock(&q->message_mutex);
        if ( messages == -1 ) {
            return NULL;
        }
        if ( (q->message_callback) && (messages == 1) ) {
            pthread_mutex_lock(&q->callback_mutex);
            q->callback_mask |= 1;
            pthread_cond_signal(&q->callback_cond);
            pthread_mutex_unlock(&q->callback_mutex);
        }
    }
}



static void *asyncdspq_reader_space_thread(void *arg) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) arg;
    int err = AEE_SUCCESS;
    int numwrites = 0;
    int exit = 0;
    int sp = 0;

    while ( 1 ) {        
        pthread_mutex_lock(&q->space_mutex);
        sp = 0;
        while ( q->space_mask == 0 ) {
            pthread_cond_wait(&q->space_cond, &q->space_mutex);
        }
        if ( q->space_mask & 2 ) {
            q->space_mask = q->space_mask & (~2);
            exit = 1;
        } else if ( q->space_mask & 1 ) {
            q->space_mask = q->space_mask & (~1);
            sp = 1;
        }
        pthread_mutex_unlock(&q->space_mutex);

        if ( exit ) {
            return NULL;
        }
        if ( sp ) {
            numwrites++;
            if ( (err = asyncdspq_rpc_signal_space(q->attach_handle)) != 0 ) {
                if ( q->error_callback ) {
                    q->error_callback((asyncdspq_t) q, q->callback_context, err);
                    return NULL;
                }
            }
        }
    }
}



static void *asyncdspq_writer_space_thread(void *arg) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) arg;
    int err = AEE_SUCCESS;
    int numwaits = 0;

    if ( q->space_callback ) {
        pthread_mutex_lock(&q->callback_mutex);
        q->callback_mask |= 1;
        pthread_cond_signal(&q->callback_cond);
        pthread_mutex_unlock(&q->callback_mutex);
    }

    while ( 1 ) {
        int32_t space;
        if ( (err = asyncdspq_rpc_wait_space(q->attach_handle, &space)) != 0 ) {
            if ( q->error_callback ) {
                q->error_callback((asyncdspq_t) q, q->callback_context, err);
                return NULL;
            }
        }
        numwaits++;
        pthread_mutex_lock(&q->space_mutex);
        if ( space == 1 ) {
            q->space_mask |= 1;
        } else if ( space == -1 ) {
            q->space_mask |= 2;
        }
        pthread_cond_signal(&q->space_cond);
        pthread_mutex_unlock(&q->space_mutex);
        if ( space == -1 ) {
            return NULL;
        }
        if ( (q->space_callback) && (space == 1) ) {
            pthread_mutex_lock(&q->callback_mutex);
            q->callback_mask |= 1;
            pthread_cond_signal(&q->callback_cond);
            pthread_mutex_unlock(&q->callback_mutex);
        }
    }
}


static void *asyncdspq_writer_message_thread(void *arg) {
    asyncdspq_local_t *q = (asyncdspq_local_t*) arg;
    int err = AEE_SUCCESS;
    int numwrites = 0;
    int exit = 0;
    int msg = 0;

    while ( 1 ) {        
        pthread_mutex_lock(&q->message_mutex);
        msg = 0;
        while ( q->message_mask == 0 ) {
            pthread_cond_wait(&q->message_cond, &q->message_mutex);
        }
        if ( q->message_mask & 2 ) {
            q->message_mask = q->message_mask & (~2);
            exit = 1;
        } else if ( q->message_mask & 1 ) {
            q->message_mask = q->message_mask & (~1);
            msg = 1;
        }
        pthread_mutex_unlock(&q->message_mutex);

        if ( exit ) {
            return NULL;
        }
        if ( msg ) {
            numwrites++;
            if ( (err = asyncdspq_rpc_signal_message(q->attach_handle)) != 0 ) {
                if ( q->error_callback ) {
                    q->error_callback((asyncdspq_t) q, q->callback_context, err);
                    return NULL;
                }
            }
        }
    }
}



#endif
