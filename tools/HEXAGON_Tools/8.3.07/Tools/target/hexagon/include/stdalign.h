/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* stdalign.h standard header.  Conformant to section 7.15 of the C spec. */
#ifndef _STDALIGN
#define _STDALIGN

#if !defined(__cplusplus)
  #define alignas _Alignas
  #define alignof _Alignof
#endif

#define __alignas_is_defined 1
#define __alignof_is_defined 1

#endif /* _STDALIGN */
