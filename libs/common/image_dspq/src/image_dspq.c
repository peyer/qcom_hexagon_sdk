/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <AEEStdDef.h>
#include <AEEStdErr.h>
#include <asyncdspq.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <semaphore.h>

#include <rpcmem.h>

#include <image_dspq_cpu.h>
#include <image_dspq.h>

/* Queue configurations */
#define REQ_QUEUE_LENGTH 4096*4
#define RESP_QUEUE_LENGTH 4096
#define MAX_MSG_LEN 256

/* Helper macro - pass error */
#define PASSERR(x) { if ( (err = x) != 0 ) { goto fail; } }

/* Helper macro - pass error if expression is false */
#define FAILFALSE(x, e) { if ( !(x) ) { err = e; goto fail; } }

/*Private data structures */
typedef struct buffer_list_ {
    imgq_buf_info *buf;
    int buf_fd;
    struct buffer_list_ *next;
}buffer_list;

typedef struct {
    asyncdspq_t req_queue;
    uint32_t req_attach_handle;
    asyncdspq_t resp_queue;
    uint32_t resp_attach_handle;
    uint32_t cpu_handle;
    void *callback_context;
    buffer_list *buf_list;
    image_dspq_message_callback msg_callback;
    image_dspq_error_callback err_callback;
} image_dspq_priv_t;

void image_dspq_error_callback_helper(asyncdspq_t q, void *context, int err)
{
    (void)q;
    (void)context;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)context;
    if(dspq->err_callback)
        dspq->err_callback(err);
    else
        printf("No error callback is registered.");
}

/*Response queue message structure [size | message]*/

void image_dspq_message_callback_helper(asyncdspq_t q, void *context)
{
    (void)q;
    uint32_t err = 0;
    uint32_t msg[MAX_MSG_LEN / 4];
    uint32_t msg_len;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)context;

    while(1)
    {
        err = asyncdspq_read_noblock(dspq->resp_queue, (uint8_t*)msg, sizeof(msg), &msg_len);
        if(err == AEE_ENOMORE)
            return;
        if(err != AEE_SUCCESS)
            goto fail;
        switch (msg[0])
        {
            case IMGQ_DSP_MSG_USR :
                {
                    if(dspq->msg_callback)
                        dspq->msg_callback(msg_len, (void*)(&msg[1]));
                    else
                        printf("No messsage_calback registered.");
                    break;
                }
            case IMGQ_DSP_MSG_ERR :
                {
                    if(dspq->err_callback)
                        dspq->err_callback(msg[1]);
                    else
                        printf("No messsage_calback registered.");
                    break;
                }
            case IMGQ_DSP_MSG_SYNC:
                {
                    imgq_sync_t *sync;
                    uint64_t msg64 = (uintptr_t)msg[2];
                    sync = (imgq_sync_t*)(((uintptr_t)msg[1]) | (uintptr_t)(msg64<<32));
                    sync->sync_callback(sync->context);
                    break;
                }
            default:
                printf("Unknown response from dspq message type = %d\n", msg[0]);
        }
    }

fail:
    return;
}

AEEResult image_dspq_init(image_dspq_t *queue, image_dspq_error_callback error_callback_, image_dspq_message_callback msg_callback_)
{
    int err = 0;
    image_dspq_priv_t *dspq = NULL;

    FAILFALSE((dspq = (image_dspq_priv_t*)malloc(sizeof(image_dspq_priv_t))), AEE_ENOMEMORY);
    memset(dspq, 0, sizeof(image_dspq_priv_t));
    dspq->callback_context = NULL;
    dspq->err_callback = error_callback_;
    dspq->msg_callback = msg_callback_;

    PASSERR(asyncdspq_create(&dspq->req_queue, &dspq->req_attach_handle,
                               ASYNCDSPQ_ENDPOINT_APP_CPU,ASYNCDSPQ_ENDPOINT_CDSP,
                               REQ_QUEUE_LENGTH,
                               image_dspq_error_callback_helper, NULL, NULL, dspq, 0));
    PASSERR(asyncdspq_create(&dspq->resp_queue, &dspq->resp_attach_handle,
                               ASYNCDSPQ_ENDPOINT_CDSP, ASYNCDSPQ_ENDPOINT_APP_CPU,
                               RESP_QUEUE_LENGTH,
                               image_dspq_error_callback_helper, image_dspq_message_callback_helper, NULL, dspq ,0));

    dspq->buf_list = NULL;

    PASSERR(image_dspq_dsp_init(dspq->req_attach_handle, dspq->resp_attach_handle, &dspq->cpu_handle));

    *queue = (image_dspq_t)dspq;
    return AEE_SUCCESS;

fail:
    if(dspq->req_queue)
        asyncdspq_destroy(dspq->req_queue);
    if(dspq->resp_queue)
        asyncdspq_destroy(dspq->resp_queue);
    free(dspq);
    return err;
}

static void image_dspq_destroy_helper(void *context) {
    sem_post((sem_t*)context);
}

/* Buffer registering deregistering apis*/

