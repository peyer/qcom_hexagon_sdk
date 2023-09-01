/*! *****************************************************************************
 * @file            dma_types.h
 *
 * @brief   Dma frame and wait type config parameters
 *
 * @description Provides parmaters for various DMA frame formats and wait                 type configurations.
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

#ifndef _DMA_FMTID_H_
#define _DMA_FMTID_H_

/*!
 * Format IDs
 */
typedef ENUM {
    eDmaFmt_RawData,
    eDmaFmt_NV12,
    eDmaFmt_NV12_Y,
    eDmaFmt_NV12_UV,
    eDmaFmt_P010,
    eDmaFmt_P010_Y,
    eDmaFmt_P010_UV,
    eDmaFmt_TP10,
    eDmaFmt_TP10_Y,
    eDmaFmt_TP10_UV,
	eDmaFmt_NV124R,
	eDmaFmt_NV124R_Y,
	eDmaFmt_NV124R_UV,
    eDmaFmt_Invalid,
    eDmaFmt_MAX=0x7FFFFFFF,
} t_eDmaFmt;

/*!
 * Format type
 */
typedef ENUM {
    //! Format is raw data
    eDmaFmtType_RawData,
    //! Format is 8bit data
    eDmaFmtType_8Bit,
    //! Format is 10bit data
    eDmaFmtType_10Bit,
    //! Format type is invalid
    eDmaFmtType_Invalid,
	//! Format type is MAX
	eDmaFmtType_MAX=0x7FFFFFFF,
} t_eDmaFmt_Type;

/*!
 * DMA Engine wait type
 */
typedef ENUM {
    //! Continously checks if done
    eDmaWaitType_Polling,
    //! Continously checks if done sleeping in between each check
    eDmaWaitType_LowPowerPolling,
    //! Triggers an interrupt when done
    eDmaWaitType_Signal,
	eDmaWaitType_Invalid,
	eDmaWaitType_MAX=0x7FFFFFFF,
} t_EDma_WaitType;

#endif /* _DMA_FMTID_H_ */
