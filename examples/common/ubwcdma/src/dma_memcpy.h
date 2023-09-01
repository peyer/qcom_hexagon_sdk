/*! *****************************************************************************
 * @file            dma_memcpy.h
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

#ifndef _DMA_MEMCPY_H_
#define _DMA_MEMCPY_H_

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef _DMA_MEMCPY_C_
#undef _DMA_MEMCPY_C_
#define EXTERN
#else
#define EXTERN EXTERN_C
#endif

#include "dma_def.h"
#include "dma_types.h"

/*!
 * Open parameters
 */
typedef struct stDmaMemCpy_OpenParm {
    //! Wait type
    t_EDma_WaitType eWaitType;
    // Enable sink (DMA writer)
    bool bSinkEn;
} t_StDmaMemCpy_OpenParm;

/*!
 * Move parameters
 */
typedef struct stDmaMemCpy_MoveParm {
    //! Physical address of the source
    void* pSrc[1];
    //! Physical address of the destination
    void* pDst[1];
    //! Physical address of the intermediate ping buffer
    addr_t addrL2PhysAddr_IntermBufPing;
    //! Physical address of the intermediate pong buffer
    addr_t addrL2PhysAddr_IntermBufPong;
    //! Virtual address of the hardware descriptor buffer
    void* pL2VirtAddr_HWDesc;
    //! Size of the intermediate buffers per Src/Dst buffer
    int nIntermBufSize;
    //! Total avaiable size of the descriptors
    int nHWDescSize;
    //! Frame width
    int nFrameWidth;
    //! Frame height
    int nFrameHeight;
    //! Walk ROI width
    int nRoiWalkWidth;
    //! Walk ROI height
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
} t_StDmaMemCpy_MoveParm;

typedef void* t_HandleMemCpy;

/*!
 * @brief       Open session for memcpy
 *
 * @description Allocates 2 engines to be used for the memcpy using the
 *              wrapper API.
 *
 * @input       stParm - DMA open config type
 * @return      Success: Non-zero handle value
 * @n           Failure: NULL
 */
t_HandleMemCpy hDmaMemCpy_Open(t_StDmaMemCpy_OpenParm stParm);

/*!
 * @brief       Executes memcpy session
 *
 * @description Copies memory from a source frame in DDR to an intermediate
 *              buffer in TCM then to a new destination buffer in DDR. Memory is
 *              copied in chunks otherwise known as ROIs in row major order.
 *
 * @input       handle - The handle provided by the open() function
 * @input       stParm - The move parameters
 * @return      Success: OK
 * @n           Failure: ERR
 */
int nDmaMemCpy_Move(t_HandleMemCpy handle, t_StDmaMemCpy_MoveParm stParm);

/*!
 * @brief       Closes memcpy session
 *
 * @description Frees all engines previous allocated during open().
 *
 * @input       handle - The handle provided by the open() function
 * @return      Success: OK
 * @n           Failure: ERR
 */
int nDmaMemCpy_Close(t_HandleMemCpy handle);

#endif /* _DMA_MEMCPY_H_ */
