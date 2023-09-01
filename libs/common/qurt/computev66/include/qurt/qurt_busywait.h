#ifndef QURT_BUSYWAIT_H
#define QURT_BUSYWAIT_H

/**
  @file qurt_busywait.h 
  @This file contains the implementation of the busywait() function for 
   hardware based blocking waits that use the QTIMER as a reference.   

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ==========================================================================*/
/*===========================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *  
 *
 *
 * when         who     what, where, why
 * ----------   ---     -----------------------------------------------------
 * 2018-03-20   pg      Add Header file
 ==========================================================================*/

/*=========================================================================*/
/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/

/* This function pauses the execution of a thread for a specified time.
 * Ideally should be used for small micro second delays 
 *
 * @param pause_time_us     Time to pause in microseconds 
 */

void qurt_busywait (unsigned int pause_time_us);

#endif /* QURT_BUSYWAIT_H */

