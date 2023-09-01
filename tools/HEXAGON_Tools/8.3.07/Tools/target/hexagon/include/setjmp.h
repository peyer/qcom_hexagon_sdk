/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* setjmp.h standard header */
#ifndef _SETJMP
#define _SETJMP
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

 #ifndef _JMP_BUF_SIZE
  #define _JMP_BUF_SIZE	23	/* assume ARM */
 #endif /* _JMP_BUF_SIZE */

_C_STD_BEGIN
		/* macros */

/*int::setjmp causes gcc3.4.6 compiler error */
/*#define setjmp(env)	_CSTD setjmp(env)*/

		/* type definitions */
typedef int jmp_buf[_JMP_BUF_SIZE];

		/* declarations */
_C_LIB_DECL
void longjmp(jmp_buf, int) _NO_THROW;
int setjmp(jmp_buf) _NO_THROW;
_END_C_LIB_DECL
_C_STD_END
#endif /* _SETJMP */

 #if defined(_STD_USING)
using _CSTD jmp_buf; using _CSTD longjmp;
 #endif /* defined(_STD_USING) */

/*
 * Copyright (c) 1994-2006 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

/*
060530 pjp: adapted for gcc
 */
