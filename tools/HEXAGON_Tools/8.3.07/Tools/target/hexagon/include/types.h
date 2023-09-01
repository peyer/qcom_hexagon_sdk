/* sys/types.h Open Group header */
#ifndef _SYS_TYPES
#define _SYS_TYPES
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */
_C_STD_BEGIN
_C_LIB_DECL

		/* system type definitions */
typedef long blkcnt_t;			/* file block count */
typedef long blksize_t;			/* file block size */

 #if !defined(_CLOCK_T) && !defined(__clock_t_defined) \
	&& !defined(_BSD_CLOCK_T_DEFINED_)
  #define _CLOCK_T
  #define __clock_t_defined
  #define _BSD_CLOCK_T_DEFINED_
  #define _STD_USING_CLOCK_T
typedef long clock_t;
 #endif /* !defined(_CLOCK_T) etc. */

#if !defined(_CLOCKID_T) && !defined(_BSD_CLOCKID_T_) \
       && !defined(__clockid_t_defined)
 #define _CLOCKID_T
 #define _BSD_CLOCKID_T_
 #define __clockid_t_defined
typedef int clockid_t;			/* clock ID */
#endif /* !defined(_CLOCKID_T) etc. */
typedef _ULonglong dev_t;		/* device ID */
typedef unsigned long fsblkcnt_t;	/* file system block count */
typedef unsigned long fsfilcnt_t;	/* file system file count */
typedef unsigned int gid_t;		/* group ID */
typedef unsigned int id_t;		/* holds gid_t, pid_t, or uid_t */
typedef _ULonglong ino_t;	/* file serial number */
typedef int key_t;				/* IPC key */
typedef unsigned int mode_t;	/* file attributes */
typedef unsigned int nlink_t;	/* file link count */
#if !defined(_OFF_T) && !defined(__off_t_defined)
 #define _OFF_T
 #define __off_t_defined
typedef long off_t;			/* offset into file */			
#endif /* !defined(_OFF_T) etc. */

#if !defined(_PID_T) && !defined(__pid_t_defined)
 #define _PID_T
 #define __pid_t_defined
typedef int pid_t;				/* process ID */
#endif /* !defined(_PID_T) etc. */

 #if !defined(_SIZE_T) && !defined(_SIZET) && !defined(_SIZE_T_DEFINED) \
	&& !defined(_BSD_SIZE_T_DEFINED_)
  #define _SIZE_T
  #define _SIZET
  #define _BSD_SIZE_T_DEFINED_
  #define _SIZE_T_DEFINED
  #define _STD_USING_SIZE_T
typedef _Sizet size_t;
 #endif /* !defined(_SIZE_T) etc. */

#if !defined(_SSIZE_T) && !defined(_BSD_SSIZE_T_) \
       && !defined(__ssize_t_defined)
 #define _SSIZE_T
 #define _BSD_SSIZE_T_
 #define __ssize_t_defined
typedef int ssize_t;			/* [-1, SSIZE_MAX] */
#endif /* !defined(_SSIZE_T) etc. */

#if !defined(_SUSECONDS_T) && !defined(_BSD_SUSECONDS_T_) \
       && !defined(__suseconds_t_defined)
 #define _SUSECONDS_T
 #define _BSD_SUSECONDS_T_
 #define __suseconds_t_defined
typedef long suseconds_t;		/* time in microseconds [-1, 1000000] */
#endif /* !defined(_SUSECONDS_T) etc. */

 #if !defined(_TIME_T) && !defined(__time_t_defined) \
	&& !defined(_BSD_TIME_T_DEFINED_) && !defined(_TIME_T_DEFINED)
  #define _TIME_T
  #define __time_t_defined
  #define _BSD_TIME_T_DEFINED_
  #define _TIME_T_DEFINED
  #define _STD_USING_TIME_T
typedef long time_t;
 #endif /* !defined(_TIME_T) etc. */

#if !defined(_TIMER_T) && !defined(_BSD_TIMER_T_) \
       && !defined(__timer_t_defined)
 #define _TIMER_T
 #define _BSD_TIMER_T_
 #define __timer_t_defined
 /* The OS is responsible for providing the definition of timer_i. */
typedef struct timer_i *timer_t;				/* timer ID */
 #endif /* !defined(_TIMER_T) etc. */

