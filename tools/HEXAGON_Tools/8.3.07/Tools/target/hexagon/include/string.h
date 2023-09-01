/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* string.h standard header */
#ifndef _STRING
#define _STRING
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* macros */
#ifndef NULL
 #define NULL	_NULL
#endif /* NULL */

		/* type definitions */

 #if !defined(_SIZE_T) && !defined(_SIZET) && !defined(_SIZE_T_DEFINED) \
	&& !defined(_BSD_SIZE_T_DEFINED_)
  #define _SIZE_T
  #define _SIZET
  #define _BSD_SIZE_T_DEFINED_
  #define _SIZE_T_DEFINED
  #define _STD_USING_SIZE_T
typedef _Sizet size_t;
 #endif /* !defined(_SIZE_T) etc. */

		/* declarations */
_C_LIB_DECL
int memcmp(const void *, const void *, size_t) _NO_THROW;
void *memcpy(void *_Restrict, const void *_Restrict, size_t) _NO_THROW;
void *memcpy_v(volatile void *_Restrict, const volatile void *_Restrict, size_t) _NO_THROW;
void *memset(void *, int, size_t) _NO_THROW;
char *strcat(char *_Restrict, const char *_Restrict) _NO_THROW;
int strcmp(const char *, const char *) _NO_THROW;
char *strcpy(char *_Restrict, const char *_Restrict) _NO_THROW;
size_t strlen(const char *) _NO_THROW;

void *memmove(void *, const void *, size_t) _NO_THROW;
void *memmove_v(volatile void *, const volatile void *, size_t) _NO_THROW;
int strcoll(const char *, const char *) _NO_THROW;
size_t strcspn(const char *, const char *) _NO_THROW;
char *strerror(int) _NO_THROW;
size_t strlcat(char *_Restrict, const char *_Restrict, size_t) _NO_THROW;
char *strncat(char *_Restrict, const char *_Restrict, size_t) _NO_THROW;
int strncmp(const char *, const char *, size_t) _NO_THROW;
size_t strlcpy(char *_Restrict, const char *_Restrict, size_t) _NO_THROW;
char *strncpy(char *_Restrict, const char *_Restrict, size_t) _NO_THROW;
size_t strspn(const char *, const char *) _NO_THROW;
char *strtok(char *_Restrict, const char *_Restrict) _NO_THROW;
char *strsep(char **_Restrict, const char *_Restrict) _NO_THROW;
size_t strxfrm(char *_Restrict, const char *_Restrict, size_t) _NO_THROW;

 #if _ADDED_C_LIB
char *strdup(const char *) _NO_THROW;
int strcasecmp(const char *, const char *) _NO_THROW;
int strncasecmp(const char *, const char *, size_t) _NO_THROW;
char *strtok_r(char *, const char *, char **) _NO_THROW;
 #endif /* _ADDED_C_LIB */
void *memccpy (void *, const void *, int, size_t) _NO_THROW;
int strerror_r (int, char *, size_t) _NO_THROW;

_END_C_LIB_DECL

 #if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
		// INLINES AND OVERLOADS, for C++
 #define _Const_return const

_C_LIB_DECL
const char *strchr(const char *, int) _NO_THROW;
const char *strpbrk(const char *, const char *) _NO_THROW;
const char *strrchr(const char *, int) _NO_THROW;
const char *strstr(const char *, const char *) _NO_THROW;
_END_C_LIB_DECL

extern "C++" {
inline char *strchr(char *_Str, int _Ch)
	{	// call with const first argument
	return ((char *)_CSTD strchr((const char *)_Str, _Ch));
	}

inline char *strpbrk(char *_Str1, const char *_Str2)
	{	// call with const first argument
	return ((char *)_CSTD strpbrk((const char *)_Str1, _Str2));
	}

inline char *strrchr(char *_Str, int _Ch)
	{	// call with const first argument
	return ((char *)_CSTD strrchr((const char *)_Str, _Ch));
	}

inline char *strstr(char *_Str1, const char *_Str2)
	{	// call with const first argument
	return ((char *)_CSTD strstr((const char *)_Str1, _Str2));
	}
}	// extern "C++"

 #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES)*/
_C_LIB_DECL
 #define _Const_return

