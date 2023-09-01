/*! *****************************************************************************
 * @file            dma_apps_test.c
 *
 * @services        dma test app
 *
 * @description     Host side test application for the UBWCDMA. The source
 *                  contained here set up the necessary buffers required
 *                  by the ubwcdma sample applications (memcpy, blend, sum_hvx).
 *                  In addition, the call flow required to execute the
 *                  applications is also demonstrated here.
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

/*******************************************************************************
 * Include files
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "AEEStdErr.h"
#include "verify.h"
#include "dma_apps.h"
#include "rpcmem.h" // helper API's for shared buffer allocation

#ifdef __hexagon__
#include "qurt.h"
#include "dma_def.h"
#include "dma_types.h"
#else
#include "dspCV.h"
#endif

/*******************************************************************************
 * Local definitions and constants
 *******************************************************************************/

#ifdef __hexagon__     // some defs/stubs so app can build for Hexagon simulation
#define rpcmem_init()
#define rpcmem_deinit()
#define rpcmem_alloc(a, b, c) sim_rpcmem_alloc(c) // simulate allocation from page boundary (4 KB)
#define rpcmem_free(a) qurt_mem_region_delete((qurt_addr_t) (a))

// Function supported by the q6 simulator to grab command line parameters
void sys_get_cmdline(char*, int);
#endif

#define MAX_WIDTH 3840
#define MAX_HEIGHT 2160
#define MAX(a, b) ((a >= b) ? b : a)
#define CEILING(num, div) ((num + div -1)/(div))
#define ALIGN(x, a) CEILING(x, a) * a

enum app
{
    APP_MEMCPY,
    APP_BLEND,
    APP_SUM_HVX,
    APP_MAX,
};

/*******************************************************************************
 * Local functions
 *******************************************************************************/
#ifdef __hexagon__
// When running off the simulator the UBWCDMA requires memory that is contiguous
// which can only be assured by allocating off the DEFAULT_PHYSPOOL using QuRT.
// When ruinning off the target we use ION buffers.
static qurt_addr_t sim_rpcmem_alloc (int size)
{
    qurt_mem_pool_t pool_ddr;
    qurt_mem_region_t region_ddr;
    qurt_mem_region_attr_t region_ddr_attr;

    int nRet = qurt_mem_pool_attach ("DEFAULT_PHYSPOOL", &pool_ddr);
    if (nRet != QURT_EOK)
    {
        printf("Failed to attach the DDR memory region. The error code is: %d.\n",
               nRet);
        return nRet;
    }
    // Initialize the region attributes.
    qurt_mem_region_attr_init (&region_ddr_attr);
    // Set the region cache properties. The DDR region must be set to be
    // non-cacheable as otherwise the DMA will read stale values.
    qurt_mem_region_attr_set_cache_mode(&region_ddr_attr,
                                        QURT_MEM_CACHE_WRITETHROUGH_NONL2CACHEABLE);
    // Set the region permissions to allow reads and writes.
    int perms = QURT_PERM_WRITE | QURT_PERM_READ;
    qurt_mem_region_attr_set_perms(&region_ddr_attr, perms);
    // Create the mappings. This maps a virtual region of the provided size to
    // a contiguous physical block.
    qurt_size_t region_ddr_size = ALIGN(size, 0x1000);
    nRet = qurt_mem_region_create(&region_ddr, region_ddr_size, pool_ddr,
                                  &region_ddr_attr);
    if (nRet !=QURT_EOK)
    {
        printf("Failed to create the DDR memory region mapping. The error code is: %d.\n",
               nRet);
        return nRet;
    }
    // Get the virtual addresses to use for the DDR regions.
    qurt_addr_t ddr_buf_vaddr;
    qurt_mem_region_attr_get(region_ddr, &region_ddr_attr);
    qurt_mem_region_attr_get_virtaddr(&region_ddr_attr, &ddr_buf_vaddr);
    return ddr_buf_vaddr;
}
#endif

