/*! *****************************************************************************
 * @file            dmaWrapper.h
 *
 * @brief        Wraps DMA Primitives and Utils APIs
 *
 * @description     Header for wrapper that abstract underlying calls to 
 *                  the DMA Engine Driver.
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */
#ifndef _DMAWRAPPER_H_
#define _DMAWRAPPER_H_ 

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef _DMAWRAPPER_C_
#undef _DMAWRAPPER_C_
#define EXTERN 
#else
#define EXTERN EXTERN_C
#endif

/*!
 * DMA status
 * Currently not use, for future development
 */
typedef void*	t_stDmaWrapperDmaStats;

/*!
 * Transfer type
 */
typedef enum eDmaWrapper_TransationType {
    //! DDR to L2 transfer
    eDmaWrapper_DdrToL2,
    //! L2 to DDR transfer
    eDmaWrapper_L2ToDdr,
} t_eDmaWrapper_TransationType;

/*!
 * Roi Properties
 */
typedef struct stDmaWrapper_Roi {
    //! ROI x position in pixels
    uint16 u16X;
    //! ROI y position in pixels
    uint16 u16Y;
    //! ROI width in pixels
    uint16 u16W;
    //! ROI height in pixels
    uint16 u16H;
} t_StDmaWrapper_Roi;

/*!
 * Frame Properties
 */
typedef struct stDmaWrapper_FrameProp {
    //! Starting physical address to buffer
    addr_t aAddr;
    //! Frame height in pixels 
    uint16 u16H;
    //! Frame width in pixels 
    uint16 u16W;
    //! Frame stride in pixels
    uint16 u16Stride;
} t_StDmaWrapper_FrameProp;

/*!
 * Format sub type for 8 bit formats
 */
typedef enum {
    //! Format is UBWC
    eFmtSubType8bit_Ubwc = 0,
    //! Format is linear
    eFmtSubType8bit_Lin,
    //! Format is linear 16 bit padded
    eFmtSubType8bit_Lin16BitPad,
} t_EFmtSubType_8bit;

/*!
 * Format sub type for 10 bit formats.
 * To help with error checking below enum values are explicily different than
 * t_EFmtSubType_8bit. Done by adding 1 to last enum in 8 bit version.
 */
typedef enum {
    //! Format is UBWC
    eFmtSubType10bit_Ubwc = eFmtSubType8bit_Lin16BitPad + 1,
    //! Format is linear 16 bit padded
    eFmtSubType10bit_Lin16BitPad,
} t_EFmtSubType_10bit;

/*!
 * Wrapper work descriptor if the format is raw data (currently unused)
 */
typedef struct stDmaWrapper_WorkDescripRawData {
    //! Starting physical address for source buffer
    //! For Napali V1: start address need to be 256Byte alignment
    //! For Napali V2: start address need to be pixel alignment 
    //!    (NV12 and NV12-4R are 1byte alignment, TP10 and P010 are 2byte alignment)
	//! If aSrcAddr is in TCM buffer, then TCM buffer must be L2$ cache lock
	//!    make sure the TCM buffer is >= the Walking ROI size.
	//! Please use nDmaWrapper_LockCache() to lock L2S cache
    addr_t aSrcAddr;
    //! Starting physical address for dst buffer
    //! For Napali V1: start address need to be 256Byte alignment
    //! For Napali V2: start address need to be pixel alignment 
    //!    (NV12 and NV12-4R are 1byte alignment, TP10 and P010 are 2byte alignment)
	//! If aDstAddr is in TCM buffer, then TCM buffer must be L2$ cache lock
	//!    make sure the TCM buffer is >= the Walking ROI size.
	//! Please use nDmaWrapper_LockCache() to lock L2S cache
    addr_t aDstAddr;
} t_StDmaWrapper_WorkDescripRawData;

/*!
 * Wrapper work descriptor if the format is 8 bit
 */
typedef struct stDmaWrapper_WorkDescrip8Bit {
    //! Source frame properties
    t_StDmaWrapper_FrameProp stSrcFrame;
    //! Destination frame properties
    t_StDmaWrapper_FrameProp stDstFrame;

    //! Source format sub type
    t_EFmtSubType_8bit eSrcFmtSubType;
    //! Destination format sub type
    t_EFmtSubType_8bit eDstFmtSubType;
} t_StDmaWrapper_WorkDescrip8Bit;

/*!
 * Wrapper work descriptor if the format is 10 bit
 */
