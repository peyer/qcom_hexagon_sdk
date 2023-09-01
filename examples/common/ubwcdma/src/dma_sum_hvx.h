/*! *****************************************************************************
 * @file            dma_sum_hvx.h
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

#ifndef _DMA_SUM_HVX_H_
#define _DMA_SUM_HVX_H_

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef _DMA_SUM_HVX_C_
#undef _DMA_SUM_HVX_C_
#define EXTERN
#else
#define EXTERN EXTERN_C
#endif

#include "dma_def.h"
#include "dma_types.h"

/*!
 * Open parameters
 */
typedef struct stDmaSum_OpenParm {
    //! Wait type
    t_EDma_WaitType eWaitType;
    //! Number of Read dma engines to use (maximum is 2)
    int nRdDmaEngine;
} t_StDmaSum_OpenParm;

/*!
 * Move parameters
 */
typedef struct stDmaSum_MoveParm {
    //! Physical address of the sources
    void* pSrc[2];
    //! Physical address of the destination
    void* pDst[1];
    //! Physical address of the intermediate ping/pong buffers for read
    addr_t addrL2PhysAddr_IntermBufPingRd;
    addr_t addrL2PhysAddr_IntermBufPongRd;
    //! Physical address of the intermediate ping/pong buffers for write
    addr_t addrL2PhysAddr_IntermBufPingWr;
    addr_t addrL2PhysAddr_IntermBufPongWr;
    //! Virtual address of the intermediate ping/pong buffers for read
    addr_t addrL2VirtAddr_IntermBufPingRd;
    addr_t addrL2VirtAddr_IntermBufPongRd;
    //! Virtual address of the intermediate ping/pong buffers for write
    addr_t addrL2VirtAddr_IntermBufPingWr;
    addr_t addrL2VirtAddr_IntermBufPongWr;
    //! Virtual address of the hardware descriptor buffer
    void* pL2VirtAddr_HWDesc;
    //! Size of the intermediate buffers per Src/Dst buffer
    int nIntermBufSize;
    //! Total avaiable size of the descriptors
    int nHWDescSize;
    //! Frame Width
    int nFrameWidth;
    //! Frame Height
    int nFrameHeight;
    //! Walk ROI Width
    int nRoiWalkWidth;
    //! Walk ROI Height
    int nRoiWalkHeight;
    //! Format type
    t_eDmaFmt eFmt;
    //! Format type for Luma
    t_eDmaFmt eFmtLuma;
    //! Format type for Chroma
    t_eDmaFmt eFmtChroma;
    //! Chroma offset in L2
    int L2ChromaOffset;
    //! Luma stride in L2
    int L2LumaStride;
    //! Chroma stride in L2
    int L2ChromaStride;
    //! Is DDR buffer src UBWC
    bool isUbwcSrc;
    //! Is DDR buffer dst UBWC
    bool isUbwcDst;
    //! Should the intermediate buffer be padded
    bool bUse16BitPaddingInL2;
} t_StDmaSum_MoveParm;

typedef void* t_HandleSum;

/*!
 * @brief       Open a session for the sum app
 *
 * @description Allocates engines to be used for the summation using the
 *              wrapper API.
 *
 * @input       stParm - DMA open config type
 * @return      Success: Non-zero handle value
 * @n           Failure: NULL
 */
t_HandleSum hDmaSum_Open(t_StDmaSum_OpenParm stParm);

/*!
 * @brief       Executes the sum session
 *
 * @description Reads frames from the DDR into the L2 buffer,
 *              sums them pixel by pixel using the hvx and then writes them to
 *              DDR in either linear/UBWC format.
 *              The summing formula is saturate(pixel_1 + pixel_2).
 *
 * @input       handle - The handle provided by the open() function
 * @input       stParm - The move parameters
 * @return      Success: OK
 * @n           Failure: ERR
 */
int nDmaSum_Move(t_HandleSum handle, t_StDmaSum_MoveParm stParm);

/*!
 * @brief       Closes the sum session
 *
 * @description Frees all engines previous allocated during open().
 *
 * @input       handle - The handle provided by the open() function
 * @return      Success: OK
 * @n           Failure: ERR
 */
int nDmaSum_Close(t_HandleSum handle);

#endif /* _DMA_SUM_HVX_H_ */
