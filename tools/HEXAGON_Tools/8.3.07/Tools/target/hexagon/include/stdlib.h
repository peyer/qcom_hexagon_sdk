/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* stdlib.h standard header */
#ifndef _STDLIB
#define _STDLIB
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#if __QDSP6_RTOS__
#include <stdint.h>
#endif

_C_STD_BEGIN

		/* MACROS */
#ifndef NULL
 #define NULL	_NULL
#endif /* NULL */

#define EXIT_FAILURE	_EXFAIL
#define EXIT_SUCCESS	0

#define MB_CUR_MAX	_CSTD _Getmbcurmax()

 #if _ILONG
  #define RAND_MAX	0x3fffffff

 #else /* _ILONG */
  #define RAND_MAX	0x7fff
 #endif /* _ILONG */

 #ifndef _NO_RETURN
  #define _NO_RETURN(fun)	void fun
 #endif /* _NO_RETURN */

		/* TYPE DEFINITIONS */

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

typedef struct
	{	/* result of int divide */
	int quot;
	int rem;
	} div_t;

typedef struct
	{	/* result of long divide */
	long quot;
	long rem;
	} ldiv_t;

 #ifndef _LLDIV_T
typedef struct
	{	/* result of long long divide */
	_Longlong quot;
	_Longlong rem;
	} _Lldiv_t;
 #endif /* _LLDIV_T */

typedef _Lldiv_t lldiv_t;

		/* DECLARATIONS */
_EXTERN_C /* low-level functions */
_NO_RETURN(exit(int));

_NO_RETURN(_Exit(int));	/* added with C99 */
char *getenv(const char *) _NO_THROW;
int system(const char *) _NO_THROW;
_END_EXTERN_C

_C_LIB_DECL
int abs(int) _NO_THROW;
long labs(long) _NO_THROW;
_NO_RETURN(abort(void));
void *calloc(size_t, size_t) _NO_THROW;
div_t div(int, int) _NO_THROW;
void free(void *) _NO_THROW;
ldiv_t ldiv(long, long) _NO_THROW;
void *malloc(size_t) _NO_THROW;
void *memalign(size_t, size_t) _NO_THROW;
int posix_memalign(void **, size_t, size_t) _NO_THROW;
void *aligned_alloc(size_t, size_t) _NO_THROW;
int mblen(const char *, size_t) _NO_THROW;
size_t mbstowcs(wchar_t *_Restrict,
	const char *_Restrict, size_t) _NO_THROW;
int mbtowc(wchar_t *_Restrict, const char *_Restrict, size_t) _NO_THROW;
int rand(void) _NO_THROW;
void srand(unsigned int) _NO_THROW;
void *realloc(void *, size_t) _NO_THROW;
long strtol(const char *_Restrict, char **_Restrict, int) _NO_THROW;
size_t wcstombs(char *_Restrict,
	const wchar_t *_Restrict, size_t) _NO_THROW;
int wctomb(char *, wchar_t) _NO_THROW;

 #if _ADDED_C_LIB
int getopt(int, char *const *, const char *) _NO_THROW;
char *mktemp(char *) _NO_THROW;
int putenv(char *) _NO_THROW;
int rand_r(unsigned *) _NO_THROW;
char *tempnam(const char *, const char *) _NO_THROW;

extern char *optarg;
extern int optind, opterr, optopt;
double drand48(void) _NO_THROW;
double erand48(unsigned short xseed[3]) _NO_THROW;
long lrand48(void) _NO_THROW;
long nrand48(unsigned short xseed[3]) _NO_THROW;
long mrand48(void) _NO_THROW;
long jrand48(unsigned short xseed[3]) _NO_THROW;
void srand48(long seed) _NO_THROW;
unsigned short * seed48(unsigned short xseed[3]) _NO_THROW;
void lcong48(unsigned short p[7]) _NO_THROW;
 #endif /* _ADDED_C_LIB */

unsigned long _Stoul(const char *, char **, int) _NO_THROW;
float _Stof(const char *, char **, long) _NO_THROW;
double _Stod(const char *, char **, long) _NO_THROW;
long double _Stold(const char *, char **, long) _NO_THROW;
_Longlong _Stoll(const char *, char **, int) _NO_THROW;
_ULonglong _Stoull(const char *, char **, int) _NO_THROW;

float _Stofx(const char *, char **, long, int *) _NO_THROW;
double _Stodx(const char *, char **, long, int *) _NO_THROW;
long double _Stoldx(const char *, char **, long, int *) _NO_THROW;
long _Stolx(const char *, char **, int, int *) _NO_THROW;
unsigned long _Stoulx(const char *, char **, int, int *) _NO_THROW;

