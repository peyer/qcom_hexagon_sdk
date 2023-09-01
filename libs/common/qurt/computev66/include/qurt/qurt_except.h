#ifndef QURT_EXCEPT_H
#define QURT_EXCEPT_H

/**
  @file qurt_except.h 
  @brief Error results- Defines Cause and Cause2 code for Error-Handling.

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc..
 ======================================================================*/

/* Error codes to distinguish from multiple error.
 * SSR and BADAVA are inconclusive without vector no.
 */
/* cause - error type - 8-bits*/
/** @addtogroup chapter_error
@{ */
#define QURT_EXCEPT_PRECISE             0x01 /**< A precise exception occurred. */
#define QURT_EXCEPT_NMI                 0x02 /**< An NMI occurred. */
#define QURT_EXCEPT_TLBMISS             0x03 /**< TLBMISS RW occurred. */
#define QURT_EXCEPT_RSVD_VECTOR         0x04 /**< Interrupt was raised on reserved vector, should never happen. */
#define QURT_EXCEPT_ASSERT              0x05 /**< Kernel Assert. */
#define QURT_EXCEPT_BADTRAP             0x06 /**< trap0(#num) was called with unsupported "num". */
#define QURT_EXCEPT_UNDEF_TRAP1         0x07 /**< trap1 is not supported. Using Trap1 causes this error. */
#define QURT_EXCEPT_EXIT                0x08 /**< Application called qurt_exit() (or called qurt_exception_raise_nonfatal()). Could be called from "C" library. */
#define QURT_EXCEPT_TLBMISS_X           0x0A /**< TLBMISS X (execution) occurred. */
#define QURT_EXCEPT_STOPPED             0x0B /**< Running thread stopped due to Fatal error on other HW thread*/
#define QURT_EXCEPT_FATAL_EXIT          0x0C /**< Application called qurt_fatal_exit(). */
#define QURT_EXCEPT_INVALID_INT         0x0D /**< Kernel received an invalid L1 interrupt. */
#define QURT_EXCEPT_FLOATING_POINT      0x0E /**< Kernel received an floating point error. */
#define QURT_EXCEPT_DBG_SINGLE_STEP     0x0F /**< */
#define QURT_EXCEPT_TLBMISS_RW_ISLAND   0x10 /**< */
#define QURT_EXCEPT_TLBMISS_X_ISLAND    0x11 /**< */
#define QURT_EXCEPT_SYNTHETIC_FAULT     0x12 /**< Synthetic fault with user request that Kernel Detected. Check cause2 for more details. */
#define QURT_EXCEPT_INVALID_ISLAND_TRAP 0x13 /**< Invalid trap in island mode. */
#define QURT_EXCEPT_UNDEF_TRAP0         0x14 /**< trap0(#num) was called with unsupported "num". */

#define QURT_ECODE_UPPER_LIBC         (0 << 16)  /**< Upper 16-bits is 0 for libc */
#define QURT_ECODE_UPPER_QURT         (0 << 16)  /**< Upper 16-bits is 0 for QuRT */
#define QURT_ECODE_UPPER_ERR_SERVICES (2 << 16)  /**< Upper 16-bits is 2 for error service */

/** @} */ /* end_addtogroup chapter_error */

/** @addtogroup chapter_error
@{ */
/** @cond RTOS_user_guide */
/* Enable floating point exceptions */
#define QURT_FP_EXCEPTION_ALL        0x1F << 25 /**< */
#define QURT_FP_EXCEPTION_INEXACT    0x1 << 29 /**< */
#define QURT_FP_EXCEPTION_UNDERFLOW  0x1 << 28 /**< */
#define QURT_FP_EXCEPTION_OVERFLOW   0x1 << 27 /**< */
#define QURT_FP_EXCEPTION_DIVIDE0    0x1 << 26 /**< */
#define QURT_FP_EXCEPTION_INVALID    0x1 << 25 /**< */

/** @endcond */
/** @} */ /* end_addtogroup chapter_error */

/* Cause2 for QURT_EXCEPT_SYNTHETIC_FAULT cause- 8bits */
#define  QURT_SYNTH_ERR                         0x01
#define  QURT_SYNTH_INVALID_OP                  0x02
#define  QURT_SYNTH_DATA_ALIGNMENT_FAULT        0x03
#define  QURT_SYNTH_FUTEX_INUSE                 0x04
#define  QURT_SYNTH_FUTEX_BOGUS                 0x05
#define  QURT_SYNTH_FUTEX_ISLAND                0x06
#define  QURT_SYNTH_FUTEX_DESTROYED             0x07
#define  QURT_SYNTH_PRIVILEGE_ERR               0x08

