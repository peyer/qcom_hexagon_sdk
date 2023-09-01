/*! *****************************************************************************
 * @file            dma_memcpy.c
 *
 * @services        Performs memory copy.
 *
 * @description     Copies memory from a source buffer in DDR to an intermediate
 *                  buffer in L2/TCM then to a destination buffer in DDR.
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

#define _DMA_MEMCPY_C_
#include "dma_memcpy.h"

#include "dma_def.h"
#include "dma_types.h"
#include "dmaWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Local definitions and constants
 *******************************************************************************/

#define MEMCPY_CEILING(num, div) ((num + div -1)/(div))
#define MEMCPY_CACHE_PING_PONG 2
#define MEMCPY_ALIGN(x, a) MEMCPY_CEILING(x, a) * a

/*!
 * Memcpy control block (Per open session)
 */
typedef struct stDmaMemCpy_SessionCb
{
    //! Read DMA wrapper handle
    t_DmaWrapper_DmaEngineHandle dmaRdWrapper;
    //! Write DMA wrapper handle
    t_DmaWrapper_DmaEngineHandle dmaWrWrapper;
} t_StDmaMemCpy_SessionCb;

/*!
 * Memcpy available descriptor region
 */
typedef struct stDmaMemCpy_DescRegion
{
    //! Virtual address of the hardware descriptor buffer,
    //! pointer is advanced each time a chunk of
    //! descriptor buffer is used
    void* pL2VirtAddr_HWDesc;
    //! Total avaiable size of the descriptors, decreased each time
    //! a descriptor region is used up
    int nHWDescSize;
} t_StDmaMemCpy_DescRegion;

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
static void* nMemCpy_AssignDescRegion(t_StDmaMemCpy_DescRegion *stParm, int size)
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
 * @description Prepares the corresponding DMA engines with the input frame's
 *              details.
 *
 * @input       pSessionCb - The DMA engine handles
 * @input       stParm - Move session parameters
 * @input       stDescParm - HW descriptor region parameters
 * @return      Success: OK
 * @n           Failure: ERR
 */
