#include <stdio.h>
#include <HAP_mem.h>
#include <image_dspq_dsp.h>
#include <HAP_farf.h>

/* Add 10 to input buffer and put it in output buffer.*/
void example_func(uint8_t *src, uint32_t width, uint32_t height, uint8_t *dst)
{
    for(uint32_t i = 0; i < width*height; i++)
    {
        dst[i] = src[i] + 10;
    }
}

/* Wrapper function to be called from image_dspq framework.
 * Get buffer pointers, write response to be placed in response queue and
 * send it to CPU.*/
int func_app(int num_bufs, intptr_t *bufs, int *res_size, void **res)
{
    (void)num_bufs;
    HAP_setFARFRuntimeLoggingParams(0x1f, NULL, 0);
    FARF(RUNTIME_HIGH, "imgq_buf_info = %p\n", (imgq_buf_info*)bufs[0]);
    FARF(RUNTIME_HIGH, "num_bufs = %d\n", num_bufs);
    imgq_buf_info *buf1 = (imgq_buf_info*)bufs[0];
    imgq_buf_info *buf2 = (imgq_buf_info*)bufs[1];
    FARF(RUNTIME_HIGH, "buf1 data ptr = %p metadaptr = %p data_size = %d\n", buf1->data_ptr, buf1->metadata, buf1->data_size);
    FARF(RUNTIME_HIGH, "buf2 data ptr = %p metadaptr = %p data_size = %d\n", buf2->data_ptr, buf2->metadata, buf2->data_size);
    FARF(RUNTIME_HIGH, "width = %d height = %d\n", buf1->metadata->width, buf1->metadata->height);
    example_func(buf1->data_ptr, buf1->metadata->width, buf1->metadata->height, buf2->data_ptr);
    *res_size = 8;
    int *fd;
    HAP_malloc(sizeof(int) * 2, (void **)&fd);
    /* Always put imgq_dsp_msg_response type of message first.*/
    *fd = IMGQ_DSP_MSG_USR;
    /* Identifier to find which buffer is processed on DSP*/
    *(fd + 1) = buf2->metadata->identifier;
    FARF(RUNTIME_HIGH,"Message data to be sent back %d %d\n", *fd, *(fd + 1));
    *res = fd;
    return 0;
}
