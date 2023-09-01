/* wchar.h minimal header for C++ */
#ifndef _WCHAR
#define _WCHAR
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#include <stddef.h>
#include <stdio.h>

		/* MACROS */
#ifndef WEOF
 #define WEOF	((wint_t)(-1))
#endif /* WEOF */

		/* TYPE DEFINITIONS */
struct tm;
typedef int mbstate_t;
typedef wchar_t	wint_t;

		/* FUNCTIONS */
_C_STD_BEGIN
_C_LIB_DECL
wint_t btowc(int);
wint_t fgetwc(FILE *);
wint_t fputwc(wchar_t, FILE *);
size_t mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
wint_t ungetwc(wint_t, FILE *);
size_t wcslen(const wchar_t *);
size_t wcrtomb(char *, wchar_t, mbstate_t *);
int wctob(wint_t);

int wmemcmp(const wchar_t *, const wchar_t *, size_t);
wchar_t *wmemcpy(wchar_t *, const wchar_t *, size_t);
wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t);
wchar_t *wmemset(wchar_t *, wchar_t, size_t);
_END_C_LIB_DECL
_C_STD_END

	/* inlines/macros */

 #ifdef __cplusplus
		/* inlines and overloads, for C++ */
_C_LIB_DECL
const wchar_t *wmemchr(const wchar_t *, wchar_t, size_t);
_END_C_LIB_DECL

inline wchar_t *wmemchr(wchar_t *_Str, wchar_t _Ch, size_t _Count)
	{	// return wmemchr with Standard C signature
	return ((wchar_t *)wmemchr((const wchar_t *)_Str, _Ch, _Count));
	}

 #else /* __cplusplus */
wchar_t *wmemchr(const wchar_t *, wchar_t, size_t);
 #endif /* __cplusplus */

#endif /* _WCHAR */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
