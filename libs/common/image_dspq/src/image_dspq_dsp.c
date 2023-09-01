/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#include <AEEStdErr.h>
#include <AEEStdDef.h>
#include <qurt.h>
#include <assert.h>
#include <HAP_mem.h>
#include <HAP_farf.h>
#include <HAP_perf.h>
#include <HAP_power.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <image_dspq_dsp.h>
#include <asyncdspq.h>

/* Helper macros*/

#define MAX_MSG_LEN 256

/* Helper macro - pass error */
#define PASSERR(x) { if ( (err = x) != 0 ) { goto fail; } }

/* Helper macro - pass error if expression is false */
#define FAILFALSE(x, e) { if ( !(x) ) { err = e; goto fail; } }

/*Private data structures */

typedef struct buffer_list {
    imgq_buf_info *buf;
    int buf_info_fd;
    struct buffer_list *next;
}buffer_list;

typedef struct {
    asyncdspq_t req_queue;
    asyncdspq_t resp_queue;
    buffer_list *buf_list;
} image_dspq_dsp_priv_t;

static void image_dspq_dsp_error_callback(asyncdspq_t queue, void *context, AEEResult error)
{
    (void)queue;
    (void)context;
    printf("asyncdspq message error = %d", error);
}

static void image_dspq_dsp_message_callback(asyncdspq_t queue, void *context);

AEEResult image_dspq_dsp_init(uint32_t req_queue_handle, uint32_t resp_queue_handle, uint32_t *queue_handle)
{
    int err = 0;
    image_dspq_dsp_priv_t *q = NULL;

    HAP_setFARFRuntimeLoggingParams(0x1f, NULL, 0);

    *queue_handle = 0;

    FAILFALSE(((q = malloc(sizeof(image_dspq_dsp_priv_t))) != NULL), AEE_ENOMEMORY);
    memset(q, 0, sizeof(image_dspq_dsp_priv_t));

    PASSERR(asyncdspq_attach(&q->req_queue, req_queue_handle,
                            image_dspq_dsp_error_callback, image_dspq_dsp_message_callback, NULL, q));
    PASSERR(asyncdspq_attach(&q->resp_queue, resp_queue_handle,
                            image_dspq_dsp_error_callback, NULL, NULL, q));

    q->buf_list = NULL;
    *queue_handle = (uint32) q;
    return AEE_SUCCESS;

fail:
    free(q);
    return err;
}

AEEResult image_dspq_dsp_destroy(uint32_t queue_handle)
{
    int err = 0;
    image_dspq_dsp_priv_t *q = (image_dspq_dsp_priv_t*)(queue_handle);

    assert(q != NULL);

    /* Cancel any outstanding requests - not that there should be any */
    PASSERR(asyncdspq_cancel(q->req_queue));
    PASSERR(asyncdspq_cancel(q->resp_queue));

    /* Detach queues and free the structure */
    PASSERR(asyncdspq_detach(q->req_queue));
    PASSERR(asyncdspq_detach(q->resp_queue));
    free(q);

fail:
    return err;
}

/* Buffer registering deregistering apis*/

