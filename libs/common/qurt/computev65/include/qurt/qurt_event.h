#ifndef QURT_EVENT_H
#define QURT_EVENT_H
/**
  @file qurt_event.h
  @brief Prototypes of Kernel event API functions

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2018 by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include "qurt_consts.h"
#include "qurt_thread.h"

/*
 * System environment object type.
 */
/**@addtogroup sys_env_types
@{ */
/** QuRT swap pool information type. */
typedef struct qurt_sysenv_swap_pools {
   /** @cond */
   unsigned int spoolsize; /*swap pool size*/
   unsigned int spooladdr;   /*swap pool start address*/
   /** @endcond */
}qurt_sysenv_swap_pools_t;

/**QuRT application heap information type. */
typedef struct qurt_sysenv_app_heap {
   /** @cond */
   unsigned int heap_base; /*heap base address*/
   unsigned int heap_limit; /*heap end address*/
   /** @endcond */
} qurt_sysenv_app_heap_t ;

/** QuRT architecture version information type. */
typedef struct qurt_sysenv_arch_version {
   /** @cond */
    unsigned int arch_version; /*architecture version*/
    /** @endcond */
}qurt_arch_version_t;

/** QuRT maximum hardware threads information type. */
typedef struct qurt_sysenv_max_hthreads {
   /** @cond */
   unsigned int max_hthreads; /*maximum number of hardware threads*/
   /** @endcond */
}qurt_sysenv_max_hthreads_t;

/** QuRT active hardware threads information type. */
typedef struct qurt_sysenv_hthreads {
   /** @cond */
   unsigned int hthreads; /*maximum number of hardware threads*/
   /** @endcond */
}qurt_sysenv_hthreads_t;

/** QuRT maximum pi priority information type. */
typedef struct qurt_sysenv_max_pi_prio {
     /** @cond */
    unsigned int max_pi_prio; /*max pi priority*/
     /** @endcond */
}qurt_sysenv_max_pi_prio_t;

/*QuRT timer information type. */
typedef struct qurt_sysenv_timer_hw {
     /** @cond */
   unsigned int base; /*time frame address base*/
   unsigned int int_num; /* timer interrupt number*/
    /** @endcond */
}qurt_sysenv_hw_timer_t;

/** QuRT process name information type. */
typedef struct qurt_sysenv_procname {
     /** @cond */
   unsigned int asid; /*address space ID*/
   char name[QURT_MAX_NAME_LEN]; /*process name*/
    /** @endcond */
}qurt_sysenv_procname_t;

/** QuRT stack profile count information type. */
typedef struct qurt_sysenv_stack_profile_count {
     /** @cond */
   unsigned int count; /*stack profile count for usage*/
   unsigned int count_watermark; /*stack profile count for watermark*/
    /** @endcond */
}qurt_sysenv_stack_profile_count_t;

/**
 QuRT system error event type.
 */
typedef struct _qurt_sysevent_error_t
{
    unsigned int thread_id; /**< Thread ID.  */
    unsigned int fault_pc;  /**< Fault PC. */
    unsigned int sp;        /**< Stack pointer. */
    unsigned int badva;     /**< Virtual data address where the exception occurred. */
    unsigned int cause;     /**< QuRT error result. */
    unsigned int ssr;       /**< Supervisor status register. */
    unsigned int fp;        /**< Frame pointer. */
    unsigned int lr;        /**< Link register. */
    unsigned int pid;       /**< PID of the process that this thread belongs to*/
 } qurt_sysevent_error_t ;

/** QuRT page fault error event information type. */
typedef struct qurt_sysevent_pagefault {
    qurt_thread_t thread_id; /**< Thread ID of the page fault thread. */
    unsigned int fault_addr; /**< Accessed address that caused the page fault. */
    unsigned int ssr_cause;  /**< SSR cause code for the page fault. */
} qurt_sysevent_pagefault_t ;
/** @} */ /* @endaddtogroup sys_env_types */
/*=============================================================================
                                    FUNCTIONS
=============================================================================*/