static int nMemCpy_PrepareRd(t_StDmaMemCpy_SessionCb* pSessionCb,
        t_StDmaMemCpy_MoveParm *stParm, t_StDmaMemCpy_DescRegion *stDescParm)
{

    // The work descriptor and DMA Prepeare structures structures are set up.
    // The descriptors describe the transaction formats, transaction source and
    // destinations as well as addresses. The prepare paramters describe the
    // number of descriptors and provide TCM regions where the firmware can set
    // up the HW descriptors.

    int nRet = OK;
    t_StDmaWrapper_PrepareParm stWrapPrepParm;
    t_StDmaWrapper_WorkDescrip staWorkDesc[2];
    t_StDmaWrapper_FrameProp walkRoi;
    t_StDmaWrapper_FrameProp frameProp;
    t_eDmaFmt efmtLumaChroma [2] = {stParm->eFmtLuma, stParm->eFmtChroma};

    for (int i = 0; i < 1; i++)
    {

        // The first iteration intitializes the Luma transfer, the second
        // initializes the Chroma transfer. They are for the most part identical
        // since the Chroma ROIs and frame parameters are represented based on
        // the frame resolution. The only exception is the stride which may be
        // different between the two.
        for (int j = 0; j < 2; j++)
        {
            int descIndex = i*2+j;

            // Initialize the frame and ROI details. The L2 address (in walkRoi)
            // does not need to be properly initialized here as it is later
            // modified via an update. The ROI provided here should be the
            // maximum that that the user intends to  use throughout the frame
            // session.
            frameProp.aAddr = (addr_t)stParm->pSrc[i];
            frameProp.u16W = stParm->nFrameWidth;
            frameProp.u16H = stParm->nFrameHeight;
            frameProp.u16Stride = stParm->nFrameWidth;
            walkRoi.aAddr = 0;
            walkRoi.u16W = stParm->nRoiWalkWidth;
            walkRoi.u16H = stParm->nRoiWalkHeight;
            walkRoi.u16Stride = (j) ? stParm->L2ChromaStride : stParm->L2LumaStride;

            // Populate the DMA work descriptor structure (this is seperate from
            // the HW descriptor, it is a firmware structure). The last parameter
            // is to track the power profile of the DMA, it is not currently
            // used and so is set to NULL.
            nRet = nDmaWrapper_WorkDescrip_populate(&staWorkDesc[descIndex],
                                                    efmtLumaChroma[j],
                                                    eDmaWrapper_DdrToL2,
                                                    &walkRoi, &frameProp,
                                                    stParm->isUbwcSrc,
                                                    stParm->bUse16BitPaddingInL2,
                                                    NULL);
        }
    }

    // The prepare parameter is set to point to the work descriptor array and
    // the number of work descriptors is provided.
    stWrapPrepParm.u32NumOfWorkDesc = 2;
    stWrapPrepParm.staWorkDesc      = staWorkDesc;

    // These are the addresses of HW descriptor regions that the firmware will
    // set up for the DMA. They are allocated externally and assigned using the
    // nMemCpy_AssignDescRegion function (defined above). The
    // nDmaWrapper_GetDescbuffsize function is used to obtain the space
    // requirement for the descriptors given an array of the formats
    // ([NV12_Y, NV12_UV] for this example).
    stWrapPrepParm.u32DescBufSize   = nDmaWrapper_GetDescbuffsize(efmtLumaChroma, 2);
    stWrapPrepParm.pPingDescBuf     = nMemCpy_AssignDescRegion(stDescParm,
                                                               stWrapPrepParm.u32DescBufSize);

    // The pong desc region can be set equal to the ping region only if the
    // ping/pong flow is not used. More generally they must be seperated if a
    // DMA update is called while a DMA move is in progress.
    stWrapPrepParm.pPongDescBuf     = nMemCpy_AssignDescRegion(stDescParm,
                                                               stWrapPrepParm.u32DescBufSize);

    // The nDmaWrapper_Prepare function is used to provide the driver with the
    // frame details so that it can fill the HW descriptors.
    nDmaWrapper_Prepare(pSessionCb->dmaRdWrapper, &stWrapPrepParm);

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
static int nMemCpy_PrepareWr(t_StDmaMemCpy_SessionCb* pSessionCb,
        t_StDmaMemCpy_MoveParm *stParm, t_StDmaMemCpy_DescRegion *stDescParm)
{
    // This function is similar to nMemCpy_PrepareRd with the exception that
    // the source is the TCM and the destination is the DDR.

    DBG_ASSERT(pSessionCb->dmaWrWrapper!=NULL);

    int nRet = OK;
    t_StDmaWrapper_PrepareParm stWrapPrepParm;
    t_StDmaWrapper_WorkDescrip staWorkDesc[2];
    t_StDmaWrapper_FrameProp walkRoi;
    t_StDmaWrapper_FrameProp frameProp;
    t_eDmaFmt efmtLumaChroma [2] = {stParm->eFmtLuma, stParm->eFmtChroma};

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
            // Note: The transaction type is eDmaWrapper_L2ToDdr since this is
            // a write transaction.
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
    stWrapPrepParm.pPingDescBuf     = nMemCpy_AssignDescRegion(stDescParm,
                                                               stWrapPrepParm.u32DescBufSize);
    stWrapPrepParm.pPongDescBuf     = nMemCpy_AssignDescRegion(stDescParm,
                                                               stWrapPrepParm.u32DescBufSize);
    nRet = nDmaWrapper_Prepare(pSessionCb->dmaWrWrapper, &stWrapPrepParm);

    return nRet;
}

/*!
 * @brief       Calculates Ping/Pong buffer address.
 *
 * @description Uses the number of read/write operations
 *              to determine if the ping or pong buffer should be used
 *              for the current operation.
 *
 *
 * @input       stParm - Move session parameters
 * @input       nIdx - Number of read/write operations so far
 * @return      The address of either the ping or pong buffer.
 */
static addr_t nMemCpy_PingPongBufferAddr(t_StDmaMemCpy_MoveParm *stParm,
        int nIdx){

    return ((nIdx%MEMCPY_CACHE_PING_PONG) ?
        stParm->addrL2PhysAddr_IntermBufPong : stParm->addrL2PhysAddr_IntermBufPing);
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
 * @input       nIdx - Number of operations so far
 * @output      stWrapUpdateParm - The update parameters
 */
void nMemCpy_UpdateRoiInfo(t_StDmaMemCpy_MoveParm *stParm, int nRowIdx, int nRow,
        int nColIdx, int nCol, int nIdx,
        t_StDmaWrapper_UpdateParm* stWrapUpdateParm)
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

        // The TCM address is updated given the index of the buffer to use
        // (ping/pong) which alternates on every DMA transfer.
        stWrapUpdateParm[j].aCacheAddr = nMemCpy_PingPongBufferAddr(stParm, nIdx) +
            j*stParm->L2ChromaOffset;
    }
}

/*******************************************************************************
 * Global functions
 *******************************************************************************/