/* fill a buffer using a random starting offset and random selected period */
static int32_t fill_buffer(uint8_t* pbuf, uint32_t buf_len, uint8_t is_10bpp,
                           uint32_t seed)
{
    uint32_t offset, period, i;

    // buf_size must be even for 10bpp (stored in 2 bytes per pixel)
    if (is_10bpp && (buf_len & 0x1) != 0)
        return -1;

    /* Intializes random number generator */
    srand(seed);

    if (is_10bpp)
    {
        period = rand() % 1023;
        offset = rand() % period;
        uint16_t* piter = (uint16_t*) pbuf;
        for (i = 0; i < (buf_len >> 1); i++)
        {
            // P010 is a 10 bit format and so we can have up to 10 bit data.
            // The data is shifted so there are zeros in the 6 LSB positions.
            *piter++ = ((offset + i) % period) << 6;
        }
    }
    else
    {
        period = rand() % 255;
        offset = rand() % period;
        uint8_t* piter = (uint8_t*) pbuf;
        for (i = 0; i < buf_len; i++)
        {
            *piter++ = (offset + i) % period;
        }
    }

    return 0;
}

/* function to perform self check after running a dma application */
static int32_t self_check(uint8_t* psrc0, uint32_t src0_len,
                          uint8_t* psrc1, uint32_t src1_len,
                          uint8_t* pdst, uint32_t dst_len,
                          uint8_t is_10bpp, uint8_t app,
                          uint16_t height, uint16_t width)
{
    uint32_t ret = 0;
    uint32_t i;
    uint32_t num_elems = (height * width * 3) >> 1;
    uint32_t count = 0;

    if (is_10bpp)
    {
        uint16_t *psrc0_iter = (uint16_t*) psrc0;
        uint16_t *psrc1_iter = (uint16_t*) psrc1;
        uint16_t *pdst_iter = (uint16_t*) pdst;
        uint16_t valSrc0 = 0, valSrc1 = 0, valDst = 0;

        if ((src0_len & 0x1) != 0 || (dst_len & 0x1) != 0 ||
            (app != APP_MEMCPY && (src1_len & 0x1) != 0))
        {
            printf("Self Check failed -- Invalid buffer lengths\n");
            return -1;
        }

        if (num_elems > src0_len || num_elems > dst_len ||
            (app != APP_MEMCPY && num_elems > src1_len))
        {
            printf("Self Check failed -- Invalid buffer lengths\n");
            return -1;
        }

        for (i = 0; i < num_elems; i++)
        {
            valSrc0 = *psrc0_iter++ >> 6;
            valDst = *pdst_iter++ >> 6;
            if (app != APP_MEMCPY)
                valSrc1 = *psrc1_iter++ >> 6;
            if ((app == APP_MEMCPY && valSrc0 != valDst) ||
                (app == APP_BLEND && MAX(((uint32_t) valSrc0 + valSrc1) >> 1, 1023) != valDst) ||
                (app == APP_SUM_HVX && MAX((uint32_t) valSrc0 + valSrc1, 1023) != valDst))
            {
                //printf("Mismatch at element %d\n", i);
                ret = -1;
                count++;
            }
        }
    }
    else
    {
        uint8_t *psrc0_iter = psrc0;
        uint8_t *psrc1_iter = psrc1;
        uint8_t *pdst_iter = pdst;
        uint8_t valSrc0 = 0, valSrc1 = 0, valDst = 0;

        if (num_elems > src0_len || num_elems > dst_len ||
            (app != APP_MEMCPY && num_elems > src1_len))
        {
            printf("Self Check failed -- Invalid buffer lengths\n");
            return -1;
        }

        for (i = 0; i < num_elems; i++)
        {
            valSrc0 = *psrc0_iter++;
            valDst = *pdst_iter++;
            if (app != APP_MEMCPY)
                valSrc1 = *psrc1_iter++;
            if ((app == APP_MEMCPY && valSrc0 != valDst) ||
                (app == APP_BLEND && MAX(((uint16_t) valSrc0 + valSrc1) >> 1, 255) != valDst) ||
                (app == APP_SUM_HVX && MAX((uint16_t) valSrc0 + valSrc1, 255) != valDst))
            {
                printf("Mismatch at element %lu\n", (unsigned long) i);
                ret = -1;
                count++;
            }
        }
    }
    if (ret != 0)
    {
        printf("Self Check failed -- %lu mismatches\n", (unsigned long) count);
    }
    else
    {
        printf("Self Check passed\n");
    }

    return ret;
}

/*******************************************************************************
 * Global functions
 *******************************************************************************/