/*======================================================================*/
/**
  Gets the environment swap pool 0 information from the kernel.

  @datatypes
  #qurt_sysenv_swap_pools_t

  @param[out] pools  Pointer to the pools information.

  @return
  EOK -- SUCCESS.

  @dependencies
  None.
*/
int qurt_sysenv_get_swap_spool0 (qurt_sysenv_swap_pools_t *pools );

/*
  Gets the environment swap pool 1 information from the kernel.

  @datatypes
  #qurt_sysenv_swap_pools_t

  @param[out] pools  Pointer to the pools information.

  @return
  EOK -- SUCCESS.

  @dependencies
  None.
*/
int qurt_sysenv_get_swap_spool1(qurt_sysenv_swap_pools_t *pools );

/**@ingroup func_qurt_sysenv_get_app_heap
  Gets information on the program heap from the kernel.

  @datatypes
  #qurt_sysenv_app_heap_t

  @param[out] aheap  Pointer to information on the program heap.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
int qurt_sysenv_get_app_heap(qurt_sysenv_app_heap_t *aheap );

/**@ingroup func_qurt_sysenv_get_hw_timer
  Gets the memory address of the hardware timer from the kernel.

  @datatypes
  #qurt_sysenv_hw_timer_t

  @param[out] timer  Pointer to the memory address of the hardware timer.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
int qurt_sysenv_get_hw_timer(qurt_sysenv_hw_timer_t *timer );

/**@ingroup func_qurt_sysenv_get_arch_version
  Gets the Hexagon processor architecture version from the kernel.

  @datatypes
  #qurt_arch_version_t

  @param[out] vers  Pointer to the Hexagon processor architecture version.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter

  @dependencies
  None.
*/
int qurt_sysenv_get_arch_version(qurt_arch_version_t *vers);

/**@ingroup func_qurt_sysenv_get_max_hw_threads
  Gets the maximum number of hardware threads supported in the Hexagon processor. 
  The API includes the disabled hardware threads in order to reflect the maximum 
  hardware thread count.
  e.g. If image is configured for 4 HW threads and hthread_mask is set to 0x5 in 
  cust_config.xml, it would mean only HW0 and HW2 would be initialized by QuRT.
  HW1 and HW3 would not be used at all. Under such scenario, 
  qurt_sysenv_get_max_hw_threads will still return 4.

  @datatypes
  #qurt_sysenv_max_hthreads_t

  @param[out] mhwt  Pointer to the maximum number of hardware threads supported in the Hexagon processor.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
int qurt_sysenv_get_max_hw_threads(qurt_sysenv_max_hthreads_t *mhwt );

/**@ingroup func_qurt_sysenv_get_hw_threads
  Gets the number of hardware threads initialized by QuRT in Hexagon processor.
  e.g. If image is configured for 4 HW threads and hthread_mask is set to 0x5 in 
  cust_config.xml, it would mean only HW0 and HW2 would be initialized by QuRT.
  HW1 and HW3 would not be used at all. Under such scenario, 
  qurt_sysenv_get_hw_threads will return 2.
  @datatypes
  #qurt_sysenv_hthreads_t

  @param[out] hwt  Pointer to the number of hardware threads active in the Hexagon processor.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
int qurt_sysenv_get_hw_threads(qurt_sysenv_hthreads_t *mhwt );

/**@ingroup func_qurt_sysenv_get_max_pi_prio
  Gets the maximum priority inheritance mutex priority from the kernel.

  @datatypes
  #qurt_sysenv_max_pi_prio_t

  @param[out] mpip  Pointer to the maximum priority inheritance mutex priority.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
int qurt_sysenv_get_max_pi_prio(qurt_sysenv_max_pi_prio_t *mpip );

/**@ingroup func_qurt_sysenv_get_process_name
  Gets information on the system environment process names from the kernel.

  @datatypes
  #qurt_sysenv_procname_t

  @param[out] pname  Pointer to information on the process names in the system.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
int qurt_sysenv_get_process_name(qurt_sysenv_procname_t *pname );

/**@ingroup func_qurt_sysenv_get_stack_profile_count
   Gets information on the stack profile count from the kernel.

   @datatypes
   #qurt_sysenv_stack_profile_count_t

   @param[out] count Pointer to information on the stack profile count.

   @return
   #QURT_EOK -- Success.

   @dependencies
   None.
*/
int qurt_sysenv_get_stack_profile_count(qurt_sysenv_stack_profile_count_t *count );

