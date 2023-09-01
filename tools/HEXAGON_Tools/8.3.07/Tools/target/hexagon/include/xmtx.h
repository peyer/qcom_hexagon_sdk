/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* xmtx.h internal header */
#ifndef _XMTX
#define _XMTX
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#include <stdlib.h>

_C_LIB_DECL

typedef void *_Rmtx;

 #if _IS_WRS	/* compiler test */

 #ifdef __RTP__
  #define _IS_WRS_RTP	1
/*
 * In the RTP world, the __diab_alloc_mutex function returns a pointer
 * to a mutex. Since the data structure is accessed through an opaque
 * void * pointer, we cannot pass an address of a mutex variable - say mutx
 * and * expect the compiler to be able to do the following structure copy:
 * mutx = *(__diab_alloc_mutex())
 * Therefore, changing the prototype of the function for that initializes
 * mutexes.
 */
void _Mtxinit(_Rmtx **);

 #else /* __RTP__ */
void _Mtxinit(_Rmtx *);
 #endif /* __RTP__ */

void _Mtxdst(_Rmtx *);
void _Mtxlock(_Rmtx *);
void _Mtxunlock(_Rmtx *);


 #else /* _IS_WRS */

void _Mtxinit(_Rmtx *);
void _Mtxdst(_Rmtx *);
void _Mtxlock(_Rmtx *);
void _Mtxunlock(_Rmtx *);

 #endif /* _IS_WRS */

 #if !_MULTI_THREAD
  #define _Mtxinit(mtx)
  #define _Mtxdst(mtx)
  #define _Mtxlock(mtx)
  #define _Mtxunlock(mtx)

typedef char _Once_t;

  #define _Once(cntrl, func)	if (*(cntrl) == 0) (func)(), *(cntrl) = 2
  #define _ONCE_T_INIT	0

 #elif _WIN32_C_LIB
typedef long _Once_t;

void _Once(_Once_t *, void (*)(void));
  #define _ONCE_T_INIT	0

 #elif _IS_WRS_RTP
  #undef _IS_WRS_RTP

typedef unsigned long __diab_atomic_level;
typedef _Rmtx __diab_mutex;

extern __diab_atomic_level __diab_atomic_enter(void);
extern void __diab_atomic_restore(__diab_atomic_level);
extern __diab_mutex __diab_alloc_mutex(void);
extern void __diab_free_mutex(__diab_mutex);
extern void __diab_lock_mutex(__diab_mutex);
extern void __diab_unlock_mutex(__diab_mutex);
extern void taskDelay(int);

typedef int _Once_t;
void _Once(_Once_t *, void (*)(void));
  #define _ONCE_T_INIT	0

#define _Mtxdst(x)	__diab_free_mutex(x)
#define _Mtxlock(x)	__diab_lock_mutex(x)
#define _Mtxunlock(x)	__diab_unlock_mutex(x)

 #elif _HAS_POSIX_C_LIB
  #include <setjmp.h>
  #include <time.h>

 #if 0x570 <= __SUNPRO_CC && !defined(_RESTRICT_KYWD)	/* compiler test */
  #define _RESTRICT_KYWD restrict
 #endif /* 0x570 <= __SUNPRO_CC etc. */

  #include <pthread.h>

typedef pthread_once_t _Once_t;

  #define _Once(cntrl, func)	pthread_once(cntrl, func)
  #define _ONCE_T_INIT	PTHREAD_ONCE_INIT

#elif __QDSP6_RTOS__
 /* Include Q6 rtos amutex defn's */

   extern void sys_Mtxinit(_Rmtx *_Mtx);
   extern void sys_Mtxdst(_Rmtx *_Mtx);
   extern void sys_Mtxlock(_Rmtx *_Mtx);
   extern void sys_Mtxunlock(_Rmtx *_Mtx);

   #define _Mtxinit(Mtx)    sys_Mtxinit(Mtx)
   #define _Mtxdst(Mtx)     sys_Mtxdst(Mtx)
   #define _Mtxlock(Mtx)    sys_Mtxlock(Mtx)
   #define _Mtxunlock(Mtx)  sys_Mtxunlock(Mtx)

   //typedef char _Once_t;
   // #define _Once(cntrl, func)	if (*(cntrl) == 0) *(cntrl) = 2, (func)()
   // #define _ONCE_T_INIT	0

  /*
   *  Allow only one client to change a memory location from zero to one
   *  Return one to the client that did the set, zero otherwise
   */

   static inline int
   atomic_test_and_set(int *p)
   {
      int result;
      __asm__ (
        "1:\n\t"
        "       %0 = memw_locked(%2)           // Try to get the lock\n\t"
        "       memw_locked(%2, p0) = %3\n\t"
        "       if !p0 jump 1b                 // Go back if we didn't get it\n\t"
        : "=&r" (result), "+m"(*p)
        : "r" (p), "r" (1)
        : "p0");
      return result == 0;
   }

/* 
 * _once_atomic_test_and_set:
 *   Very similar to the above function, but used by the _Once
 *   macro.
 * 	- Try to get the lock.
 *	- Check the value:
 *	    0: No one has the lock
 *	    1: The function passed to Once is still running.
 *	    2: The function passed to Once has already run.
 */
   static inline int
   _once_atomic_test_and_set(int *p)
   {
      int result; 
      asm volatile (
        "1:\n\t"
        "       %0 = memw_locked(%2)           // Try to get the lock\n\t"
        "       p0 = cmp.gt (%0, #0)           // func is running \n\t"
        "       if p0 jump 2f \n\t"
        "       memw_locked(%2, p0) = %3\n\t"
        "       if !p0 jump 1b                 // Go back if we didn't get it\n\t"
        "2:\n\t"
        : "=&r" (result), "+m"(*p)
        : "r" (p), "r" (1)
        : "p0");
      return result == 0;
   }
   typedef volatile int _Once_t;
/*
 * _Once:
 *  Modification from original because behavior in the library
 *  assumes that func is always done when _Once returned.
 *  Original code:
 *    #define _Once(cntrl, func)\
 * 		 if (*(cntrl) == 0) (func)(), *(cntrl) = 2
 *  Thread safe code:
 *    #define  _Once(cntrl, func) if (atomic_test_and_set(cntrl)) func()
 *
 *  The issue with both of the above macros is that _Once could return before
 *  "func" was done, the code below does not.
 */
   #define _Once(cntrl, func) \
   {                                                    \
        if (_once_atomic_test_and_set(cntrl))           \
        {                                               \
                (func)();                               \
                *(cntrl) = 2;                           \
        }                                               \
        while (*(cntrl) < 2);                           \
   }
   #define _ONCE_T_INIT 0

 #else /* library type */
  #error unknown library type
 #endif /* library type */

_END_C_LIB_DECL
#endif /* _XMTX */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