static void add_buf_info(image_dspq_priv_t *dspq, imgq_buf_info *buf)
{
    buffer_list *node = (buffer_list*)malloc(sizeof(buffer_list));
    node->buf = buf;
    node->next = NULL;

    if(dspq->buf_list == NULL)
    {
        dspq->buf_list = node;
        return;
    }

    buffer_list *temp = dspq->buf_list;

    while(temp->next != NULL)
        temp = temp->next;
    temp->next = node;
}

static void delete_buf_info(image_dspq_priv_t *dspq, buffer_list *buf_entry)
{
    buffer_list **temp = &(dspq->buf_list);

    while((*temp) != buf_entry)
        temp = &(*temp)->next;
    *temp = buf_entry->next;
    rpcmem_free(buf_entry->buf);
    free(buf_entry);
}

static buffer_list* find_buf_info_entry(image_dspq_priv_t *dspq, imgq_buf_info *buf)
{
    buffer_list *list = dspq->buf_list;

    while(list != NULL)
    {
        if(list->buf == buf)
            return list;
        list = list->next;
    }
    return NULL;
}

imgq_buf_info* image_dspq_allocate_buf_info(image_dspq_t queue, uint32_t priv_data_size)
{
    int err = 0;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)queue;
    assert(dspq != NULL);
    buffer_list *lst = NULL;

    imgq_buf_info *buf = NULL;
    void *priv_data_ptr = NULL;
    imgq_buf_metadata *metadata_ptr = NULL;
    int priv_data_fd = 0, metadata_fd = 0;

    /*TODO: Find a way to move this allocation to normal malloc. Currently using it for tracking buffers through fds.*/
    FAILFALSE(((buf = (imgq_buf_info*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, sizeof(imgq_buf_info))) != NULL), AEE_ENOMEMORY);

    add_buf_info(dspq, buf);
    lst = find_buf_info_entry(dspq, buf);

    lst->buf_fd = rpcmem_to_fd(buf);
    if(lst->buf_fd <= 0)
        goto fail;

    FAILFALSE(((metadata_ptr = (imgq_buf_metadata*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, sizeof(imgq_buf_metadata))) != NULL ), AEE_ENOMEMORY);

    metadata_fd = rpcmem_to_fd(metadata_ptr);

    if(metadata_fd <= 0)
        goto fail;

    buf->metadata = metadata_ptr;

    PASSERR(image_dspq_map_buf_metadata_rpc(dspq->cpu_handle, metadata_fd, 0, sizeof(imgq_buf_metadata), lst->buf_fd));
    if(priv_data_size != 0)
    {
        FAILFALSE(((priv_data_ptr = (void*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, priv_data_size)) != NULL), AEE_ENOMEMORY);
        priv_data_fd = rpcmem_to_fd(priv_data_ptr);
        if(priv_data_fd <= 0)
            goto fail;
        buf->buf_priv_data = priv_data_ptr;
        buf->buf_priv_data_size = priv_data_size;
        PASSERR(image_dspq_map_buf_priv_data_rpc(dspq->cpu_handle, priv_data_fd, 0, priv_data_size, lst->buf_fd));
    }

    buf->data_ptr = NULL;
    buf->data_size = 0;
    buf->imported = 0;
    buf->metadata->owner = IMGQ_CPU;

    return buf;
fail:
    if(buf)
        rpcmem_free(buf);
    if(priv_data_ptr)
        rpcmem_free(priv_data_ptr);
    if(metadata_ptr)
        rpcmem_free(metadata_ptr);
    return NULL;
}

AEEResult image_dspq_allocate_buffer(image_dspq_t queue, imgq_buf_info *buf, uint32_t size)
{
    int err = 0;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)queue;
    assert(dspq != NULL);
    int buf_fd = 0;
    buffer_list *lst = NULL;
    buf->data_ptr = NULL;
    buf->data_size = size;

    FAILFALSE(((buf->data_ptr = (void*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, (buf->data_size))) != NULL), AEE_ENOMEMORY);
    buf_fd = rpcmem_to_fd(buf->data_ptr);

    if(buf_fd <= 0)
        goto fail;

    lst = find_buf_info_entry(dspq, buf);

    PASSERR(image_dspq_map_buffer_rpc(dspq->cpu_handle, buf_fd, 0, buf->data_size, lst->buf_fd));
    return AEE_SUCCESS;

fail:
    if(buf->data_ptr)
        rpcmem_free(buf->data_ptr);
    return err;
}

AEEResult image_dspq_import_buffer(image_dspq_t queue, imgq_buf_info *buf, void *imported_buf_ptr, uint32_t imported_buf_size, int imported_buf_fd)
{
    int err = 0;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)queue;
    assert(dspq != NULL);
    buffer_list *lst = NULL;

    buf->imported_fd = imported_buf_fd;
    buf->data_ptr = imported_buf_ptr;
    buf->data_size = imported_buf_size;
    buf->imported = 1;

    lst = find_buf_info_entry(dspq, buf);


    PASSERR(image_dspq_map_buffer_rpc(dspq->cpu_handle, imported_buf_fd, 0, imported_buf_size, lst->buf_fd));
    return AEE_SUCCESS;

