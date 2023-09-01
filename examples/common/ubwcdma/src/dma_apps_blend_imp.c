/*! *****************************************************************************
 * @file            dma_apps_blend_imp.c
 *
 * @services        dma blend app implementation
 *
 * @description     Implementation of the dma_apps_sum_hvx functions defined in
 *                  the dma_apps.idl. In addition it manages the necessary L2
 *                  cache for UBWCDMA which the dma_sum_hvx core code requires.
 *                  The core blend operation copies 2 frames into L2 in ROI
 *                  sized chunks, blend them and writes the final result into
 *                  the provided buffers.
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

/*******************************************************************************
 * Include files
 *******************************************************************************/

#include "dma_apps.h"
#include "dma_blend.h"

#include "HAP_power.h"
#include "sysmon_cachelock.h"
#include "dma_def.h"
#include "dma_types.h"
#include "dmaWrapper.h"

#include "qurt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AEEStdErr.h"

/*******************************************************************************
 * Local definitions and constants
 *******************************************************************************/

#define CEILING(num, div) ((num + div -1)/(div))
#define ALIGN(x, a) CEILING(x, a) * a

typedef struct
{
    // Handle return by the core blend app
    t_HandleBlend hdlBlend;
    // Structures to provide to the core blend app.
    t_StDmaBlend_OpenParm stOpenParm;
    t_StDmaBlend_MoveParm stMoveParm;
    // QURT variables to be kept for each context
    qurt_mem_region_t region_tcm;
    qurt_size_t region_tcm_size;
    qurt_mem_region_t region_tcm_desc;
    qurt_size_t region_tcm_desc_size;
    qurt_addr_t tcm_buf_vaddr;
    qurt_addr_t tcm_desc_vaddr;
} t_StDmaBlendAppHdl;

/*******************************************************************************
 * Local functions
 *******************************************************************************/

#ifdef UBWCDMA_LEGACY_DRIVER
// DMA voting svs2 (sdm845/sdm710/sdm610)
static void vote_dma()
{
    HAP_power_vapss_payload vreq = {0};
    HAP_freq_match_type vfreqMatch = HAP_FREQ_AT_LEAST;
    HAP_power_request_t request;

    // Setting core clock frequency
    vreq.set_clk = TRUE;
    vreq.clkFreqHz = 136500000;
    vreq.freqMatch = vfreqMatch;

    // Setting external bus bandwidth
    vreq.dma_ext.set_bus_bw = TRUE;
    vreq.dma_ext.bwBytePerSec  = 1092000000;
    vreq.dma_ext.busbwUsagePercentage = 100;

    //Setting internal bus bandwidth
    vreq.dma_int.set_bus_bw = TRUE;
    vreq.dma_int.bwBytePerSec  = 546000000;
    vreq.dma_int.busbwUsagePercentage = 100;

    request.type = HAP_power_set_vapss;
    request.vapss = vreq;

    HAP_power_set(NULL, &request);
}
#endif

static t_eDmaFmt get_dma_fmt(enum dma_apps_pix_fmt fmt)
{
    t_eDmaFmt dma_fmt;

    switch (fmt)
    {
        case FMT_NV12:
            dma_fmt = eDmaFmt_NV12;
            break;
        case FMT_NV21:
            dma_fmt = eDmaFmt_NV12;
            break;
        case FMT_NV124R:
            dma_fmt = eDmaFmt_NV124R;
            break;
        case FMT_P010:
            dma_fmt = eDmaFmt_P010;
            break;
        default:
            dma_fmt = eDmaFmt_Invalid;
            break;
    }

    return dma_fmt;
}

/*******************************************************************************
 * Global functions
 *******************************************************************************/

AEEResult dma_apps_blend_scratch_size(int *size)
{
    *size = sizeof(t_StDmaBlendAppHdl);
    DBG_LOGI("dma_apps_blend required scratch size is %d bytes\n",
             sizeof(t_StDmaBlendAppHdl));
    return AEE_SUCCESS;
}

