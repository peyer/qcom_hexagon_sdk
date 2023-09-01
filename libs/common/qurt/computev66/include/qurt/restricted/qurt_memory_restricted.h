#ifndef QURT_MEMORY_RESTRICTED_H
#define QURT_MEMORY_RESTRICTED_H
/**
  @file qurt_memory.h
  @brief  Prototypes of Kernel memory API functions

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/


/**@ingroup func_qurt_l2fetch_disable
  Disables L2FETCH activities on all hardware threads.

  @return
  None.

  @dependencies
  None.

 */
void qurt_l2fetch_disable(void);



#endif /* QURT_MEMORY_RESTRICTED_H */
