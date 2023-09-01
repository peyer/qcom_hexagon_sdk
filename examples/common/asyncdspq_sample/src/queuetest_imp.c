/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Tests for asyncdspq */

#ifndef _DEBUG
//#define _DEBUG
#endif
#include <stdlib.h>
#include <assert.h>
#include "HAP_farf.h"
#include "HAP_perf.h"
#include "HAP_mem.h"
#include "HAP_power.h"
#include "AEEStdErr.h"
#include "qurt.h"
#include "queuetest.h"
#include "asyncdspq.h"


#define REQBUFSIZE 256
#define RESPBUFSIZE 32


AEEResult queuetest_asyncdspq_test_reads(uint32 attach_handle, uint32 nummsgs, uint32 *lastmsg) {

    int err = 0;
    asyncdspq_t queue = NULL;
    FARF(RUNTIME_LOW, "queuetest_asyncdspq_test_reads: attach_handle %u; nummsgs: %u\n", attach_handle, nummsgs);

    if ( (err = asyncdspq_attach(&queue, attach_handle, NULL, NULL, NULL, NULL)) != 0 ) {
        err = __LINE__;
        goto fail;
    }
    while ( nummsgs-- ) {
        uint32_t msg[1];
        unsigned msglen;
        if ( (err = asyncdspq_read(queue, (uint8_t*)msg, 4, &msglen)) != 0 ) {
            err = __LINE__;
            goto fail;
        }
        if ( msglen != 4 ) {
            err = __LINE__;
            goto fail;
        }
        *lastmsg = msg[0];
    }
    if ( (err = asyncdspq_detach(queue)) != 0 ) {
        err = __LINE__;
        goto fail;
    }
    FARF(RUNTIME_LOW, "queuetest_asyncdspq_test_reads: Done\n");
    return AEE_SUCCESS;

fail:
    return err;
    if ( queue != NULL ) {
        asyncdspq_detach(queue);
    }
    return err;
}


AEEResult queuetest_asyncdspq_test_writes(uint32 attach_handle, uint32 nummsgs, uint32 firstmsg) {

    int err = 0;
    asyncdspq_t queue = NULL;
    FARF(RUNTIME_LOW, "queuetest_asyncdspq_test_writes: attach_handle %u; nummsgs: %u\n", attach_handle, nummsgs);

    if ( (err = asyncdspq_attach(&queue, attach_handle, NULL, NULL, NULL, NULL)) != 0 ) {
        err = __LINE__;
        goto fail;
    }
    while ( nummsgs-- ) {
        uint32_t msg[1];
        msg[0] = firstmsg++;
        if ( (err = asyncdspq_write(queue, (const uint8_t*)msg, 4)) != 0 ) {
            err = __LINE__;
            goto fail;
        }
    }
    if ( (err = asyncdspq_detach(queue)) != 0 ) {
        err = __LINE__;
        goto fail;
    }
    FARF(RUNTIME_LOW, "queuetest_asyncdspq_test_reads: Done\n");
    return AEE_SUCCESS;

fail:
    if ( queue != NULL ) {
        asyncdspq_detach(queue);
    }
    return err;
}