typedef struct stDmaWrapper_WorkDescrip10Bit {
    //! Source frame properties
    t_StDmaWrapper_FrameProp stSrcFrame;
    //! Destination frame properties
    t_StDmaWrapper_FrameProp stDstFrame;

    //! Source format sub type
    t_EFmtSubType_10bit eSrcFmtSubType;
    //! Destination format sub type
    t_EFmtSubType_10bit eDstFmtSubType;
} t_StDmaWrapper_WorkDescrip10Bit;

/*!
 * Wrapper work descriptor
 */
typedef struct stDmaWrapper_WorkDescrip {
    //! Format
    t_eDmaFmt eFmt;
    //! Transfer type
    t_eDmaWrapper_TransationType eTxType;
    //! Union of work type, based on format selected
    union {
        //! Raw data work descriptor (currently unused)
        t_StDmaWrapper_WorkDescripRawData stRawData;
        //! 8bit format type work descriptor
        t_StDmaWrapper_WorkDescrip8Bit st8Bit;
        //! 10bit format type work descriptor
        t_StDmaWrapper_WorkDescrip10Bit st10Bit;
    } uWorkDesc;
	//! gralloc ubwc stat
	t_stDmaWrapperDmaStats* pDma_stat_handle;
} t_StDmaWrapper_WorkDescrip;

/*!
 * Roi alignment
 */
typedef struct stDmaWrapper_RoiAlignInfo {
    //! ROI width in pixels
    uint16 u16W;
    //! ROI height in pixels
    uint16 u16H;
} t_StDmaWrapper_RoiAlignInfo;

/*!
 * Parameters needed by the Prepare() wrapper API
 */
typedef struct stDmaWrapper_PrepareParm {
    //! Number of elements in staWorkDesc
    uint32 u32NumOfWorkDesc;                    
    //! Array of descriptor parameters, each element describes complete frame and ROI details
    t_StDmaWrapper_WorkDescrip* staWorkDesc;    
    //! Virtual address of the ping HW descriptor buffer (must be locked in the L2$). 
	void* pPingDescBuf;                         
    //! Virtual address of the pong HW descriptor buffer (must be locked in the L2$), 
    //! it may be set to the address of the ping buffer if not used.
    void* pPongDescBuf;                         
    //! Size in bytes of ping or pong buffer (they must have the same size).
    uint32 u32DescBufSize;                      
} t_StDmaWrapper_PrepareParm;

/*!
 * Parameters needed by the Update() wrapper API
 */
typedef struct stDmaWrapper_UpdateParm {
    //! Union of transfer update type and data
    union {
        //! Pixel data update parameters
        struct {
            //! ROI to transfer in the next Move()
            t_StDmaWrapper_Roi stRoi;
        } stPixData;
        //! Raw data update parameters (currently unused)
        struct {
            //! Offset into buffer in DDR
            uint32 u32Offset;
            //! Number of bytes to transfer in next Move()
            uint32 u32NumOfBytesToTrans;
        } stRawData;
    } u;
    //! Physical address of the TCM buffer, not used in DDR to DDR mode.
    //! The TCM address must be locked in the L2$ cache.
    //! For Napali V1 (Linear/UBWC): The start address needs to be 256 byte aligned.
    //! For Napali V2 (Linear): The start address needs to be pixel aligned. 
    //! For Napali V2 (UBWC): The start address needs to be 256 byte aligned. 
    addr_t aCacheAddr;
} t_StDmaWrapper_UpdateParm;

/*!
 * Handle for wrapper DMA engine
 */
typedef void* t_DmaWrapper_DmaEngineHandle;

/*!
 * @brief       Allocates a DMA Engine to be used
 *
 * @description Allocates a DMA Engine to be used by specifying the wait type.
 *
 * @input       eWaitType - Wait type for the DMA to use
 *
 * @return      Success: Engine's DMA Handle
 * @n           Failure: NULL
 */
EXTERN t_DmaWrapper_DmaEngineHandle hDmaWrapper_AllocDmaSpecifyWaitType(t_EDma_WaitType eWaitType);

/*!
 * @brief       Allocates a DMA Engine to be used
 *
 * @description Allocates a DMA Engine to be used by using the default
 *              wait type (polling).
 *
 * @return      Success: Engine's DMA Handle
 * @n           Failure: NULL
 */
EXTERN t_DmaWrapper_DmaEngineHandle hDmaWrapper_AllocDma(void);

