#ifndef QURT_ASSERT_H
#define QURT_ASSERT_H
/**
  @file qurt_assert.h   
  @brief  Prototypes of qurt_assert API  

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2014  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/



/*=============================================================================
                        CONSTANTS AND MACROS
=============================================================================*/
/** @ingroup func_qurt_assert_error
  Write diagnostic information to debug buffer, and raise an error to Qurt kernel.
  
  @datatypes
  None.
  
  @param[in] filename     pointer to the file name string
  @param[in] lineno       line number
  
  @param[out] 
  None.

  @return
  none

  @dependencies
  None.
  
 */

void qurt_assert_error(const char *filename, int lineno) __attribute__((noreturn));

#define qurt_assert(cond) ((cond)?(void)0:qurt_assert_error(__QURTFILENAME__,__LINE__))

/** @} */ /* end_ingroup func_qurt_assert */

#endif /* QURT_ASSERT_H */

