/*
  Copyright (c) 2015,2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Example asynchronous FastCV-based image processing operation queue - test application */

#include "fcvqueue.h"
#include "fcvqueuetest.h"
#include <dspCV.h>
#include <AEEStdErr.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <rpcmem.h>
#include <semaphore.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>


/* Default configuration */
#define WIDTH 1920
#define HEIGHT 1080
#define STRIDE 1920


/* Error messages and helper macros */

static void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static void error_code_fail(const char *file, int line, const char *expr, int code) {
    fprintf(stderr, "%s(%d): %s failed with %d\n", file, line, expr, code);
    exit(EXIT_FAILURE);
}

static void check_fail(const char *file, int line, const char *expr) {
    fprintf(stderr, "%s(%d): %s failed\n", file, line, expr);
    exit(EXIT_FAILURE);
}

#define TEST_ERROR_CODE(x) { int e; if ( (e = x) != 0 ) { error_code_fail(__FILE__, __LINE__, #x, e); } }
#define CHECK(x) { if ( !(x) ) { check_fail(__FILE__, __LINE__, #x); } }


static void error_callback(fcvqueue_t queue, void *context, AEEResult error) {
    fprintf(stderr, "error_callback: %d\n", error);
    exit(EXIT_FAILURE);
}


static uint64_t time_usec(void) {

    struct timespec ts;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
    return ts.tv_sec*1000000ULL + ts.tv_nsec/1000;
}


/* Simple fcvqueue usage test */
static void test_simple(void) {

    fcvqueue_t queue;
    fcvqueue_buffer_t input_buf, output_buf;
    void *p;

    TEST_ERROR_CODE(fcvqueue_create(&queue, error_callback, NULL));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &input_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, input_buf, &p));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &output_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_destroy(queue));
}



static void sync_callback_post_sem(fcvqueue_t queue, void *context) {
    TEST_ERROR_CODE(sem_post((sem_t*) context));
}


/* Test copy */
static void test_sync(void) {

    fcvqueue_t queue;
    sem_t sync_sem;

    TEST_ERROR_CODE(sem_init(&sync_sem, 0, 0));

    TEST_ERROR_CODE(fcvqueue_create(&queue, error_callback, NULL));
    TEST_ERROR_CODE(fcvqueue_enqueue_sync(queue, sync_callback_post_sem, (void*) &sync_sem));
    TEST_ERROR_CODE(sem_wait(&sync_sem));
    TEST_ERROR_CODE(fcvqueue_sync(queue));
    TEST_ERROR_CODE(fcvqueue_destroy(queue));

    sem_destroy(&sync_sem);
}


/* Fill a buffer with pseudo-random data */
static void fill_random(void *buf, size_t size) {
    uint32_t m_w = 0x12349876;
    uint32_t m_z = 0xabcd4321;
    uint32_t *p = (uint32_t*) buf;
    size_t len = size / 4;
    assert((size & 3) == 0);
    while ( len-- ) {
        m_z = 36969 * (m_z & 65535) + (m_z >> 16);
        m_w = 18000 * (m_w & 65535) + (m_w >> 16);
        *p++ = m_w;
    }
}


/* Test copy */
static void test_copy(void) {

    fcvqueue_t queue;
    fcvqueue_buffer_t input_buf, output_buf;
    void *in, *out;
    size_t size = STRIDE * HEIGHT;

    /* Create queue and allocate buffers */
    TEST_ERROR_CODE(fcvqueue_create(&queue, error_callback, NULL));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &input_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, input_buf, &in));
    fill_random(in, size);
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &output_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, output_buf, &out));
    memset(out, 0, size);

    /* Pass buffers to the DSP, copy, synchronize output, sync, and check result */
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_COPY, input_buf, output_buf));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_sync(queue));
    if ( memcmp(in, out, size) != 0 ) {
        error("Copy output mismatch");
    }

    /* Cleanup. */
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_destroy(queue));
}


