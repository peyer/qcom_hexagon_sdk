/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* stdarg.h standard header for gcc */
#ifndef _STDARG
#define _STDARG

/* _C_STD_BEGIN */
		/* type definitions */
typedef __builtin_va_list va_list;

		/* macros */
#define va_arg(ap, T)		__builtin_va_arg(ap, T)
#define va_copy(apd, aps)	__builtin_va_copy(apd, aps)
#define va_end(ap)			__builtin_va_end(ap)

 #if 0
  #define va_start(ap, A)	__builtin_stdarg_start(ap, A)
 #else
  #define va_start(ap, A)	__builtin_va_start(ap, A)
 #endif
/* _C_STD_END */
#endif /* _STDARG */

#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#if 0
 #if defined(_STD_USING)
using _CSTD va_list;
 #endif /* defined(_STD_USING) */
#endif

/*
 * Copyright (c) 1994-2006 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

/*
060530 pjp: adapted for gcc
060921 pjp: added __builtin_stdarg_start
061102 pjp: disabled _STD_USING logic
 */
