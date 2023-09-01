/*
  Copyright (c) 2015,2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Example to demonstrate use of image_dspq library with separate threads
 * to enqueue buffers and validating them once processing is done on DSP
 * side. */

#include <AEEStdErr.h>
#include <stdlib.h>
#include <stdio.h>
#include <rpcmem.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>

#include <asyncdspq.h>
#include <image_dspq_cpu.h>
#include "image_dspq_test.h"

/* Helper macros*/
#define FRAMES 10
#define WIDTH 1000
#define HEIGHT 1000

/* Buffer information variables for input and output buffers*/
imgq_buf_info *in_bufs[FRAMES];
imgq_buf_info *out_bufs[FRAMES];

/* queue handle*/
image_dspq_t dspq;

/* Semaphores to sync Frames queuing.*/
sem_t sem_frame_in;
sem_t sem_frame_queued;

/* Semaphore to sync validation of processed data.*/
sem_t sem_verify;

typedef struct {
    int term;
    int frame_idx;
}params;
params *command_parms;

/* Any error in queue would land in this callback.
 * This callback is registered while initializing image_dspq*/
void err_callback_app(int err)
{
    printf("Failed with error code = 0x%x\n", err);
}

/* Verification function for output. On DSP side, we are adding
 * 10 to src and putting that in dst.
 * */
static int verify_app(int id, uint8_t *src, uint8_t *dst)
{
    for(int i = 0; i < WIDTH*HEIGHT; i++)
    {
        if(dst[i] != (src[i] + 10))
        {
            printf("Verification failed for id = %d, actual = %d, expected = %d\n", id, dst[i], src[i] - 10);
            return -1;
        }
    }
    return 0;
}

/* Any IMGQ_DSP_MSG_USR type message in response queue would land here.
 * This callback is registered while initializing image_dspq.
 * Here identifier of processed buffer is expected.*/
void message_callback_app(uint32_t length, void *context)
{
    int id = *((int*)context);
    int err = 0;
    err = verify_app(id, (uint8_t*)(in_bufs[id]->data_ptr), (uint8_t*)(out_bufs[id]->data_ptr));
    if(err == 0)
        printf("Verification for %d frame is successful\n", id);
    sem_post(&sem_verify);
}

/* image_dspq_sync_callback type of function to synchronization queue.*/
static void sync_helper_app(void *context) {
    sem_post((sem_t*)context);
}

/* Fill random data in data_ptr ranging from 0 to 200 */
static void fill_in_buf(void *data_ptr)
{
    uint8_t *data = (uint8_t*)data_ptr;
    for(int i = 0; i < WIDTH*HEIGHT; i++)
    {
        data[i] = rand() % 200;
    }
}

/* Producer thread to enqueue operations in queue*/
void *handler_command_app(void *t) {
    params *local_params = (params *) t;
    intptr_t op_bufs[2];
    while(!sem_wait(&sem_frame_in)){
        int cnt = local_params->frame_idx;
        op_bufs[0] = (intptr_t)in_bufs[cnt];
        op_bufs[1] = (intptr_t)out_bufs[cnt];
        printf("Queued frame of id : %d to DSP \n", cnt);
        /* Enqueue operations with input and output buffers*/
        image_dspq_enqueue_operation(dspq,"libimage_dspq_example_lib.so","func_app",2, op_bufs);
        /* Check if this frame is the last frame and terminate the thread*/
        if ( local_params->term == 1) {
            pthread_exit(0);
        }
        /* Up Semaphore to let main thread know that enqueuing is done*/
        sem_post(&sem_frame_queued);
    }
    return NULL;
}


