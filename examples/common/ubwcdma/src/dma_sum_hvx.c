/*! *****************************************************************************
 * @file            dma_sum_hvx.c
 *
 * @services        Performs a summation of two YUV frames.
 *
 * @description     Reads frames from the DDR into the L2 buffer,
 *                  sums them and then writes the result to DDR in either
 *                  linear/UBWC format.
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

#define _DMA_SUM_C_
#include "dma_sum_hvx.h"

#include "dma_def.h"
#include "dma_types.h"
#include "dmaWrapper.h"

#include "hexagon_types.h"
#include "hexagon_protos.h"         // part of Q6 tools, contains intrinsics definitions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Local definitions and constants
 *******************************************************************************/

#define SUM_APP_CEILING(num, div) ((num + div -1)/(div))
#define SUM_APP_CACHE_PING_PONG 2
#define SUM_APP_ALIGN(x, a) SUM_APP_CEILING(x, a) * a

/*!
 * Sum App control block (Per open session)
 */
typedef struct stDmaSum_SessionCb {
    //! Read DMA wrapper handle for the inputs
    t_DmaWrapper_DmaEngineHandle dmaRdWrapper[2];
    //! Write DMA wrapper handle for the output
    t_DmaWrapper_DmaEngineHandle dmaWrWrapper;
    //! Number of read engines to use for this session
    int nRdDmaEngine;
} t_StDmaSum_SessionCb;

/*!
 * Sum App available descriptor region
 */
typedef struct stDmaSum_DescRegion {
    //! Virtual address of the hardware descriptor buffer,
    //! pointer is advanced each time a chunk of
    //! descriptor buffer is used
    void* pL2VirtAddr_HWDesc;
    //! Total avaiable size of the descriptors, decreased each time
    //! a descriptor region is used up
    int nHWDescSize;
} t_StDmaSum_DescRegion;

/*******************************************************************************
 * Local functions
 *******************************************************************************/

/*!
 * @brief       Assign a descriptor region to the calling process
 *
 * @description Assigns a descriptor region equivalent to the input size
 *              from an allocated memory region. This function will assert
 *              if not enough memory was allocated externally.
 *
 * @input       stParm - Descriptor region parameters
 * @input       size - The size of the region to assign
 * @return      The pointer to a descriptor region that can be used.
 */
static void* nDmaSum_AssignDescRegion(t_StDmaSum_DescRegion *stParm, int size)
{
    void* descVirtAddr = stParm->pL2VirtAddr_HWDesc;
    stParm->nHWDescSize -= size;
    DBG_ASSERT(stParm->nHWDescSize >= 0);
    stParm->pL2VirtAddr_HWDesc = (void*) (((unsigned char*) stParm->pL2VirtAddr_HWDesc) +
                                          size);
    return descVirtAddr;
}

/*!
 * @brief       Prepare the read DMA session
 *
 * @description Prepares the corresponding DMA engines with the input frames'
 *              details.
 *
 * @input       pSessionCb - The DMA engine handles
 * @input       stParm - Move session parameters
 * @input       stDescParm - HW descriptor region parameters
 * @return      Success: OK
 * @n           Failure: ERR
 */