int test_main_start(int argc, char* argv[])
{
    dma_apps_hdl_t dma_apps_hdl;
    dma_apps_cfg_t dma_apps_cfg;
    uint8_t app = 0;
    uint8_t ubwc_en = 0;
    uint8_t *src0 = NULL;
    uint8_t *src1 = NULL;
    uint32_t src_len = 0;
    uint8_t *dst = NULL;
    uint32_t dst_len = 0;
    uint64_t height;
    uint64_t width;
    uint64_t fmt;
    uint64_t tmp;
    uint8_t *src0_linear = NULL;
    uint8_t *src1_linear = NULL;
    uint8_t *dst_linear = NULL;
    uint32_t linear_len = 0;
    uint8_t run_self_check = 0;

    int32_t ret = -1;
    int32_t retVal = 0;

    if (argc != 6)
    {
        printf("===========================================================\n");
        printf("Usage: %s [app] [height] [width] [fmt] [ubwc_en]\n", argv[0]);
        printf("       app     = [ memcpy | blend | sum_hvx ]\n");
        printf("       fmt     = [ 0=NV12 | 1=NV21| 2=NV124R | 3=P010 ]\n");
        printf("       ubwc_en = [ 0=linear | 1=ubwc ]\n");
        printf("===========================================================\n");
        return 0;
    }

    if (strcmp("memcpy", argv[1]) == 0)
    {
        app = APP_MEMCPY;
        printf("MEMCPY application selected\n");
    }
    else if (strcmp("blend", argv[1]) == 0)
    {
        app = APP_BLEND;
        printf("BLEND application selected\n");
    }
    else if (strcmp("sum_hvx", argv[1]) == 0)
    {
        app = APP_SUM_HVX;
        printf("SUM HVX application selected\n");
    }

    errno = 0;
    height = strtoul(argv[2], NULL, 10);
    if (height == ULONG_MAX && errno == ERANGE)
    {
        printf("Invalid frame height specified\n");
        goto err1;
    }
    printf("Frame height = %llu\n", (unsigned long long) height);

    errno = 0;
    width = strtoul(argv[3], NULL, 10);
    if (width == ULONG_MAX && errno == ERANGE)
    {
        printf("Invalid frame width specified\n");
        goto err1;
    }
    printf("Frame width = %llu\n", (unsigned long long) width);

    errno = 0;
    fmt = strtoul(argv[4], NULL, 10);
    if (fmt == ULONG_MAX && errno == ERANGE)
    {
        printf("Invalid format specified\n");
        goto err1;
    }
    printf("Format enum = %llu\n", (unsigned long long) fmt);

    errno = 0;
    tmp = strtoul(argv[5], NULL, 10);
    if (tmp == ULONG_MAX && errno == ERANGE)
    {
        printf("Invalid value for ubwc_en specified\n");
        goto err1;
    }
    ubwc_en = tmp ? 1 : 0;
    printf("UBWC enabled = %u\n", ubwc_en);

    if (height >= MAX_HEIGHT || width >= MAX_WIDTH || fmt >= FMT_MAX)
    {
        printf("Invalid frame properties height\n");
        goto err1;
    }

    // Initialize rpcmem
    rpcmem_init();

#ifndef __hexagon__
    // call dspCV_initQ6_with_attributes() to define Q6 clock and bus frequencies.
    // Since this app is not real-time, and can fully load the DSP clock & bus
    // resources throughout its lifetime, vote for the maximum available MIPS & BW.
    // The selection of values in this initialization is crucial in defining the
    // desired power vs. performance trade-off.
    dspCV_Attribute attrib[] =
    {
        // The below values will result in the maximum aDSP performance, at
        // Turbo voltage.
        {DSP_TOTAL_MCPS, 1000},             // Slightly more MCPS than are
                                            // available on current targets
        {DSP_MCPS_PER_THREAD, 500},         // drive the clock to MAX on known
                                            // targets
        {PEAK_BUS_BANDWIDTH_MBPS, 12000},   // 12 GB/sec is slightly higher than
                                            // the max realistic max BW on
                                            // existing targets.
        {BUS_USAGE_PERCENT, 100},           // This app is non-real time, and
                                            // constantly reading/writing memory
    };

    retVal = dspCV_initQ6_with_attributes(attrib, sizeof(attrib)/sizeof(attrib[0]));
    printf("Calling dspCV_initQ6() : %d \n", retVal);
    if (retVal != 0)
    {
        printf("Error initializing the Q6 for this application\n");
        goto err1;
    }
#endif

    // We allocate the buffers in linear form as required. Then we
    // fill the buffers that will be passed to the dma application
    src_len = dst_len = ((height * width * 3) >> 1) * (fmt == FMT_P010 ? 2 : 1);
    printf("Linear buffer inputs, allocating %lu bytes\n", (unsigned long) src_len);

    // Allocate the necessary memory
    // ION buffers that the dma will use must be allocated with the
    // RPCMEM_FLAG_UNCACHED flag, otherwise unexpected results may be encountered
    src0 = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_FLAG_UNCACHED,
                                   src_len);

    if (app != APP_MEMCPY)
        src1 = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_FLAG_UNCACHED,
                                       src_len);

    dst = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_FLAG_UNCACHED,
                                  dst_len);

    if (src0 == NULL || dst == NULL || (app != APP_MEMCPY && src1 == NULL))
    {
        printf("Error allocating frame buffers for the dma\n");
        goto err2;
    }

    if (fill_buffer(src0, src_len, (fmt == FMT_P010), 0x2345678) != 0)
    {
        printf("Error filling the src0 buffer\n");
        goto err2;
    }
    if (app != APP_MEMCPY &&
        fill_buffer(src1, src_len, (fmt == FMT_P010), 0x87654321) != 0)
    {
        printf("Error filling the src1 buffer\n");
        goto err2;
    }

    // Fill the destination buffer with zero (not necessary)
    memset((void*) dst, 0x00, dst_len);

    if (ubwc_en)
    {
        // In the UBWC case we need to create some UBWC inputs. We run the
        // dma memcpy app to convert the linear source buffers, allocated
        // and prefilled above, to generate UBWC buffers.
        //
        // This conversion stage is purely for illustrative purposes in this
        // example. Normally an application would receive buffers that are
        // already UBWC compressed from other sources such as the video decoder,
        // camera, etc.
        dma_apps_hdl_t dma_apps_hdl;
        dma_apps_cfg_t dma_apps_cfg;
        int32_t conv_ret = -1;

        printf("Performing linear->UBWC conversion of the source buffers\n");

        linear_len = src_len;
        src0_linear = src0;
        src1_linear = src1;
        dst_linear = dst;

        src0 = src1 = dst = NULL;

        // Determine the scratch size required by the memcpy app
        retVal = dma_apps_memcpy_scratch_size(&dma_apps_hdl.app_scratchLen);
        if (retVal != AEE_SUCCESS)
        {
            printf("Linear->UBWC: Error querying the scratch size required by the dma memcpy\n");
            goto conv1_err1;
        }

        // Allocate the scratch space and store in the handle
        dma_apps_hdl.app_scratch = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM,
                                                           RPCMEM_DEFAULT_FLAGS,
                                                           dma_apps_hdl.app_scratchLen);
        if (dma_apps_hdl.app_scratch == NULL)
        {
            printf("Linear->UBWC: Error allocating scratch space for the dma memcpy\n");
            goto conv1_err1;
        }

        // Setup the linear->UBWC conversion
        dma_apps_cfg.frm_ht = (uint16_t) height;
        dma_apps_cfg.frm_wd = (uint16_t) width;
        dma_apps_cfg.fmt = (dma_apps_pix_fmt) fmt;
        dma_apps_cfg.src_is_ubwc = FALSE;
        dma_apps_cfg.dst_is_ubwc = TRUE;

        retVal = dma_apps_memcpy_open(&dma_apps_cfg, &dma_apps_hdl);
        if (retVal != AEE_SUCCESS)
        {
            printf("Linear->UBWC: Error opening the dma memcpy app\n");
            goto conv1_err2;
        }

        // dma_apps_hdl.dst_buf_size is the expected size of the UBWC buffer
        // this with be the source and destination buffer sizes for the dma app
        // to be run as requested by the user
        src_len = dst_len = dma_apps_hdl.dst_buf_size;

        // Allocate the necessary memory
        // ION buffers that the dma will use must be allocated with the
        // RPCMEM_FLAG_UNCACHED flag, otherwise unexpected results may be encountered
        src0 = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_FLAG_UNCACHED,
                                       src_len);

        if (app != APP_MEMCPY)
            src1 = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_FLAG_UNCACHED,
                                           src_len);

        dst = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_FLAG_UNCACHED,
                                      dst_len);

        if (src0 == NULL || dst == NULL || (app != APP_MEMCPY && src1 == NULL))
        {
            printf("Linear->UBWC: Error allocating frame buffers for the dma\n");
            goto conv1_err3;
        }

        // Run the memcpy app on the linear src buffers and store in ubwc buffers
        retVal = dma_apps_memcpy_run(&dma_apps_hdl, src0_linear, linear_len,
                                     src0, src_len);
        if (retVal != AEE_SUCCESS)
        {
            printf("Linear->UBWC: Error running the dma memcpy app\n");
            goto conv1_err3;
        }

        if (app != APP_MEMCPY)
        {
            retVal = dma_apps_memcpy_run(&dma_apps_hdl, src1_linear, linear_len,
                                         src1, src_len);
            if (retVal != AEE_SUCCESS)
            {
                printf("Linear->UBWC: Error running the dma memcpy app\n");
                goto conv1_err3;
            }
        }

        // Prefill the destination buffer with zeros (not necessary)
        memset((void*) dst, 0x00, dst_len);
        conv_ret = 0; // Conversion successful