size_t _Getmbcurmax(void) _NO_THROW;

_Longlong llabs(_Longlong) _NO_THROW;
lldiv_t lldiv(_Longlong, _Longlong) _NO_THROW;

int at_quick_exit(void (*)(void));
_NO_RETURN(quick_exit(int));


_END_C_LIB_DECL

 #if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
_EXTERN_C
typedef void _Atexfun(void) _NO_THROW;
typedef int _Cmpfun(const void *, const void *) _NO_THROW;

int atexit(void (*)(void)) _NO_THROW;
void *bsearch(const void *, const void *,
	size_t, size_t, _Cmpfun *) _NO_THROW;
void qsort(void *, size_t, size_t, _Cmpfun *) _NO_THROW;
long a64l (const char *) _NO_THROW;
char *l64a (long int) _NO_THROW;
char *ecvt(double, int, int *, int *) _NO_THROW;
char *fcvt(double, int, int *, int *) _NO_THROW;
char *gcvt(double, int, char *) _NO_THROW;
int getsubopt (char **, char *const *, char **) _NO_THROW;
int mkstemp (char *) _NO_THROW;
_END_EXTERN_C

extern "C++" {
  #if _HAS_STRICT_LINKAGE
typedef int _Cmpfun2(const void *, const void *);

inline int atexit(void (*_Pfn)(void))
	{	// register a function to call at exit
	return (atexit((_Atexfun *)_Pfn));
	}

inline void *bsearch(const void *_Key, const void *_Base,
	size_t _Nelem, size_t _Size, _Cmpfun2 *_Cmp)
	{	// search by binary chop
	return (bsearch(_Key, _Base, _Nelem, _Size, (_Cmpfun *)_Cmp));
	}

inline void qsort(void *_Base,
	size_t _Nelem, size_t _Size, _Cmpfun2 *_Cmp)
	{	// sort
	qsort(_Base, _Nelem, _Size, (_Cmpfun *)_Cmp);
	}

inline int at_quick_exit(void (*_Pfn)(void))
        {       // register a function to call at exit
        return (at_quick_exit((_Atexfun *)_Pfn));
        }

  #endif /* _HAS_STRICT_LINKAGE */


		// OVERLOADS, FOR C++

  #if !defined(_LLDIV_T) || defined(_LONGLONG)
inline long abs(long _Left)
	{	// compute abs
	return (labs(_Left));
	}

inline ldiv_t div(long _Left, long _Right)
	{	// compute quotient and remainder
	return (ldiv(_Left, _Right));
	}
  #endif /* !defined(_LLDIV_T) || defined(_LONGLONG) */


  #if !defined(_LLDIV_T) && defined(_LONGLONG)
inline _Longlong abs(_Longlong _Left)
	{	// compute abs
	return (llabs(_Left));
	}

inline _Lldiv_t div(_Longlong _Left, _Longlong _Right)
	{	// compute quotient and remainder
	return (lldiv(_Left, _Right));
	}
  #endif /* !defined(_LLDIV_T) && defined(_LONGLONG) */

}	// extern "C++"

 #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */
_C_LIB_DECL
		/* DECLARATIONS AND MACRO OVERRIDES, FOR C */
typedef int _Cmpfun(const void *, const void *);

/* */ int atexit(void (*)(void)) _NO_THROW;
void *bsearch(const void *, const void *,
	size_t, size_t, _Cmpfun *) _NO_THROW;
void qsort(void *, size_t, size_t, _Cmpfun *) _NO_THROW;

long a64l (const char *) _NO_THROW;
char *l64a (long int) _NO_THROW;
char *ecvt(double, int, int *, int *) _NO_THROW;
char *fcvt(double, int, int *, int *) _NO_THROW;
char *gcvt(double, int, char *) _NO_THROW;
int getsubopt (char **, char *const *, char **) _NO_THROW;
int mkstemp (char *) _NO_THROW;

_END_C_LIB_DECL
 #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

 #if _HAS_INLINE
_C_LIB_DECL
_GNUC_EXTERN_INLINE inline double atof(const char *_Str)
	{	// convert string to double
	return (_Stod(_Str, 0, 0));
	}

_GNUC_EXTERN_INLINE inline int atoi(const char *_Str)
	{	// convert string to int
	return ((int)_Stoul(_Str, 0, 10));
	}

_GNUC_EXTERN_INLINE inline long atol(const char *_Str)
	{	// convert string to long
	return ((long)_Stoul(_Str, 0, 10));
	}

_GNUC_EXTERN_INLINE inline double strtod(const char *_Restrict _Str,
	char **_Restrict _Endptr)
	{	// convert string to double, with checking
	return (_Stod(_Str, _Endptr, 0));
	}