t_HandleMemCpy hDmaMemCpy_Open(t_StDmaMemCpy_OpenParm stParm)
{
    t_StDmaMemCpy_SessionCb* pSessionCb;

    pSessionCb = (t_StDmaMemCpy_SessionCb*) malloc(
        sizeof(t_StDmaMemCpy_SessionCb));

    if (!pSessionCb) {
        DBG_LOGE("Could not allocate memory to store the session control block.\n");
        goto done;
    }

    // The function hDmaWrapper_AllocDmaSpecifyWaitType is used to allocate the
    // DMA engine.
    pSessionCb->dmaRdWrapper = hDmaWrapper_AllocDmaSpecifyWaitType(stParm.eWaitType);
    if (!pSessionCb->dmaRdWrapper)
    {
        DBG_LOGE("Failed to allocate the read DMA engine.\n");
        goto err1;
    }

    // If the bSinkEn option is enabled, the output is written to DDR, otherwise
    // only the read from DDR is done.
    if (stParm.bSinkEn)
    {
        pSessionCb->dmaWrWrapper = hDmaWrapper_AllocDmaSpecifyWaitType(stParm.eWaitType);
        if (!pSessionCb->dmaWrWrapper)
        {
            DBG_LOGE("Failed to allocate the write DMA engine.\n");
            goto err2;
        }
    }
    else
        pSessionCb->dmaWrWrapper = 0;
    goto done;

err2:
    nDmaWrapper_FreeDma(pSessionCb->dmaRdWrapper);
err1:
    free((void*) pSessionCb);
    pSessionCb = NULL;
done:
    return pSessionCb;
}

int nDmaMemCpy_Close(t_HandleMemCpy handle)
{
    t_StDmaMemCpy_SessionCb* pSessionCb = (t_StDmaMemCpy_SessionCb*) handle;
    int nRet = OK;

    if (!pSessionCb)
    {
        DBG_LOGE("Invalid handle provided.\n");
        nRet = ERR;
        return nRet;
    }

    // The function nDmaWrapper_FreeDma is used to release the DMA engine.
    nRet = nDmaWrapper_FreeDma(pSessionCb->dmaRdWrapper);
    DBG_ASSERT(nRet == OK);
    if (pSessionCb->dmaWrWrapper)
    {
        nRet = nDmaWrapper_FreeDma(pSessionCb->dmaWrWrapper);
        DBG_ASSERT(nRet == OK);
    }

    pSessionCb->dmaRdWrapper = 0;
    pSessionCb->dmaWrWrapper = 0;
    free((void*) pSessionCb);

    return nRet;
}

