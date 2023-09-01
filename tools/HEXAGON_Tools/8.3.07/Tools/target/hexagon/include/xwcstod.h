/* xwcstod.h internal header */
#ifndef _WCSTOD
#define _WCSTOD

  #if _HAS_INLINE
		/* INLINES, FOR C++ */
_C_LIB_DECL
_GNUC_EXTERN_INLINE inline double wcstod(const wchar_t *_Restrict _Str,
	wchar_t **_Restrict _Endptr)
	{return (_WStod(_Str, _Endptr, 0));
	}

_GNUC_EXTERN_INLINE inline unsigned long wcstoul(const wchar_t *_Restrict _Str,
	wchar_t **_Restrict _Endptr, int _Base)
	{return (_WStoul(_Str, _Endptr, _Base));
	}
_END_C_LIB_DECL

  #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */
		/* MACROS AND DECLARATIONS, FOR C90 */
_C_LIB_DECL
double wcstod(const wchar_t *, wchar_t **);
unsigned long wcstoul(const wchar_t *, wchar_t **, int);
_END_C_LIB_DECL

   #define wcstod(str, endptr)	_WStod(str, endptr, 0)
   #define wcstoul(str, endptr, base)	_WStoul(str, endptr, base)
  #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

#endif /* _WCSTOD */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
