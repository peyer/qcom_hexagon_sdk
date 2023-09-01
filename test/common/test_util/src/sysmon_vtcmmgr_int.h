/*-----------------------------------------------------------------------------
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
-----------------------------------------------------------------------------*/

#ifndef SYSMON_VTCMMGR_INT_H_
#define SYSMON_VTCMMGR_INT_H_


/** HAP VTCM allocation error list */
#define HAP_VTCM_ALLOC_FAIL_NO_MEM                          1
#define HAP_VTCM_ALLOC_FAIL_INVALID_ARGS                    2
#define HAP_VTCM_ALLOC_FAIL_MAILBOX_FULL                    3

#define HAP_VTCM_ALLOC_ASYNC_INVALID_ARGS                   1
#define HAP_VTCM_ALLOC_ASYNC_WAITQ_FULL                     2

/** HAP_release_VTCM error list */
#define HAP_VTCM_RELEASE_FAIL_QDI                          -1
#define HAP_VTCM_RELEASE_FAIL_INVALID_ARGS                  1
#define HAP_VTCM_RELEASE_FAIL_MEMCLEAR                      2
#define HAP_VTCM_RELEASE_FAIL_MEMREGION_DEL                 3

/** HAP query VTCM error list */
#define HAP_VTCM_QUERY_FAIL_QDI                            -1
#define HAP_VTCM_QUERY_FAIL_QDI_INVOKE                     -2
#define HAP_VTCM_QUERY_FAIL_INIT_NOTDONE                   -3

/**************************************************************************//**
 * @fn sysmon_vtcm_mgr_init
 * @brief Init function to be called at DSP init. Initializes VTCM 
 *        Manager mem nodes and QDI interfaces.
 *****************************************************************************/
void sysmon_vtcmMgr_init(void);

#endif /* SYSMON_VTCMMGR_INT_H_ */
