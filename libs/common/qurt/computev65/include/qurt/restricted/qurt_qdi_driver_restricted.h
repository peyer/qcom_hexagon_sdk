#ifndef QURT_QDI_DRIVER_RESTRICTED_H
#define QURT_QDI_DRIVER_RESTRICTED_H

/**
  @file qurt_qdi_driver.h
  @brief  Definitions, macros, and prototypes used when writing a
  QDI driver

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 20138  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

void qurt_qdi_safe_to_kill(void);      /* Mark the current thread as safe to be killed with no notification */
void qurt_qdi_unsafe_to_kill(void);    /* Mark the current thread as unsafe to be killed with no notification */

#endif
