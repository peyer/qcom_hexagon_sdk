/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* limits.h standard header -- 8-bit version */
#ifndef _LIMITS
#define _LIMITS
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

 #if _BITS_BYTE != 8
  #error <limits.h> assumes 8 bits per byte
 #endif /* _BITS_BYTE != 8 */


 #if defined(_LIMITS_H___)
  #undef MB_LEN_MAX
  #define MB_LEN_MAX	_MBMAX

 #else /* defined(_LIMITS_H___) */
  #define _LIMITS_H___
		/* CHAR PROPERTIES */
  #define CHAR_BIT	_BITS_BYTE

  #if _CSIGN
   #define CHAR_MAX	SCHAR_MAX
   #define CHAR_MIN	SCHAR_MIN

  #else /* _CSIGN */
   #define CHAR_MAX	UCHAR_MAX
   #define CHAR_MIN	0
  #endif /* _CSIGN */

		/* MULTIBYTE PROPERTIES */
  #define MB_LEN_MAX	_MBMAX

		/* SIGNED CHAR PROPERTIES */
  #define SCHAR_MAX	0x7f
  #define SCHAR_MIN	(-SCHAR_MAX - _C2)

		/* SHORT PROPERTIES */
  #define SHRT_MAX	0x7fff
  #define SHRT_MIN	(-SHRT_MAX - _C2)

		/* INT PROPERTIES */

  #if _ILONG
   #define INT_MAX		0x7fffffff
   #define INT_MIN		(-INT_MAX - _C2)
   #define UINT_MAX		0xffffffffU

  #else /* _ILONG */
   #define INT_MAX		SHRT_MAX
   #define INT_MIN		SHRT_MIN
   #define UINT_MAX		USHRT_MAX
  #endif /* _ILONG */

		/* LONG PROPERTIES */

  #if _LLONG
   #define LONG_MAX		0x7fffffffffffffffL
   #define LONG_MIN		(-LONG_MAX - _C2)
   #define ULONG_MAX	0xffffffffffffffffUL

  #else /* _LLONG */
   #define LONG_MAX		0x7fffffffL
   #define LONG_MIN		(-LONG_MAX - _C2)
   #define ULONG_MAX	0xffffffffUL
  #endif /* _LLONG */

		/* UNSIGNED PROPERTIES */
  #define UCHAR_MAX	0xff
  #define USHRT_MAX	0xffff
 #endif /* defined(_LIMITS_H___) */

/* #if _HAS_C9X */
#define LLONG_MAX	_LLONG_MAX
#define LLONG_MIN	(-_LLONG_MAX - _C2)
#define ULLONG_MAX	_ULLONG_MAX
/* #endif*/ /* _IS_C9X */

#endif /* _LIMITS */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