conv1_err3:
        // Close the memcpy application to release dma resources
        retVal = dma_apps_memcpy_close(&dma_apps_hdl);
        if (retVal != AEE_SUCCESS)
        {
            conv_ret = -1;
            printf("Linear->UBWC: Error closing the dma memcpy app\n");
        }

conv1_err2:
        // Free the scratch memory allocated for linear->UBWC conversion
        if (dma_apps_hdl.app_scratch != NULL)
            rpcmem_free(dma_apps_hdl.app_scratch);
conv1_err1:
        if (conv_ret != 0)
            goto err2;
    }

    // Determine the scratch space required by the application
    if (app == APP_MEMCPY)
        retVal = dma_apps_memcpy_scratch_size(&dma_apps_hdl.app_scratchLen);
    else if (app == APP_BLEND)
        retVal = dma_apps_blend_scratch_size(&dma_apps_hdl.app_scratchLen);
    else
        retVal = dma_apps_sum_hvx_scratch_size(&dma_apps_hdl.app_scratchLen);

    if (retVal != AEE_SUCCESS)
    {
        printf("Error querying the scratch size required by the dma application\n");
        goto err2;
    }
    printf("Scratch space required for this application -- %d bytes\n",
           (int) dma_apps_hdl.app_scratchLen);

    // Allocate scratch space and store in the handle
    dma_apps_hdl.app_scratch = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM,
                                                       RPCMEM_DEFAULT_FLAGS,
                                                       dma_apps_hdl.app_scratchLen);
    if (dma_apps_hdl.app_scratch == NULL)
    {
        printf("Error allocating scratch space for the application\n");
        goto err2;
    }

    // Setup the dma application configuration parameters as specified by the user
    dma_apps_cfg.frm_ht = (uint16_t) height;
    dma_apps_cfg.frm_wd = (uint16_t) width;
    dma_apps_cfg.fmt = (dma_apps_pix_fmt) fmt;
    dma_apps_cfg.src_is_ubwc = (ubwc_en) ? TRUE : FALSE;
    dma_apps_cfg.dst_is_ubwc = (ubwc_en) ? TRUE : FALSE;

    // Call of the open function of the required application
    if (app == APP_MEMCPY)
        retVal = dma_apps_memcpy_open(&dma_apps_cfg, &dma_apps_hdl);
    else if (app == APP_BLEND)
        retVal = dma_apps_blend_open(&dma_apps_cfg, &dma_apps_hdl);
    else
        retVal = dma_apps_sum_hvx_open(&dma_apps_cfg, &dma_apps_hdl);

    if (retVal != AEE_SUCCESS)
    {
        printf("Error opening the dma application\n");
        goto err3;
    }

    printf("dma application opened succesfully\n");

    printf("===============================================================\n");
    printf("Expected buffer sizes:\n");
    printf("----------------------\n");
    printf("src buffer size: %d byte\n", dma_apps_hdl.src_buf_size);
    printf("dst buffer size: %d byte\n", dma_apps_hdl.dst_buf_size);
    printf("===============================================================\n");

    printf("Prefill the src buffer and clear the dst buffer\n");
    if (src_len < dma_apps_hdl.src_buf_size || dst_len < dma_apps_hdl.dst_buf_size)
    {
        printf("Allocated buffer sizes to the application are not as the app expects\n");
        goto close;
    }

        printf("Running the application\n");

    if (app == APP_MEMCPY)
        retVal = dma_apps_memcpy_run(&dma_apps_hdl, src0, src_len, dst, dst_len);
    else if (app == APP_BLEND)
        retVal = dma_apps_blend_run(&dma_apps_hdl, src0, src_len, src1, src_len,
                                    dst, dst_len);
    else
        retVal = dma_apps_sum_hvx_run(&dma_apps_hdl, src0, src_len, src1, src_len,
                                      dst, dst_len);

    if (retVal != AEE_SUCCESS)
    {
        printf("Error running the application\n");
    }

    run_self_check = 1; // We have succesfully run the application. Perform self-check.
