#ifndef QURT_ERROR_H
#define QURT_ERROR_H

/**
  @file qurt_error.h 
  @brief Error results- QURT defines a set of standard symbols for the error result values. This file lists the
  symbols and their corresponding values.

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc..
 ======================================================================*/
#include "qurt_except.h"

/** @addtogroup chapter_error
@{ */

/*=====================================================================
Constants and macros
======================================================================*/
#define QURT_EOK                             0  /**< Operation was successfully performed. */
#define QURT_EVAL                            1  /**< Wrong values for the parameters. The specified page does not exist. */
#define QURT_EMEM                            2  /**< Not enough memory to perform the operation.*/

#define QURT_EINVALID                        4  /**< Invalid argument value; invalid key. */ 
/** @cond  */
#define QURT_EUNKNOWN                        6  /**< Defined but never used in QuRT. */
#define QURT_ENOMSGS                         7  /**< Message queue is empty. */ 
#define QURT_EBADF                           9  /**< Bad message queue descriptor. */
/** @endcond */
#define QURT_EFAILED                        12  /**< Operation failed. */ 
/** @cond RTOS_user_guide */
#define QURT_ENOTALLOWED                    13  /**< Operation not allowed. */
/** @endcond */
/** @} */ /* end_addtogroup chapter_error */
#define QURT_EDUPCLSID                      14  /**< Duplicate class ID. */
#define QURT_ENOREGISTERED                  20  /**< No registered interrupts.*/ 


/** @cond RTOS_user_guide_supplement */
/** @addtogroup chapter_error
@{ */
#define QURT_EISDB                          21  /**< Power collapse failed due to ISDB being enabled. */
#define QURT_ESTM                           22  /**< Power collapse failed in a Single-threaded mode check. */
/** @} */ /* end_addtogroup chapter_error */
/** @endcond */


/** @addtogroup chapter_error
@{ */
/** @cond RTOS_user_guide */
#define QURT_ETLSAVAIL                      23  /**< No free TLS key is available. */
#define QURT_ETLSENTRY                      24  /**< TLS key is not already free. */ 
/** @endcond */
#define QURT_EINT                           26  /**< Invalid interrupt number (not registered). */  
/** @cond RTOS_user_guide */
#define QURT_ESIG                           27  /**< Invalid signal bitmask (cannot set more than one signal at a time). */
/** @cond */
#define QURT_EHEAP                          28  /**< No heap space is available. */
/** @cond */
#define QURT_ENOSPC                         28  /**< No space to create another queue in the system. */
/** @endcond */
#define QURT_EMEMMAP                        29  /**< Physical address layout is not supported by the kernel. */
/** @endcond */
#define QURT_ENOTHREAD                      30  /**< Thread no longer exists. */
/** @cond */
#define QURT_EL2CACHE                       31  /**< L2cachable is not supported in kernel invalidate/cleaninv. */
/** @endcond */
#define QURT_EALIGN                         32  /**< Not aligned. */
#define QURT_EDEREGISTERED                  33  /**< Interrupt is already deregistered.*/
/** @endcond */
/** @} */ /* end_addtogroup chapter_error */


/** @cond RTOS_user_guide_supplement */
/** @addtogroup chapter_error
@{ */
#define QURT_ETLBCREATESIZE                 34  /**< TLB create error -- Incorrect size.*/
#define QURT_ETLBCREATEUNALIGNED            35  /**< TLB create error -- Unaligned address.*/
/** @} */ /* end_addtogroup chapter_error */
/** @endcond */
#define QURT_ENAMETOOLONG                   36  /**< Name too long for message queue creation. */
#define QURT_EPRIVILEGE                     36  /**< Caller does not have enough privilege for this operation.*/
/** @cond RTOS_user_guide */
/** @addtogroup chapter_error
@{ */
#define QURT_ECANCEL                        37  /**< A cancellable request was cancelled due to the associated process being asked to exit.*/
/** @} */ /* end_addtogroup chapter_error */
/** @endcond */

/** @cond RTOS_user_guide_supplement */
/** @addtogroup chapter_error
@{ */
#define QURT_EISLANDTRAP                    38  /**< Unsupported TRAP is called in Island mode.*/ 
/** @} */ /* end_addtogroup chapter_error */
/** @endcond */

/** @addtogroup chapter_error
@{ */
#define QURT_ERMUTEXUNLOCKNONHOLDER         39  /**< Rmutex unlock by a non-holder.*/
#define QURT_ERMUTEXUNLOCKFATAL             40  /**< Rmutex unlock error, all except the non-holder error.*/
#define QURT_EMUTEXUNLOCKNONHOLDER          41  /**< Mutex unlock by a non-holder.*/
#define QURT_EMUTEXUNLOCKFATAL              42  /**< Mutex unlock error, all except the non-holder error.*/
#define QURT_EINVALIDPOWERCOLLAPSE          43  /**< Invalid power collapse mode requested. */ 

#define QURT_EISLANDUSEREXIT                44  /**< User call has resulted in island exit.*/
#define QURT_ENOISLANDENTRY                 45  /**< Island mode had not yet been entered.*/
#define QURT_EISLANDINVALIDINT              46  /**< Island mode has been exited due to an invalid island interrupt.*/
#define QURT_ETIMEDOUT                      47  /**< Operation timed-out */
#define QURT_EALREADY                       48  /**< Operation already in progress */
#define QURT_ERETRY                         49  /**< Retry the operation */
#define QURT_EDISABLED                      50  /**< Resource disabled */
#define QURT_EDUPLICATE                     51  /**< Duplicate Resource */
#define QURT_EBADR                          53  /**< Invalid request descriptor */
#define QURT_EMSGSIZE                       90  /**< Message queue msg_len is greater than 
                                                     mq_msgsize attribute of the message queue. */

#define QURT_EFATAL                         -1  /**< Fatal error. */ 
/** @} */ /* end_addtogroup chapter_error */

#endif /* QURT_ERROR_H */
