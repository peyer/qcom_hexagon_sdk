/**=============================================================================
Copyright (c) 2015,2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

/* Tests for asyncdspq */

#include "queuetest.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#include <assert.h>
#include <malloc.h>
#include <unistd.h>
#include "rpcmem.h" // helper API's for shared buffer allocation
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "asyncdspq.h"


#define BUFLEN (5*4)
#define LOOPS 100000

#define DEBUGPRINT(x, ...)
//#define DEBUGPRINT(x, ...) printf(x, ##__VA_ARGS__)

//#define QUEUE_SIZE 4096
//#define QUEUE_ALIGN 4096
#define QUEUE_SIZE 256
#define QUEUE_ALIGN 256


static void usage(void) {
    fprintf(stderr," Usage: queuetest [test loops]\n");
    exit(EXIT_FAILURE);
}


unsigned long long GetTime( void ) {
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}


void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void error_code(char *msg, int code) {
    fprintf(stderr, "Error: %s (%d)\n", msg, code);
    exit(EXIT_FAILURE);
}


void error_code_fail(const char *file, int line, const char *expr, int code) {
    fprintf(stderr, "%s(%d): %s failed with %d\n", file, line, expr, code);
    exit(EXIT_FAILURE);
}

#define TEST_ERROR_CODE(x) { int e; if ( (e = x) != 0 ) { error_code_fail(__FILE__, __LINE__, #x, e); } }


#define NUMADDS 1000000
//#define NUMADDS 1000

static void error_callback(asyncdspq_t queue, void *context, AEEResult error) {
    error_code("error_callback", error);
}

typedef struct {
    asyncdspq_t *queue;
} asyncdspq_thread_args_t;


static void *asyncdspq_add_write_thread(void *data) {
    int err;
    asyncdspq_thread_args_t *a = (asyncdspq_thread_args_t*) data;
    int i;
    for ( i = 0; i < NUMADDS; i++ ) {
        // Add
        DEBUGPRINT("Write: add %d\n", i);
        uint32_t req[3];
        req[0] = 1;
        req[1] = i;
        req[2] = 65536 + i;
        if ( (err = asyncdspq_write(a->queue, (uint8_t*)req, sizeof(req))) != 0 ) {
            error_code("asyncdspq_write failed", err);
        }
    }

    // Exit
    {
        uint32_t req[1];
        req[0] = 2;
        if ( (err = asyncdspq_write(a->queue, (uint8_t*)req, sizeof(req))) != 0 ) {
            error_code("asyncdspq_write failed", err);
        }
    }
    
    return NULL;
}


static void *asyncdspq_add_read_thread(void *data) {
    int err;
    asyncdspq_thread_args_t *a = (asyncdspq_thread_args_t*) data;
    int i = 0;
    uint32_t lastsum = 0;

    while ( 1 ) {
        // Add
        uint32_t resp[16];
        uint32_t resplen;
        DEBUGPRINT("Read: Getting\n");
        if ( (err = asyncdspq_read(a->queue, (uint8_t*)resp, sizeof(resp), &resplen)) != 0 ) {
            error_code("asyncdspq_read_resp failed", err);
        }
        if ( resplen < 4 ) {
            error("Bad response length");
        }
        if ( resp[0] == 1 ) {
            // Sum
            if ( resplen != 8 ) {
                error("Bad response length for sum");
            }
            lastsum = resp[1];
            if ( lastsum != (2*i + 65536) ) {
                fprintf(stderr, "%d: %u %u\n", i, resp[0], resp[1]);
                error("Sum mismatch");
            }
            DEBUGPRINT("Read: Got sum %d\n", i);
            i++;
        } else if ( resp[0] == 2 ) {
            DEBUGPRINT("Read: Got exit\n");
            // Exit
            if ( resplen != 8 ) {
                error("Bad response length for exit");
            }
            if ( resp[1] != 0 ) {
                error_code("Exit response with an error", resp[1]);
            }
            return (void*) ((uintptr_t) lastsum);
        } else {
            fprintf(stderr, "Read: Got unknown response %u\n", resp[1]);            
        }
    }
        
    return NULL;
}


static int adder_write_i;


static void adder_space_callback(asyncdspq_t queue, void *context) {

    int err;

    while ( adder_write_i < NUMADDS ) {
        uint32_t req[3];
        req[0] = 1;
        req[1] = adder_write_i;
        req[2] = 65536 + adder_write_i;
        DEBUGPRINT("adder_space_callback: Writing %i\n", adder_write_i);
        err = asyncdspq_write_noblock(queue, (uint8_t*)req, sizeof(req));
        if ( err == AEE_ENOMORE ) {
            DEBUGPRINT("adder_space_callback: full\n");
            return;
        }
        if ( err != 0 ) {
            error_code("adder_space_callback: asyncdspq_write_noblock failed", err);
        }
        adder_write_i++;
    }

    if ( adder_write_i == NUMADDS ) {
        uint32_t req[1];
        req[0] = 2;
        DEBUGPRINT("adder_space_callback: Writing exit\n");
        err = asyncdspq_write_noblock(queue, (uint8_t*)req, sizeof(req));
        if ( err == AEE_ENOMORE ) {
            DEBUGPRINT("adder_space_callback: full\n");
            return;
        }
        if ( err != 0 ) {
            error_code("adder_space_callback: asyncdspq_write_noblock for exit failed", err);
        }
        adder_write_i++;
    }    
}