AEEResult dma_apps_blend_open(const dma_apps_cfg_t* cfg, dma_apps_hdl_t* p_hdl)
{
    AEEResult ret = AEE_SUCCESS;

    t_StDmaWrapper_RoiAlignInfo stAlignInfo;
    t_StDmaWrapper_FrameProp stFrameProp;
    t_StDmaWrapper_Roi stRoi;
    t_StDmaBlendAppHdl* p_app_hdl = (t_StDmaBlendAppHdl*) p_hdl->app_scratch;

    // Perform some checks
    if (p_hdl->app_scratchLen != sizeof(t_StDmaBlendAppHdl))
    {
        DBG_LOGE("app scratch buffer not allocated as required.\n");
        goto err1;
    }

    if (cfg->fmt >= FMT_MAX)
    {
        DBG_LOGE("Unrecognized format chosen.\n");
        goto err1;
    }

    // Check resolution alignment
    nDmaWrapper_GetFmtAlignment(get_dma_fmt(cfg->fmt) + 1,
                                cfg->src_is_ubwc || cfg->dst_is_ubwc,
                                &stAlignInfo);

    // The above API provide the minimum luma alignment requirements. The frame
    // resolution must be divisible by this alignment. Resolutions that are not
    // an even multiple of the required height will not be able to move the
    // Chroma ROIs at the last row (ROI height too small). To remedy this the
    // frame is padded to increase the height of the Chroma component.
    // Note: this means that more space must be allocated for the frame.
    if (cfg->frm_ht % (stAlignInfo.u16H) != 0 ||
        cfg->frm_wd % (stAlignInfo.u16W) != 0)
    {
        DBG_LOGE("Frame width and height for this application must be aligned to %d and %d respectively. Received unaligned parameters of %dx%d instead.\n",
                 stAlignInfo.u16W, stAlignInfo.u16H,
                 cfg->frm_wd, cfg->frm_ht);
        goto err1;
    }

    if (cfg->frm_ht % 2 != 0 || cfg->frm_wd % 2 != 0)
    {
        DBG_LOGE("Frame width and height for this application must be even. Received %dx%d instead.\n",
                 cfg->frm_wd, cfg->frm_ht);
        goto err1;
    }

    // The frame resolution is set. This resolution
    // will work for both the linear and UBWC cases.
    p_app_hdl->stMoveParm.nFrameHeight = cfg->frm_ht;
    p_app_hdl->stMoveParm.nFrameWidth = cfg->frm_wd;

    // Obtain the number of bytes in the src frame.
    stFrameProp.aAddr = 0;
    stFrameProp.u16H = cfg->frm_ht;
    stFrameProp.u16W = cfg->frm_wd;
    stFrameProp.u16Stride = cfg->frm_wd;
    p_hdl->src_buf_size = nDmaWrapper_GetFramesize(get_dma_fmt(cfg->fmt) + 1,
                                                   &stFrameProp,
                                                   cfg->src_is_ubwc);
    // Align the frame height and width to the Chroma alignment to determine
    // the padded size.
    nDmaWrapper_GetFmtAlignment(get_dma_fmt(cfg->fmt) + 2, cfg->src_is_ubwc,
                                &stAlignInfo);
    stFrameProp.u16H = ALIGN(stFrameProp.u16H, stAlignInfo.u16H);
    p_hdl->src_buf_size += nDmaWrapper_GetFramesize(get_dma_fmt(cfg->fmt) + 2,
                                                    &stFrameProp,
                                                    cfg->src_is_ubwc);

    stFrameProp.u16H = cfg->frm_ht;
    stFrameProp.u16W = cfg->frm_wd;
    stFrameProp.u16Stride = cfg->frm_wd;
    p_hdl->dst_buf_size = nDmaWrapper_GetFramesize(get_dma_fmt(cfg->fmt) + 1,
                                                   &stFrameProp,
                                                   cfg->dst_is_ubwc);
    // Align the frame height and width to the Chroma alignment to determine
    // the padded size.
    nDmaWrapper_GetFmtAlignment(get_dma_fmt(cfg->fmt) + 2, cfg->dst_is_ubwc,
                                &stAlignInfo);
    stFrameProp.u16H = ALIGN(stFrameProp.u16H, stAlignInfo.u16H);
    p_hdl->dst_buf_size += nDmaWrapper_GetFramesize(get_dma_fmt(cfg->fmt) + 2,
                                                    &stFrameProp,
                                                    cfg->dst_is_ubwc);

    // Configure the Open/Move parameters for blending which are required by
    // the core blend app
    p_app_hdl->stMoveParm.eFmt = get_dma_fmt(cfg->fmt);

    // The format is composed to its Luma and
    // Chroma components for use by the DMA.
    if (p_app_hdl->stMoveParm.eFmt == eDmaFmt_NV12)
    {
        p_app_hdl->stMoveParm.eFmtLuma = eDmaFmt_NV12_Y;
        p_app_hdl->stMoveParm.eFmtChroma = eDmaFmt_NV12_UV;
    }
    else if (p_app_hdl->stMoveParm.eFmt == eDmaFmt_P010)
    {
        p_app_hdl->stMoveParm.eFmtLuma = eDmaFmt_P010_Y;
        p_app_hdl->stMoveParm.eFmtChroma = eDmaFmt_P010_UV;
    }
    else if (p_app_hdl->stMoveParm.eFmt == eDmaFmt_NV124R)
    {
        p_app_hdl->stMoveParm.eFmtLuma = eDmaFmt_NV124R_Y;
        p_app_hdl->stMoveParm.eFmtChroma = eDmaFmt_NV124R_UV;
    }
    else
    {
        DBG_LOGE("Unrecognized format chosen. No Luma and Chroma split provided for this format.\n");
        goto err1;
    }

    // Padding in L2 is off so the format will be read into TCM
    // exactly as it is in DDR. Padding can be set to TRUE to produce
    // 16 bit pixels in TCM.
    p_app_hdl->stMoveParm.bUse16BitPaddingInL2 = FALSE;

    // Set whether src or dst is ubwc as passed in by the host
    p_app_hdl->stMoveParm.isUbwcSrc = cfg->src_is_ubwc;
    p_app_hdl->stMoveParm.isUbwcDst = cfg->dst_is_ubwc;

    // A polling wait type indicates the driver will poll
    // the DMA when a wait is called.
    p_app_hdl->stOpenParm.eWaitType = eDmaWaitType_Polling;
#ifdef COSIM_PLATFORM
    // The UBWCDMA cosim (Hexagon Simulator) cannot at this time cannot support
    // Low Power Polling and so should only be used when running on the target
    if (p_app_hdl->stOpenParm.eWaitType == eDmaWaitType_LowPowerPolling)
    {
        DBG_LOGE("Low Power Polling is not a supported wait type in the simulator/cosim environment\n");
        goto err1;
    }
#endif

    // Indicates that 2 Read engines are to be used.
    // One read engine can also be used.
    p_app_hdl->stOpenParm.nRdDmaEngine = 1;

    // A blend factor of 0.5 is used meaning that the pixels
    // will be averaged.
    p_app_hdl->stMoveParm.alpha = 0.5;

    // The ROI width and height are then set, the ROI is chosen to be 256x32,
    // however, by using nDmaWrapper_GetRecommendedRoi the passed in dimensions
    // will be aligned as required by the source format.
    //
    // There are alignment constraints on the ROI in UBWC mode and linear mode.
    // In linear mode, the ROI width and height must be even.
    // In UBWC mode, the ROI width must align to the DMA requirements for the
    // chosen format. The ROI height must, as in the memcpy, be a multiple of
    // the minimum Chroma alignment requirements (which is more restrictive
    // than the UBWC Luma requirements). This is so that corresponding Luma and
    // Chroma planes can be transferred together.
    stRoi.u16W = 256;
    stRoi.u16H = 32;
    nDmaWrapper_GetRecommendedRoi(p_app_hdl->stMoveParm.eFmt,
                                  (p_app_hdl->stMoveParm.isUbwcSrc ||
                                   p_app_hdl->stMoveParm.isUbwcDst),
                                  &stRoi);
    p_app_hdl->stMoveParm.nRoiWalkWidth = stRoi.u16W;
    p_app_hdl->stMoveParm.nRoiWalkHeight = stRoi.u16H;

    // Next, allocations for TCM space for the ROI buffers and descriptors is done.
    // Allocations are done using the QURT mapping functions.
    //
    // The size of the required TCM region is determined. Each frame will occupy
    // a region in TCM equivalent to the ROI buffer size needed.
    // Three frames are processed at any one time (2 read and 1 written), this
    // leads to 3 ROI buffer regions being required in the TCM. A ping/pong flow
    // is used so double the preceeding region is allocated (6 ROI regions in all).
    int tcmStride;
    stAlignInfo.u16W = p_app_hdl->stMoveParm.nRoiWalkWidth;
    stAlignInfo.u16H = p_app_hdl->stMoveParm.nRoiWalkHeight;
    tcmStride = nDmaWrapper_GetRecommendedIntermBufStride(p_app_hdl->stMoveParm.eFmtLuma,
                                                          &stAlignInfo,
                                                          (p_app_hdl->stMoveParm.isUbwcSrc ||
                                                           p_app_hdl->stMoveParm.isUbwcDst));
    p_app_hdl->stMoveParm.L2LumaStride = tcmStride;

    // Get the size for the Luma component.
    qurt_size_t tcm_buf_size =
        nDmaWrapper_GetRecommendedIntermBufSize(p_app_hdl->stMoveParm.eFmtLuma,
                                                p_app_hdl->stMoveParm.bUse16BitPaddingInL2,
                                                &stAlignInfo,
                                                (p_app_hdl->stMoveParm.isUbwcSrc ||
                                                 p_app_hdl->stMoveParm.isUbwcDst),
                                                tcmStride);

    // The size of the Luma plane is the Chroma offset in this example.
    p_app_hdl-> stMoveParm.L2ChromaOffset = tcm_buf_size;

    // Add it to the Chroma component, in this application, the buffers are contiguous.
    stAlignInfo.u16W = p_app_hdl->stMoveParm.nRoiWalkWidth;
    stAlignInfo.u16H = p_app_hdl->stMoveParm.nRoiWalkHeight;
    tcmStride =
        nDmaWrapper_GetRecommendedIntermBufStride (p_app_hdl->stMoveParm.eFmtChroma,
                                                   &stAlignInfo,
                                                   (p_app_hdl->stMoveParm.isUbwcSrc ||
                                                    p_app_hdl->stMoveParm.isUbwcDst));
    p_app_hdl->stMoveParm.L2ChromaStride = tcmStride;
    tcm_buf_size +=
        nDmaWrapper_GetRecommendedIntermBufSize(p_app_hdl->stMoveParm.eFmtChroma,
                                                p_app_hdl->stMoveParm.bUse16BitPaddingInL2,
                                                &stAlignInfo,
                                                (p_app_hdl->stMoveParm.isUbwcSrc ||
                                                 p_app_hdl->stMoveParm.isUbwcDst),
                                                tcmStride);

    // The above is the size of one ROI buffer, as noted 6 are needed.
    // The required size is also aligned to 4k.
    p_app_hdl->region_tcm_size = ALIGN(tcm_buf_size*6, 0x1000);
    // It is a good idea to check that this size is not too large, as while we
    // are still in DDR, when locked to the TCM large sizes will become
    // problematic.
    qurt_size_t region_tcm_limit = 0x40000; // Limit is set to 256k
    if (p_app_hdl->region_tcm_size > region_tcm_limit)
    {
        DBG_LOGE("The required TCM region for this ROI %d exceeds the set limit of %d. The ROI must be lowered or the allowed region made larger.\n",
                 p_app_hdl->region_tcm_size, region_tcm_limit);
        goto err1;
    }

    // The size of the required HW descriptor region is determined. There are 3
    // engines each using one pair of Luma/Chroma descriptors, or 2 engines where
    // one uses 2 pairs and the other uses 1 (3 total). The ping/pong flow is
    // being used so the required region must be doubled.
    t_eDmaFmt aeFmtId[2] = {p_app_hdl->stMoveParm.eFmtLuma,
        p_app_hdl->stMoveParm.eFmtChroma};
    p_app_hdl->region_tcm_desc_size = ALIGN(nDmaWrapper_GetDescbuffsize(aeFmtId, 2)
                                            * 3 * 2, 0x1000);

    // Allocate all the required regions and lock them.
    unsigned long long tcm_buf_paddr,tcm_desc_buf_paddr;

    p_app_hdl->tcm_desc_vaddr = (addr_t) HAP_cache_lock((unsigned int) (p_app_hdl->region_tcm_desc_size),
                                                        &tcm_desc_buf_paddr);
    if (p_app_hdl->tcm_desc_vaddr == 0)
    {
        DBG_LOGF("QURT TCM lock failed due to QURT_EALIGN ERROR misaligned u32Size = 0x%x .\n",
                 p_app_hdl->region_tcm_desc_size);
        goto err1;
    }

    p_app_hdl->tcm_buf_vaddr = (addr_t) HAP_cache_lock((unsigned int) (p_app_hdl->region_tcm_size),
                                                       &tcm_buf_paddr);
    if (p_app_hdl->tcm_buf_vaddr == 0)
    {
        DBG_LOGF("QURT TCM lock failed due to QURT_EALIGN ERROR misaligned u32Size = 0x%x .\n",
                 p_app_hdl->region_tcm_desc_size);
        goto err4;
    }

    // 3 buffers are used, one for each frame. Each of these is a ping/pong
    // buffer pair. The buffers for the read has the inputs as contiguous and
    // thus space for 2 ROIs must be used for the reads. The ping buffers for
    // all frames are set up first, followed by the pong buffers. The virtual
    // addresses are also needed since the TCM will be accessed to do the blend,
    // which was not the case in the memcpy.
    p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPingRd = p_app_hdl->tcm_buf_vaddr;
    p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPingWr =
        p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPingRd + tcm_buf_size*2;
    p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPongRd =
        p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPingWr + tcm_buf_size*1;
    p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPongWr =
        p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPongRd + tcm_buf_size*2;
    p_app_hdl->stMoveParm.addrL2PhysAddr_IntermBufPingRd =
        qurt_lookup_physaddr(p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPingRd);
    p_app_hdl->stMoveParm.addrL2PhysAddr_IntermBufPingWr =
        qurt_lookup_physaddr(p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPingWr);
    p_app_hdl->stMoveParm.addrL2PhysAddr_IntermBufPongRd =
        qurt_lookup_physaddr(p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPongRd);
    p_app_hdl->stMoveParm.addrL2PhysAddr_IntermBufPongWr =
        qurt_lookup_physaddr(p_app_hdl->stMoveParm.addrL2VirtAddr_IntermBufPongWr);
    p_app_hdl->stMoveParm.nIntermBufSize = tcm_buf_size;

    // The descriptor start address and size is provided to the core app. The app
    // will consume the space as needed. The descriptor address must be virtual.
    p_app_hdl->stMoveParm.pL2VirtAddr_HWDesc = (void*) p_app_hdl->tcm_desc_vaddr;
    p_app_hdl->stMoveParm.nHWDescSize = p_app_hdl->region_tcm_desc_size;

    // The core Blend app is invoked to initialize the dma
    p_app_hdl->hdlBlend = hDmaBlend_Open(p_app_hdl->stOpenParm);

    if (!p_app_hdl->hdlBlend) {
        DBG_LOGE("Failed to open a session to start the blend application.\n");
        goto err5;
    }

    // Power vote for the DMA
#ifdef UBWCDMA_LEGACY_DRIVER
    vote_dma();
#else
    nDmaWrapper_PowerVoting(PW_SVS2);
#endif
    
    DBG_LOGI("Successfully opened the blend application\n");
    goto done;

err5:
    if (QURT_EOK != HAP_cache_unlock((void*) p_app_hdl->tcm_buf_vaddr))
    {
        DBG_LOGF("QURT TCM unlock failed due to QURT_EALIGN ERROR misaligned u32Size = 0x%x .\n",
                 p_app_hdl->region_tcm_size);
    }
err4:
    if (QURT_EOK != HAP_cache_unlock((void*) p_app_hdl->tcm_desc_vaddr))
    {
        DBG_LOGF("QURT TCM descriptor unlock failed due to QURT_EALIGN ERROR misaligned u32Size = 0x%x .\n",
                 p_app_hdl->region_tcm_size);
    }
err1:
    ret = AEE_EFAILED;
done:
    return ret;
}