/*!
 * @brief       Frees a DMA Engine
 *
 * @description Frees a DMA Engine that was previously allocated by AllocDma().
 *
 * @input       hDmaHandle - Engine's DMA Handle
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_FreeDma(t_DmaWrapper_DmaEngineHandle hDmaHandle);

/*!
 * @brief       Prepares HW descriptors with frame and maximum ROI information
 *
 * @description Prepares what is required to make the first set of
 *              transfers. Method allows for ROIs to be transferred from
 *              single or multiple frames. The maximum number of wrapper descriptors
 *              must be initialized at this stage as well as the maximum usable ROI
 *              size. These ROIs can be later updated in the update call
 *              (to smaller or equal values). 
 *
 * @input       hDmaHandle - Engine's DMA Handle
 * @input       stpParm - Prepare parameters, each element describes
 *                        complete frame and ROI details
 *
 * @return      Success: OK 
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_Prepare(t_DmaWrapper_DmaEngineHandle hDmaHandle,
                                 t_StDmaWrapper_PrepareParm* stpParm);

/*!
 * @brief       Starts a transfer request on the DMA engine
 *
 * @description Starts a transfer on the provided DMA engine. The transfer is based
 *              on descriptors constructed in earlier nDmaWrapper_Prepare() and
 *              nDmaWrapper_Update() calls.
 *
 * @input       hDmaHandle - Engine's DMA Handle 
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_Move(t_DmaWrapper_DmaEngineHandle hDmaHandle);

/*!
 * @brief       Waits for all outstanding transfers on the DMA to complete
 *
 * @description Blocks until all outstanding transfers on the DMA are complete.
 *              The wait type is based on the type specified when allocating the
 *              engine.
 *
 * @input       hDmaHandle - Engine's DMA Handle 
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_Wait(t_DmaWrapper_DmaEngineHandle hDmaHandle);

/*!
 * @brief       Update the descriptors for the next transfer
 *
 * @description Updates HW descriptors that will be used on the next nDmaWrapper_Move().
 *              Updates ROIs and L2 addresses. Each update parameter describes an 
 *              ROI transfer on next nDmaWrapper_Move() for the corresponding frame
 *              provided in the nDmaWrapper_Prepare() call. Setting all entries in
 *              the t_stDmaWrapper_Roi struct of a descriptor(u16W, u16H, u16X, u16Y)
 *              to 0xFFFF causes the DMA engine to skip over the corresponding
 *              descriptor during the next nDmaWrapper_Move().
 * 
 *              
 * @input       hDmaHandle - Engine's DMA Handle
 * @input       pStUpdateParm - Array of t_StDmaWrapper_UpdateParm structures
 *                              of the same number of elements (u32NumOfWorkDesc)
 *                              given in nDmaWrapper_Prepare().
 *                              The ordering must be the same as the staWorkDsc
 *                              array given in nDmaWrapper_Prepare().
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_Update(t_DmaWrapper_DmaEngineHandle hDmaHandle,
                                t_StDmaWrapper_UpdateParm* pStUpdateParm);

/*!
 * @brief       Cleans up all transfers and flushes DMA buffers
 *
 * @description This call flushes the DMA buffers.
 *              Blocks until the flush of the DMA is complete. 
 *
 * @input       hDmaHandle - Engine's DMA Handle 
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_FinishFrame(t_DmaWrapper_DmaEngineHandle hDmaHandle);

/*!
 * @brief       Get the recommended walk ROI width and height
 *
 * @description Get the recommended walk ROI width and height that should 
 *              be used if walking the entire frame. The ROI returned is always
 *              in terms of frame dimensions. This function is different from
 *              nDmaWrapper_GetRecommendedRoi() as coordinates are not used.
 *
 * @input       eFmtId - Format ID
 * @input       bIsUbwc - Is the format UBWC (TRUE/FALSE)
 * @inout       pStWalkSize - Initial walk size, will be overwritten with
 *                            the recommended walk size to align with DMA 
 *                            requirements
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_GetRecommendedWalkSize(t_eDmaFmt eFmtId, bool bIsUbwc,
                                                t_StDmaWrapper_RoiAlignInfo* pStWalkSize);

/*!
 * @brief       Get the recommended ROI
 *
 * @description Get the recommended ROI based on format and initial ROI. This is different 
 *              from nDmaWrapper_GetRecommendedWalkSize in that the coordinates (X and Y) 
 *              of the ROI are also adjusted to align with DMA requirements.
 *				Similar to nDmaWrapper_GetRecommendedWalkSize, the ROI returned
 *				is relative to frame dimensions.
 *
 * @input       eFmtId - Format ID
 * @input       bIsUbwc - Is the format UBWC (TRUE/FALSE)
 * @inout       pStRoi - Initial ROI, will be overwritten with the recommended ROI
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_GetRecommendedRoi(t_eDmaFmt eFmtId, bool bIsUbwc,
                                           t_StDmaWrapper_Roi* pStRoi);

/*!
 * @brief       Get the minimum ROI size
 *
 * @description Get the minimum ROI size that can be used for the provided format.
 *				Similar to nDmaWrapper_GetRecommendedWalkSize, the ROI returned
 *				is relative to frame dimensions.
 *
 * @input       eFmtId - Format ID
 * @input       bIsUbwc - Is the format UBWC (TRUE/FALSE)
 * @output      pStRoiAlign - The minimum ROI that may be used
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_GetMinRoiSize(t_eDmaFmt eFmtId, bool bIsUbwc,
                                       t_StDmaWrapper_RoiAlignInfo* pStRoiAlign);

/*!
 * @brief       Get the format type
 *
 * @description Determines which union element
 *              to use in the work descriptor.
 *
 * @input       eFmtId - Format ID
 *
 * @return      Success: enum of format type
 * @n           Failure: eDmaFmt_Invalid
 */