close:
    if (app == APP_MEMCPY)
        retVal = dma_apps_memcpy_close(&dma_apps_hdl);
    else if (app == APP_BLEND)
        retVal = dma_apps_blend_close(&dma_apps_hdl);
    else
        retVal = dma_apps_sum_hvx_close(&dma_apps_hdl);

    if (retVal != AEE_SUCCESS)
        printf("Error closing the dma application\n");
    else
        printf("Successfully closed the dma application\n");

    if (!run_self_check)
        goto err3;

    if (ubwc_en)
    {
        // In this stage we are converting the UBWC compressed destination buffer
        // to linear so that we can perform a self-check. It is an illustrative
        // stage for these examples.

        dma_apps_hdl_t dma_apps_hdl;
        dma_apps_cfg_t dma_apps_cfg;

        printf("Performing UBWC->linear conversion of the destination buffer\n");

        // Determine the scratch size required by the memcpy app
        retVal = dma_apps_memcpy_scratch_size(&dma_apps_hdl.app_scratchLen);
        if (retVal != AEE_SUCCESS)
        {
            printf("UBWC->Linear: Error querying the scratch size required by the dma memcpy\n");
            goto err3;
        }

        // Allocate scratch space and assign it to the app handle
        dma_apps_hdl.app_scratch = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM,
                                                           RPCMEM_DEFAULT_FLAGS,
                                                           dma_apps_hdl.app_scratchLen);
        if (dma_apps_hdl.app_scratch == NULL)
        {
            printf("UBWC->Linear: Error allocating scratch space for the dma memcpy\n");
            goto err3;
        }

        // Setup the UBWC->linear conversion
        dma_apps_cfg.frm_ht = (uint16_t) height;
        dma_apps_cfg.frm_wd = (uint16_t) width;
        dma_apps_cfg.fmt = (dma_apps_pix_fmt) fmt;
        dma_apps_cfg.src_is_ubwc = TRUE;
        dma_apps_cfg.dst_is_ubwc = FALSE;

        // Open the memcpy dma application
        retVal = dma_apps_memcpy_open(&dma_apps_cfg, &dma_apps_hdl);
        if (retVal != AEE_SUCCESS)
        {
            printf("UBWC->Linear: Error opening the dma memcpy application\n");
            goto conv2_err1;
        }

        // Open the memcpy dma application
        retVal = dma_apps_memcpy_run(&dma_apps_hdl, dst, dma_apps_hdl.dst_buf_size,
                                     dst_linear, linear_len);
        if (retVal != AEE_SUCCESS)
        {
            printf("UBWC->Linear: Error running the dma memcpy application\n");
            goto conv2_err2;
        }

        // Perform the self check
        ret = self_check(src0_linear, linear_len, src1_linear, linear_len,
                         dst_linear, linear_len, (fmt == FMT_P010), app,
                         height, width);