static uint32_t adder_lastsum;
static int adder_got_exit;
static int adder_read_i;
static int adder_have_sem;
static sem_t adder_exit_sem;

static void adder_message_callback(asyncdspq_t queue, void *context) {
    int err;
    uint32_t resp[16];
    uint32_t resplen;

    while ( 1 ) {
        DEBUGPRINT("adder_message_callback: Reading\n");
        err = asyncdspq_read_noblock(queue, (uint8_t*)resp, sizeof(resp), &resplen);
        if ( err == AEE_ENOMORE ) {
            DEBUGPRINT("adder_message_callback: Empty\n");
            return;
        }
        if ( err != 0 ) {
            error_code("adder_message_callback: asyncdspq_read_noblock failed", err);
        }
        if ( resplen < 4 ) {
            error("Bad response length");
        }
        DEBUGPRINT("adder_message_callback: Got %u\n", resp[0]);
        if ( resp[0] == 1 ) {
            // Sum
            if ( resplen != 8 ) {
                error("Bad response length for sum");
            }
            adder_lastsum = resp[1];
            if ( adder_lastsum != (2*adder_read_i + 65536) ) {
                fprintf(stderr, "%d: %u %u\n", adder_read_i, resp[0], resp[1]);
                error("Sum mismatch");
            }
            adder_read_i++;
        } else if ( resp[0] == 2 ) {
            // Exit
            if ( resplen != 8 ) {
                error("Bad response length for exit");
            }
            if ( resp[1] != 0 ) {
                error_code("Exit response with an error", resp[1]);
            }
            if ( adder_got_exit ) {
                error("Multiple exits");
            }
            adder_got_exit = 1;
            if ( adder_have_sem ) {
                TEST_ERROR_CODE(sem_post(&adder_exit_sem));
            }
            return;
        } else if ( resp[0] == 3 ) {
            // Error
            if ( resplen != 8 ) {
                error("Bad response length for error");
            }
            error_code("Received an error message", (int) resp[1]);
        } else {
            printf("Read: Got unknown response %u\n", resp[1]);            
        }
    }
}



extern int asyncdspq_read_waits;
extern int asyncdspq_write_waits;