/* Cause2 - Abort cause reason - 8bits */
/* ERR_ASSERT cause */
#define   QURT_ABORT_FUTEX_WAKE_MULTIPLE           0x01    // futex_asm.s: Abort cause - futex Wake multiple 
#define   QURT_ABORT_WAIT_WAKEUP_SINGLE_MODE       0x02    // power.c: Abort cause -Thread waiting to wake up in Single threaded  mode
#define   QURT_ABORT_TCXO_SHUTDOWN_NOEXIT          0x03    // power.c : Abort cause - tcxo shutdown is call without exit
#define   QURT_ABORT_FUTEX_ALLOC_QUEUE_FAIL        0x04    // futex.c: Abort cause - Futex aloc queue fail -  QURTK_futexhash_lifo empty
#define   QURT_ABORT_INVALID_CALL_QURTK_WARM_INIT  0x05    // init_asm.S: Abort cause - invalid Call QURTK_warm_init() in NONE CONFIG_POWER_MGMT mode
#define   QURT_ABORT_THREAD_SCHEDULE_SANITY        0x06    // switch.S: Abort cause - Sanity schedule thread is not supposed to run on current hw thread 
#define   QURT_ABORT_REMAP                         0x07    //Remap in the page table. The right behavior should be always remove mapping if necessary
#define   QURT_ABORT_NOMAP                         0x08    //No mapping in page table when remove a user mapping
#define   QURT_ABORT_OUT_OF_SPACES                 0x09
#define   QURT_ABORT_INVALID_MEM_MAPPING_TYPE      0x0A   //Invalid memory mapping type when creating qmemory
#define   QURT_ABORT_NOPOOL                        0x0B   //No pool available to attach 
#define   QURT_ABORT_LIFO_REMOVE_NON_EXIST_ITEM    0x0C   //Can not allocate more futex waiting queue
#define   QURT_ABORT_ARG_ERROR                     0x0D
#define   QURT_ABORT_ASSERT                        0x0E   // Assert abort
#define   QURT_ABORT_FATAL                         0x0F   //FATAL error that shall never happens
#define   QURT_ABORT_FUTEX_RESUME_INVALID_QUEUE    0x10   // futex_asm.s: Abort cause - invalid queue Id in futex resume
#define   QURT_ABORT_FUTEX_WAIT_INVALID_QUEUE      0x11   // futex_asm.s: Abort cause - invalid queue Id in futex wait
#define   QURT_ABORT_FUTEX_RESUME_INVALID_FUTEX    0x12   // futex.c: Abort cause - invalid futex object in hashtable
#define   QURT_ABORT_NO_ERHNDLR                    0x13   // No registered Error handler
#define   QURT_ABORT_ERR_REAPER                    0x14   // Exception in Reaper thread itself
#define   QURT_ABORT_FREEZE_UNKNOWN_CAUSE          0x15   // Abort in "thread freeze" operation
#define   QURT_ABORT_FUTEX_WAIT_WRITE_FAILURE      0x16   // During futex wait processing, could not perform a necessary
                                                          //  write operation to userland data; most likely due to a
                                                          //  DLPager eviction.
#define   QURT_ABORT_ERR_ISLAND_EXP_HANDLER        0x17   // Exception in Island exception handler task
#define   QURT_ABORT_L2_TAG_DATA_CHECK_FAIL        0x18   //  Detected error in L2 Tag/Data during warm boot
                                                          //  The L2 Tag/Data check is done when CONFIG_DEBUG_L2_POWER_COLLAPSE is enabled
#define   QURT_ABORT_ERR_SECURE_PROCESS            0x19   //  Abort Error in secure process
#define   QURT_ABORT_ERR_EXP_HANDLER               0x20   //  Either no exception handler or handler itself caused an exception
#define   QURT_ABORT_ERR_NO_PCB                    0x21   //  Thread context's PCB failed initialization, PCB was null

/* Cause2 - TLB-miss_X - 8bits */
#define  QURT_TLB_MISS_X_FETCH_PC_PAGE             0x60
#define  QURT_TLB_MISS_X_2ND_PAGE                  0x61
#define  QURT_TLB_MISS_X_ICINVA                    0x62

/* Cause2 - TLB-miss_RW - 8bits */
#define  QURT_TLB_MISS_RW_MEM_READ                 0x70
#define  QURT_TLB_MISS_RW_MEM_WRITE                0x71

/* Cause2 - Floating point exception - 8bits */
#define  QURT_FLOATING_POINT_EXEC_ERR              0xBF    // Excecute of floating-point

#endif /* QURT_EXCEPT_H */