static AEEResult validate_dilate_gaussian_median(const uint8_t *in, const uint8_t *out) {

    uint8_t *t1, *t2, *ref;
    size_t size = STRIDE * HEIGHT;

    CHECK((t1 = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    CHECK((t2 = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    CHECK((ref = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    /* Note that FastCV is also available on the apps CPU in some environments, but
       not in the Hexagon SDK. */
    TEST_ERROR_CODE(fcvqueuetest_fastcv_dilate3x3(in, size, t1, size, WIDTH, HEIGHT, STRIDE));
    TEST_ERROR_CODE(fcvqueuetest_fastcv_gaussian3x3(t1, size, t2, size, WIDTH, HEIGHT, STRIDE));
    TEST_ERROR_CODE(fcvqueuetest_fastcv_median3x3(t2, size, ref, size, WIDTH, HEIGHT, STRIDE));
    if ( memcmp(ref, out, size) != 0 ) {
        error("dilate_gaussian_median comparison failed");
    }
    rpcmem_free(ref);
    rpcmem_free(t2);
    rpcmem_free(t1);
    return AEE_SUCCESS;
}


/* Test filters */
static void test_filters(void) {

    fcvqueue_t queue;
    fcvqueue_buffer_t input_buf, temp_buf_1, temp_buf_2, output_buf;
    void *in, *out;
    size_t size = STRIDE * HEIGHT;

    /* Create queue and allocate buffers */
    TEST_ERROR_CODE(fcvqueue_create(&queue, error_callback, NULL));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &input_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, input_buf, &in));
    fill_random(in, size);
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &output_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, output_buf, &out));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &temp_buf_2));

    /* Pass buffers to the DSP */
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, temp_buf_2));

    /* Operate on buffers: dilate->gaussian->median */
    TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_DILATE3X3, input_buf, temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_GAUSSIAN3X3, temp_buf_1, temp_buf_2));
    TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_MEDIAN3X3, temp_buf_2, output_buf));

    /* Get back and validate. Note that we don't need to do cache ops on the
       temporary buffers since we're not interested in their contents. */
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_sync(queue));
    TEST_ERROR_CODE(validate_dilate_gaussian_median(in, out));
        
    /* Cleanup. */
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, temp_buf_2));
    TEST_ERROR_CODE(fcvqueue_destroy(queue));
}


/* Benchmark: Dilate with queue */
static void benchmark_dilate(int loops, uint64_t *t) {
    
    fcvqueue_t queue;
    fcvqueue_buffer_t input_buf, output_buf;
    void *in, *out;
    size_t size = STRIDE * HEIGHT;
    uint64_t t1, t2;

    TEST_ERROR_CODE(fcvqueue_create(&queue, error_callback, NULL));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &input_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, input_buf, &in));
    fill_random(in, size);
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &output_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, output_buf, &out));
    memset(out, 0, size);

    t1 = time_usec();

    while ( loops-- ) {
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, input_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, output_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_DILATE3X3, input_buf, output_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, output_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, input_buf));
        TEST_ERROR_CODE(fcvqueue_sync(queue));
    }

    t2 = time_usec();
    *t = t2 - t1;
        
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_destroy(queue));
}


/* Benchmark: Dilate with FastCV */
static void benchmark_dilate_fastcv(int loops, uint64_t *t) {
    
    uint8_t *in, *out;
    size_t size = STRIDE * HEIGHT;
    uint64_t t1, t2;

    CHECK((in = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    fill_random(in, size);
    CHECK((out = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    memset(out, 0, size);

    t1 = time_usec();
    while ( loops-- ) {
        TEST_ERROR_CODE(fcvqueuetest_fastcv_dilate3x3(in, size, out, size, WIDTH, HEIGHT, STRIDE));
    }
    t2 = time_usec();
    *t = t2-t1;
    
    rpcmem_free(in);
    rpcmem_free(out);
}



/* Benchmark: Dilate-gaussian-median with queue */
static void benchmark_gaussian_dilate_median(int loops, int dilate_loops, uint64_t *t) {
    
    fcvqueue_t queue;
    fcvqueue_buffer_t input_buf, output_buf, temp_buf_1, temp_buf_2;
    void *in, *out;
    size_t size = STRIDE * HEIGHT;
    uint64_t t1, t2;

    assert((dilate_loops == 1) || ((dilate_loops & 1) == 0));

    TEST_ERROR_CODE(fcvqueue_create(&queue, error_callback, NULL));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &input_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, input_buf, &in));
    fill_random(in, size);
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &output_buf));
    TEST_ERROR_CODE(fcvqueue_buffer_ptr(queue, output_buf, &out));
    memset(out, 0, size);
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_alloc_buffer(queue, WIDTH, HEIGHT, STRIDE, &temp_buf_2));
    TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, temp_buf_2));
    TEST_ERROR_CODE(fcvqueue_sync(queue));

    t1 = time_usec();

    /* Note that we do cache maintenence on input and output buffers (since we
       could generate new data for each run) but not temporary buffers */
    while ( loops-- ) {
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, input_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_in(queue, output_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_GAUSSIAN3X3, input_buf, temp_buf_1));
        if ( dilate_loops == 1 ) {
            TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_DILATE3X3, temp_buf_1, temp_buf_2));
            TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_MEDIAN3X3, temp_buf_2, output_buf));
        }
        else {
            int dl = dilate_loops / 2;
            while ( dl-- ) {
                TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_DILATE3X3, temp_buf_1, temp_buf_2));
                TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_DILATE3X3, temp_buf_2, temp_buf_1));
            }
            TEST_ERROR_CODE(fcvqueue_enqueue_op(queue, FCVQUEUE_OP_MEDIAN3X3, temp_buf_1, output_buf));
        }
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, output_buf));
        TEST_ERROR_CODE(fcvqueue_enqueue_buffer_out(queue, input_buf));
        TEST_ERROR_CODE(fcvqueue_sync(queue));
    }

    t2 = time_usec();
    *t = t2 - t1;

    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, temp_buf_1));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, temp_buf_2));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, input_buf));
    TEST_ERROR_CODE(fcvqueue_free_buffer(queue, output_buf));
    TEST_ERROR_CODE(fcvqueue_destroy(queue));
}