static void add_buf_info(image_dspq_dsp_priv_t *dspq, imgq_buf_info *buf)
{
    buffer_list *node = (buffer_list*) malloc(sizeof(buffer_list));
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

static void delete_buf_info(image_dspq_dsp_priv_t *dspq, buffer_list *buf_entry)
{
    buffer_list **temp = &(dspq->buf_list);

    while((*temp) != buf_entry)
        temp = &(*temp)->next;
    *temp = buf_entry->next;
    free(buf_entry->buf);
    free(buf_entry);
}

static buffer_list* find_buf_info_entry(image_dspq_dsp_priv_t *dspq, int buf_fd)
{
    buffer_list *list = dspq->buf_list;

    while(list != NULL)
    {
        if(list->buf_info_fd == buf_fd)
            return list;
        list = list->next;
    }
    return NULL;
}


AEEResult image_dspq_map_buf_metadata_rpc(uint32_t queue_handle, int buf_fd, uint32_t buf_offset, uint32_t buf_len, int buf_info_fd)
{
    (void)buf_offset;
    image_dspq_dsp_priv_t *dspq = (image_dspq_dsp_priv_t*)queue_handle;
    int err = 0;
    buffer_list *buf_node = NULL;
    imgq_buf_info *buf = (imgq_buf_info*)malloc(sizeof(imgq_buf_info));

    FARF(RUNTIME_HIGH, "image_dspq: metadata buf_fd = %d buf_len = %d buf_info_fd = %d\n", buf_fd, buf_len, buf_info_fd);
    FAILFALSE(((buf->metadata = HAP_mmap(NULL, buf_len, HAP_PROT_READ|HAP_PROT_WRITE, 0, buf_fd, 0)) != NULL), AEE_ENOMEMORY);
    FARF(RUNTIME_HIGH, "image_dspq: Mapped metadata data pointer = %p", buf->metadata);

    add_buf_info(dspq, buf);

    buf_node = dspq->buf_list;
    while(buf_node->next != NULL)
        buf_node = buf_node->next;

    buf_node->buf_info_fd = buf_info_fd;

    return AEE_SUCCESS;
fail:
    return err;
}

AEEResult image_dspq_map_buf_priv_data_rpc(uint32_t queue_handle, int buf_fd, uint32_t buf_offset, uint32_t buf_len, int buf_info_fd)
{
    (void)buf_offset;
    image_dspq_dsp_priv_t *dspq = (image_dspq_dsp_priv_t*)queue_handle;
    int err = 0;

    buffer_list *buf_node = find_buf_info_entry(dspq, buf_info_fd);
    if(buf_node == NULL)
        return AEE_ENOSUCH;

    FARF(RUNTIME_HIGH, "image_dspq: priv data buf_fd = %d buf_len = %d buf_info_fd = %d\n", buf_fd, buf_len, buf_info_fd);

    FAILFALSE(((buf_node->buf->buf_priv_data = HAP_mmap(NULL, buf_len, HAP_PROT_READ|HAP_PROT_WRITE,
                                              0, buf_fd, 0)) != NULL), AEE_ENOMEMORY);
    FARF(RUNTIME_HIGH, "image_dspq: Mapped priv data pointer = %p", buf_node->buf->buf_priv_data);
    buf_node->buf->buf_priv_data_size = buf_len;

    return AEE_SUCCESS;
fail:
    return err;
}

AEEResult image_dspq_map_buffer_rpc(uint32_t queue_handle, int buf_fd, uint32_t buf_offset, uint32_t buf_len, int buf_info_fd)
{
    (void)buf_offset;
    image_dspq_dsp_priv_t *dspq = (image_dspq_dsp_priv_t*)queue_handle;
    int err = 0;
    buffer_list *buf_node = find_buf_info_entry(dspq, buf_info_fd);
    if(buf_node == NULL)
        return AEE_ENOSUCH;

    FARF(RUNTIME_HIGH, "image_dspq: image_dspq_map_buffer_rpc: buffer buf_fd = %d buf_len = %d buf_info_fd = %d\n", buf_fd, buf_len, buf_info_fd);
    FAILFALSE(((buf_node->buf->data_ptr = HAP_mmap(NULL, buf_len, HAP_PROT_READ|HAP_PROT_WRITE,
                                              0, buf_fd, 0)) != NULL), AEE_ENOMEMORY);
    FARF(RUNTIME_HIGH, "image_dspq: Mapped buffer data pointer = %p", buf_node->buf->data_ptr);

    buf_node->buf->data_size = buf_len;
    buf_node->buf->imported_fd = buf_fd;
    return AEE_SUCCESS;
fail:
    return err;
}

AEEResult image_dspq_unmap_buffer_rpc(uint32_t queue_handle, int buf_info_fd)
{
    int err = 0;

    image_dspq_dsp_priv_t *dspq = (image_dspq_dsp_priv_t*)queue_handle;

    buffer_list *buf_node = find_buf_info_entry(dspq, buf_info_fd);
    if(buf_node == NULL)
        return AEE_ENOSUCH;

    FARF(RUNTIME_HIGH, "image_dspq: unmapping buffer fd = %d metadat_ptr = %p privds_ptr = %p buffer_ptr = %p\n",
                buf_info_fd, buf_node->buf->metadata, buf_node->buf->buf_priv_data, buf_node->buf->data_ptr);
    if(buf_node->buf->buf_priv_data_size != 0)
        PASSERR(HAP_munmap(buf_node->buf->buf_priv_data, buf_node->buf->buf_priv_data_size));
    PASSERR(HAP_munmap(buf_node->buf->metadata, sizeof(imgq_buf_metadata)));
    PASSERR(HAP_munmap(buf_node->buf->data_ptr, buf_node->buf->data_size));

    delete_buf_info(dspq, buf_node);
    return AEE_SUCCESS;
fail:
    return err;
}

static AEEResult execute_function(const char *library, const char *symbol, int num_bufs, intptr_t *bufs, int *result_size, void **res)
{
    void *library_handle;
    imgq_dsp_function symbol_handle;
    library_handle = dlopen(library, RTLD_LAZY);
    if(library_handle == NULL)
    {
        FARF(RUNTIME_ERROR, "Failed to open library %s", library);
        return AEE_ENOSUCH;
    }
    symbol_handle = dlsym(library_handle, symbol);
    if(symbol_handle == NULL)
    {
        FARF(RUNTIME_ERROR,"Failed to open library %s", library);
        return AEE_ENOSUCH;
    }

    for(int i = 0; i < num_bufs; i++)
    {
        imgq_buf_info *temp = (imgq_buf_info*)bufs[i];
        if(temp->metadata->type == IMGQ_BUF_IN)
        {
            qurt_mem_cache_clean((qurt_addr_t)temp->data_ptr, temp->data_size, QURT_MEM_CACHE_INVALIDATE, QURT_MEM_DCACHE);
            qurt_mem_cache_clean((qurt_addr_t)temp->metadata, sizeof(imgq_buf_metadata), QURT_MEM_CACHE_INVALIDATE, QURT_MEM_DCACHE);
            if(temp->buf_priv_data_size != 0)
                qurt_mem_cache_clean((qurt_addr_t)temp->buf_priv_data, temp->buf_priv_data_size, QURT_MEM_CACHE_INVALIDATE, QURT_MEM_DCACHE);
        }
    }

    /*Execute function*/
    symbol_handle(num_bufs, bufs, result_size, res);

    FARF(RUNTIME_HIGH, "image_dspq: Execution of function completed. Flushing cache\n");
    for(int i = 0; i < num_bufs; i++)
    {
        imgq_buf_info *temp = (imgq_buf_info*)bufs[i];
        if(temp->metadata->type == IMGQ_BUF_OUT)
        {
            qurt_mem_cache_clean((qurt_addr_t)temp->data_ptr, temp->data_size, QURT_MEM_CACHE_FLUSH, QURT_MEM_DCACHE);
            if(temp->buf_priv_data_size != 0)
                qurt_mem_cache_clean((qurt_addr_t)temp->buf_priv_data, temp->buf_priv_data_size, QURT_MEM_CACHE_FLUSH, QURT_MEM_DCACHE);
        }
    }
    FARF(RUNTIME_HIGH, "image_dspq: successfully completed function\n");
    return AEE_SUCCESS;
}

static char *get_string(uint32_t *msg, int index)
{
    uint32_t size = msg[index];
    char *lib_name = (char*)malloc(size);
    memcpy(lib_name, (char*)(&msg[index+1]), size);
    return lib_name;
}

static AEEResult image_dspq_dsp_exec_op(image_dspq_dsp_priv_t *queue, uint32_t *msg)
{
    uint32_t err = 0;
    image_dspq_dsp_priv_t *dspq = (image_dspq_dsp_priv_t*)queue;
    uint32_t index = 0;
    char *library, *symbol;
    intptr_t *buf_infos;
    int num_bufs;
    int result_size;
    void *res = NULL;

    library = get_string(msg, index);
    //library = "libexample_lib.so";
    index += (msg[index]/sizeof(uint32_t)) + ((msg[index] % sizeof(uint32_t))?1:0) + 1;

    symbol = get_string(msg, index);

    index += (msg[index]/sizeof(uint32_t)) + ((msg[index] % sizeof(uint32_t))?1:0) + 1;

    num_bufs = msg[index];
    buf_infos = (intptr_t*)malloc(sizeof(intptr_t)*num_bufs);
    index++;

    for(int i = 0; i < num_bufs; i++)
    {
        buffer_list *ll = find_buf_info_entry(dspq, msg[index]);
        buf_infos[i] = (intptr_t)(ll->buf);
        index++;
    }

    /*open library and call function*/
    PASSERR(execute_function(library, symbol, num_bufs, buf_infos, &result_size, &res));

    /*Work is done. Transfer ownership to CPU*/
    for(int i = 0; i < num_bufs; i++)
    {
        imgq_buf_info *temp = (imgq_buf_info*)buf_infos[i];
        temp->metadata->owner = IMGQ_CPU;
    }


    if(result_size)
    {
        PASSERR(asyncdspq_write(dspq->resp_queue, (uint8_t*)res, result_size));
        free(res);
    }
    free(library);
    free(symbol);
    return AEE_SUCCESS;

fail:
    if(library)
        free(library);
    if(symbol)
        free(symbol);
    if(res)
        free(res);
    asyncdspq_write(dspq->resp_queue, (uint8_t*)(&err), sizeof(int));
    return err;
}

static void image_dspq_dsp_message_callback(asyncdspq_t queue, void *context)
{
    uint32_t err = 0;
    image_dspq_dsp_priv_t *dspq = (image_dspq_dsp_priv_t*)context;
    uint32_t msg[MAX_MSG_LEN/4];
    unsigned int msg_len;

    assert(dspq);
    assert(queue == dspq->req_queue);

    while(1)
    {
        err = asyncdspq_read_noblock(dspq->req_queue, (uint8_t*)msg, sizeof(msg), &msg_len);
        if(err == AEE_ENOMORE)
            return;
        if(err != AEE_SUCCESS)
            goto fail;
        switch(msg[0]) {
            case IMGQ_CPU_MSG_OPERATION :
                {
                    PASSERR(image_dspq_dsp_exec_op(dspq, &msg[1]));
                    break;
                }
            case IMGQ_CPU_MSG_SYNC :
                {
                    msg[0] = IMGQ_DSP_MSG_SYNC;
                    PASSERR(asyncdspq_write(dspq->resp_queue, (uint8_t *)msg, 12));
                    break;
                }
            default:
                FARF(RUNTIME_ERROR,"Unknown message type\n", msg[0]);
                err = AEE_ESCHEMENOTSUPPORTED;
                goto fail;
        }
    }

fail:
    msg[0] = IMGQ_DSP_MSG_ERR;
    msg[1] = err;

    asyncdspq_write(dspq->resp_queue, (uint8_t*)msg, 8);
    return;
}