fail:
    return err;
}

AEEResult image_dspq_deregister_buffer(image_dspq_t queue, imgq_buf_info *buf)
{
    int err = 0;
    image_dspq_priv_t *dspq = (image_dspq_priv_t *)queue;
    buffer_list *delete_node = NULL;
    delete_node = find_buf_info_entry(dspq, buf);

    if(delete_node == NULL)
    {
        printf("image_dspq: %p Buffer Node not found", buf);
        return AEE_ENOSUCH;
    }

    PASSERR(image_dspq_unmap_buffer_rpc(dspq->cpu_handle, delete_node->buf_fd));

    if(buf->imported == 0)
        rpcmem_free(buf->data_ptr);

    if(buf->buf_priv_data_size != 0)
        rpcmem_free(buf->buf_priv_data);

    if(buf->metadata)
        rpcmem_free(buf->metadata);

    delete_buf_info(dspq, delete_node);
    return AEE_SUCCESS;
fail:
    return err;
}
/* Message structure
 *
 * [library_size | library | symbol_size | symbol | num_bufs | buf_fds]
 *
 * */

AEEResult image_dspq_enqueue_operation(image_dspq_t queue, const char *library, const char *symbol, uint32_t num_bufs, intptr_t *bufs)
{
    int err = 0;
    int library_size = strlen(library);
    int symbol_size = strlen(symbol);
    buffer_list *lst = NULL;
    int index = 0;

    image_dspq_priv_t *dspq = (image_dspq_priv_t *)queue;

    assert(dspq != NULL);
    if((library_size == 0) || (symbol_size == 0))
    {
        printf("Library and/or Symbol name are not given\n");
        return AEE_ENOSUCH;
    }
    uint32_t msg_length = (num_bufs*sizeof(uint32_t)) + library_size + symbol_size + 4*sizeof(uint32_t) + (4-(library_size%4)) + (4-(symbol_size%4));

    int *message = (int*)malloc(msg_length);

    message[0] = IMGQ_CPU_MSG_OPERATION;
    index++;

    message[index] = library_size;
    index++;

    memcpy((char*)(&message[index]), library, library_size);
    index += (library_size/sizeof(uint32_t)) + ((library_size%sizeof(uint32_t))?1:0);

    message[index] = symbol_size;
    index++;

    memcpy((char*)(&message[index]), symbol, symbol_size);
    index += ((symbol_size)/sizeof(uint32_t)) + (((symbol_size)%sizeof(uint32_t))?1:0);

    message[index] = num_bufs;
    index++;

    for(uint32_t i = 0; i < num_bufs; i++)
    {
        imgq_buf_info *temp = (imgq_buf_info*)bufs[i];
        lst = find_buf_info_entry(dspq, temp);
        temp->metadata->owner = IMGQ_DSP;
        message[index] = lst->buf_fd;
        index++;
    }

    PASSERR(asyncdspq_write(dspq->req_queue, (uint8_t*)message, msg_length));

    free(message);

    return AEE_SUCCESS;
fail:
    return err;
}

AEEResult image_dspq_sync(image_dspq_t queue, image_dspq_sync_callback callback, void *context)
{
    int err = 0;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)queue;
    assert(dspq != NULL);
    imgq_sync_t *sync;
    sync = (imgq_sync_t*)malloc(sizeof(imgq_sync_t));
    sync->sync_callback = callback;
    sync->context = context;
    int msg[3];

    msg[0] = IMGQ_CPU_MSG_SYNC;
    msg[1] = (uint32_t) (((uintptr_t)sync));
    uint64_t sync64 = (uintptr_t)sync;
    msg[2] = (uint32_t) (sync64 >> 32);

    PASSERR(asyncdspq_write(dspq->req_queue, (uint8_t*)msg, 12));

    free(sync);
    return AEE_SUCCESS;

fail:

    free(sync);
    return err;
}

AEEResult image_dspq_destroy(image_dspq_t queue)
{
    uint32_t err = 0;
    image_dspq_priv_t *dspq = (image_dspq_priv_t*)queue;

    assert(dspq != NULL);
    assert(dspq->req_queue);
    assert(dspq->resp_queue);

    sem_t sem1;
    sem_init(&sem1, 0, 0);
    PASSERR(image_dspq_sync(dspq, image_dspq_destroy_helper, &sem1));
    sem_wait(&sem1);
    sem_destroy(&sem1);

    while(dspq->buf_list != NULL)
        image_dspq_deregister_buffer(dspq, dspq->buf_list->buf);

    PASSERR(image_dspq_dsp_destroy(dspq->cpu_handle));
    PASSERR(asyncdspq_destroy(dspq->req_queue));
    dspq->req_queue = NULL;
    PASSERR(asyncdspq_destroy(dspq->resp_queue));
    dspq->resp_queue = NULL;

    free(queue);
    return AEE_SUCCESS;

fail:
    return err;
}

