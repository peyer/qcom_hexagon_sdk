/*! *****************************************************************************
 * @file            dma_blend.h
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

#ifndef _DMA_BLEND_H_
#define _DMA_BLEND_H_

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef _DMA_BLEND_C_
#undef _DMA_BLEND_C_
#define EXTERN
#else
#define EXTERN EXTERN_C
#endif

#include "dma_def.h"
#include "dma_types.h"

/*!
 * Open parameters
 */
typedef struct stDmaBlend_OpenParm
{
    //! Wait type
    t_EDma_WaitType eWaitType;
    //! Number of Read dma engines to use (maximum is 2)
    int nRdDmaEngine;
} t_StDmaBlend_OpenParm;

/*!
 * Move parameters
 */
typedef struct stDmaBlend_MoveParm
{
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
    //! Blend alpha
    float alpha;
} t_StDmaBlend_MoveParm;

typedef void* t_HandleBlend;

/*!
 * @brief       Open session for blend
 *
 * @description Allocates engines to be used for the blend using the
 *              wrapper API.
 *
 * @input       stParm - DMA open config type
 * @return      Success: Non-zero handle value
 * @n           Failure: NULL
 */
t_HandleBlend hDmaBlend_Open(t_StDmaBlend_OpenParm stParm);

/*!
 * @brief       Executes blend session
 *
 * @description Reads frames from the DDR into the L2 buffer,
 *              blends them pixel by pixel and then writes them to DDR in either
 *              linear/UBWC format. The blending formula is
 *              alpha*pixel_1+(1-alpha)*pixel_2 where alpha is a parameter in
 *              stParm.
 *
 * @input       handle - The handle provided by the open() function
 * @input       stParm - The move parameters
 * @return      Success: OK
 * @n           Failure: ERR
 */
int nDmaBlend_Move(t_HandleBlend handle, t_StDmaBlend_MoveParm stParm);

/*!
 * @brief       Closes blend session
 *
 * @description Frees all engines previous allocated during open().
 *
 * @input       handle - The handle provided by the open() function
 * @return      Success: OK
 * @n           Failure: ERR
 */
int nDmaBlend_Close(t_HandleBlend handle);

#endif /* _DMA_BLEND_H_ */
