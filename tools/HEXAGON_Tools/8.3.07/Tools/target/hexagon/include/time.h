/* time.h standard header */
#ifndef _TIME
#define _TIME
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */
#include <types.h>

#if _PROVIDE_POSIX_TIME_DECLS
_EXTERN_C
struct timespec {
       time_t tv_sec;
       long tv_nsec;
};

/* nanosleep implementation will be provided by the OS library
 */
int nanosleep(const struct timespec *req, struct timespec *rem);

#define CLOCK_REALTIME (0)
#define CLOCK_MONOTONIC (1)
#define CLOCK_THREAD_CPUTIME_ID (2)
#define CLOCK_PROCESS_CPUTIME_ID (3)

/* clock_gettime implementation will be provided by the OS library
 */
int clock_gettime(clockid_t clock_id, struct timespec *tp);

_END_EXTERN_C
#endif /*_PROVIDE_POSIX_TIME_DECLS*/

_C_STD_BEGIN

		/* macros */
#ifndef NULL
 #define NULL	_NULL
#endif /* NULL */

#define CLOCKS_PER_SEC	_CPS

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

 #if !defined(_CLOCK_T) && !defined(__clock_t_defined) \
	&& !defined(_BSD_CLOCK_T_DEFINED_)
  #define _CLOCK_T
  #define __clock_t_defined
  #define _BSD_CLOCK_T_DEFINED_
  #define _STD_USING_CLOCK_T
typedef long clock_t;
 #endif /* !defined(_CLOCK_T) etc. */

 #if !defined(_TIME_T) && !defined(__time_t_defined) \
	&& !defined(_BSD_TIME_T_DEFINED_) && !defined(_TIME_T_DEFINED)
  #define _TIME_T
  #define __time_t_defined
  #define _BSD_TIME_T_DEFINED_
  #define _TIME_T_DEFINED
  #define _STD_USING_TIME_T
typedef long time_t;
 #endif /* !defined(_TIME_T) etc. */

 #ifndef _TM_DEFINED
  #define _TM_DEFINED
struct tm
	{	/* date and time components */
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
	};
 #endif /* _TM_DEFINED */

 #if _HAS_POSIX_C_LIB

  #if defined(__linux) || defined(__linux__)	/* compiler test */ \
	|| !defined(__APPLE__) && (0x570 < __SUNPRO_CC)
struct timespec
	{	/* POSIX.4, needed for pthread_cond_timedwait */
	time_t tv_sec;
	long tv_nsec;
	};
  #endif /* defined(__linux) etc. */

  #if !defined(_CLOCKID_T) && !defined(__clockid_t_defined)
   #define _CLOCKID_T
   #define __clockid_t_defined
   #define _STD_USING_CLOCKID_T
typedef int clockid_t;
  #endif /* !defined(_CLOCKID_T) && !defined(__clockid_t_defined) */

 #endif /* _HAS_POSIX_C_LIB */

_EXTERN_C	/* low-level functions */
time_t time(time_t *) _NO_THROW;
_END_EXTERN_C

_C_LIB_DECL	/* declarations */
char *asctime(const struct tm *) _NO_THROW;
clock_t clock(void) _NO_THROW;
char *ctime(const time_t *) _NO_THROW;
double difftime(time_t, time_t) _NO_THROW;
struct tm *gmtime(const time_t *) _NO_THROW;
struct tm *localtime(const time_t *) _NO_THROW;
time_t mktime(struct tm *) _NO_THROW;
size_t strftime(char *_Restrict, size_t,
	const char *_Restrict, const struct tm *_Restrict) _NO_THROW;
char *asctime_r(const struct tm *, char *) _NO_THROW;
char *ctime_r(const time_t *, char *) _NO_THROW;
struct tm *gmtime_r(const time_t *, struct tm *) _NO_THROW;
struct tm *localtime_r(const time_t *, struct tm *) _NO_THROW;
char *strptime(const char *, const char *, struct tm *) _NO_THROW;
_END_C_LIB_DECL
_C_STD_END
#endif /* _TIME */

 #if defined(_STD_USING)

  #ifdef _STD_USING_CLOCK_T
using _CSTD clock_t;
  #endif /* _STD_USING_CLOCK_T */

  #ifdef _STD_USING_SIZE_T
using _CSTD size_t;
  #endif /* _STD_USING_SIZE_T */

  #ifdef _STD_USING_TIME_T
using _CSTD time_t;
  #endif /* _STD_USING_TIME_T */

  #ifdef _STD_USING_CLOCKID_T
using _CSTD clockid_t;
  #endif /* _STD_USING_CLOCKID_T */

using _CSTD tm;
using _CSTD asctime; using _CSTD clock; using _CSTD ctime;
using _CSTD difftime; using _CSTD gmtime; using _CSTD localtime;
using _CSTD mktime; using _CSTD strftime; using _CSTD time;
 #endif /* defined(_STD_USING) */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