static int nDmaSum_PrepareRd(t_StDmaSum_SessionCb* pSessionCb,
        t_StDmaSum_MoveParm *stParm, t_StDmaSum_DescRegion *stDescParm)
{
     int nRet = OK;
     // Prepare parmaters and wrapper work descriptors. There
     // are 4 descriptors in all (Luma/Chroma for each input)
     // and up to 2 prepares (one for each read engine).
     t_StDmaWrapper_PrepareParm stWrapPrepParm[2];
     t_StDmaWrapper_WorkDescrip staWorkDesc[4];
     // ROI and frame parameter structures.
     t_StDmaWrapper_FrameProp walkRoi;
     t_StDmaWrapper_FrameProp frameProp;
     t_eDmaFmt efmtLumaChroma[2] = {stParm->eFmtLuma, stParm->eFmtChroma};

     for (int i = 0; i < 2; i++)
     {
         for (int j = 0; j < 2; j++)
         {
             // Populate the DMA work descriptor structure (this is seperate
             // from the HW descriptor, it is a firmware structure).
             int descIndex = i*2+j;
             frameProp.aAddr = (addr_t)stParm->pSrc[i];
             frameProp.u16W = stParm->nFrameWidth;
             frameProp.u16H = stParm->nFrameHeight;
             frameProp.u16Stride = stParm->nFrameWidth;
             walkRoi.aAddr = 0;
             walkRoi.u16W = stParm->nRoiWalkWidth;
             walkRoi.u16H = stParm->nRoiWalkHeight;
             walkRoi.u16Stride = (j) ? stParm->L2ChromaStride : stParm->L2LumaStride;
             nRet = nDmaWrapper_WorkDescrip_populate(&staWorkDesc[descIndex],
                                                     efmtLumaChroma[j],
                                                     eDmaWrapper_DdrToL2,
                                                     &walkRoi, &frameProp,
                                                     stParm->isUbwcSrc,
                                                     stParm->bUse16BitPaddingInL2,
                                                     NULL);
         }
     }

     if (pSessionCb->nRdDmaEngine == 1)
     {
         // If one engine is used, the two sources are passed as 4 descriptors
         // to the engine and only the first prepare parameter in the array is
         // used.
         stWrapPrepParm[0].u32NumOfWorkDesc = 4;
         stWrapPrepParm[0].staWorkDesc      = staWorkDesc;
         // Note: Double the number of descriptors are needed since one engine
         // is being used here (2 Luma/Chroma pairs).
         stWrapPrepParm[0].u32DescBufSize   = nDmaWrapper_GetDescbuffsize(efmtLumaChroma, 2)*2;
         stWrapPrepParm[0].pPingDescBuf     = nDmaSum_AssignDescRegion(stDescParm,
                                                                       stWrapPrepParm[0].u32DescBufSize);
         stWrapPrepParm[0].pPongDescBuf     = nDmaSum_AssignDescRegion(stDescParm,
                                                                       stWrapPrepParm[0].u32DescBufSize);
         // Prepare the DMA's HW descriptors with the wrapper descriptors.
         nDmaWrapper_Prepare(pSessionCb->dmaRdWrapper[0], &stWrapPrepParm[0]);
     }
     else
     {
         for (int i = 0; i < pSessionCb->nRdDmaEngine; i++)
         {
             // Both prepeare array entries are used, one is used per engine.
             // 2 descriptors (a Luma/Chroma pair) are passed to each engine.
             stWrapPrepParm[i].u32NumOfWorkDesc = 2;
             stWrapPrepParm[i].staWorkDesc      = &staWorkDesc[i*2];
             stWrapPrepParm[i].u32DescBufSize   = nDmaWrapper_GetDescbuffsize(efmtLumaChroma, 2);
             stWrapPrepParm[i].pPingDescBuf     = nDmaSum_AssignDescRegion(stDescParm,
                                                                           stWrapPrepParm[i].u32DescBufSize);
             stWrapPrepParm[i].pPongDescBuf     = nDmaSum_AssignDescRegion(stDescParm,
                                                                           stWrapPrepParm[i].u32DescBufSize);
             // Prepare the DMA's HW descriptors with the wrapper descriptors.
             nDmaWrapper_Prepare(pSessionCb->dmaRdWrapper[i], &stWrapPrepParm[i]);
         }
     }

     return nRet;
}

/*!
 * @brief       Prepare the write DMA session
 *
 * @description Prepares the write DMA engine with the output frame's details.
 *
 * @input       pSessionCb - The DMA engine handles
 * @input       stParm - Move session parameters
 * @input       stDescParm - HW descriptor region parameters
 * @return      Success: OK
 * @n           Failure: ERR
 */
