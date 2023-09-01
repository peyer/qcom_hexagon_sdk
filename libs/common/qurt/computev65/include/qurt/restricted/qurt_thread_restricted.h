#ifndef QURT_THREAD_RESTRICTED_H
#define QURT_THREAD_RESTRICTED_H
/**
  @file qurt_thread.h 
  @brief  Prototypes of Thread API  

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/

#include "qurt_thread.h"

/**@ingroup func_qurt_system_stm_thread_attr_get
  Gets the attributes of the specified thread.\n
  This function assumes that Q6 is in single thread mode.\n
  
  @note1hang1 This function is to be called only when Q6 has encountered an exception and 
             has entered single thread mode. For all other purposes use qurt_thread_attr_get()
             to retrieve the thread attributes.

  @note2 User process threads have two different thread attribute structures. Only the currently
         executing thread attributes are returned.

  @datatypes
  #qurt_thread_t \n
  #qurt_thread_attr_t

  @param[in]  thread_id	    Thread identifier.
  
  @return
   Thread attributes -- Pointer to thread attributes of the specified thread.
   NULL -- If the Thread identifier is invalid.

  @dependencies
  None.
 */
qurt_thread_attr_t * qurt_system_stm_thread_attr_get (qurt_thread_t thread_id);

/* request pager wakeup, arg indicates reason */
int qurt_thread_wake_pager(unsigned arg);

/**@ingroup func_qurt_get_all_thread_ids
  Gets the list all the threads in requested process.\n
  This function needs to be called in a loop until all the thread IDs have been 
  returned.
  
  @datatypes
  #unsigned \n

  @param[in]     pid	            Process ID of the threads being requested for
  @param[in]     index           Pointer to the index from where to start reporting from.
                                 This should be the last return value passed back to the user by this API.
  @param[in/out] tlist           Pointer to the buffer to store the thread IDs
  @param[in]     bufsize         The size of the buffer (tlist) passed in  
  @param[in/out] total_tids      Pointer that will be populated by the callee as to how 
                                 many thread IDs have been returned.
  
  @return
   Index The next index in the thread list that the caller has to pass back to get more 
         thread IDs

  @dependencies
  None.
 */
unsigned int qurt_get_all_thread_ids(unsigned pid, unsigned *index, void *tlist, unsigned bufsize, unsigned *total_tids);

#endif /* QURT_THREAD_RESTRICTED_H */

