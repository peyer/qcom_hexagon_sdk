/* stddef.h standard header */
#ifndef _STDDEF
#define _STDDEF
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* macros */
#ifndef NULL
 #define NULL	_NULL
#endif /* NULL */

#ifndef offsetof

 #if __EDG__
  #define offsetof(T, member) ((_Sizet)__INTADDR__(&(((T *)0)->member)))

 #elif (__GNUC__ > 3)
  #define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER) 
 #elif defined(__cplusplus) && ((__GNUC__ == 3) && (3 < __GNUC_MINOR__))
  #define offsetof(T, member) \
	(__offsetof__ (reinterpret_cast<_CSTD size_t> \
	(&reinterpret_cast<char &> \
	(static_cast<T *>(0)->member))))

 #else /* __EDG__ */
  #define offsetof(T, member)	((_CSTD size_t)&(((T *)0)->member))
 #endif /* __EDG__ */

#endif /* offsetof */

		/* type definitions */

 #if !defined(_PTRDIFF_T) && !defined(_PTRDIFFT) \
	&& !defined(_PTRDIFF_T_DEFINED)
  #define _PTRDIFF_T
  #define _PTRDIFFT
  #define _PTRDIFF_T_DEFINED
  #define _STD_USING_PTRDIFF_T
typedef _Ptrdifft ptrdiff_t;
 #endif /* !defined(_PTRDIFF_T) etc. */

 #if !defined(_SIZE_T) && !defined(_SIZET) && !defined(_SIZE_T_DEFINED) \
	&& !defined(_BSD_SIZE_T_DEFINED_)
  #define _SIZE_T
  #define _SIZET
  #define _BSD_SIZE_T_DEFINED_
  #define _SIZE_T_DEFINED
  #define _STD_USING_SIZE_T
typedef _Sizet size_t;
 #endif /* !defined(_SIZE_T) etc. */

 #if !defined(_WCHART) && !defined(_WCHAR_T_DEFINED)
  #define _WCHART
  #define _WCHAR_T_DEFINED
typedef _Wchart wchar_t;
 #endif /* _WCHART etc. */

typedef long double max_align_t;

_C_STD_END
#endif /* _STDDEF */

 #if defined(_STD_USING)

  #ifdef _STD_USING_PTRDIFF_T
using _CSTD ptrdiff_t;
  #endif /* _STD_USING_PTRDIFF_T */

  #ifdef _STD_USING_SIZE_T
using _CSTD size_t;
  #endif /* _STD_USING_SIZE_T */

 #endif /* defined(_STD_USING) */

/* C11 Common definitions */
#if (defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__ >= 1)
typedef size_t rsize_t;
#endif


/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