int main(int argc, char* argv[]) {

    int err = AEE_SUCCESS;
    sem_t sem1;

    /* Initialize rpcmem. User is expected to do it*/
    rpcmem_init();

    /* Set clocks.*/
    err = image_dspq_test_set_clocks();
    if(err)
    {
        printf("Unable to set clocks, err = %d\n", err);
        return -1;
    }

    /* Initialize dspq with error and response message callbacks*/
    err = image_dspq_init(&dspq, err_callback_app, message_callback_app);

    if(err != AEE_SUCCESS)
    {
        printf("unable to initialize dspq pointer, err = %d\n", err);
        return -1;
    }

    /* Semaphore initialization.*/
    sem_init(&sem_frame_in, 1, 0);
    sem_init(&sem_frame_queued, 1, 1);
    sem_init(&sem_verify, 1, 0);
    pthread_t thread[1];
    long t = 0;
    int processed_frames = 0;
    command_parms = (params *)malloc(sizeof(params));
    command_parms->frame_idx = 0;
    command_parms->term = 0;


    /* Thread creating to enqueueing operations in queue*/
    err = pthread_create(&thread[t], NULL, handler_command_app, (void *)command_parms);
    if(err != 0)
    {
        printf("Thread Creation failed, err = 0x%x\n", err);
        return -1;
    }


    /* Following tasks are performed in for loop for input and output buffers
     *     -> Allocate imagq_buf_info for input and output buffers.
     *     -> Fill buffer metadata.
     *     -> Create rpcmem allocated buffers for input and outputs.
     *     -> Mapping of buffers is taken care by image_dspq_allocate_buffer.
     *     -> Fill input buffer data with random data between 0 to 200.*/
    for(int i = 0; i < FRAMES; i++)
    {
        in_bufs[i] = image_dspq_allocate_buf_info(dspq, 0);
        if(in_bufs[i] == NULL)
        {
            printf("Unable to allocate buf info, err = 0x%x\n", err);
            return -1;
        }

        in_bufs[i]->metadata->type = IMGQ_BUF_IN;
        in_bufs[i]->metadata->width = WIDTH;
        in_bufs[i]->metadata->height = HEIGHT;
        in_bufs[i]->metadata->identifier = i;
        err = image_dspq_allocate_buffer(dspq, in_bufs[i], (WIDTH*HEIGHT));
        if(err != AEE_SUCCESS)
        {
            printf("Unable to allocate input buffer %d, err = 0x%x\n", i, err);
            return -1;
        }
        fill_in_buf(in_bufs[i]->data_ptr);

        out_bufs[i] = image_dspq_allocate_buf_info(dspq, 0);
        if(out_bufs[i] == NULL)
        {
            printf("Unable to allocate buf info, err = 0x%x\n", err);
            return -1;
        }
        out_bufs[i]->metadata->type = IMGQ_BUF_OUT;
        out_bufs[i]->metadata->identifier = i;
        err = image_dspq_allocate_buffer(dspq, out_bufs[i], (WIDTH*HEIGHT));
        if(err != AEE_SUCCESS)
        {
            printf("Unable to allocate output buffer %d, err = 0x%x\n", i, err);
            return -1;
        }

        sem_wait(&sem_frame_queued);
        command_parms->frame_idx = i;
        // To indicate the termination of frame queueing thread on last frame to be processed
        processed_frames++;
        if (processed_frames == FRAMES) {
            command_parms->term = 1;
        }
        // Input Frame is available now start queueing the frame to DSP queue
        sem_post(&sem_frame_in); /* up semaphore */
    }

    /* enqueue sync to wait till processing of all data is performed.*/
    sem_init(&sem1, 0, 0);
    image_dspq_sync(dspq, sync_helper_app, &sem1);
    sem_wait(&sem1);
    sem_destroy(&sem1);

    /* Wait for all the verification to complete. */
    int count_done =0;
    while(count_done < FRAMES) {
        sem_wait(&sem_verify);
        count_done++;
    }

    /* Free data structures related to buffes. Also, unmap them on DSP side.*/
    for(int i = 0; i < FRAMES; i++)
    {
        image_dspq_deregister_buffer(dspq, in_bufs[i]);
        image_dspq_deregister_buffer(dspq, out_bufs[i]);
    }
    /* Destroy queue*/
    err = image_dspq_destroy(dspq);

    if(err != AEE_SUCCESS)
        printf("Unable to destroy dspq, err = 0x%x\n", err);

    /* Deinit rpcmem*/
    rpcmem_deinit();

    return 0;
}