static int nDmaSum_PrepareWr(t_StDmaSum_SessionCb* pSessionCb,
        t_StDmaSum_MoveParm *stParm, t_StDmaSum_DescRegion *stDescParm)
{
     // This function is similar to nDmaSum_PrepareRd with the exception that
     // the source is the TCM and the destination is the DDR.

     int nRet = OK;
     t_StDmaWrapper_PrepareParm stWrapPrepParm;
     t_StDmaWrapper_WorkDescrip staWorkDesc[2];
     t_StDmaWrapper_FrameProp walkRoi;
     t_StDmaWrapper_FrameProp frameProp;
     t_eDmaFmt efmtLumaChroma[2] = {stParm->eFmtLuma, stParm->eFmtChroma};

     for (int i = 0; i < 1; i++)
     {
         for (int j = 0; j < 2; j++)
         {
             int descIndex = i*2+j;
             frameProp.aAddr = (addr_t)stParm->pDst[i];
             frameProp.u16W = stParm->nFrameWidth;
             frameProp.u16H = stParm->nFrameHeight;
             frameProp.u16Stride = stParm->nFrameWidth;
             walkRoi.aAddr = 0;
             walkRoi.u16W = stParm->nRoiWalkWidth;
             walkRoi.u16H = stParm->nRoiWalkHeight;
             walkRoi.u16Stride = (j) ? stParm->L2ChromaStride : stParm->L2LumaStride;
             // Note: The transaction type is eDmaWrapper_L2ToDdr since this
             // is a write transaction.

             nRet = nDmaWrapper_WorkDescrip_populate(&staWorkDesc[descIndex],
                                                     efmtLumaChroma[j],
                                                     eDmaWrapper_L2ToDdr,
                                                     &walkRoi, &frameProp,
                                                     stParm->isUbwcDst,
                                                     stParm->bUse16BitPaddingInL2,
                                                     NULL);
          }
     }

     stWrapPrepParm.u32NumOfWorkDesc = 2;
     stWrapPrepParm.staWorkDesc      = staWorkDesc;
     stWrapPrepParm.u32DescBufSize   = nDmaWrapper_GetDescbuffsize(efmtLumaChroma, 2);
     stWrapPrepParm.pPingDescBuf     = nDmaSum_AssignDescRegion(stDescParm,
                                                                stWrapPrepParm.u32DescBufSize);
     stWrapPrepParm.pPongDescBuf     = nDmaSum_AssignDescRegion(stDescParm,
                                                                stWrapPrepParm.u32DescBufSize);
     nRet = nDmaWrapper_Prepare(pSessionCb->dmaWrWrapper, &stWrapPrepParm);

     return nRet;
}

/*!
 * @brief       Calculates Ping/Pong virtual buffer address.
 *
 * @description Uses the number of read/write operations
 *              to determine which of the ping/pong buffers should be used
 *              for the current operation.
 *
 * @input       stParm - Move session parameters
 * @input       nIdx - Number of read/write operations so far
 * @input       Rd - read/write
 * @return      The address of either the ping or pong buffer.
 */
static addr_t nDmaSum_PingPongBufferVirtAddr(t_StDmaSum_MoveParm *stParm,
        int nIdx, bool Rd)
{

    if (Rd)
    {
        return ((nIdx%SUM_APP_CACHE_PING_PONG) ?
           stParm->addrL2VirtAddr_IntermBufPongRd : stParm->addrL2VirtAddr_IntermBufPingRd);
    }
    else
    {
        return ((nIdx%SUM_APP_CACHE_PING_PONG) ?
           stParm->addrL2VirtAddr_IntermBufPongWr : stParm->addrL2VirtAddr_IntermBufPingWr);
    }
}

/*!
 * @brief       Calculates Ping/Pong physical buffer address.
 *
 * @description Uses the number of read/write operations
 *              to determine which of the ping/pong buffers should be used
 *              for the current operation.
 *
 *
 * @input       stParm - Move session parameters
 * @input       nIdx - Number of read/write operations so far
 * @input       Rd - read/write
 * @return      The address of either the ping or pong buffer.
 */
static addr_t nDmaSum_PingPongBufferPhysAddr(t_StDmaSum_MoveParm *stParm,
        int nIdx, bool Rd){

    if (Rd){
        return ((nIdx%SUM_APP_CACHE_PING_PONG) ?
           stParm->addrL2PhysAddr_IntermBufPongRd : stParm->addrL2PhysAddr_IntermBufPingRd);
    }
    else{
        return ((nIdx%SUM_APP_CACHE_PING_PONG) ?
           stParm->addrL2PhysAddr_IntermBufPongWr : stParm->addrL2PhysAddr_IntermBufPingWr);
    }
}

/*!
 * @brief       Calculates updated ROI information
 *
 * @description Determines the x,y coordinates, the height, width
 *              and L2 buffer address for the ROI.
 *
 * @input       stParm - Move session parameters
 * @input       nRowIdx - Index of the current row (divided by ROI height)
 * @input       nRow - Number of ROI rows (number of rows / ROI height)
 * @input       nColIdx - Index of the current column (divided by ROI width)
 * @input       nCol - Number of ROI columns (number of columns / ROI width)
 * @input       nIdxRd - Number of reads so far
 * @input       nIdxWr - Number of writes so far
 * @output      stWrapUpdateParmRd - The update read parameters
 * @output      stWrapUpdateParmWr - The update write parameters
 */