EXTERN t_eDmaFmt_Type eDmaWrapper_GetFmtType(t_eDmaFmt eFmtId);

/*!
 * @brief       Get the DMA alignment requirements based on format
 *
 * @description Determines the DMA alignment requirements given the frame format ID.
 *
 * @input       eFmt - Format ID
 * @input       bIsUbwc - Is the format UBWC (TRUE/FALSE)
 * @output      pStPixAlignInfo - Alignment information   
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_GetFmtAlignment(t_eDmaFmt eFmt, bool bIsUbwc,
                                         t_StDmaWrapper_RoiAlignInfo* pStPixAlignInfo);

/*!
 * @brief       Lock the L2 cache
 *
 * @description Lock the virtual address provided with the required size. This is used
 *              to map the locked area into the L2 cache (TCM).
 *
 * @input       pVirtAddr - Virtual address to lock in the cache (needs to be aligned to 32 bytes)
 * @input       u32Size - The number of bytes to lock (needs to be a multiple of 32)
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_LockCache(void* pVirtAddr, uint32 u32Size);

/*!
 * @brief       Unlock the L2 cache
 *
 * @description Unlock the virtual address provided in the L2 cache. The size of the
 *              region to unlock must be provided (typically the same as what was locked).
 *
 * @input       pVirtAddr - Virtual address to unlock in the cache (needs to be aligned to 32 bytes)
 * @input       u32Size - The number of bytes to unlock (needs to be a multiple of 32)
 *
 * @return      Success: OK
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_UnlockCache(void* pVirtAddr, uint32 u32Size);

/*!
 * @brief       Get the recommended intermediate buffer size
 *
 * @description Get the recommended intermediate buffer size for the L2 cache
 *              that is used to transfer data to/from DDR. 
 *         
 * @input       eFmtId - Format ID
 * @input       bUse16BitPaddingInL2 - Is padding to 16 bits done in the L2 (TRUE/FALSE)
 * @input       pStRoiSize - The ROI that will be used (should be aligned with
 *                           DMA requirements for the format). The Chroma ROI must
 *                           follow the standing convention that the provided
 *                           width and height must be specified in terms of the
 *                           Luma plane also such that when the width is divided
 *                           by 2 it specifies the number of interleaved Chroma
 *                           pixels.
 * @input		bIsUbwc - Is the format UBWC (TRUE/FALSE), note that this should
 *                        be set to TRUE if either the DDR input or output will
 *                        be UBWC
 * @input		u16IntermBufStride - The stride (in pixels) to use, the minimum
 *                                   stride may be obtained by calling
 *                                   nDmaWrapper_GetRecommendedIntermBufStride
 *
 * @return      Success: The intermediate buffer size in bytes
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_GetRecommendedIntermBufSize(t_eDmaFmt eFmtId, bool bUse16BitPaddingInL2,
                                                     t_StDmaWrapper_RoiAlignInfo* pStRoiSize,
                                                     bool bIsUbwc, uint16 u16IntermBufStride);

/*!
 * @brief       Get the offset to the Chroma address from the Luma plane
 *
 * @description Calculates the Chroma offset from the start of the Luma plane.
 *              This function should only be used on frames in DDR and not for
 *              buffers in the L2 (use nDmaWrapper_GetRecommendedIntermBufSize
 *              in the L2 instead). The format ID passed into this function
 *              should be a combination of the Y and UV planes. 
 *              For example, eDmaFmt_NV12 should be used instead of eDmaFmt_NV12_Y
 *              or eDmaFmt_NV12_UV.
 *
 * @input       eFmtId - Format ID
 * @input       pStFrameProp - Frame Properties
 * @input       bIsUbwc - Is the format UBWC (TRUE/FALSE)
 *
 * @return      Offset to the Chroma address in bytes
 */
