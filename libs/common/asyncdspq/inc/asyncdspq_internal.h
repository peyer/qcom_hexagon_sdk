/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef ASYNCDSPQ_INTERNAL_H
#define ASYNCDSPQ_INTERNAL_H

#include <stdint.h>




#if defined(__ANDROID__)
#  define ASYNCDSPQ_ANDROID
#  define ASYNCDSPQ_SUPPORT_CREATE
#  define ASYNCDSPQ_APP_CPU
#  define ASYNCDSPQ_LINUX
#  define ASYNCDSPQ_PTHREADS
#  if defined(__arm__) || defined(__aarch64__)
#    define ASYNCDSPQ_ARM
#  else
#    error Unsupported Android architecture
#  endif
#elif defined(__hexagon__)
#  define ASYNCDSPQ_SUPPORT_ATTACH
#  define ASYNCDSPQ_HEXAGON
#  define ASYNCDSPQ_DSP
#  define ASYNCDSPQ_QURT
#  define ASYNCDSPQ_QURT_THREADS
#  include "HAP_farf.h"
#elif defined(__LE__)
#  define ASYNCDSPQ_ANDROID
#  define ASYNCDSPQ_SUPPORT_CREATE
#  define ASYNCDSPQ_APP_CPU
#  define ASYNCDSPQ_LINUX
#  define ASYNCDSPQ_PTHREADS
#  if defined(__arm__) || defined(__aarch64__)
#    define ASYNCDSPQ_ARM
#  else
#    error Unsupported Android architecture
#  endif
#else
#  error Unsupported/unknown platform
#endif

#if defined(ASYNCDSPQ_PTHREADS)
#  include <pthread.h>
#  define ASYNCDSPQ_THREAD_FUNC_TYPE void*
#  define ASYNCDSPQ_MUTEX_LOCK(x) pthread_mutex_lock(&x)
#  define ASYNCDSPQ_MUTEX_UNLOCK(x) pthread_mutex_unlock(&x)
#  define ASYNCDSPQ_MUTEX_LOCK_RETURN_ERR(x) \
    { int err; if ( (err = pthread_mutex_lock(&x)) != 0 ) { return err; } }
#  define ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(x) \
    { int err; if ( (err = pthread_mutex_unlock(&x)) != 0 ) { return err; } }
#elif defined(ASYNCDSPQ_QURT_THREADS)
#  include <qurt.h>
#  define ASYNCDSPQ_THREAD_FUNC_TYPE void
#  define ASYNCDSPQ_MUTEX_LOCK(x)  qurt_mutex_lock(&x)
#  define ASYNCDSPQ_MUTEX_UNLOCK(x) qurt_mutex_unlock(&x)
#  define ASYNCDSPQ_MUTEX_LOCK_RETURN_ERR(x)  qurt_mutex_lock(&x)
#  define ASYNCDSPQ_MUTEX_UNLOCK_RETURN_ERR(x) qurt_mutex_unlock(&x)
#endif

#include "asyncdspq.h"



/* Shared memory queue header */
typedef struct {
    uint32_t version; /* 1 */
    int32_t error;
    uint32_t queue_offset; /* queue offset in bytes, must be on a separate cache line */
    uint32_t queue_length; /* in bytes */
    uint32_t read_offset_offset; /* offset to read offset in bytes.
                                    Read offset is a 32-bit unsigned byte offset to the
                                    beginning of the queue */
    uint32_t write_offset_offset; /* offset to write offset in bytes */
    uint32_t reader; /* Reader endpoint */
    uint32_t writer; /* Writer endpoint */
    uint32_t flags; /* Creation flags */
} asyncdspq_header_t;



/* Internal queue structure local to each endpoint */
typedef struct {
    asyncdspq_header_t *header;
    unsigned alloc_size;
    int fd;
    asyncdspq_error_callback_t error_callback;
    asyncdspq_callback_t message_callback;
    asyncdspq_callback_t space_callback;
    void *callback_context;
    uint32_t attach_handle;
#if defined(ASYNCDSPQ_PTHREADS)
#if defined(ASYNCDSPQ_APP_CPU)
    pthread_t message_thread;
    pthread_t space_thread;
#endif
    uint32_t message_mask; // 1 = messages, 2 = end
    pthread_cond_t message_cond;
    pthread_mutex_t message_mutex;
    uint32_t space_mask; // 1 = messages, 2 = end
    pthread_cond_t space_cond;
    pthread_mutex_t space_mutex;
    pthread_t callback_thread;
    pthread_cond_t callback_cond;
    pthread_mutex_t callback_mutex;
    uint32_t callback_mask; // 1 = callback, 2 = end
    pthread_mutex_t access_mutex; // read/write access
#elif defined(ASYNCDSPQ_QURT_THREADS)
    qurt_signal_t message_signal; // 1 = messages, 2 = end
    qurt_mutex_t message_mutex;
    qurt_signal_t space_signal; // 1 = space, 2 = end
    qurt_mutex_t space_mutex;
    qurt_signal_t callback_signal; // 1 = callback, 2 = end
    qurt_thread_t callback_thread;
    void *callback_thread_stack;
    qurt_mutex_t access_mutex;
#endif
} asyncdspq_local_t;



#endif /* ASYNCDSPQ_INTERNAL_H */