static void nDmaSum_UpdateRoiInfo(t_StDmaSum_MoveParm *stParm, int nRowIdx,
                                  int nRow, int nColIdx, int nCol,
                                  int nIdxRd, int nIdxWr,
                                  t_StDmaWrapper_UpdateParm stWrapUpdateParmRd[4],
                                  t_StDmaWrapper_UpdateParm stWrapUpdateParmWr[2])
{
    // This function calculates the ROI from the row and column information.
    // nRowIdx and nColIdx represent the row and column index of the current
    // ROI. For example, a row index of 0 and column index of 1 means that we
    // are on the ROI which is to the right of the very first ROI. nRow and
    // nCol specify the number of rows and columns in terms of the ROI. The X
    // coordinate and Y coordinate are straightforward to calculate given these
    // definition. The width and height calculation includes a check which
    // checks for the trailing ROIs in each frame and adjust the width and
    // height accordingly.

    // Fill in an Update structure for the Luma and Chroma.
    t_StDmaWrapper_UpdateParm stWrapUpdateParm[2];

    for (int j = 0; j < 2; j++)
    {
        stWrapUpdateParm[j].u.stPixData.stRoi.u16Y = stParm->nRoiWalkHeight * nRowIdx;
        if (nRowIdx == (nRow - 1))
        {
            // Last row
            stWrapUpdateParm[j].u.stPixData.stRoi.u16H = stParm->nFrameHeight -
                (stParm->nRoiWalkHeight * nRowIdx);
        }
        else
        {
            stWrapUpdateParm[j].u.stPixData.stRoi.u16H = stParm->nRoiWalkHeight;
        }

        stWrapUpdateParm[j].u.stPixData.stRoi.u16X = stParm->nRoiWalkWidth * nColIdx;
        if (nColIdx == (nCol - 1))
        {
            // Last column
            stWrapUpdateParm[j].u.stPixData.stRoi.u16W = stParm->nFrameWidth -
                (stParm->nRoiWalkWidth * nColIdx);
        }
        else
        {
            stWrapUpdateParm[j].u.stPixData.stRoi.u16W = stParm->nRoiWalkWidth;
        }
        if (j == 1)
        {
            // Align the ROI height for the Chroma if it is not divisible by the
            // minimum required ROI height. The TCM buffers and frame are assumed
            // to be padded externally.
            t_StDmaWrapper_RoiAlignInfo pStPixAlignInfo;
            nDmaWrapper_GetFmtAlignment(stParm->eFmtChroma, (stParm->isUbwcSrc ||
                                                             stParm->isUbwcDst),
                                        &pStPixAlignInfo);
            stWrapUpdateParm[j].u.stPixData.stRoi.u16H =
                SUM_APP_ALIGN(stWrapUpdateParm[j].u.stPixData.stRoi.u16H,
                              pStPixAlignInfo.u16H);
        }
    }

    // Copy the common update ROI information and add the address information for the read and
    // write engines.
    for (int i= 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            int descIndex = i*2+j;
            stWrapUpdateParmRd[descIndex] = stWrapUpdateParm[j];
            // We find which of the ping/pong buffer we are using and then index
            // appropriately into the buffer based on the input frame and L2
            // Chroma offset.
            stWrapUpdateParmRd[descIndex].aCacheAddr =
                nDmaSum_PingPongBufferPhysAddr(stParm, nIdxRd, TRUE) +
                (i*stParm->nIntermBufSize) + (j*stParm->L2ChromaOffset);
        }
    }
    for (int i= 0; i < 1; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            int descIndex = i*2+j;
            stWrapUpdateParmWr[descIndex] = stWrapUpdateParm[j];
            stWrapUpdateParmWr[descIndex].aCacheAddr =
                nDmaSum_PingPongBufferPhysAddr(stParm, nIdxWr, FALSE) +
                (j * stParm->L2ChromaOffset);
        }
    }
}