AEEResult queuetest_asyncdspq_adder_spin(uint32 req_queue_attach_handle, uint32 resp_queue_attach_handle) {

    int err = 0;
    asyncdspq_t req_queue = NULL;
    asyncdspq_t resp_queue = NULL;
    FARF(RUNTIME_LOW, "queuetest_asyncdspq_adder_spin: req_queue_attach_handle %u; resp_queue_attach_handle: %u\n", req_queue_attach_handle, resp_queue_attach_handle);

    if ( (err = asyncdspq_attach(&req_queue, req_queue_attach_handle, NULL, NULL, NULL, NULL)) != 0 ) {
        err = __LINE__;
        goto fail;
    }
    if ( (err = asyncdspq_attach(&resp_queue, resp_queue_attach_handle, NULL, NULL, NULL, NULL)) != 0 ) {
        err = __LINE__;
        goto fail;
    }

    while ( 1 ) {
        uint32_t req[REQBUFSIZE];
        unsigned reqlen;
        uint32_t resp[RESPBUFSIZE];

        if ( (err = asyncdspq_read(req_queue, (uint8_t*) req, sizeof(req), &reqlen)) != 0 ) {
            err = __LINE__;
            goto fail;
        }
        if ( reqlen < 4 ) {
            err = __LINE__;
            goto fail;
        }

        if ( req[0] == 1 ) {
            // Request: Add numbers
            if ( reqlen < 12 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            uint32_t sum = req[1] + req[2];
            resp[0] = 1;
            resp[1] = sum;
            if ( (err = asyncdspq_write(resp_queue, (uint8_t*) resp, 8)) != 0 ) {
                err = __LINE__;
                goto fail;
            }
            
        } else if ( req[0] == 2 ) {
            // Request: Terminate
            if ( reqlen != 4 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            resp[0] = 2;
            resp[1] = 0;
            if ( (err = asyncdspq_write(resp_queue, (uint8_t*) resp, 8)) != 0 ) {
                err = __LINE__;
                goto fail;
            }
            break;
        }
    }

fail:
    if ( resp_queue != NULL ) {
        asyncdspq_detach(resp_queue);
    }
    if ( req_queue != NULL ) {
        asyncdspq_detach(req_queue);
    }
    FARF(RUNTIME_LOW, "queuetest_asyncdspq_adder_spin: Done (%d)\n", err);
    return err;
}


static asyncdspq_t adder_req_queue = NULL;
static asyncdspq_t adder_resp_queue = NULL;
static qurt_sem_t adder_sem;


#define PASSERR(x) { if ( (err = x) != 0 ) { err = __LINE__; goto fail; } }
//#define PASSERR(x) { if ( (err = x) != 0 ) { goto fail; } }


static void adder_error_callback(asyncdspq_t queue, void *context, int error) {
    assert(0);
}


static void adder_message_callback(asyncdspq_t queue, void *context) {

    uint32_t req[REQBUFSIZE];
    unsigned reqlen;
    uint32_t resp[RESPBUFSIZE];
    int err = 0;

    qurt_sem_down(&adder_sem);
    qurt_sem_up(&adder_sem);
    while ( 1 ) {
        err = asyncdspq_read_noblock(adder_req_queue, (uint8_t*) req, sizeof(req), &reqlen);
        if ( err == AEE_ENOMORE ) {
            return;
        }
        if ( err != 0 ) {
            err = __LINE__;
            goto fail;
        }
        if ( reqlen < 4 ) {
            err = __LINE__;
            goto fail;
        }

        if ( req[0] == 1 ) {
            // Request: Add numbers
            if ( reqlen != 12 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            uint32_t sum = req[1] + req[2];
            resp[0] = 1;
            resp[1] = sum;
            PASSERR(asyncdspq_write(adder_resp_queue, (uint8_t*) resp, 8));
            
        } else if ( req[0] == 2 ) {
            // Request: Terminate
            if ( reqlen != 4 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            resp[0] = 2;
            resp[1] = 0;
            PASSERR(asyncdspq_write(adder_resp_queue, (uint8_t*) resp, 8));
            return;
        }
    }


fail:
    resp[0] = 3;
    resp[1] = err;
    asyncdspq_write(adder_resp_queue, (uint8_t*) resp, 8);
}


AEEResult queuetest_asyncdspq_adder_message_callbacks_start(uint32 req_queue_attach_handle,
                                                           uint32 resp_queue_attach_handle) {

    int err = 0;

    adder_req_queue = NULL;
    adder_resp_queue = NULL;
    qurt_sem_init_val(&adder_sem, 0);
    PASSERR(asyncdspq_attach(&adder_req_queue, req_queue_attach_handle, adder_error_callback,
                            adder_message_callback, NULL, NULL));
    PASSERR(asyncdspq_attach(&adder_resp_queue, resp_queue_attach_handle, adder_error_callback,
                            NULL, NULL, NULL));
    qurt_sem_up(&adder_sem);
    return AEE_SUCCESS;
    
fail:
    if ( adder_resp_queue != NULL ) {
        asyncdspq_detach(adder_resp_queue);
        adder_resp_queue = NULL;
    }
    if ( adder_req_queue != NULL ) {
        asyncdspq_detach(adder_req_queue);
        adder_req_queue = NULL;
    }
    return err;
}


AEEResult queuetest_asyncdspq_adder_message_callbacks_stop(void) {
    int err = 0;
    PASSERR(asyncdspq_detach(adder_resp_queue));
    adder_resp_queue = NULL;
    PASSERR(asyncdspq_detach(adder_req_queue));
    adder_req_queue = NULL;
    qurt_sem_destroy(&adder_sem);

fail:
    return err;
}


static uint32_t test_count;
static asyncdspq_t test_queue;

static void test_space_callback(asyncdspq_t queue, void *context) {

    int err;
    
    while ( 1 ) {
        if ( test_count == 0 ) {
            return;
        }
        err = asyncdspq_write_noblock(queue, (uint8_t*)&test_count, 4);
        if ( err == AEE_ENOMORE ) {
            return;
        }
        test_count--;
    }
}


AEEResult queuetest_asyncdspq_space_callback_start(uint32 attach_handle, uint32 nummsgs) {
    int err = 0;

    test_queue = NULL;
    test_count = nummsgs;
    PASSERR(asyncdspq_attach(&test_queue, attach_handle, NULL, NULL, test_space_callback, NULL));
    return err;
            
fail:
    if ( test_queue != NULL ) {
        asyncdspq_detach(test_queue);
        test_queue = NULL;
    }
    return err;
}


AEEResult queuetest_asyncdspq_space_callback_stop(void) {
    
    int err = 0;
    PASSERR(asyncdspq_detach(test_queue));
    test_queue = NULL;
            
fail:
    return err;
}




static asyncdspq_t multiadder_req_queue = NULL;
static asyncdspq_t multiadder_resp_queue = NULL;
static qurt_sem_t multiadder_sem;


static void multiadder_error_callback(asyncdspq_t queue, void *context, int error) {
    assert(0);
}


static void multiadder_message_callback(asyncdspq_t queue, void *context) {

    uint32_t req[REQBUFSIZE];
    unsigned reqlen;
    uint32_t resp[RESPBUFSIZE];
    int err = 0;

    qurt_sem_down(&multiadder_sem);
    qurt_sem_up(&multiadder_sem);
    while ( 1 ) {
        err = asyncdspq_read_noblock(multiadder_req_queue, (uint8_t*) req, sizeof(req), &reqlen);
        if ( err == AEE_ENOMORE ) {
            return;
        }
        if ( err != 0 ) {
            err = __LINE__;
            goto fail;
        }
        if ( reqlen < 4 ) {
            err = __LINE__;
            goto fail;
        }

        if ( req[0] == 1 ) {
            // Request: Add numbers
            if ( reqlen != 16 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            uint32_t sum = req[1] + req[2];
            resp[0] = 1;
            resp[1] = sum;
            resp[2] = req[3];
            PASSERR(asyncdspq_write(multiadder_resp_queue, (uint8_t*) resp, 12));
            
        } else if ( req[0] == 2 ) {
            // Request: Terminate
            if ( reqlen != 4 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            resp[0] = 2;
            resp[1] = 0;
            PASSERR(asyncdspq_write(multiadder_resp_queue, (uint8_t*) resp, 8));
            return;
        }
    }


fail:
    resp[0] = 3;
    resp[1] = err;
    asyncdspq_write(multiadder_resp_queue, (uint8_t*) resp, 8);
}


AEEResult queuetest_asyncdspq_multiadder_message_callbacks_start(uint32 req_queue_attach_handle,
                                                                uint32 resp_queue_attach_handle) {

    int err = 0;

    multiadder_req_queue = NULL;
    multiadder_resp_queue = NULL;
    qurt_sem_init_val(&multiadder_sem, 0);
    PASSERR(asyncdspq_attach(&multiadder_resp_queue, resp_queue_attach_handle, multiadder_error_callback,
                            NULL, NULL, NULL));
    PASSERR(asyncdspq_attach(&multiadder_req_queue, req_queue_attach_handle, multiadder_error_callback,
                            multiadder_message_callback, NULL, NULL));
    qurt_sem_up(&multiadder_sem);

    return AEE_SUCCESS;
    
fail:
    if ( multiadder_resp_queue != NULL ) {
        asyncdspq_detach(multiadder_resp_queue);
        multiadder_resp_queue = NULL;
    }
    if ( multiadder_req_queue != NULL ) {
        asyncdspq_detach(multiadder_req_queue);
        multiadder_req_queue = NULL;
    }
    return err;
}


AEEResult queuetest_asyncdspq_multiadder_message_callbacks_stop(void) {
    int err = 0;
    PASSERR(asyncdspq_detach(multiadder_resp_queue));
    multiadder_resp_queue = NULL;
    PASSERR(asyncdspq_detach(multiadder_req_queue));
    multiadder_req_queue = NULL;
    qurt_sem_destroy(&multiadder_sem);

fail:
    return err;
}


static void queuetest_asyncdspq_multiadder_thread(void *arg) {

    uint32_t req[REQBUFSIZE];
    unsigned reqlen;
    uint32_t resp[RESPBUFSIZE];
    int err = 0;

    while ( 1 ) {
        err = asyncdspq_read(multiadder_req_queue, (uint8_t*) req, sizeof(req), &reqlen);
        if ( err == AEE_EEXPIRED ) {
            // Queue being detached
            qurt_thread_exit(0);
        }
        if ( err != 0 ) {
            err = __LINE__;
            goto fail;
        }
        if ( reqlen < 4 ) {
            // err = AEE_EBADITEM;
            err = __LINE__;
            goto fail;
        }

        if ( req[0] == 1 ) {
            // Request: Add numbers
            if ( reqlen != 16 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            uint32_t sum = req[1] + req[2];
            resp[0] = 1;
            resp[1] = sum;
            resp[2] = req[3];
            PASSERR(asyncdspq_write(multiadder_resp_queue, (uint8_t*) resp, 12));
            
        } else if ( req[0] == 2 ) {
            // Request: Terminate
            if ( reqlen != 4 ) {
                // err = AEE_EBADITEM;
                err = __LINE__;
                goto fail;
            }
            resp[0] = 2;
            resp[1] = 0;
            PASSERR(asyncdspq_write(multiadder_resp_queue, (uint8_t*) resp, 8));
            qurt_thread_exit(0);
            
        } else {
            // Unknown request
            err = __LINE__;
            goto fail;
        }
    }


fail:
    resp[0] = 3;
    resp[1] = err;
    asyncdspq_write(multiadder_resp_queue, (uint8_t*) resp, 8);
    qurt_thread_exit(0);
}


#define MAXTHREADS 8
//#define NUMTHREADS 1
#define THREAD_STACK_SIZE 8192
static qurt_thread_t multiadder_threads[MAXTHREADS];
static void* multiadder_thread_stacks[MAXTHREADS];
static uint32_t multiadder_num_threads;


AEEResult queuetest_asyncdspq_multiadder_threads_start(uint32 req_queue_attach_handle,
                                                      uint32 resp_queue_attach_handle,
                                                      uint32 num_threads) {

    int err = 0;
    int i;

    multiadder_num_threads = num_threads;
    multiadder_req_queue = NULL;
    multiadder_resp_queue = NULL;
    PASSERR(asyncdspq_attach(&multiadder_req_queue, req_queue_attach_handle, multiadder_error_callback,
                            NULL, NULL, NULL));
    PASSERR(asyncdspq_attach(&multiadder_resp_queue, resp_queue_attach_handle, multiadder_error_callback,
                            NULL, NULL, NULL));

    for ( i = 0; i < multiadder_num_threads; i++ ) {        
        qurt_thread_attr_t attr;
        if ( (multiadder_thread_stacks[i] = malloc(THREAD_STACK_SIZE)) == NULL ) {
            //err = AEE_ENOMEMORY;
            err = __LINE__;
            goto fail;
        }
        qurt_thread_attr_init(&attr);
        qurt_thread_attr_set_stack_size(&attr, THREAD_STACK_SIZE);
        qurt_thread_attr_set_stack_addr(&attr, multiadder_thread_stacks[i]);
        PASSERR(qurt_thread_create(&multiadder_threads[i], &attr,
                                   queuetest_asyncdspq_multiadder_thread, (void*) i));
    }
    
    return AEE_SUCCESS;
    
fail:
    if ( multiadder_resp_queue != NULL ) {
        asyncdspq_detach(multiadder_resp_queue);
        multiadder_resp_queue = NULL;
    }
    if ( multiadder_req_queue != NULL ) {
        asyncdspq_detach(multiadder_req_queue);
        multiadder_req_queue = NULL;
    }
    return err;
}


AEEResult queuetest_asyncdspq_multiadder_threads_stop(void) {
    int err = 0;
    int i;

    PASSERR(asyncdspq_cancel(multiadder_req_queue));

    for ( i = 0; i < multiadder_num_threads; i++ ) {
        int status;
        err = qurt_thread_join(multiadder_threads[i], &status);
        if ( (err != QURT_ENOTHREAD) && (err != QURT_EOK) ) {
            err = __LINE__;
            goto fail;
        }
        if ( (err != QURT_ENOTHREAD) && (status != 0) ) {
//            err = __LINE__;
            err = status;
            goto fail;
        }
        err = 0;
        free(multiadder_thread_stacks[i]);
    }
    
    PASSERR(asyncdspq_detach(multiadder_resp_queue));
    multiadder_resp_queue = NULL;
    PASSERR(asyncdspq_detach(multiadder_req_queue));
    multiadder_req_queue = NULL;

fail:
    return err;
}


AEEResult queuetest_enable_logging(void) {
    HAP_setFARFRuntimeLoggingParams(0x1f, NULL, 0);
    return AEE_SUCCESS;
}



AEEResult queuetest_asyncdspq_test_cancel_read(uint32 attach_handle) {

    int err = 0;
    asyncdspq_t queue = NULL;
    uint8_t d[4];
    unsigned l;
    
    PASSERR(asyncdspq_attach(&queue, attach_handle, NULL, NULL, NULL, NULL));
    PASSERR(asyncdspq_cancel(queue));
    err = asyncdspq_read(queue, d, sizeof(d), &l);
    if ( err != AEE_EEXPIRED ) {
        return err;
    }
    PASSERR(asyncdspq_detach(queue));
    queue = NULL;
    
fail:
    if ( queue != NULL ) {
        asyncdspq_detach(NULL);
    }
    return err;
}



AEEResult queuetest_asyncdspq_test_cancel_write(uint32 attach_handle) {

    int err = 0;
    asyncdspq_t queue = NULL;
    uint8_t d[4];
    
    PASSERR(asyncdspq_attach(&queue, attach_handle, NULL, NULL, NULL, NULL));
    // Fill queue
    while ( (err = asyncdspq_write_noblock(queue, d, sizeof(d))) != AEE_ENOMORE ) {
        if ( err != AEE_SUCCESS ) {
//            return err;
            return __LINE__;
        }
    }
    PASSERR(asyncdspq_cancel(queue));
    err = asyncdspq_write(queue, d, sizeof(d));
    if ( err != AEE_EEXPIRED ) {
        return err;
    }
    PASSERR(asyncdspq_detach(queue));
    queue = NULL;
    
fail:
    if ( queue != NULL ) {
        asyncdspq_detach(NULL);
    }
    return err;
}


AEEResult queuetest_set_clocks(void) {

    int err;

    // Set client class
    HAP_power_request_t request;
    memset(&request, 0, sizeof(HAP_power_request_t));
    request.type = HAP_power_set_apptype;
    request.apptype = HAP_POWER_COMPUTE_CLIENT_CLASS;
    if ( (err = HAP_power_set(NULL, &request)) != 0 ) {
        return err;
    }

    // Set to turbo and disable DCVS
    memset(&request, 0, sizeof(HAP_power_request_t));
    request.type = HAP_power_set_DCVS_v2;
    request.dcvs_v2.dcvs_enable = FALSE;
    request.dcvs_v2.set_dcvs_params = TRUE;
    request.dcvs_v2.dcvs_params.min_corner = HAP_DCVS_VCORNER_DISABLE;
    request.dcvs_v2.dcvs_params.max_corner = HAP_DCVS_VCORNER_DISABLE;
    request.dcvs_v2.dcvs_params.target_corner = HAP_DCVS_VCORNER_TURBO;
    request.dcvs_v2.set_latency = TRUE;
    request.dcvs_v2.latency = 100;
    return HAP_power_set(NULL, &request);
}
