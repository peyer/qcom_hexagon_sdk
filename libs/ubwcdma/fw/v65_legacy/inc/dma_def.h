/*! *****************************************************************************
 * @file            dma_def.h
 * 
 * @services        Common definitions
 * 
 * @description     Header file for UBWCDMA sub-system definitions for
 *                  the Cosim platform.
 * 
 * 
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 ********************************************************************************
 */

#ifndef _UBWCDMA_DEF_H_
#define _UBWCDMA_DEF_H_

#include "qurt.h"
#ifndef COSIM_PLATFORM_QURT
#include "HAP_farf.h"
#endif

/*!
 * status and errors
 */
#ifdef OK
#undef OK
#endif
#define OK                                  0
#define ERR                                 -1

/*!
 * NULL
 */
#ifndef NULL
#define NULL                                0
#endif

/*!
 * BOOL
 */
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
#define FALSE                               0
#define TRUE                                (!FALSE)

/*!
 * Platform specific
 */
//#define DMA_FW_OP_STANDALONE   //! FW runs on PC stubbed simulation
//#define DMA_FW_OP_CVERIF       //! FW runs on CVerif model
//#define COSIM_PLATFORM
//#define UBWCDMA_DEBUG 1
/*!
 * Defaults
 */
//#define DMA_ASSERT_FLAG                     eDmaAssert_Continue   //! continue and no blocking on assert

/* Mapping to COSIM platform*/
#ifdef COSIM_PLATFORM_QURT
#define UBWCDMA_REGS_ADDR 0x91000000
#else
#define UBWCDMA_REGS_ADDR 0x082C0000 
#endif
#define UBWCDMA_REGS_SIZE 0x20000

#define DDR_BUF_ADDR 0x82000000
#define DDR_BUF_SIZE 0x7DFFFF00

#define TCM_BUF_ADDR 0x80000000
#define TCM_BUF_SIZE 0x1000000

#define TCM_DESC_ADDR 0x81000000
#define TCM_DESC_SIZE 0x10000

#define UBWCDMA_WRITE_REG(reg, value) (*(volatile unsigned int *)(UBWCDMA_REGS_ADDR+reg)=value)
#define UBWCDMA_READ_REG(reg) (*(volatile unsigned int *)(UBWCDMA_REGS_ADDR+reg))
#define LINEAR_PIXEL_ALIGNMENT

/*!
 * Debug helpers and options
 */
#if defined VP_BRINGUP || defined COSIM_PLATFORM_QURT
#define LOGF    qurt_printf
#define LOGE    qurt_printf
#define LOGP    qurt_printf
#define LOGM    qurt_printf
#else
#ifdef __hexagon__
#define LOGF(...)    FARF(ALWAYS, __VA_ARGS__)
#define LOGE(...)    FARF(ALWAYS, __VA_ARGS__)
#define LOGP(...)    FARF(ALWAYS, __VA_ARGS__)
#define LOGM(...)    FARF(ALWAYS, __VA_ARGS__)
#else
#define LOGF    printf
#define LOGE    printf
#define LOGP    printf
#define LOGM    printf
#endif
#endif

#define DBG_LOGF(fmt,...)                   LOGF("!!!\tUBWCDMA:Fatal: %s: " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DBG_LOGE(fmt,...)                   LOGE("***\tUBWCDMA:Err: %s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#ifdef UBWCDMA_DEBUG
#define DBG_ASSERT(cond)                    vDmaSvcApp_Assert((cond), __FUNCTION__, __LINE__, #cond, __FILE__)
#define DBG_LOGP(fmt,...)                   LOGP("\tUBWCDMA:Perf: %s " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DBG_LOGI(fmt,...)                   LOGM("\tUBWCDMA:Info: %s " fmt, __FUNCTION__, ##__VA_ARGS__)
#define DBG_ENTER()                         LOGM("\tUBWCDMA_Enter: %s\n", __FUNCTION__)
#define DBG_EXIT()                          LOGM("\tUBWCDMA_Exit: %s\n", __FUNCTION__)
#else
#define DBG_ASSERT(cond)
#define DBG_LOGP(...)
#define DBG_LOGI(...)
#define DBG_ENTER()
#define DBG_EXIT()
#endif

#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif

#define MAX32                               0xffffffff

/*!
 * types
 */
#ifdef __LP64__
typedef unsigned long long addr_t; 
#else
typedef unsigned long addr_t; 
#endif

typedef signed long long int64;
#if defined(__LP64__) || defined(__chess__) || defined(_MACH_I32)
typedef signed int int32;
#else
typedef signed long int32;
#endif
typedef signed short int16;
typedef signed char int8;

typedef unsigned long long uint64;
#if defined(__LP64__) || defined(__chess__) || defined(_MACH_I32)
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef float float32;
typedef int32 FLOAT16_16;

#ifndef __cplusplus
typedef unsigned char bool;
#endif

/*!
 * Assert
 */
    //! don't loop, continue on
#define eDmaAssert_Continue                 0
    //! loop, stuck infinitely
#define eDmaAssert_Loop                     1
    //! sys, map to system assert, which may abort
#define eDmaAssert_Sys                      2

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

#ifdef UBWCDMA_DEBUG
EXTERN_C void vDmaSvcApp_Assert (int iCond, const char * fName, int iLine,
    const char* sCond, const char* sName);
#endif
#ifdef VP_BRINGUP
EXTERN_C int dma_main(void);
#endif

/*******************************************************************************
 * Cosim Init Function Prototypes
 *******************************************************************************/
EXTERN_C int nDma_InitializeUbwcRegisterMemory();
EXTERN_C int nDma_Initialize();

#endif /* _UBWCDMA_DEF_H_ */