/*!
 * @brief       Sums 2 ROI regions together using the HVX intrinsics
 *
 * @description Sums 2 ROI regions together and writes the result into
 *              the destination buffer.
 *
 * @input       stParm - Move session parameters
 * @input       addrSrc1 - Address of the first frame ROI in L2
 * @input       addrSrc2 - Address of the second frame ROI in L2
 * @input       addrSrc1 - Address of the destination frame ROI in L2
 * @input       width - The width of the ROI
 * @input       height - The height of the ROI
 * @return      Success: OK
 * @n           Failure: ERR
 */
static int nDmaSum_SumROIs(t_StDmaSum_MoveParm *stParm, void* addrSrc1,
        void* addrSrc2, void* addrDst, int width, int height)
{

    // Given two source ROIs in TCM along with the destination, the function
    // sums the pixels of the 2 sources using the HVX.

    int nRet = OK;
    // Stride relative to the Luma plane.
    int stride = stParm->L2LumaStride;
    // We will sum the pixels using 128 byte mode
    unsigned int VLEN = 128;
    unsigned int log2VLEN = Q6_R_ct0_R(128);

    // The summation equation is: dst = sat(src1 + src2).
    void* addrSrc1_ = addrSrc1;
    void* addrSrc2_ = addrSrc2;
    void* addrDst_ = addrDst;
    for (int i = 0; i < height*3/2; i++)
    {
        if (i >= height)
        {
            // The UV offset is applied here, this is because the width and
            // height provided may be smaller than the walk width and height
            // (e.g. at the frame boundary). However, the UV offset corresponds
            // to the ROI provided to the nDmaWrapper_Prepare function which in
            // this example is the walk width and height provided in stParm.
            if (i == height)
            {
                addrSrc1_ = (void *) (((unsigned char*) addrSrc1) + stParm->L2ChromaOffset);
                addrSrc2_ = (void *) (((unsigned char*) addrSrc2) + stParm->L2ChromaOffset);
                addrDst_ = (void *) (((unsigned char*) addrDst) + stParm->L2ChromaOffset);
                // The stride becomes equal to the Chroma plane stride.
                stride = stParm->L2ChromaStride;
            }
        }
        // Note: All addresses must be 128 byte aligned when placed in a
        // HVX_Vector, this is ensured because the stride is also a multiple
        // of 128 bytes.
        HVX_Vector* addrSrc1_HVX = (HVX_Vector*) addrSrc1_;
        HVX_Vector* addrSrc2_HVX = (HVX_Vector*) addrSrc2_;
        HVX_Vector* addrDst_HVX = (HVX_Vector*) addrDst_;
        HVX_Vector valSrc1, valSrc2, valDst;
        // The bytes per pixel for each format in TCM which can be used to
        // find the number of required iterations.
        int bpp;
        if ((stParm->eFmt == eDmaFmt_NV12 || stParm->eFmt == eDmaFmt_NV124R) &&
            !stParm->bUse16BitPaddingInL2)
        {
            // 8 bit non padded formats have 1 byte per pixel in TCM.
            bpp = 1;
        }
        else
        {
            // 10 bit formats and padded 8 bit formats have 2 bytes per pixel
            // in TCM.
            bpp = 2;
        }
        // Number of iterations that can be done at full vector width.
        unsigned int widthIterations = (width * bpp) >> (log2VLEN);
        // Width remaining beyond blocks of VLEN.
        unsigned int remainingWidth = (width * bpp) - (widthIterations * VLEN);
        if ((stParm->eFmt == eDmaFmt_NV12 || stParm->eFmt == eDmaFmt_NV124R) &&
            !stParm->bUse16BitPaddingInL2)
        {
            // 8 bit processing for non padded 8 bit formats.
            for (int j = 0; j < widthIterations; j++)
            {
                // Load the sources into the vector registers.
                valSrc1 = *addrSrc1_HVX++;
                valSrc2 = *addrSrc2_HVX++;
                // Add them using an HVX saturating add.
                valDst = Q6_Vub_vadd_VubVub_sat(valSrc1, valSrc2);
                // Store the result.
                *addrDst_HVX++ = valDst;
            }
            if (remainingWidth > 0)
            {
                // Bitmask for the remaining width.
                HVX_VectorPred tailMask = Q6_Q_vsetq_R(remainingWidth);
                // Load the sources into the vector registers.
                valSrc1 = *addrSrc1_HVX++;
                valSrc2 = *addrSrc2_HVX++;
                // Add them using an HVX saturating add.
                valDst = Q6_Vub_vadd_VubVub_sat(valSrc1, valSrc2);
                // Read the destination to save the bytes that should not
                // be written to.
                HVX_Vector valDstOld = *addrDst_HVX;
                // For each byte, choose the original destination if it is not
                // meant to be replaced and choose the new saturated value if
                // it is.
                valDst = Q6_V_vmux_QVV(tailMask, valDst, valDstOld);
                // Store the multiplexed result.
                *addrDst_HVX++ = valDst;
            }
            addrSrc1_ = (void*) (((unsigned char*) addrSrc1_) + stride);
            addrSrc2_ = (void*) (((unsigned char*) addrSrc2_) + stride);
            addrDst_ = (void*) (((unsigned char*) addrDst_) + stride);
        }
        else
        {
            // 16 bit processing for 10 bit formats and padded 8 bit formats.

            for (int j = 0; j < widthIterations; j++)
            {
                // Load the sources into the vector registers.
                valSrc1 = *addrSrc1_HVX++;
                valSrc2 = *addrSrc2_HVX++;
                // Add them using a 16 bit HVX saturating add.
                valDst = Q6_Vuh_vadd_VuhVuh_sat(valSrc1, valSrc2);
                // Store the result.
                *addrDst_HVX++ = valDst;
            }
            if (remainingWidth > 0)
            {
                // Bitmask for the remaining width.
                HVX_VectorPred tailMask = Q6_Q_vsetq_R(remainingWidth);
                // Load the sources into the vector registers.
                valSrc1 = *addrSrc1_HVX++;
                valSrc2 = *addrSrc2_HVX++;
                // Add them using a 16 bit HVX saturating add.
                valDst = Q6_Vuh_vadd_VuhVuh_sat(valSrc1, valSrc2);
                // Read the destination to save the bytes that should not
                // be written to.
                HVX_Vector valDstOld = *addrDst_HVX;
                // For each byte, choose the original destination if it is not
                // meant to be replaced and choose the new saturated value if
                // it is.
                valDst = Q6_V_vmux_QVV(tailMask, valDst, valDstOld);
                // Store the multiplexed result.
                *addrDst_HVX++ = valDst;
            }
            addrSrc1_ = (void*) (((unsigned short*) addrSrc1_) + stride);
            addrSrc2_ = (void*) (((unsigned short*) addrSrc2_) + stride);
            addrDst_ = (void*) (((unsigned short*) addrDst_) + stride);
        }
    }

    return nRet;
}