conv2_err2:
        // Close the memcpy dma application
        retVal = dma_apps_memcpy_close(&dma_apps_hdl);
        if (retVal != AEE_SUCCESS)
        {
            printf("UBWC->Linear: Error querying the scratch size required by the dma memcpy\n");
        }

conv2_err1:
        if (dma_apps_hdl.app_scratch != NULL)
            rpcmem_free(dma_apps_hdl.app_scratch);
    }
    else
    {
        // Run self test on the source vs destination buffers
        ret = self_check(src0, src_len, src1, src_len, dst, dst_len,
                         (fmt == FMT_P010), app, height, width);
    }

err3:
    if (dma_apps_hdl.app_scratch != NULL)
        rpcmem_free(dma_apps_hdl.app_scratch);

err2:
    if (src0 != NULL)
        rpcmem_free(src0);
    if (src1 != NULL)
        rpcmem_free(src1);
    if (dst != NULL)
        rpcmem_free(dst);
    if (src0_linear != NULL)
        rpcmem_free(src0_linear);
    if (src1_linear != NULL)
        rpcmem_free(src1_linear);
    if (dst_linear != NULL)
        rpcmem_free(dst_linear);

    // free ion buffers
    rpcmem_deinit();

#ifndef __hexagon__
    retVal = dspCV_deinitQ6();
    printf("Calling dspCV_deinitQ6() : %d\n", retVal);
    if (retVal != 0)
    {
        printf("Error calling deinit on the Q6 for this application\n");
    }