/**@ingroup func_qurt_exception_wait
  Registers the program exception handler.
  This function assigns the current thread as the QuRT program exception handler and suspends the
  thread until a program exception occurs.

  When a program exception occurs, the thread is awakened with error information
  assigned to the parameters of this operation.

  @note1hang If no program exception handler is registered, or if the registered handler
             calls exit, then QuRT raises a kernel exception.
             If a thread runs in Supervisor mode, any errors are treated as kernel
             exceptions.

  @param[out]  ip      Pointer to the instruction memory address where the exception occurred.
  @param[out]  sp      Stack pointer.
  @param[out]  badva   Pointer to the virtual data address where the exception occurred.
  @param[out]  cause   Pointer to the QuRT error result code.

  @return
  Registry status: \n
  - Thread identifier -- Handler successfully registered. \n
  - #QURT_EFATAL -- Registration failed.

  @dependencies
  None.
*/
unsigned int qurt_exception_wait (unsigned int *ip, unsigned int *sp,
                                  unsigned int *badva, unsigned int *cause);

unsigned int qurt_exception_wait_ext (qurt_sysevent_error_t * sys_err);

/**@ingroup func_qurt_exception_raise_nonfatal
  Raises a nonfatal program exception in the QuRT program system.

  For more information on program exceptions, see Section @xref{dox:exception_handling}.

  This operation never returns -- the program exception handler is assumed to perform all
  exception handling before terminating or reloading the QuRT program system.

  @note1hang The C library function abort() calls this operation to indicate software
             errors.

  @param[in] error QuRT error result code (Section @xref{dox:error_results}).

  @return
  Integer -- Unused.

  @dependencies
  None.
*/
int qurt_exception_raise_nonfatal (int error) __attribute__((noreturn));


/**@ingroup func_qurt_exception_raise_fatal
  Raises a fatal program exception in the QuRT system.

  Fatal program exceptions terminate the execution of the QuRT system without invoking
  the program exception handler.

  For more information on fatal program exceptions, see Section @xref{dox:exception_handling}.

  This operation always returns, so the calling program can perform the necessary shutdown
  operations (data logging, etc.).

  @note1hang Context switches do not work after this operation has been called.

  @return
  None.

  @dependencies
  None.
*/
void qurt_exception_raise_fatal (void);

unsigned int qurt_enable_floating_point_exception(unsigned int mask);

/**@ingroup func_qurt_exception_enable_fp_exceptions
  Enables the specified floating point exceptions as QuRT program exceptions.

  The exceptions are enabled by setting the corresponding bits in the Hexagon
  control register USR.

  The mask argument specifies a mask value identifying the individual floating
  point exceptions to be set. The exceptions are represented as defined symbols
  that map into bits 0 through 31 of the 32-bit flag value.
  Multiple floating point exceptions are specified by OR'ing together the individual
  exception symbols.\n
  @note1hang This function must be called before any floating point operations are performed.

  @param mask Floating point exception types. Values: \n
             - #QURT_FP_EXCEPTION_ALL    \n
             - #QURT_FP_EXCEPTION_INEXACT    \n
             - #QURT_FP_EXCEPTION_UNDERFLOW  \n
             - #QURT_FP_EXCEPTION_OVERFLOW  \n
             - #QURT_FP_EXCEPTION_DIVIDE0    \n
             - #QURT_FP_EXCEPTION_INVALID   @tablebulletend

  @return
  Updated contents of the USR register.

  @dependencies
  None.
*/

static inline unsigned int qurt_exception_enable_fp_exceptions(unsigned int mask)
{
   return qurt_enable_floating_point_exception(mask);
}




#endif /* QURT_EVENT_H */