/*******************************************************************************
 * Global Functions
 *******************************************************************************/

t_HandleSum hDmaSum_Open(t_StDmaSum_OpenParm stParm)
{
    t_StDmaSum_SessionCb* pSessionCb;

    pSessionCb = (t_StDmaSum_SessionCb*) malloc(
        sizeof(t_StDmaSum_SessionCb));

    if (!pSessionCb) {
        DBG_LOGE("Could not allocate memory to store session control block.\n");
        goto done;
    }

    if (stParm.nRdDmaEngine != 1 && stParm.nRdDmaEngine != 2)
    {
        DBG_LOGE("Number of read engines to use for this application must be 1 or 2\n");
        goto err1;
    }
    pSessionCb->nRdDmaEngine = stParm.nRdDmaEngine;

    for (int i = 0; i < pSessionCb->nRdDmaEngine; i++)
    {
        pSessionCb->dmaRdWrapper[i] = hDmaWrapper_AllocDmaSpecifyWaitType(stParm.eWaitType);
        if (!pSessionCb->dmaRdWrapper[i])
        {
            DBG_LOGE("Failed to allocate a read DMA engine.\n");
            if (i == 0)
            {
                goto err1;
            }
            else if (i == 1)
            {
                goto err2;
            }
        }
    }

    pSessionCb->dmaWrWrapper = hDmaWrapper_AllocDmaSpecifyWaitType(stParm.eWaitType);
    if (!pSessionCb->dmaWrWrapper)
    {
        DBG_LOGE("Failed to allocate the write DMA engine\n");
        goto err3;
    }
    goto done;

err3:
    nDmaWrapper_FreeDma(pSessionCb->dmaRdWrapper[1]);
err2:
    nDmaWrapper_FreeDma(pSessionCb->dmaRdWrapper[0]);
err1:
    free((void*) pSessionCb);
    pSessionCb = NULL;
done:
    return pSessionCb;
}