void test_asyncdspq() {

    asyncdspq_t queue;
    asyncdspq_attach_handle_t attach_handle;
    asyncdspq_t resp_queue;
    asyncdspq_attach_handle_t resp_attach_handle;
    uint32_t msg[4];
    uint32_t lastmsg;
    unsigned msglen;
    int err;
    int i;
    unsigned t;
    uint64_t t1, t2;

    printf("\nasyncdspq: One message apps-DSP\n");
    err = asyncdspq_create(&queue, &attach_handle,
                          ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                          256, // Queue length
                          error_callback, NULL, NULL, // Callbacks
                          NULL, 0); // context, flags
    if ( err != 0 ) {
        error_code("asyncdspq_create failed", err);
    }
    printf("Queue created; attach handle 0x%08x\n", attach_handle);
    msg[0] = 1742;
    if ( (err = asyncdspq_write(queue, (const uint8_t*)msg, 4)) != 0 ) {
        error_code("asyncdspq_write failed", err);
    }
    if ( (err = queuetest_asyncdspq_test_reads(attach_handle, 1, &lastmsg)) != 0 ) {
        error_code("queuetest_test_reads failed", err);
    }
    if ( lastmsg != 1742) {
        error_code("Bad lastmsg", (int)lastmsg);
    }
    if ( (err = asyncdspq_destroy(queue)) != 0 ) {
        error_code("asyncdspq_destroy failed", err);
    }
    

    printf("\nasyncdspq: One message apps-DSP\n");
    err = asyncdspq_create(&queue, &attach_handle,
                          ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                          256, // Queue length
                          error_callback, NULL, NULL, // Callbacks
                          NULL, 0); // context, flags
    if ( err != 0 ) {
        error_code("asyncdspq_create failed", err);
    }
    printf("Queue created; attach handle 0x%08x\n", attach_handle);
    msg[0] = 1742;
    printf("Writing\n");
    if ( (err = asyncdspq_write(queue, (const uint8_t*)msg, 4)) != 0 ) {
        error_code("asyncdspq_write failed", err);
    }
    printf("Calling DSP\n");
    if ( (err = queuetest_asyncdspq_test_reads(attach_handle, 1, &lastmsg)) != 0 ) {
        error_code("queuetest_test_reads failed", err);
    }
    if ( lastmsg != 1742) {
        error_code("Bad lastmsg", (int)lastmsg);
    }
    if ( (err = asyncdspq_destroy(queue)) != 0 ) {
        error_code("asyncdspq_destroy failed", err);
    }
    
    printf("asyncdspq: One message DSP-apps\n");
    queue = NULL;
    attach_handle = 0;
    err = asyncdspq_create(&queue, &attach_handle,
                          ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU, // DSP to apps
                          256, // Queue length
                          error_callback, NULL, NULL, // Callbacks
                          NULL, 0); // context, flags
    if ( err != 0 ) {
        error_code("asyncdspq_create failed", err);
    }
    printf("Queue created; attach handle 0x%08x\n", attach_handle);
    if ( (err = queuetest_asyncdspq_test_writes(attach_handle, 1, 12765)) != 0 ) {
        error_code("queuetest_test_writes failed", err);
    }
    printf("Reading\n");
    if ( (err = asyncdspq_read(queue, (uint8_t*)msg, 4, &msglen)) != 0 ) {
        error_code("asyncdspq_read failed", err);
    }
    if ( msglen != 4 ) {
        error("Unexpected message length");
    }
    if ( msg[0] != 12765 ) {
        error_code("Bad message", (int)msg[0]);
    }
    printf("Destroy queue\n");
    if ( (err = asyncdspq_destroy(queue)) != 0 ) {
        error_code("asyncdspq_destroy failed", err);
    }
    
    printf("\nasyncdspq: Ten messages DSP-apps\n");
    queue = NULL;
    attach_handle = 0;
    err = asyncdspq_create(&queue, &attach_handle,
                          ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU, // DSP to apps
                          256, // Queue length
                          error_callback, NULL, NULL, // Callbacks
                          NULL, 0); // context, flags
    if ( err != 0 ) {
        error_code("asyncdspq_create failed", err);
    }
    printf("Queue created; attach handle 0x%08x\n", attach_handle);
    if ( (err = queuetest_asyncdspq_test_writes(attach_handle, 10, 12765)) != 0 ) {
        error_code("queuetest_test_writes failed", err);
    }
    printf("Reading\n");
    for ( i = 0; i < 10; i++ ) {
        if ( (err = asyncdspq_read(queue, (uint8_t*)msg, 4, &msglen)) != 0 ) {
            error_code("asyncdspq_read failed", err);
        }
        if ( msglen != 4 ) {
            error("Unexpected message length");
        }
        if ( msg[0] != 12765+i ) {
            error_code("Bad message", (int)msg[0]);
        }
    }
    if ( (err = asyncdspq_destroy(queue)) != 0 ) {
        error_code("asyncdspq_destroy failed", err);
    }


    printf("\nasyncdspq: Two queues - adder on DSP\n");
    asyncdspq_read_waits = 0;
    asyncdspq_write_waits = 0;
    queue = NULL;
    attach_handle = 0;
    resp_queue= NULL;
    resp_attach_handle = 0;
    err = asyncdspq_create(&queue, &attach_handle,
                          ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                          4096, error_callback, NULL, NULL, NULL, 0);
    if ( err != 0 ) {
        error_code("asyncdspq_create failed for Apps->DSP", err);
    }
    err = asyncdspq_create(&resp_queue, &resp_attach_handle,
                          ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                          4096, error_callback, NULL, NULL, NULL, 0);
    if ( err != 0 ) {
        error_code("asyncdspq_create failed for DSP->Apps", err);
    }
    printf("Queues created. Handles 0x%08x, 0x%08x. Starting adder on DSP\n",
           attach_handle, resp_attach_handle);

    t1 = GetTime();
    
    pthread_attr_t attr;
    pthread_t read_thread, write_thread;
    asyncdspq_thread_args_t read_args, write_args;
    write_args.queue = queue;
    read_args.queue = resp_queue;
    if ( (err = pthread_attr_init(&attr)) != 0 ) {
        error_code("pthread_attr_init failed", err);
    }
    if ( (err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0 ) {
        error_code("pthread_attr_setdetachstate failed", err);
    }
    if ( (err = pthread_create(&write_thread, &attr, asyncdspq_add_write_thread, &write_args)) != 0 ) {
        error_code("Writer pthread_create failed", err);
    }
    if ( (err = pthread_create(&read_thread, &attr, asyncdspq_add_read_thread, &read_args)) != 0 ) {
        error_code("Reader pthread_create failed", err);
    }

    if ( (err = queuetest_asyncdspq_adder_spin(attach_handle, resp_attach_handle)) != 0 ) {
        error_code("queuetest_asyncdspq_adder_spin failed", err);
    }

    void *ret;
    if ( (err = pthread_join(write_thread, &ret)) != 0 ) {
        error_code("Writer pthread_join failed", err);
    }
    if ( ret != NULL ) {
        error_code("Writer pthread failed", (uintptr_t)ret);
    }
    if ( (err = pthread_join(read_thread, &ret)) != 0 ) {
        error_code("Reader pthread_join failed", err);
    }
    uintptr_t lastsum = (uintptr_t) ret;
    if ( lastsum != (2 * (NUMADDS-1) + 65536) ) {
        error("Unexpected last sum");
    }

    t2 = GetTime();
    
    t = (unsigned) (t2 - t1);
    printf("%d usec for %d loops - %u usec/operation\n", t, NUMADDS, t/NUMADDS);
    printf("asyncdspq_read_waits = %d\n", asyncdspq_read_waits);
    printf("asyncdspq_write_waits = %d\n", asyncdspq_write_waits);

    if ( (err = asyncdspq_destroy(queue)) != 0 ) {
        error_code("asyncdspq_destroy failed for request queue", err);
    }
    if ( (err = asyncdspq_destroy(resp_queue)) != 0 ) {
        error_code("asyncdspq_destroy failed for response queue", err);
    }        


    printf("\nasyncdspq: Two queues - adder on DSP. Using callbacks.\n");
    adder_lastsum = 0;
    adder_got_exit = 0;
    adder_read_i = 0;
    adder_write_i = 0;
    asyncdspq_read_waits = 0;
    asyncdspq_write_waits = 0;
    adder_have_sem = 0;
    
    queue = NULL;
    attach_handle = 0;
    resp_queue= NULL;
    resp_attach_handle = 0;
    err = asyncdspq_create(&queue, &attach_handle,
                          ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                          4096, error_callback, NULL, adder_space_callback, NULL, 0);
    if ( err != 0 ) {
        error_code("asyncdspq_create failed for Apps->DSP", err);
    }
    err = asyncdspq_create(&resp_queue, &resp_attach_handle,
                          ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                          4096, error_callback, adder_message_callback, NULL, NULL, 0);
    if ( err != 0 ) {
        error_code("asyncdspq_create failed for DSP->Apps", err);
    }

    t1 = GetTime();
    
    if ( (err = queuetest_asyncdspq_adder_spin(attach_handle, resp_attach_handle)) != 0 ) {
        error_code("queuetest_asyncdspq_adder_spin failed", err);
    }
    
    if ( (err = asyncdspq_destroy(queue)) != 0 ) {
        error_code("asyncdspq_destroy failed for request queue", err);
    }
    if ( (err = asyncdspq_destroy(resp_queue)) != 0 ) {
        error_code("asyncdspq_destroy failed for response queue", err);
    }        

    if ( !adder_got_exit ) {
        error("Didn't get exit response");
    }
    if ( adder_lastsum != (2 * (NUMADDS-1) + 65536) ) {
        error("Unexpected last sum");
    }

    t2 = GetTime();
    
    t = (unsigned) (t2 - t1);
    printf("%d usec for %d loops - %u usec/operation\n", t, NUMADDS, t/NUMADDS);
    printf("asyncdspq_read_waits = %d\n", asyncdspq_read_waits);
    printf("asyncdspq_write_waits = %d\n", asyncdspq_write_waits);



    printf("\nasyncdspq: Two queues - adder on DSP. Message callback on DSP, both on apps\n");
    adder_lastsum = 0;
    adder_got_exit = 0;
    adder_read_i = 0;
    adder_write_i = 0;
    asyncdspq_read_waits = 0;
    asyncdspq_write_waits = 0;
    adder_have_sem = 1;
    TEST_ERROR_CODE(sem_init(&adder_exit_sem, 0, 0));
    
    queue = NULL;
    attach_handle = 0;
    resp_queue = NULL;
    resp_attach_handle = 0;
    TEST_ERROR_CODE(asyncdspq_create(&queue, &attach_handle,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                                    4096, error_callback, NULL, adder_space_callback, NULL, 0));
    TEST_ERROR_CODE(asyncdspq_create(&resp_queue, &resp_attach_handle,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, adder_message_callback, NULL, NULL, 0));

    t1 = GetTime();

    TEST_ERROR_CODE(queuetest_asyncdspq_adder_message_callbacks_start(
                        attach_handle, resp_attach_handle));
    TEST_ERROR_CODE(sem_wait(&adder_exit_sem));
    TEST_ERROR_CODE(queuetest_asyncdspq_adder_message_callbacks_stop());

    TEST_ERROR_CODE(asyncdspq_destroy(queue));
    TEST_ERROR_CODE(asyncdspq_destroy(resp_queue));

    if ( !adder_got_exit ) {
        error("Didn't get exit response");
    }
    if ( adder_lastsum != (2 * (NUMADDS-1) + 65536) ) {
        error("Unexpected last sum");
    }

    t2 = GetTime();
    
    t = (unsigned) (t2 - t1);
    printf("%d usec for %d loops - %u usec/operation\n", t, NUMADDS, t/NUMADDS);
    printf("asyncdspq_read_waits = %d\n", asyncdspq_read_waits);
    printf("asyncdspq_write_waits = %d\n", asyncdspq_write_waits);

    sem_destroy(&adder_exit_sem);
    adder_have_sem = 0;

    
    printf("\nasyncdspq: DSP-side space callback\n");
    queue = NULL;
    attach_handle = 0;
    TEST_ERROR_CODE(asyncdspq_create(&queue, &attach_handle,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU, // DSP to apps
                                    4096, // Queue length
                                    error_callback, NULL, NULL, // Callbacks
                                    NULL, 0));
    t1 = GetTime();
    TEST_ERROR_CODE(queuetest_asyncdspq_space_callback_start(attach_handle, NUMADDS));
    printf("Reading messages\n");
    uint32_t c = NUMADDS;
    while ( c ) {
        TEST_ERROR_CODE(asyncdspq_read(queue, (uint8_t*)msg, sizeof(msg), &msglen));
        if ( msglen != 4 ) {
            error("Unexpected message length");
        }
        if ( msg[0] != c ) {
            error_code("Bad message", (int)msg[0]);
        }
        c--;
    }
    t2 = GetTime();
    TEST_ERROR_CODE(queuetest_asyncdspq_space_callback_stop());
    TEST_ERROR_CODE(asyncdspq_destroy(queue));
    t = (unsigned) (t2 - t1);
    printf("%d usec for %d loops - %u usec/operation\n", t, NUMADDS, t/NUMADDS);
}


#define NUMTHREADS 4
#define MULTIADDER_DSP_THREADS 3
//#define NUMTHREADS 1


static void *cancel_read_thread(void *arg) {

    asyncdspq_t queue = (asyncdspq_t) arg;
    uint32_t resp[16];
    uint32_t resplen;
    int err = 0;

    DEBUGPRINT("Reading...\n");
    err = asyncdspq_read(queue, (uint8_t*)resp, sizeof(resp), &resplen);
    DEBUGPRINT("... Got error %d\n", err);
    if ( err != AEE_EEXPIRED ) {
        error_code("cancel_read_thread: Unexpected error code", err);
    }
    return NULL;
}


static void *cancel_write_thread(void *arg) {

    asyncdspq_t queue = (asyncdspq_t) arg;
    uint32_t req[1];
    int err = 0;

    req[0] = 1;
    DEBUGPRINT("Writing...\n");
    err = asyncdspq_write(queue, (uint8_t*)req, 4);
    DEBUGPRINT("... Got error %d\n", err);
    if ( err != AEE_EEXPIRED ) {
        error_code("cancel_read_thread: Unexpected error code", err);
    }
    return NULL;
}


void test_asyncdspq_cancel() {

    asyncdspq_t q1;
    asyncdspq_attach_handle_t a1;
    pthread_t read_threads[NUMTHREADS];
    pthread_t write_threads[NUMTHREADS];
    pthread_attr_t attr;
    void *ret;
    int i;
    int err;
    uint8_t d[4];
    unsigned l;

    printf("\nasyncdspq: Cancel reads on apps\n");
    
    q1 = NULL;
    a1 = 0;
    TEST_ERROR_CODE(asyncdspq_create(&q1, &a1,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_attr_init(&attr));
        TEST_ERROR_CODE(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        TEST_ERROR_CODE(pthread_create(&read_threads[i], &attr, cancel_read_thread, q1));
    }
    usleep(150000);
    printf("Canceling\n");
    TEST_ERROR_CODE(asyncdspq_cancel(q1));
    printf("Joining threads\n");
    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_join(read_threads[i], &ret));
        DEBUGPRINT("Joined %d\n", i);
        if ( ret != NULL ) {
            error_code("Cancel read thread failed", (uintptr_t)ret);
        }
    }
    TEST_ERROR_CODE(asyncdspq_destroy(q1));

    
    printf("\nasyncdspq: Cancel reads on apps #2\n");
    
    q1 = NULL;
    a1 = 0;
    TEST_ERROR_CODE(asyncdspq_create(&q1, &a1,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    printf("Cancel before read\n");   
    TEST_ERROR_CODE(asyncdspq_cancel(q1));
    printf("asyncdspq_read()\n");
    err = asyncdspq_read(q1, d, sizeof(d), &l);
    if ( err != AEE_EEXPIRED ) {
        error_code("Unexpected return value", err);
    }
    TEST_ERROR_CODE(asyncdspq_destroy(q1));


    printf("\nasyncdspq: Cancel writes on apps\n");
    
    q1 = NULL;
    a1 = 0;
    TEST_ERROR_CODE(asyncdspq_create(&q1, &a1,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    printf("Filling queue\n");
    while ( 1 ) {
        uint32_t req = 0;
        int err = asyncdspq_write_noblock(q1, (uint8_t*)&req, 4);
        if ( err == AEE_ENOMORE ) {
            break;
        }
        if ( err != AEE_SUCCESS ) {
            error_code("asyncdspq_write_noblock failed", err);
        }
    }
    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_attr_init(&attr));
        TEST_ERROR_CODE(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        TEST_ERROR_CODE(pthread_create(&write_threads[i], &attr, cancel_write_thread, q1));
    }
    usleep(50000);
    printf("Canceling\n");
    TEST_ERROR_CODE(asyncdspq_cancel(q1));
    printf("Joining threads\n");
    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_join(write_threads[i], &ret));
        DEBUGPRINT("Joined %d\n", i);
        if ( ret != NULL ) {
            error_code("Cancel write thread failed", (uintptr_t)ret);
        }
    }
    TEST_ERROR_CODE(asyncdspq_destroy(q1));

    
    printf("\nasyncdspq: Cancel writes on apps #2\n");
    
    q1 = NULL;
    a1 = 0;
    TEST_ERROR_CODE(asyncdspq_create(&q1, &a1,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    printf("Filling queue\n");
    while ( 1 ) {
        uint32_t req = 0;
        int err = asyncdspq_write_noblock(q1, (uint8_t*)&req, 4);
        if ( err == AEE_ENOMORE ) {
            break;
        }
        if ( err != AEE_SUCCESS ) {
            error_code("asyncdspq_write_noblock failed", err);
        }
    }
    printf("Canceling before write\n");
    TEST_ERROR_CODE(asyncdspq_cancel(q1));
    printf("asyncdspq_write()\n");
    if ( err != AEE_EEXPIRED ) {
        error_code("Unexpected return value", err);
    }
    TEST_ERROR_CODE(asyncdspq_destroy(q1));

    
    printf("\nasyncdspq: Cancel reads on DSP - cancel before read\n");
    
    q1 = NULL;
    a1 = 0;
    TEST_ERROR_CODE(asyncdspq_create(&q1, &a1,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    TEST_ERROR_CODE(queuetest_asyncdspq_test_cancel_read(a1));
    TEST_ERROR_CODE(asyncdspq_destroy(q1));

    
    printf("\nasyncdspq: Cancel writes on DSP - cancel before write\n");
    
    q1 = NULL;
    a1 = 0;
    TEST_ERROR_CODE(asyncdspq_create(&q1, &a1,
                                     ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    TEST_ERROR_CODE(queuetest_asyncdspq_test_cancel_write(a1));
    TEST_ERROR_CODE(asyncdspq_destroy(q1));
}



static void *multiadder_read_thread(void *arg) {

    asyncdspq_t queue;
    uint32_t resp[16];
    uint32_t resplen;
    uint32_t index[NUMTHREADS];
    int i;
    uint32_t x;

    queue = (asyncdspq_t) arg;

    for ( i = 0; i < NUMTHREADS; i++ ) {
        index[i] = 0;
    }

    while ( 1 ) {
        TEST_ERROR_CODE(asyncdspq_read(queue, (uint8_t*)resp, sizeof(resp), &resplen));
        if ( resplen < 4 ) {
            error("Bad response length");
        }
        DEBUGPRINT("Got resp %u len %u\n", resp[0], resplen);
        if ( resp[0] == 1 ) {
            if ( resplen != 12 ) {
                error("Bad multisum response length");
            }
            i = (int) resp[2];
            if ( i >= NUMTHREADS ) {
                error("Bad thread ID");
            }
            adder_lastsum = resp[1];
            if ( adder_lastsum != (2*index[i] + 65536) ) {
                fprintf(stderr, "%d: %u %u\n", i, resp[1], index[i]);
                error("Sum mismatch");
            }
            index[i]++;
        } else if ( resp[0] == 2 ) {
            // Exit
            if ( resplen != 8 ) {
                error("Bad response length for exit");
            }
            if ( resp[1] != 0 ) {
                error_code("Exit response with an error", resp[1]);
            }

            x = index[0];
            for ( i = 0; i < NUMTHREADS; i++ ) {
                if ( index[i] != x ) {
                    fprintf(stderr, "Index mixmatch at %d: %u != %u\n", i, index[i], x);
                    error("Index mismatch");
                }
            }
            
            return NULL;
        } else if ( resp[0] == 3 ) {
            // Error
            if ( resplen != 8 ) {
                error("Bad response length for error");
            }
            error_code("Received an error message", (int) resp[1]);
        } else {
            fprintf(stderr, "Unknown response %u\n", resp[1]);
            error("Bad response");
        }
    }
}


typedef struct {
    asyncdspq_t queue;
    int id;
} multiadder_write_thread_arg_t;


static void *multiadder_write_thread(void *arg) {
    int i;
    multiadder_write_thread_arg_t *a = (multiadder_write_thread_arg_t*) arg;
    for ( i = 0; i < NUMADDS; i++ ) {
        uint32_t req[4];
        req[0] = 1;
        req[1] = i;
        req[2] = 65536 + i;
        req[3] = a->id;
        DEBUGPRINT("%i writing %u + %u\n", a->id, i, 65536+i);
        TEST_ERROR_CODE(asyncdspq_write(a->queue, (uint8_t*)req, sizeof(req)));
    }
    DEBUGPRINT("%i done\n", a->id);
    return NULL;
}



static void *multiadder_read_ooo_thread(void *arg) {

    asyncdspq_t queue;
    uint32_t resp[16];
    uint32_t resplen;
    uint32_t index[NUMTHREADS];
    int i;
    uint32_t x;
    int exitcount = 0;

    queue = (asyncdspq_t) arg;

    for ( i = 0; i < NUMTHREADS; i++ ) {
        index[i] = 0;
    }

    while ( 1 ) {
        TEST_ERROR_CODE(asyncdspq_read(queue, (uint8_t*)resp, sizeof(resp), &resplen));
        if ( resplen < 4 ) {
            error("Bad response length");
        }
        DEBUGPRINT("Got resp %u len %u\n", resp[0], resplen);
        if ( resp[0] == 1 ) {
            if ( resplen != 12 ) {
                error("Bad multisum response length");
            }
            i = (int) resp[2];
            if ( i >= NUMTHREADS ) {
                error("Bad thread ID");
            }
            index[i]++;
        } else if ( resp[0] == 2 ) {
            // Exit
            if ( resplen != 8 ) {
                error("Bad response length for exit");
            }
            if ( resp[1] != 0 ) {
                error_code("Exit response with an error", resp[1]);
            }
            exitcount++;
            printf("Got %d exits\n", exitcount);
            if ( exitcount < MULTIADDER_DSP_THREADS ) {
                continue;
            }

            x = index[0];
            if ( x != NUMADDS ) {
                error_code("Unexpected number of results", x);
            }
            for ( i = 0; i < NUMTHREADS; i++ ) {
                if ( index[i] != x ) {
                    fprintf(stderr, "Index mixmatch at %d: %u != %u\n", i, index[i], x);
                    error("Index mismatch");
                }
            }
            
            return NULL;
        } else if ( resp[0] == 3 ) {
            // Error
            if ( resplen != 8 ) {
                error("Bad response length for error");
            }
            error_code("Received an error message", (int) resp[1]);
        } else {
            fprintf(stderr, "Unknown response %u\n", resp[1]);
            error("Bad response");
        }
    }
}



void test_asyncdspq_threads() {

    asyncdspq_t queue;
    asyncdspq_attach_handle_t attach_handle;
    asyncdspq_t resp_queue;
    asyncdspq_attach_handle_t resp_attach_handle;
    int i;
    unsigned t;
    uint64_t t1, t2;
    pthread_t threads[NUMTHREADS];
    pthread_t read_thread;
    pthread_attr_t attr;
    multiadder_write_thread_arg_t writearg[NUMTHREADS];
    void *ret;
    uint32_t exitreq;

    printf("\nasyncdspq: Multiple apps writer threads\n");
    adder_lastsum = 0;
    adder_got_exit = 0;
    adder_read_i = 0;
    adder_write_i = 0;
    asyncdspq_read_waits = 0;
    asyncdspq_write_waits = 0;
    
    queue = NULL;
    attach_handle = 0;
    resp_queue = NULL;
    resp_attach_handle = 0;
    TEST_ERROR_CODE(asyncdspq_create(&queue, &attach_handle,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                                    4096, error_callback, NULL, NULL, NULL, 0));
    TEST_ERROR_CODE(asyncdspq_create(&resp_queue, &resp_attach_handle,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, NULL, NULL, NULL, 0));

    t1 = GetTime();

    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_attr_init(&attr));
        TEST_ERROR_CODE(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        writearg[i].queue = queue;
        writearg[i].id = i;
        TEST_ERROR_CODE(pthread_create(&threads[i], &attr, multiadder_write_thread, &writearg[i]));
    }

    TEST_ERROR_CODE(pthread_attr_init(&attr));
    TEST_ERROR_CODE(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
    TEST_ERROR_CODE(pthread_create(&read_thread, &attr, multiadder_read_thread, resp_queue));

    TEST_ERROR_CODE(queuetest_asyncdspq_multiadder_message_callbacks_start(attach_handle, resp_attach_handle));

    printf("Joining writer threads\n");
    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_join(threads[i], &ret));
        if ( ret != NULL ) {
            error_code("Multiadder writer thread failed", (uintptr_t)ret);
        }
    }

    printf("Writing exit request\n");
    exitreq = 2;
    TEST_ERROR_CODE(asyncdspq_write(queue, (uint8_t*)&exitreq, 4));

    printf("Joining read thread\n");
    TEST_ERROR_CODE(pthread_join(read_thread, &ret));
    if ( ret != NULL ) {
        error_code("Multiadder read thread failed", (uintptr_t)ret);
    }

    TEST_ERROR_CODE(queuetest_asyncdspq_multiadder_message_callbacks_stop());

    t2 = GetTime();
    TEST_ERROR_CODE(asyncdspq_destroy(queue));
    TEST_ERROR_CODE(asyncdspq_destroy(resp_queue));
    
    t = (unsigned) (t2 - t1);
    printf("%d usec for %d loops - %u usec/operation\n", t, NUMTHREADS*NUMADDS, t/(NUMTHREADS*NUMADDS));
    printf("asyncdspq_read_waits = %d\n", asyncdspq_read_waits);
    printf("asyncdspq_write_waits = %d\n", asyncdspq_write_waits);

    
    printf("\nasyncdspq: Multiple thread: Apps read, DSP read/write\n");
    adder_lastsum = 0;
    adder_got_exit = 0;
    adder_read_i = 0;
    adder_write_i = 0;
    asyncdspq_read_waits = 0;
    asyncdspq_write_waits = 0;
    
    queue = NULL;
    attach_handle = 0;
    resp_queue = NULL;
    resp_attach_handle = 0;
    TEST_ERROR_CODE(asyncdspq_create(&queue, &attach_handle,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                                    4096, error_callback, NULL, NULL, NULL, 0));
    TEST_ERROR_CODE(asyncdspq_create(&resp_queue, &resp_attach_handle,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, NULL, NULL, NULL, 0));

    t1 = GetTime();

    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_attr_init(&attr));
        TEST_ERROR_CODE(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
        writearg[i].queue = queue;
        writearg[i].id = i;
        TEST_ERROR_CODE(pthread_create(&threads[i], &attr, multiadder_write_thread, &writearg[i]));
    }

    TEST_ERROR_CODE(pthread_attr_init(&attr));
    TEST_ERROR_CODE(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));
    TEST_ERROR_CODE(pthread_create(&read_thread, &attr, multiadder_read_ooo_thread, resp_queue));

    TEST_ERROR_CODE(queuetest_asyncdspq_multiadder_threads_start(attach_handle, resp_attach_handle,
                                                                MULTIADDER_DSP_THREADS));

    printf("Joining writer threads\n");
    for ( i = 0; i < NUMTHREADS; i++ ) {
        TEST_ERROR_CODE(pthread_join(threads[i], &ret));
        if ( ret != NULL ) {
            error_code("Multiadder writer thread failed", (uintptr_t)ret);
        }
    }

    printf("Writing %d exit requests\n", MULTIADDER_DSP_THREADS);
    for ( i = 0; i < MULTIADDER_DSP_THREADS; i++ ) {
        exitreq = 2;
        TEST_ERROR_CODE(asyncdspq_write(queue, (uint8_t*)&exitreq, 4));
    }

    printf("Joining read thread\n");
    TEST_ERROR_CODE(pthread_join(read_thread, &ret));
    if ( ret != NULL ) {
        error_code("Multiadder read thread failed", (uintptr_t)ret);
    }

    printf("Stopping DSP threads\n");
    TEST_ERROR_CODE(queuetest_asyncdspq_multiadder_threads_stop());
    t2 = GetTime();
    
    TEST_ERROR_CODE(asyncdspq_destroy(queue));
    TEST_ERROR_CODE(asyncdspq_destroy(resp_queue));
    
    t = (unsigned) (t2 - t1);
    printf("%d usec for %d loops - %u usec/operation\n", t, NUMTHREADS*NUMADDS, t/(NUMTHREADS*NUMADDS));
    printf("asyncdspq_read_waits = %d\n", asyncdspq_read_waits);
    printf("asyncdspq_write_waits = %d\n", asyncdspq_write_waits);

    
    printf("\nDSP Threads - Testing for cancel\n");
    queue = NULL;
    attach_handle = 0;
    resp_queue = NULL;
    resp_attach_handle = 0;
    TEST_ERROR_CODE(asyncdspq_create(&queue, &attach_handle,
                                    ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                                    4096, error_callback, NULL, NULL, NULL, 0));
    TEST_ERROR_CODE(asyncdspq_create(&resp_queue, &resp_attach_handle,
                                    ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                                    4096, error_callback, NULL, NULL, NULL, 0));
    TEST_ERROR_CODE(queuetest_asyncdspq_multiadder_threads_start(attach_handle, resp_attach_handle,
                                                                MULTIADDER_DSP_THREADS));
    printf("queuetest_asyncdspq_multiadder_threads_stop()\n");
    TEST_ERROR_CODE(queuetest_asyncdspq_multiadder_threads_stop());
    TEST_ERROR_CODE(asyncdspq_destroy(queue));
    TEST_ERROR_CODE(asyncdspq_destroy(resp_queue));
}


int main(int argc, char* argv[])
{
    int testloops = 1;

    if ( argc > 1 ) {
        if ( argc > 2 ) {
            usage();
        }
        testloops = atoi(argv[1]);
        if ( testloops < 1 ) {
            usage();
        }
    }
    setbuf(stdout,NULL);
	setbuf(stderr,NULL);
    rpcmem_init();
    TEST_ERROR_CODE(queuetest_set_clocks());
    queuetest_enable_logging();

    printf("%d test loops\n", testloops);
    while ( testloops-- ) {
        printf("\n------------\n%d\n", testloops);
        test_asyncdspq();
        test_asyncdspq_cancel();
        test_asyncdspq_threads();
    }
    
    
    rpcmem_deinit();

    printf("Done\n");
    return 0;
}
