#ifndef QURT_THREAD_CONTEXT_RESTRICTED_H
#define QURT_THREAD_CONTEXT_RESTRICTED_H
/**
  @file qurt_thread_context.h 
  @brief Kernel thread context structure
			
EXTERNAL FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/



//#include <qurt_qdi_constants.h>



/* return: size of debug thread info structure, may change without warning */
size_t qurt_system_tcb_dump_get_size(void);

#endif