#if !defined(_UID_T) && !defined(__uid_t_defined)
 #define _UID_T
 #define __uid_t_defined
typedef unsigned int uid_t;		/* user ID */
#endif /* !defined(_UID_T) etc. */

#if !defined(_USECONDS_T) && !defined(_BSD_USECONDS_T_) \
       && !defined(__useconds_t_defined)
 #define _USECONDS_T
 #define _BSD_USECONDS_T_
 #define __useconds_t_defined
typedef unsigned long useconds_t;	/* time in microseconds [0, 1000000] */
#endif /* !defined(_USECONDS_T) etc. */

#ifdef __QDSP6_DINKUM_PTHREAD_TYPES__
		/* pthread types */
struct __sched_param
	{	/* schedule parameter */
	int __sched_priority;
	};

struct _pthread_fastlock
	{	/* fast lock for pthreads */
	long __status;
	int __spinlock;
	};

typedef struct _pthread_descr_struct *_pthread_descr;	/* opaque type */

typedef struct
	{	/* thread attributes */
	int __detachstate;
	int __schedpolicy;
	struct __sched_param __schedparam;
	int __inheritsched;
	int __scope;
	size_t __guardsize;
	int __stackaddr_set;
	void *__stackaddr;
	size_t __stacksize;
	} pthread_attr_t;

typedef struct
	{	/* thread condition variable */
	struct _pthread_fastlock __c_lock;
	_pthread_descr __c_waiting;
	char __padding[48 - sizeof (struct _pthread_fastlock)
		- sizeof (_pthread_descr)];
	} pthread_cond_t;

typedef struct
	{	/* thread condition attributes */
	int __dummy;
	} pthread_condattr_t;

typedef unsigned int pthread_key_t;	/* thread data key */

typedef struct
	{	/* thread mutex */
	int __m_reserved;
	int __m_count;
	_pthread_descr __m_owner;
	int __m_kind;
	struct _pthread_fastlock __m_lock;
	} pthread_mutex_t;

typedef struct
	{	/* thread mutex attributes */
	int __mutexkind;
	} pthread_mutexattr_t;

typedef int pthread_once_t;	/* thread one-time flag */

typedef struct
	{	/* thread read-write lock */
	struct _pthread_fastlock	__rw_lock;
	int __rw_readers;
	_pthread_descr __rw_writer;
	_pthread_descr __rw_read_waiting;
	_pthread_descr __rw_write_waiting;
	int __rw_kind;
	int __rw_pshared;
	} pthread_rwlock_t;

typedef struct
	{	/* thread read-write lock attributes */
	int __lockkind;
	int __pshared;
	} pthread_rwlockattr_t;

typedef unsigned long pthread_t;	/* thread ID */

#endif /* __QDSP6_DINKUM_PTHREAD_TYPES__ */

_END_C_LIB_DECL
_C_STD_END
#endif /* _SYS_TYPES */

#ifdef _STD_USING
using _CSTD blkcnt_t;
using _CSTD blksize_t;

 #ifdef _STD_USING_CLOCK_T
using _CSTD clock_t;
 #endif /* _STD_USING_CLOCK_T */

using _CSTD clockid_t;
using _CSTD dev_t;
using _CSTD fsblkcnt_t;
using _CSTD fsfilcnt_t;
using _CSTD gid_t;
using _CSTD id_t;
using _CSTD ino_t;
using _CSTD key_t;
using _CSTD mode_t;
using _CSTD nlink_t;
using _CSTD off_t;
using _CSTD pid_t;

using _CSTD pthread_attr_t;
using _CSTD pthread_cond_t;
using _CSTD pthread_condattr_t;
using _CSTD pthread_key_t;
using _CSTD pthread_mutex_t;
using _CSTD pthread_mutexattr_t;
using _CSTD pthread_once_t;
using _CSTD pthread_rwlock_t;
using _CSTD pthread_rwlockattr_t;
using _CSTD pthread_t;

using _CSTD size_t;
using _CSTD ssize_t;
using _CSTD suseconds_t;
using _CSTD time_t;
using _CSTD timer_t;
using _CSTD uid_t;
using _CSTD useconds_t;
#endif /* _STD_USING */

/*
 * Copyright (c) 2006 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

/*
060622 pjp: added new file
 */