AEEResult dma_apps_blend_run(const dma_apps_hdl_t* p_hdl,
                             const unsigned char* src0, int src0Len,
                             const unsigned char* src1, int src1Len,
                             unsigned char* dst, int dstLen)
{
    AEEResult ret = AEE_EFAILED;
    t_StDmaBlendAppHdl* p_app_hdl = (t_StDmaBlendAppHdl*) p_hdl->app_scratch;

    p_app_hdl->stMoveParm.pSrc[0] = (void*) qurt_lookup_physaddr((qurt_addr_t) src0);
    p_app_hdl->stMoveParm.pSrc[1] = (void*) qurt_lookup_physaddr((qurt_addr_t) src1);
    p_app_hdl->stMoveParm.pDst[0] = (void*) qurt_lookup_physaddr((qurt_addr_t) dst);

    // The core Blend app is then invoked to begin the transfer.
    if (OK != nDmaBlend_Move(p_app_hdl->hdlBlend, p_app_hdl->stMoveParm))
    {
        DBG_LOGE("Failed to execute the blend application.\n");
        goto done;
    }

    DBG_LOGI("Successfully executed the blend application\n");
    ret = AEE_SUCCESS;

done:
    return ret;
}

AEEResult dma_apps_blend_close(const dma_apps_hdl_t* p_hdl)
{
    AEEResult ret = AEE_SUCCESS;
    t_StDmaBlendAppHdl* p_app_hdl = (t_StDmaBlendAppHdl*) p_hdl->app_scratch;

    if (!p_app_hdl->hdlBlend || p_app_hdl->tcm_buf_vaddr == NULL ||
        p_app_hdl->tcm_desc_vaddr == NULL)
    {
        DBG_LOGF("Possible corruption with application handle. May not be able to recover.\n");
        ret = AEE_EFAILED;
    }

    // Close the Blend session.
    if (p_app_hdl->hdlBlend && nDmaBlend_Close(p_app_hdl->hdlBlend) != OK)
    {
        DBG_LOGE("Failed to close the blend application.\n");
        ret = AEE_EFAILED;
    }

    // Release the TCM regions that were locked.
    // This can also be done through the firmware (using the
    // nDmaWrapper_UnlockCache function).
    if (p_app_hdl->tcm_buf_vaddr != NULL &&
        HAP_cache_unlock((void*) p_app_hdl->tcm_buf_vaddr) != 0)
    {
        DBG_LOGF("QURT TCM unlock failed due to QURT_EALIGN ERROR misaligned u32Size = 0x%x .\n",
                 p_app_hdl->region_tcm_size);
        ret = AEE_EFAILED;
    }

    if (p_app_hdl->tcm_desc_vaddr != NULL &&
        HAP_cache_unlock((void*) p_app_hdl->tcm_desc_vaddr) != 0)
    {
        DBG_LOGF("QURT TCM descriptor unlock failed due to QURT_EALIGN ERROR misaligned u32Size = 0x%x .\n",
                 p_app_hdl->region_tcm_desc_size);
        ret = AEE_EFAILED;
    }

    if (ret != AEE_EFAILED)
    {
        DBG_LOGI("Successfully closed the blend application\n");
    }

    return ret;
}