int nDmaMemCpy_Move(t_HandleMemCpy handle, t_StDmaMemCpy_MoveParm stParm)
{
    t_StDmaMemCpy_SessionCb* pSessionCb = (t_StDmaMemCpy_SessionCb*) handle;
    t_StDmaWrapper_UpdateParm stWrapUpdateParm[2];
    int nIdx = 0;
    int nRet = OK;
    int nRow, nCol;
    int nRowIdx, nColIdx;
    bool preventFirstWriteWait = TRUE;

    if (!pSessionCb) {
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
    t_StDmaMemCpy_DescRegion stDescParm;
    stDescParm.pL2VirtAddr_HWDesc = stParm.pL2VirtAddr_HWDesc;
    stDescParm.nHWDescSize = stParm.nHWDescSize;
    if (nMemCpy_PrepareRd(pSessionCb, &stParm, &stDescParm) != OK)
    {
        DBG_LOGI("Failed to prepare the DMA for read. Check that the input formats and ROIs are correct.\n");
    }
    if (pSessionCb->dmaWrWrapper)
    {
        if (nMemCpy_PrepareWr(pSessionCb, &stParm, &stDescParm) != OK)
        {
            DBG_LOGI("Failed to prepare the DMA for write. Check that the input formats and ROIs are correct.\n");
        }
    }

    // This is the number of rows and columns in terms of the ROI.
    nRow = MEMCPY_CEILING(stParm.nFrameHeight, stParm.nRoiWalkHeight);
    nCol = MEMCPY_CEILING(stParm.nFrameWidth, stParm.nRoiWalkWidth);
    // Obtain the first transfer's ROI information.
    nMemCpy_UpdateRoiInfo(&stParm, 0, nRow, 0, nCol, nIdx, &stWrapUpdateParm[0]);
    // The function nDmaWrapper_Update is used to update the DMA engine with the
    // first transfer's ROI information.
    nDmaWrapper_Update(pSessionCb->dmaRdWrapper, stWrapUpdateParm);
    if (pSessionCb->dmaWrWrapper) nDmaWrapper_Update(pSessionCb->dmaWrWrapper,
                                                     stWrapUpdateParm);
    // The function nDmaWrapper_Move is used to initiate the first read.
    nDmaWrapper_Move(pSessionCb->dmaRdWrapper);
    nIdx = (nIdx + 1) % MEMCPY_CACHE_PING_PONG;

    // Loop through all ROIs after the first read is initiated.
    // The loop should be interpreted as setting up the next read ROI
    // transactions with the write delayed by one iteration.  Thus, the first
    // ROI is skipped (already read above).
    for (nRowIdx = 0; nRowIdx < nRow; nRowIdx++)
    {
        for (nColIdx = (nRowIdx)? 0:1 ; nColIdx < nCol; nColIdx++)
        {
            // Update ROI information.
            nMemCpy_UpdateRoiInfo(&stParm, nRowIdx, nRow, nColIdx, nCol, nIdx,
                                  &stWrapUpdateParm[0]);

            // The current write, if any is using the same buffer the next read
            // will use as there are only 2 L2 buffers (ping/pong). Thus, we
            // wait for this write to finish. The first wait is skipped as no
            // write is occurring the very first time the loop is entered.
            if (!preventFirstWriteWait)
            {
                if (pSessionCb->dmaWrWrapper) nDmaWrapper_Wait(pSessionCb->dmaWrWrapper);
            }
            else
            {
                preventFirstWriteWait = FALSE;
            }

            for (int i = 0; i < 1; i++)
            {
               DBG_LOGI("MemCpy roi{%u,%u,%u,%u}\n",
                        stWrapUpdateParm[0].u.stPixData.stRoi.u16X,
                        stWrapUpdateParm[0].u.stPixData.stRoi.u16Y,
                        stWrapUpdateParm[0].u.stPixData.stRoi.u16W,
                        stWrapUpdateParm[0].u.stPixData.stRoi.u16H);
            }

            // Update the read engine with the new ROI information,
            // wait for the previous read to conclude and then initiate the next
            // read.
            nDmaWrapper_Update(pSessionCb->dmaRdWrapper, stWrapUpdateParm);
            nDmaWrapper_Wait(pSessionCb->dmaRdWrapper);
            nDmaWrapper_Move(pSessionCb->dmaRdWrapper);

            // Since the previous read is done, a write can be initiated to copy
            // its ROI into the destination in DDR. Note that the write engine's
            // update is done after the move so that the old information
            // containing the previous ROI is used in the write. Otherwise, the
            // write will move the current ROI being read which has not
            // completed yet.
            if (pSessionCb->dmaWrWrapper) nDmaWrapper_Move(pSessionCb->dmaWrWrapper);
            if (pSessionCb->dmaWrWrapper) nDmaWrapper_Update(pSessionCb->dmaWrWrapper,
                                                             stWrapUpdateParm);

            nIdx = (nIdx + 1) % MEMCPY_CACHE_PING_PONG;
        }
    }

    // The last read has not necessarily completed by this point, we wait for it
    // to finish and initiate and wait for the final write to finish.
    if (!preventFirstWriteWait)
    {
        if (pSessionCb->dmaWrWrapper) nDmaWrapper_Wait(pSessionCb->dmaWrWrapper);
    }
    nDmaWrapper_Wait(pSessionCb->dmaRdWrapper);
    if (pSessionCb->dmaWrWrapper) nDmaWrapper_Move(pSessionCb->dmaWrWrapper);
    if (pSessionCb->dmaWrWrapper) nDmaWrapper_Wait(pSessionCb->dmaWrWrapper);

    // The function nDmaWrapper_FinishFrame is used to indicate that the current
    // set of frame transactions are done. It is called after all updates and
    // transfers corresponding to the frames provided in the prepare call
    // have concluded. It must be called before any subsequent calls to the DMA
    // prepare function and before the DMA engine is freed.
    nDmaWrapper_FinishFrame(pSessionCb->dmaRdWrapper);
    if (pSessionCb->dmaWrWrapper) nDmaWrapper_FinishFrame(pSessionCb->dmaWrWrapper);

    // Note that not all valid programming sequences are correct. For example,
    // the following sequence:
    // 1.   nDmaWrapper_Move(pSessionCb->dmaRdWrapper);
    // 2.   nDmaWrapper_Wait(pSessionCb->dmaRdWrapper);
    // 3.   nDmaWrapper_Move(pSessionCb->dmaWrWrapper);
    // 4.   nDmaWrapper_Wait(pSessionCb->dmaWrWrapper);
    // would still work but will essentially cause the DMA engine to perform all
    // transactions sequentially abandoning any concurrency between the read and
    // write engines.

    return nRet;
}
