/* xwstr.h internal header */
#ifndef _WSTR
#define _WSTR

  #if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
		/* INLINES AND OVERLOADS, FOR C++ */
_C_LIB_DECL
const wchar_t *wcschr(const wchar_t *, wchar_t);
const wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
const wchar_t *wcsrchr(const wchar_t *, wchar_t);
const wchar_t *wcsstr(const wchar_t *, const wchar_t *);
_END_C_LIB_DECL

inline wchar_t *wcschr(wchar_t *_Str, wchar_t _Ch)
	{return ((wchar_t *)wcschr((const wchar_t *)_Str, _Ch));
	}

inline wchar_t *wcspbrk(wchar_t *_Str1, const wchar_t *_Str2)
	{return ((wchar_t *)wcspbrk((const wchar_t *)_Str1, _Str2));
	}

inline wchar_t *wcsrchr(wchar_t *_Str, wchar_t _Ch)
	{return ((wchar_t *)wcsrchr((const wchar_t *)_Str, _Ch));
	}

inline wchar_t *wcsstr(wchar_t *_Str1, const wchar_t *_Str2)
	{return ((wchar_t *)wcsstr((const wchar_t *)_Str1, _Str2));
	}

  #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */
   #define _WConst_return

_C_LIB_DECL
wchar_t *wcschr(const wchar_t *, wchar_t);
wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t *wcsrchr(const wchar_t *, wchar_t);
wchar_t *wcsstr(const wchar_t *, const wchar_t *);
_END_C_LIB_DECL

  #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

 #if _HAS_INLINE
_C_LIB_DECL

_GNUC_EXTERN_INLINE inline wint_t btowc(int _By)
	{	// convert single byte to wide character
	return (_Btowc(_By));
	}

_GNUC_EXTERN_INLINE inline int wctob(wint_t _Wc)
	{	// convert wide character to single byte
	return (_Wctob(_Wc));
	}

_GNUC_EXTERN_INLINE inline float wcstof(const wchar_t *_Restrict _Str,
	wchar_t **_Restrict _Endptr)
	{	// convert wide string to float
	return (_WStof(_Str, _Endptr, 0));
	}

_GNUC_EXTERN_INLINE inline long double wcstold(const wchar_t *_Restrict _Str,
	wchar_t **_Restrict _Endptr)
	{	// convert wide string to double
	return (_WStold(_Str, _Endptr, 0));
	}

_END_C_LIB_DECL

 #else /* _HAS_INLINE */
   /* MACROS AND DECLARATIONS, FOR C90 */

_C_LIB_DECL
wint_t btowc(int);
int wctob(wint_t);

 #if _HAS_C9X
float wcstof(const wchar_t *_Restrict,
	wchar_t **_Restrict);
long double wcstold(const wchar_t *_Restrict,
	wchar_t **_Restrict);
 #endif /* _IS_C9X */

_END_C_LIB_DECL

   #define btowc(by)	_Btowc(by)
   #define wcstof(str, endptr)	_WStof(str, endptr, 0)
   #define wcstold(str, endptr)	_WStold(str, endptr, 0)
   #define wctob(wc)	_Wctob(wc)

 #endif /* _HAS_INLINE */

#endif /* _WSTR */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