int nDmaSum_Close(t_HandleSum handle)
{
    t_StDmaSum_SessionCb* pSessionCb = (t_StDmaSum_SessionCb*) handle;
    int nRet = OK;

    if (!pSessionCb)
    {
        DBG_LOGE("Invalid handle provided.\n");
        nRet = ERR;
        return nRet;
    }

    for (int i = 0; i < pSessionCb->nRdDmaEngine; i++)
    {
        nRet = nDmaWrapper_FreeDma(pSessionCb->dmaRdWrapper[i]);
        DBG_ASSERT(nRet == OK);
        pSessionCb->dmaRdWrapper[i] = 0;
    }

    nRet = nDmaWrapper_FreeDma(pSessionCb->dmaWrWrapper);
    DBG_ASSERT(nRet == OK);
    pSessionCb->dmaWrWrapper = 0;

    free((void*) pSessionCb);

    return nRet;
}

int nDmaSum_Move(t_HandleSum handle, t_StDmaSum_MoveParm stParm)
{
    t_StDmaSum_SessionCb* pSessionCb = (t_StDmaSum_SessionCb*) handle;
    // One update parameter is needed per descriptor.
    t_StDmaWrapper_UpdateParm stWrapUpdateParmRd[4];
    t_StDmaWrapper_UpdateParm stWrapUpdateParmWr[2];
    int nIdxRd = 0;
    int nIdxWr = 0;
    int nRet = OK;
    int nRow, nCol;
    int nRowIdx, nColIdx;
    int i;

    if (!pSessionCb)
    {
        DBG_LOGE("Invalid handle provided.\n");
        nRet = ERR;
        return nRet;
    }

    // Only the NV12, P010 and NV124R frame formats are handled.
    if (!(stParm.eFmt == eDmaFmt_NV12 || stParm.eFmt == eDmaFmt_NV124R ||
          stParm.eFmt == eDmaFmt_P010))
    {
        DBG_LOGE("Invalid frame format provided. The format must be one of NV12, NV124R, P010.\n");
        nRet = ERR;
        return nRet;
    }

    // Prepare the DMA with the corresponding frame and maximum ROI information.
    t_StDmaSum_DescRegion stDescParm;
    stDescParm.pL2VirtAddr_HWDesc = stParm.pL2VirtAddr_HWDesc;
    stDescParm.nHWDescSize = stParm.nHWDescSize;
    if (nDmaSum_PrepareRd(pSessionCb, &stParm, &stDescParm) != OK)
    {
        DBG_LOGI("Failed to prepare the DMA for read. Check that the input formats and ROIs are correct.\n");
    }
    if (pSessionCb->dmaWrWrapper)
    {
        if (nDmaSum_PrepareWr(pSessionCb, &stParm, &stDescParm) != OK)
        {
            DBG_LOGI("Failed to prepare the DMA for write. Check that the input formats and ROIs are correct.\n");
        }
    }

    // Number of rows and columns in terms of the ROI.
    nRow = SUM_APP_CEILING(stParm.nFrameHeight, stParm.nRoiWalkHeight);
    nCol = SUM_APP_CEILING(stParm.nFrameWidth, stParm.nRoiWalkWidth);
    // Height and Width of previous iteration ROIs in the loop.
    int roiWidth = 0;
    int roiHeight = 0;
    // As in the memcpy, the loop should be interpreted as setting up the next
    // read ROI transactions. Thus, the summation and writes are delayed by a
    // number of iterations since they must wait for the first read to finish.
    // We run for one more row index to enable the last write.
    for (nRowIdx = 0; nRowIdx < nRow+1; nRowIdx++)
    {
        for (nColIdx = 0; nColIdx < nCol; nColIdx++)
        {
            // Update the parameters to reflect the latest ROI information.
            nDmaSum_UpdateRoiInfo(&stParm, nRowIdx, nRow, nColIdx, nCol, nIdxRd,
                                  nIdxWr, stWrapUpdateParmRd, stWrapUpdateParmWr);

            // Note: The update parameters are equivalent for read and write ROIs.
            DBG_LOGI("Sum roi{%u,%u,%u,%u}\n",
                stWrapUpdateParmRd[0].u.stPixData.stRoi.u16X,
                stWrapUpdateParmRd[0].u.stPixData.stRoi.u16Y,
                stWrapUpdateParmRd[0].u.stPixData.stRoi.u16W,
                stWrapUpdateParmRd[0].u.stPixData.stRoi.u16H);

            // Update the read engine(s) with the ROI information and begin a
            // move operation. The last read is skipped as all ROIs would have
            // been read since nRowIdx == nRow does not correspond to a valid
            // read ROI.
            if (nRowIdx < nRow)
            {
                for (i = 0; i < pSessionCb->nRdDmaEngine; i++)
                {
                    // Note: This works regardless of the number of engines,
                    // since if there is 1 engine the four update parameters
                    // are interpreted as belonging to that engine, otherwise
                    // each engine gets two parameters.
                    nDmaWrapper_Update(pSessionCb->dmaRdWrapper[i], &stWrapUpdateParmRd[i*2]);
                    // Start the read transfer.
                    nDmaWrapper_Move(pSessionCb->dmaRdWrapper[i]);
                }
            }

            // Sum the ROIs read in the last iteration and write them to an
            // output buffer, the first sum operation is skipped as on the first
            // iteration, the ROIs to sum are being fetched. The parameters
            // passed into the sum algorithm are those from the last iteration.
            if (nIdxRd > 0)
            {
                // Note: The read to do this summation on was done in the last
                // iteration, and so the indices-1 should be used to determine
                // the addresses to use. Additionally, roiWidth and roiHeight
                // hold the ROI parameters from the last iteration.
                nDmaSum_SumROIs(&stParm,
                                (void*) nDmaSum_PingPongBufferVirtAddr(&stParm,
                                                                       nIdxRd-1,
                                                                       TRUE),
                                (void*) (nDmaSum_PingPongBufferVirtAddr(&stParm,
                                                                        nIdxRd-1,
                                                                        TRUE) +
                                         stParm.nIntermBufSize),
                                (void*) nDmaSum_PingPongBufferVirtAddr(&stParm,
                                                                       nIdxWr-1,
                                                                       FALSE),
                                roiWidth, roiHeight);
            }
            roiWidth = stWrapUpdateParmRd[0].u.stPixData.stRoi.u16W;
            roiHeight = stWrapUpdateParmRd[0].u.stPixData.stRoi.u16H;

            // Wait for ongoing writes to finish. We skip this call for the
            // first two iterations of the loop, as no write is initiated prior
            // to the second iteration of the loop and this call precedes
            // the write initiation on that iteration.
            if (nIdxWr > 1)
            {
                nDmaWrapper_Wait(pSessionCb->dmaWrWrapper);
            }
            // Initiate the write of the output ROI. This is skipped for the
            // first iteration as the output is not ready at that time.
            if (nIdxWr > 0)
            {
                nDmaWrapper_Move(pSessionCb->dmaWrWrapper);
            }
            // Update the write, note that the update is called after the write
            // move so that the write with the set of ROI parameters from the
            // last iteration is launched first.
            // Skips the update on the last write(not strictly necessary to do).
            if (nRowIdx < nRow)
            {
                nDmaWrapper_Update(pSessionCb->dmaWrWrapper, stWrapUpdateParmWr);
            }

            if (nRowIdx < nRow)
            {
                // Wait for the current reads to finish as there are no more
                // steps that can be taken until they complete.
                for (i = 0; i < pSessionCb->nRdDmaEngine; i++)
                {
                    nDmaWrapper_Wait(pSessionCb->dmaRdWrapper[i]);
                }
            }
            else
            {
                // Break out of the loop or it will continue for the rest of
                // the columns. The last write has been completed.
                break;
            }
            nIdxRd++;
            nIdxWr++;
        }
    }
    // Wait for the last write to finish.
    nDmaWrapper_Wait(pSessionCb->dmaWrWrapper);
    // Instruct the DMA that current frame transactions have been completed.
    for (i = 0; i < pSessionCb->nRdDmaEngine; i++)
    {
        nDmaWrapper_FinishFrame(pSessionCb->dmaRdWrapper[i]);
    }
    nDmaWrapper_FinishFrame(pSessionCb->dmaWrWrapper);

    return nRet;

}


