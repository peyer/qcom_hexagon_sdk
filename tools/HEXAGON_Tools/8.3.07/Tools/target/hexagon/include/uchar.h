/* uchar.h added header for TR 19769 */
#ifndef _UCHAR
#define _UCHAR
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* macros */
#ifndef NULL
 #define NULL	_NULL
#endif /* NULL */

#define __STDC_UTF_16__
#define __STDC_UTF_32__

_C_LIB_DECL
		/* TYPE DEFINITIONS */
 #ifndef _MBSTATET
  #define _MBSTATET
typedef struct _Mbstatet
	{	/* state of a multibyte translation */
	unsigned long _Wchar;
	unsigned short _Byte, _State;
	} _Mbstatet;
 #endif /* _MBSTATET */

typedef _Mbstatet mbstate_t;

 #if !defined(_SIZE_T) && !defined(_SIZET) && !defined(_SIZE_T_DEFINED) \
	&& !defined(_BSD_SIZE_T_DEFINED_)
  #define _SIZE_T
  #define _SIZET
  #define _BSD_SIZE_T_DEFINED_
  #define _SIZE_T_DEFINED
  #define _STD_USING_SIZE_T
typedef _Sizet size_t;
 #endif /* !defined(_SIZE_T) etc. */

#ifndef __cplusplus
/* C++11 has these as built in types.  In C, the built-ins don't exist. */
typedef unsigned short char16_t;
typedef unsigned long char32_t;
#endif

		/* declarations */
size_t mbrtoc16(char16_t *_Restrict, const char *_Restrict,
	size_t, mbstate_t *_Restrict) _NO_THROW;
size_t c16rtomb(char *_Restrict, char16_t,
	mbstate_t *_Restrict) _NO_THROW;

size_t mbrtoc32(char32_t *_Restrict, const char *_Restrict,
	size_t, mbstate_t *_Restrict) _NO_THROW;
size_t c32rtomb(char *_Restrict, char32_t,
	mbstate_t *_Restrict) _NO_THROW;
_END_C_LIB_DECL

_C_STD_END
#endif /* _UCHAR */

 #if defined(_STD_USING)

  #ifdef _STD_USING_SIZE_T
using _CSTD size_t;
  #endif /* _STD_USING_SIZE_T */

using _CSTD char16_t; using _CSTD char32_t; using _CSTD mbstate_t;
using _CSTD mbrtoc16; using _CSTD c16rtomb;
using _CSTD mbrtoc32; using _CSTD c32rtomb;
 #endif /* defined(_STD_USING) */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