_GNUC_EXTERN_INLINE inline unsigned long strtoul(const char *_Restrict _Str,
	char **_Restrict _Endptr, int _Base)
	{	// convert string to unsigned long, with checking
	return (_Stoul(_Str, _Endptr, _Base));
	}

_GNUC_EXTERN_INLINE inline _Longlong atoll(const char *_Str)
	{	// convert string to long long
	return ((_Longlong)_Stoull(_Str, 0, 10));
	}

_GNUC_EXTERN_INLINE inline float strtof(const char *_Restrict _Str,
	char **_Restrict _Endptr)
	{	// convert string to float, with checking
	return (_Stof(_Str, _Endptr, 0));
	}

_GNUC_EXTERN_INLINE inline long double strtold(const char *_Restrict _Str,
	char **_Restrict _Endptr)
	{	// convert string to long double, with checking
	return (_Stold(_Str, _Endptr, 0));
	}

_GNUC_EXTERN_INLINE inline _Longlong strtoll(const char *_Restrict _Str,
	char **_Restrict _Endptr, int _Base)
	{	// convert string to long long, with checking
	return (_Stoll(_Str, _Endptr, _Base));
	}

_GNUC_EXTERN_INLINE inline _ULonglong strtoull(const char *_Restrict _Str,
	char **_Restrict _Endptr, int _Base)
	{	// convert string to unsigned long long, with checking
	return (_Stoull(_Str, _Endptr, _Base));
	}
_END_C_LIB_DECL
 #else /* _HAS_INLINE */
		/* DECLARATIONS AND MACRO OVERRIDES, FOR C90 */
_C_LIB_DECL
double atof(const char *) _NO_THROW;
int atoi(const char *) _NO_THROW;
long atol(const char *) _NO_THROW;
double strtod(const char *_Restrict, char **_Restrict) _NO_THROW;
unsigned long strtoul(const char *_Restrict,
	char **_Restrict, int) _NO_THROW;
#define atof(str)	_Stod(str, 0, 0)
#define atoi(str)	(int)_Stoul(str, 0, 10)
#define atol(str)	(long)_Stoul(str, 0, 10)
#define strtod(str, endptr)	_Stod(str, endptr, 0)
#define strtoul(str, endptr, base)	_Stoul(str, endptr, base)

_Longlong atoll(const char *) _NO_THROW;
float strtof(const char *_Restrict,
	char **_Restrict) _NO_THROW;
long double strtold(const char *_Restrict,
	char **_Restrict) _NO_THROW;
_Longlong strtoll(const char *_Restrict,
	char **_Restrict, int) _NO_THROW;
_ULonglong strtoull(const char *_Restrict,
	char **_Restrict, int) _NO_THROW;
#define atoll(str)	(_Longlong)_Stoull(str, 0, 10)
#define strtof(str, endptr)	_Stof(str, endptr, 0)
#define strtold(str, endptr)	_Stold(str, endptr, 0)
#define strtoll(str, endptr, base)	_Stoll(str, endptr, base)
#define strtoull(str, endptr, base)	_Stoull(str, endptr, base)
_END_C_LIB_DECL
 #endif

 #ifndef _LLDIV_T
  #define _LLDIV_T
 #endif /* _LLDIV_T */

_C_STD_END
#endif /* _STDLIB */

 #if defined(_STD_USING)

  #ifdef _STD_USING_SIZE_T
using _CSTD size_t;
  #endif /* _STD_USING_SIZE_T */

using _CSTD div_t; using _CSTD ldiv_t;

using _CSTD abort; using _CSTD abs; using _CSTD atexit;
using _CSTD atof; using _CSTD atoi; using _CSTD atol;
using _CSTD bsearch; using _CSTD calloc; using _CSTD div;
using _CSTD exit; using _CSTD free; using _CSTD getenv;
using _CSTD labs; using _CSTD ldiv; using _CSTD malloc;
using _CSTD memalign;
using _CSTD posix_memalign;
using _CSTD mblen; using _CSTD mbstowcs; using _CSTD mbtowc;
using _CSTD qsort; using _CSTD rand; using _CSTD realloc;
using _CSTD srand; using _CSTD strtod; using _CSTD strtol;
using _CSTD strtoul; using _CSTD system;
using _CSTD wcstombs; using _CSTD wctomb;

using _CSTD lldiv_t;

using _CSTD atoll; using _CSTD llabs; using _CSTD lldiv;
using _CSTD strtof; using _CSTD strtold;
using _CSTD strtoll; using _CSTD strtoull;
using _CSTD _Exit;

using _CSTD at_quick_exit;
using _CSTD quick_exit;
 #endif /* defined(_STD_USING) */



/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