/* Benchmark: Dilate-gaussian-median with FastCV */
static void benchmark_gaussian_dilate_median_fastcv(int loops, int dilate_loops, uint64_t *t) {
    
    uint8_t *in, *out, *temp1, *temp2;
    size_t size = STRIDE * HEIGHT;
    uint64_t t1, t2;

    assert((dilate_loops == 1) || ((dilate_loops & 1) == 0));
    
    CHECK((in = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    fill_random(in, size);
    CHECK((out = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    memset(out, 0, size);
    CHECK((temp1 = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);
    CHECK((temp2 = rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, size)) != NULL);

    t1 = time_usec();
    while ( loops-- ) {
        TEST_ERROR_CODE(fcvqueuetest_fastcv_gaussian3x3(in, size, temp1, size, WIDTH, HEIGHT, STRIDE));
        if ( dilate_loops == 1 ) {
            TEST_ERROR_CODE(fcvqueuetest_fastcv_dilate3x3(temp1, size, temp2, size, WIDTH, HEIGHT, STRIDE));
            TEST_ERROR_CODE(fcvqueuetest_fastcv_median3x3(temp2, size, out, size, WIDTH, HEIGHT, STRIDE));
        } else {
            int dl = dilate_loops / 2;
            while ( dl-- ) {
                TEST_ERROR_CODE(fcvqueuetest_fastcv_dilate3x3(temp1, size, temp2, size, WIDTH, HEIGHT, STRIDE));
                TEST_ERROR_CODE(fcvqueuetest_fastcv_dilate3x3(temp2, size, temp1, size, WIDTH, HEIGHT, STRIDE));
            }
            TEST_ERROR_CODE(fcvqueuetest_fastcv_median3x3(temp1, size, out, size, WIDTH, HEIGHT, STRIDE));
        }
    }
    t2 = time_usec();
    *t = t2-t1;

    rpcmem_free(temp1);
    rpcmem_free(temp2);
    rpcmem_free(in);
    rpcmem_free(out);
}


int main(int argc, char* argv[]) {

    int loops = 1;
    uint64_t t;

    /* fcvqueue expects the client to initialize rpcmem */
    rpcmem_init();
    setbuf(stdout,NULL);
	setbuf(stderr,NULL);

    /* Vote high clocks and bandwidth on the DSP */
    TEST_ERROR_CODE(fcvqueuetest_set_clocks());

    if ( argc > 1 ) {
        loops = atoi(argv[1]);
        if ( (loops < 1) || (argc != 2) ) {
            printf("Usage: fcvqueuetest [loops]\n");
            exit(EXIT_FAILURE);
        }
    }

    while ( loops-- ) {
        printf("Loops left: %d\n", loops);
        printf("Simple test\n");
        test_simple();
        printf("Test sync\n");
        test_sync();
        printf("Test copy\n");
        test_copy();
        printf("Test filters\n");
        test_filters();
    }

    printf("\nBenchmarking simple dilate\n");
    benchmark_dilate(100, &t);
    printf("Dilate with queue: Average %llu usec\n", t / 100LLU);
    benchmark_dilate_fastcv(100, &t);
    printf("Dilate with FastCV: Average %llu usec\n", t / 100LLU);
    
    printf("\nBenchmarking gaussian-dilate-median\n");
    benchmark_gaussian_dilate_median(100, 1, &t);
    printf("Gaussian-dilate-median with queue: Average %llu usec\n", t / 100LLU);
    benchmark_gaussian_dilate_median_fastcv(100, 1, &t);
    printf("Gaussian-dilate-median with FastCV: Average %llu usec\n", t / 100LLU);
    
    printf("\nBenchmarking gaussian-10xdilate-median\n");
    benchmark_gaussian_dilate_median(100, 10, &t);
    printf("Gaussian-10xdilate-median with queue: Average %llu usec\n", t / 100LLU);
    benchmark_gaussian_dilate_median_fastcv(100, 10, &t);
    printf("Gaussian-10xdilate-median with FastCV: Average %llu usec\n", t / 100LLU);
    
    printf("\nDone\n");

    /* Uninitialize rpcmem */
    rpcmem_deinit();

    return 0;
}
