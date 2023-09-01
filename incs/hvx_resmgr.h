/*-----------------------------------------------------------------------------
   Copyright (c) 2017 QUALCOMM Technologies, Incorporated.
   All Rights Reserved.
   QUALCOMM Proprietary.
-----------------------------------------------------------------------------*/

#ifndef HVX_RESMGR_H_
#define HVX_RESMGR_H_

#include "qurt_hvx.h"

/* Return values */
#define RESMGR_HVX_SUCCESS                  0 //Success
#define RESMGR_HVX_LOCK_FAIL_NOAVAIL        1 //No available HVX unit to lock
#define RESMGR_HVX_LOCK_FAIL_NOASID         2 //No available ASID tracking slot
#define RESMGR_HVX_LOCK_FAIL_NOSHARE        3 //No shareable HVX unit found
#define RESMGR_HVX_LOCK_FAIL_TIMEOUT        4 //Timeout on acquiring resource lock
#define RESMGR_HVX_LOCK_FAIL_QUEUE_FULL     5 //Unable to push to the wait queue due to full event
#define RESMGR_HVX_LOCK_FAIL_INVALID_MODE   6 //Invalid mode request
#define RESMGR_HVX_LOCK_FAIL_EXIT           7 //Unknown Exit from the wait Queue
#define RESMGR_HVX_LOCK_FAIL_ALREADY        8 //Already locked from the calling thread
#define RESMGR_HVX_CONFIG_UPDATE_NO_ASID    9 //Calling ASID is not found

typedef struct {
    unsigned char bTryLockTimeout; 
    /**< 1 - enable tryLockTimeout override
         0 - use default tryLockTimeout */
    unsigned int tryLockTimeout;
    /**< Desired timeout for qurt_hvx_try_lock() API in micro seconds.
         Will be honored only if btryLockTimeout == 1.
         Should be a multiple of 100 micro-seconds */
    unsigned char bLockTimeout;
    /**< 1 - enable lockTimeout override
         0 - use default lockTimeout */
    unsigned int lockTimeout;
    /**< Desired timeout for qurt_hvx_lock() API in micro seconds.
         Will be honored only if bLockTimeout == 1.
         Should be a multiple of 100 micro-seconds */
}resmgr_hvx_config_t;

/***************************************************************************//**
 * @fn resmgr_hvx_lock_timeout
 * @brief HVX lock with user provided timeout in micro-seconds. Provided timeout
 *        should be a multiple of 100, the API will internally round it to the
 *        nearest 100 micro-second multiple otherwise (< 100 will be treated 
 *        as 0).
 * @param lock_mode - qurt_hvx_mode_t
 * @param timeout - timeout value in micro-seconds (multiple of 100)
 * @return int 0 - for success, greater than 0 for failures
 ******************************************************************************/
int resmgr_hvx_lock_timeout(qurt_hvx_mode_t lock_mode, unsigned int timeout);

/***************************************************************************//**
 * @fn resmgr_hvx_config_update
 * @brief Overrides default HVX try lock and lock timeouts with user provided
 *        values for the subsequent HVX try lock and lock calls originating from 
 *        the caller process.
 * @param config - pointer to resmgr_hvx_config_t structure
 * @return int 0 - for success
 ******************************************************************************/
int resmgr_hvx_config_update(resmgr_hvx_config_t* config);

#endif /* HVX_RESMGR_H_ */
