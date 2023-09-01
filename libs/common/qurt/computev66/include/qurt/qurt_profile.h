#pragma once
/**
  @file qurt_profile.h
  @brief QuRT profiling support

EXTERNAL FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2018 by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

==============================================================================*/
#include "qurt_thread.h"

/** @addtogroup profiling_macros
    @{ */
#define QURT_PROFILE_DISABLE 0 /**< Enable profiling */
#define QURT_PROFILE_ENABLE  1 /**< Disable profiling */

typedef enum
{
    QURT_PROFILE_PARAM_THREAD_READY_TIME, /**< Profile thread ready time */
} qurt_profile_param_t;
/** @} */ /* end_addtogroup profiling_macros */

/** @addtogroup profiling_types
    @{ */
/** @brief Profiling results */
typedef union
{
    /** @brief Result associated with #QURT_PROFILE_PARAM_THREAD_READY_TIME  */
    struct
    {
        unsigned int ticks; /**< Cumulative ticks the thread was ready */
    } thread_ready_time;

} qurt_profile_result_t;
/** @} */ /* end_addtogroup profiling_types */

/**
 * @ingroup func_qurt_profile_enable2
 * Start profiling of a specific parameter on a specific thread (as applicable)
 *  
 * @param[in] param Profiling parameter
 * @param[in] thread_id ID of the thread (if applicable) for which the specified 
 *       paramter needs to be profiled
 * @param[in] enable #QURT_PROFILE_DISABLE = disable, #QURT_PROFILE_ENABLE = 
 *       enable
 *  
 * @return 
 * #QURT_EOK -- Success \n 
 * #QURT_EALREADY -- Measurement already in progress or already stopped \n 
 * #QURT_ENOTHREAD -- Thread does not exist \n 
 * #QURT_EINVALID -- Invalid profiling parameter \n
 *  
 * @dependencies 
 * None 
 *  
 */
extern int qurt_profile_enable2 (
    qurt_profile_param_t param,
    qurt_thread_t        thread_id,
    int                  enable
);

/**
 * @ingroup func_qurt_profile_get
 * Get value of the profiling parameter that was previously enabled 
 *  
 * @param[in] param Profiling parameter
 * @param[in] thread_id ID of thread (if applicable) for which the specified 
 *       profiling paramter needs to be retrieved
 * @param [out] result Profiling result associated with parameter for specified 
 *        thread (if applicable)
 *  
 * @return 
 * #QURT_EOK -- Success \n 
 * #QURT_EFAILED -- Operation failed. Profiling was not enabled \n 
 * #QURT_ENOTHREAD -- Thread does not exist \n 
 * #QURT_EINVALID -- Invalid profiling parameter \n
 *  
 * @dependencies 
 * None 
 *  
 */
extern int qurt_profile_get (
    qurt_profile_param_t    param,
    qurt_thread_t           thread_id,
    qurt_profile_result_t * result
);