char *strchr(const char *, int) _NO_THROW;
char *strpbrk(const char *, const char *) _NO_THROW;
char *strrchr(const char *, int) _NO_THROW;
char *strstr(const char *, const char *) _NO_THROW;
_END_C_LIB_DECL
 #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

 #ifndef _MEMCHR
  #define _MEMCHR

  #if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
_C_LIB_DECL
const void *memchr(const void *, int, size_t) _NO_THROW;
_END_C_LIB_DECL

extern "C++" {
inline void *memchr(void *_Str, int _Ch, size_t _Num)
	{	// call with const first argument
	return ((void *)_CSTD memchr((const void *)_Str, _Ch, _Num));
	}
}	// extern "C++"

  #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */
_C_LIB_DECL
void *memchr(const void *, int, size_t) _NO_THROW;
_END_C_LIB_DECL
  #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

 #endif /* _MEMCHR */

 #if __STDC_WANT_LIB_EXT1__
_C_LIB_DECL

  #if !defined(_ERRNO_T_DEFINED)
   #define _ERRNO_T_DEFINED
typedef int errno_t;
  #endif /* _ERRNO_T_DEFINED */

  #if !defined(_RSIZE_T_DEFINED)
   #define _RSIZE_T_DEFINED
typedef size_t rsize_t;
  #endif /* _RSIZE_T_DEFINED */

errno_t memcpy_s(void *_Restrict, rsize_t,
	const void *_Restrict, rsize_t) _NO_THROW;
errno_t memmove_s(void *, rsize_t,
	const void *, rsize_t) _NO_THROW;

errno_t memset_s(void* s, rsize_t smax, int c, rsize_t n);
errno_t strcpy_s(char *_Restrict, rsize_t,
	const char *_Restrict) _NO_THROW;
errno_t strncpy_s(char *_Restrict, rsize_t,
	const char *_Restrict, rsize_t) _NO_THROW;
errno_t strcat_s(char *_Restrict, rsize_t,
	const char *_Restrict) _NO_THROW;
errno_t strncat_s(char *_Restrict, rsize_t,
	const char *_Restrict, rsize_t) _NO_THROW;
char *strtok_s(char *_Restrict, rsize_t *_Restrict,
	const char *_Restrict, char **_Restrict) _NO_THROW;

errno_t strerror_s(char *, rsize_t, errno_t) _NO_THROW;
size_t strerrorlen_s(errno_t) _NO_THROW;

size_t strnlen_s(const char *_Restrict, size_t) _NO_THROW;
_END_C_LIB_DECL
 #endif /* __STDC_WANT_LIB_EXT1__ */

_C_STD_END
#endif /* _STRING */

 #if defined(_STD_USING)

  #ifdef _STD_USING_SIZE_T
using _CSTD size_t;
  #endif /* _STD_USING_SIZE_T */

using _CSTD memchr; using _CSTD memcmp;
using _CSTD memcpy; using _CSTD memmove; using _CSTD memset;
using _CSTD memcpy_v; using _CSTD memmove_v;
using _CSTD strcat; using _CSTD strchr; using _CSTD strcmp;
using _CSTD strcoll; using _CSTD strcpy; using _CSTD strcspn;
using _CSTD strerror; using _CSTD strlen; using _CSTD strncat;
using _CSTD strlcat; using _CSTD strlcpy;
using _CSTD strncmp; using _CSTD strncpy; using _CSTD strpbrk;
using _CSTD strrchr; using _CSTD strspn; using _CSTD strstr;
using _CSTD strtok; using _CSTD strxfrm; using _CSTD strsep;

 #if __STDC_WANT_LIB_EXT1__
using _CSTD errno_t;
using _CSTD rsize_t;

using _CSTD memcpy_s;
using _CSTD memmove_s;
using _CSTD strcpy_s;
using _CSTD strncpy_s;
using _CSTD strcat_s;
using _CSTD strncat_s;
using _CSTD strtok_s;
using _CSTD strerror_s;
using _CSTD strerrorlen_s;
using _CSTD strnlen_s;
 #endif /* __STDC_WANT_LIB_EXT1__ */

 #endif /* defined(_STD_USING) */

/*
 * Copyright (c) 1992-2007 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.04:1566 */
