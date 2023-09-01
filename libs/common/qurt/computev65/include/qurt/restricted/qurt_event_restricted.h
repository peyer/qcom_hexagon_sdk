#ifndef QURT_EVENT_RESTRICTED_H
#define QURT_EVENT_RESTRICTED_H


/**
  @file qurt_event.h
  @brief Prototypes of Kernel event API functions

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/


/**@ingroup func_qurt_exception_wait2
  Registers the current thread as the QuRT program exception handler, and suspends the thread until a
  program exception occurs.

  When a program exception occurs, the thread is awakened with error information assigned to the specified
  error event record.

  If a program exception is raised when no handler is registered (or when a handler is registered, but it calls
  exit), the exception is treated as fatal.\n
  @note1hang If a thread runs in monitor mode, all exceptions are treated as kernel exceptions.\n
  @note1cont This function differs from qurt_exception_wait() by returning the error information in a data
              structure rather than as individual variables. It also returns additional information (for example, SSR, FP, and LR).
  @datatypes
  #qurt_sysevent_error_t

  @param[out] sys_err  Pointer to the error event record.

  @return
  Registry status: \n
  - #QURT_EFATAL -- Failure. \n
  - Thread ID -- Success.

  @dependencies
  None.
*/

static inline unsigned int qurt_exception_wait2(qurt_sysevent_error_t * sys_err)
{
   return qurt_exception_wait_ext(sys_err);
}

/**@ingroup func_qurt_exception_shutdown_fatal
  Performs the fatal shutdown procedure for handling a fatal program exception.

  For more information on the fatal shutdown procedure, see Section @xref{dox:exception_handling}.

  @note1hang This operation does not return, as it shuts down the system.

  @return
  None.

  @dependencies
  None.
*/
void qurt_exception_shutdown_fatal(void) __attribute__((noreturn));

/**@ingroup func_qurt_exception_shutdown_fatal2
  Performs the fatal shutdown procedure for handling a fatal program exception.
  This operation always returns, so the calling program can complete the fatal shutdown procedure.
  For more information on the fatal shutdown procedure, see Section @xref{dox:exception_handling}.

  @note1hang This function differs from qurt_exception_shutdown_fatal() by always returning to the caller.

  @return
  None.

  @dependencies
  None.
*/
void qurt_exception_shutdown_fatal2(void);

/**@ingroup func_qurt_exception_register_fatal_notification
  Registers a fatal exception notification handler with the RTOS kernel.

  The handler function is intended to perform the final steps of system
  shutdown after all the other shutdown actions have been performed (e.g.,
  notifying the host processor of the shutdown). It should perform only a
  minimal amount of execution.\n
  @note1hang The fatal notification handler executes on the Hexagon processor in user mode.

  @param[in] entryfuncpoint    Pointer to the handler function.
  @param[in] argp   Pointer to the argument list passed to handler function when it
                    is invoked.

  @return
  Registry status: \n
  #QURT_EOK -- Success \n
  #QURT_EVAL -- Failure; invalid parameter

  @dependencies
  None.
*/
unsigned int qurt_exception_register_fatal_notification ( void(*entryfuncpoint)(void *), void *argp);

/**@ingroup func_qurt_exception_wait_pagefault
  Registers the page fault handler.
  This function assigns the current thread as the QuRT page fault handler and
  suspends the thread until a page fault occurs.

  When a page fault occurs, the thread is awakened with page fault information
  assigned to the parameters of this operation.

  @param[out] sys_pagefault   Pointer to the page fault event record, the instruction
                              memory address where the exception occurred.

  @return
  Registry status: \n
  #QURT_EOK -- Success. \n
  #QURT_EFAILED -- Failure due to existing pager registration. \n

  @dependencies
  None.
*/

unsigned int qurt_exception_wait_pagefault (qurt_sysevent_pagefault_t *sys_pagefault);

#endif /* QURT_EVENT_RESTRICTED_H */

