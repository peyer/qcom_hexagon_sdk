/**=============================================================================
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

/* Performance Tests for asyncdspq */

#include "queuetest.h"
#include "dspCV.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include "rpcmem.h" // helper API's for shared buffer allocation

#include "asyncdspq.h"


#define DEBUGPRINT(x, ...)
//#define DEBUGPRINT(x, ...) printf(x, ##__VA_ARGS__)

#define QUEUE_SIZE 256
#define QUEUE_ALIGN 256


unsigned long long GetTime(void)
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

void error(char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void error_code(char *msg, int code)
{
    fprintf(stderr, "Error: %s (%d)\n", msg, code);
    exit(EXIT_FAILURE);
}

void error_code_fail(const char *file, int line, const char *expr, int code)
{
    fprintf(stderr, "%s(%d): %s failed with %d\n", file, line, expr, code);
    exit(EXIT_FAILURE);
}

#define TEST_ERROR_CODE(x) { int e; if ( (e = x) != 0 ) { error_code_fail(__FILE__, __LINE__, #x, e); } }


#define NUMADDS 100000

static void error_callback(asyncdspq_t queue, void *context, AEEResult error)
{
    error_code("error_callback", error);
}

typedef struct {
    asyncdspq_t *queue;
    int reqBufSize;
} asyncdspq_thread_args_t;

static void* asyncdspq_add_write_thread(void *data)
{
    int err;
    asyncdspq_thread_args_t *a = (asyncdspq_thread_args_t *)data;
    int i;
    for (i = 0; i < NUMADDS; i++) {
        // Add
        DEBUGPRINT("Write: add %d\n", i);
        uint32_t *req = malloc(a->reqBufSize);
        req[0] = 1;
        req[1] = i;
        req[2] = 65536 + i;
        if ((err = asyncdspq_write(a->queue, (uint8_t *)req, a->reqBufSize)) != 0) {
            free(req);
            error_code("asyncdspq_write failed", err);
        }
        free(req);
    }

    // Exit
    {
        uint32_t req[1];
        req[0] = 2;
        if ((err = asyncdspq_write(a->queue, (uint8_t *)req, sizeof(req))) != 0) {
            error_code("asyncdspq_write failed", err);
        }
    }

    return NULL;
}


static void* asyncdspq_add_read_thread(void *data)
{
    int err;
    asyncdspq_thread_args_t *a = (asyncdspq_thread_args_t *)data;
    int i = 0;
    uint32_t lastsum = 0;

    while (1) {
        // Add
        uint32_t resp[16];
        uint32_t resplen;
        DEBUGPRINT("Read: Getting\n");
        if ((err = asyncdspq_read(a->queue, (uint8_t *)resp, sizeof(resp), &resplen)) != 0) {
            error_code("asyncdspq_read_resp failed", err);
        }
        if (resplen < 4) {
            error("Bad response length");
        }
        if (resp[0] == 1) {
            // Sum
            if (resplen != 8) {
                error("Bad response length for sum");
            }
            lastsum = resp[1];
            if (lastsum != (2 * i + 65536)) {
                fprintf(stderr, "%d: %u %u\n", i, resp[0], resp[1]);
                error("Sum mismatch");
            }
            DEBUGPRINT("Read: Got sum %d\n", i);
            i++;
        } else if (resp[0] == 2) {
            DEBUGPRINT("Read: Got exit\n");
            // Exit
            if (resplen != 8) {
                error("Bad response length for exit");
            }
            if (resp[1] != 0) {
                error_code("Exit response with an error", resp[1]);
            }
            return (void *)((uintptr_t)lastsum);
        } else {
            fprintf(stderr, "Read: Got unknown response %u\n", resp[1]);
        }
    }

    return NULL;
}


extern int asyncdspq_read_waits;
extern int asyncdspq_write_waits;

int test_asyncdspq()
{
    asyncdspq_t queue;
    asyncdspq_attach_handle_t attach_handle;
    asyncdspq_t resp_queue;
    asyncdspq_attach_handle_t resp_attach_handle;
    int err = AEE_SUCCESS;
    unsigned t;
    uint64_t t1, t2;
    int sizes[][2] = {
        { 12, 256 }, //{request buffer size, queue size}
        { 12, 512 },
        { 12, 1024 },
        { 12, 2048 },
        { 12, 4096 },
        { 244, 256 },
        { 244, 512 },
        { 244, 1024 },
        { 244, 2048 },
        { 244, 4096 }
    };

    printf("\nqueueperf: Creating queues and performing %d operations on DSP\n", NUMADDS);
    printf("This can take a few minutes. Please wait...\n\n");
    printf("%25s %25s %25s %25s %25s\n",
           "Request buf size (Bytes)",
           "Queue size (Bytes)",
           "Average Time (usec)",
           "Read queue waits",
           "Write queue waits"
           );
    printf("---------------------------");
    printf("---------------------------");
    printf("---------------------------");
    printf("---------------------------");
    printf("---------------------------\n");
    for (int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        asyncdspq_read_waits = 0;
        asyncdspq_write_waits = 0;
        queue = NULL;
        attach_handle = 0;
        resp_queue = NULL;
        resp_attach_handle = 0;
        err = asyncdspq_create(&queue, &attach_handle,
                              ASYNCDSPQ_ENDPOINT_APP_CPU, ASYNCDSPQ_ENDPOINT_CDSP, // apps to DSP
                              sizes[i][1], error_callback, NULL, NULL, NULL, 0);
        if (err != 0) {
            fprintf(stderr, "Error: asyncdspq_create failed for Apps->DSP (%d)\n", err);
            goto error;
        }
        err = asyncdspq_create(&resp_queue, &resp_attach_handle,
                              ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                              sizes[i][1], error_callback, NULL, NULL, NULL, 0);
        if (err != 0) {
            fprintf(stderr, "Error: asyncdspq_create failed for DSP->Apps (%d)\n", err);
            goto error_destroy_req;
        }
        DEBUGPRINT("Queues created. Handles 0x%08x, 0x%08x. Starting adder on DSP\n",
                   attach_handle, resp_attach_handle);

        t1 = GetTime();

        pthread_attr_t attr;
        pthread_t read_thread, write_thread;
        asyncdspq_thread_args_t read_args, write_args;
        write_args.queue = queue;
        write_args.reqBufSize = sizes[i][0];
        read_args.queue = resp_queue;
        if ((err = pthread_attr_init(&attr)) != 0) {
            error_code("pthread_attr_init failed", err);
            goto error_destroy_resp;
        }
        if ((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE)) != 0) {
            fprintf(stderr, "Error: pthread_attr_setdetachstate failed (%d)\n", err);
            goto error_destroy_resp;
        }
        if ((err = pthread_create(&write_thread, &attr, asyncdspq_add_write_thread, &write_args)) != 0) {
            fprintf(stderr, "Error: Writer pthread_create failed (%d)\n", err);
            goto error_destroy_resp;
        }
        if ((err = pthread_create(&read_thread, &attr, asyncdspq_add_read_thread, &read_args)) != 0) {
            fprintf(stderr, "Error: Reader pthread_create failed (%d)\n", err);
            goto error_destroy_resp;
        }

        if ((err = queuetest_asyncdspq_adder_spin(attach_handle, resp_attach_handle)) != 0) {
            fprintf(stderr, "Error: queuetest_asyncdspq_adder_spin failed (%d)\n", err);
            goto error_destroy_resp;
        }

        void *ret;
        if ((err = pthread_join(write_thread, &ret)) != 0) {
            fprintf(stderr, "Error: Writer pthread_join failed (%d)\n", err);
            goto error_destroy_resp;
        }
        if (ret != NULL) {
            fprintf(stderr, "Error: Writer pthread join failed (%d)\n", err);
            goto error_destroy_resp;
        }
        if ((err = pthread_join(read_thread, &ret)) != 0) {
            fprintf(stderr, "Error: Reader pthread_join failed (%d)\n", err);
            goto error_destroy_resp;
        }
        uintptr_t lastsum = (uintptr_t)ret;
        if (lastsum != (2 * (NUMADDS - 1) + 65536)) {
            fprintf(stderr, "Unexpected last sum\n");
            goto error_destroy_resp;
        }

        t2 = GetTime();

        t = (unsigned)(t2 - t1);
        printf("%25d", sizes[i][0]);
        printf("%25d", sizes[i][1]);
        printf("%25d", t / NUMADDS);
        printf("%25d", asyncdspq_read_waits);
        printf("%25d\n", asyncdspq_write_waits);

    error_destroy_resp:
        if ((err = asyncdspq_destroy(resp_queue)) != 0) {
            fprintf(stderr, "Error: asyncdspq_destroy failed for response queue (%d)\n", err);
            break;
        }
    error_destroy_req:
        if ((err = asyncdspq_destroy(queue)) != 0) {
            fprintf(stderr, "Error: asyncdspq_destroy failed for request queue (%d)\n", err);
            break;
        }
    }

    printf("\n");
error:
    return err;
}


int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(stderr, " Usage: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

    rpcmem_init();
    TEST_ERROR_CODE(queuetest_set_clocks());
    queuetest_enable_logging();

    test_asyncdspq();

    rpcmem_deinit();

    printf("Done\n");
    return 0;
}
