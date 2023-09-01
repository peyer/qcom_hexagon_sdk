#ifndef _MMDEFS_H
#define _MMDEFS_H
/*==============================================================================
  Copyright (c) 2012-2013, 2018 Qualcomm Technologies Incorporated.
  All Rights Reserved Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S.
  Government. Diversion contrary to U.S. law prohibited.
 ==============================================================================*/

/*------------------------------------------------------------------------------
   Standard Integer Types
------------------------------------------------------------------------------*/

#if defined( __H2XML__)/* If H2XML compiler is being Used */ 
  
  /* The following definitions are the same accross platforms.  This first
   * group are the sanctioned types. */
  typedef unsigned long long uint64_t;  /* Unsigned 64 bit value */
  typedef unsigned long int  uint32_t;  /* Unsigned 32 bit value */
  typedef unsigned short     uint16_t;  /* Unsigned 16 bit value */
  typedef unsigned char      uint8_t;   /* Unsigned 8  bit value */

  typedef signed long long   int64_t;   /* Signed 64 bit value */
  typedef signed long int    int32_t;   /* Signed 32 bit value */
  typedef signed short       int16_t;   /* Signed 16 bit value */
  typedef signed char        int8_t;    /* Signed 8  bit value */ 
 
#else /* else If H2XML compiler is not Used */

  /* If we're hosted, fall back to the system's stdint.h, which might have
   * additional definitions. */
  #include "stdint.h"

  
/*=============================================================================
  FUNCTION : align_to_8_byte                                                
  DESCRIPTION: Ceil to the next multiple of 8                               
==============================================================================*/
static inline uint32_t align_to_8_byte(const uint32_t num)
{
   return ((num + 7) & (0xFFFFFFF8));
}
  
#endif /* Supported Compilers */


/*------------------------------------------------------------------------------
   Constants
------------------------------------------------------------------------------*/

#undef TRUE
#undef FALSE

#define TRUE   (1)  /* Boolean true value */
#define FALSE  (0)  /* Boolean false value */

#ifndef NULL
  #define NULL (0)
#endif

/*------------------------------------------------------------------------------
   Character and boolean
------------------------------------------------------------------------------*/

typedef char char_t;           /* Character type */
typedef unsigned char bool_t;  /* Boolean value type */


#endif /* _MMDEFS_H */