EXTERN addr_t addrDmaWrapper_GetChromaOffset(t_eDmaFmt eFmtId,
                                             t_StDmaWrapper_FrameProp* pStFrameProp,
                                             bool bIsUbwc);

/*!
 * @brief       Check if the DMA driver is ready for use
 *
 * @description Check if the DMA driver is ready to be used.
 *
 * @input       none
 *
 * @return      Success: OK; Ready for use
 * @n           Failure: ERR; Not ready
 */
EXTERN int32 nDmaWrapper_isDmaDriverReady(void);

/*!
 * @brief       Populate the wrapper's work descriptor
 *
 * @description Populate the wrapper's work descriptor with format, ROI and frame details.
 *
 * @input       eFmt - Pixel format
 * @input       eTxType - Translation type: eDmaWrapper_DdrtoL2 or eDmaWrapper_L2toDdr
 * @input       WalkRoi - Pointer to a structure of WalkROI parameters
 *                        (aAddr, u16W, u16H, u16Stride), the stride can be set
 *                        to zero to let the firmware choose an appropriate stride
 * @input       Frame - Pointer to a Structure of frame parameter
 *                      (aAddr, u16W, u16H, u16Stride)
 * @input       isUbwc - Is the format UBWC (TRUE/FALSE)
 * @input       bUse16BitPaddingInL2 - Is padding to 16 bits done in the L2 (TRUE/FALSE)
 * @input       pDma_status_handle - Pointer to the DMA status,
 *                                  currently not use. Set to NULL
 *
 * @output		staWorkDesc - The populated wrapper work descriptor structure.
 *
 * @return      Success: OK
 * @n           Failure: ERR 
 */
EXTERN int32 nDmaWrapper_WorkDescrip_populate(t_StDmaWrapper_WorkDescrip *staWorkDesc,
                                              t_eDmaFmt eFmt,
                                              t_eDmaWrapper_TransationType eTxType,
                                              t_StDmaWrapper_FrameProp *WalkRoi,
                                              t_StDmaWrapper_FrameProp *Frame,
                                              bool isUbwc,
                                              bool bUse16BitPaddingInL2,
                                              t_stDmaWrapperDmaStats *pDma_status_handle);

/*!
 * @brief       Get the frame size
 *
 * @description Calculates the frame size for a given UBWC/Linear format.
 *
 * @input       eFmtId - Format ID
 * @input       pStFrameProp - Frame Properties (the aAddr member is not used)
 * @input       bIsUbwc - Is the frame UBWC (TRUE/FALSE)
 *
 * @return      Frame size in bytes
 */
EXTERN int32 nDmaWrapper_GetFramesize(t_eDmaFmt eFmtId,
                                      t_StDmaWrapper_FrameProp* pStFrameProp,
                                      bool bIsUbwc);

/*!
 * @brief       Get the HW descriptor buffer size per DMA engine
 *
 * @description Calculates the HW descriptor buffer size based
 *              on the formats that will be used with the engine.
 *
 * @input       aeFmtId - Array of format IDs, such as eDmaFmt_NV12, eDmaFmt_NV12_Y,
 *                        eDmaFmt_NV12_UV etc..
 * @input       nsize - Number of format IDs provided
 *
 * @return      Descriptor buffer size in bytes
 */
EXTERN int32 nDmaWrapper_GetDescbuffsize(t_eDmaFmt *aeFmtId, uint16 nsize);

/*!
 * @brief       Get the recommended intermediate buffer stride.
 *
 * @description Get the recommended (minimum) intermediate buffer stride for the
 *              L2 Cache that is used transfer data from/to DDR. The stride is
 *              greater than or equal to the width and must be a multiple of 256. 
 *
 * @input       eFmtId - Format ID
 * @input		pStRoiSize - The ROI that will be used (should be aligned with
 *                           the DMA requirements for the format)
 * @input		bIsUbwc - Is the format UBWC (TRUE/FALSE)
 *
 * @return      Success: The intermediate buffer stride in pixels
 * @n           Failure: ERR
 */
EXTERN int32 nDmaWrapper_GetRecommendedIntermBufStride(t_eDmaFmt eFmtId,
                                                       t_StDmaWrapper_RoiAlignInfo* pStRoiSize,
                                                       bool bIsUbwc);

#endif /* _DMAWRAPPER_H_ */

/** @example dma_memcpy.c
 *  Copies one region of memory to another.
 *  @example dma_memcpy.h
 *  @example dma_memcpy_test.c
 */

/** @example dma_blend.c
 *  DMA Blend App. Will read 2 frames from DDR, blend them and output
 *  the result to DDR.
 *  @example dma_blend.h
 *  @example dma_blend_test.c
 */
