#ifndef QURT_RESTRICTED_H
#define QURT_RESTRICTED_H 

/**
  @file qurt.h 
  @brief  Contains kernel header files which provides kernel OS API functions, constants, and 
  definitions 

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/
/*======================================================================
 *
 *											 EDIT HISTORY FOR FILE
 *
 *	 This section contains comments describing changes made to the
 *	 module. Notice that changes are listed in reverse chronological
 *	 order.
 *
 *	
 *
 *
 * when 				who 		what, where, why
 * ---------- 	--- 		------------------------------------------------
 * 2011-02-25 	op			Add Header file
   2012-12-16   cm          (Tech Pubs) Edited/added Doxygen comments and markup.
 ======================================================================*/
 

#ifdef __cplusplus
extern "C" {
#endif

#include "qurt.h"
#include "qurt_event_restricted.h"
#include "qurt_fs_restricted.h"
#include "qurt_island_restricted.h"
#include "qurt_memory_restricted.h"
#include "qurt_power_restricted.h"
#include "qurt_qdi_driver_restricted.h"
#include "qurt_sclk_restricted.h"
#include "qurt_thread_context_restricted.h"
#include "qurt_thread_restricted.h"
#include "qurt_trace_restricted.h"

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* QURT_RESTRICTED_H */