#endif

err1:
    if (ret != 0)
    {
        printf("Error Detected: Please see logs.\n");
    }
    else
    {
        printf("Execution Completed Successfully\n");
    }
    return ret;
}

#ifdef __hexagon__
void test(void* p)
{
    int ret = 0;
    char buf[2048];  // for holding & parsing the command line
    char argvbuf[2048];

    char** argv = (char**) argvbuf;
    int argc = 0;

    // system call to retrieve the command line, supported by q6 simulator.
    sys_get_cmdline(buf, sizeof(buf));

    // 1st argv is the program being run (i.e. "fastcv_test.ext") and its path
    argv[0] = strtok(buf, " ");
    while(strstr(argv[0], EXEC_NAME) == 0)
    {
        argv[0] = strtok(NULL, " ");
    }

    // loop to pick up the rest of the command line args from the command line
    while (NULL != (argv[++argc] = strtok(NULL, " "))) {;};

    ret = test_main_start(argc, argv);
}
#endif

int main(int argc, char* argv[])
{
#ifdef __hexagon__
    // In the simulator the following sequence must be called to configure
    // the UBWCDMA cosim correctly

    // Initialize the register region for the DMA (must always be first).
    int nRet;
    nRet = nDma_InitializeUbwcRegisterMemory();
    if (nRet != QURT_EOK){
        return 1;
    }
    // Initialize the DMA block and firmware.
    nRet = nDma_Initialize();
    if (nRet != OK){
        return 1;
    }

    // In the q6 simulator, spawn a thread to run test_main_start.
    // This is due to a limitation on the simulator where the stack size is
    // too limited on the main thread.
    int ret = 0;
    int status = 0;
    void* test_thread_stack = NULL;
    int test_thread_stack_size = 128*2048;
    qurt_thread_attr_t test_thread_attr;
    qurt_thread_t thread_id;

    test_thread_stack = (void*) malloc(test_thread_stack_size);
    if (!test_thread_stack)
    {
        ret = -1;
        printf("Stack allocation failed\n");
        goto adr_exit;
    }
    qurt_thread_attr_init(&test_thread_attr);
    qurt_thread_attr_set_name(&test_thread_attr, (char*) "dma_apps_test");
    qurt_thread_attr_set_stack_size(&test_thread_attr, test_thread_stack_size);
    qurt_thread_attr_set_stack_addr(&test_thread_attr, test_thread_stack);
    qurt_thread_attr_set_priority(&test_thread_attr, QURT_THREAD_ATTR_PRIORITY_DEFAULT);

    status = qurt_thread_create(&thread_id, &test_thread_attr, test, NULL);
    if (status != QURT_EOK)
    {
        printf("Failed to create dma apps test thread, status: %d", status);
        ret = -1;
        goto adr_exit;
    }

    status = qurt_thread_join (thread_id, &status);
    if (status != QURT_EOK && status != QURT_ENOTHREAD){
        printf("dma apps test thread failed, status: %d", status);
    }

adr_exit:
    if (test_thread_stack != NULL){
        free(test_thread_stack);
    }

    return ret;
#else
    return test_main_start(argc, argv);
#endif
}


