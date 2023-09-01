/* signal.h standard header */
#ifndef _SIGNAL
#define _SIGNAL
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

 #if defined(__GLIBC__)	/* compiler test */
  #include <bits/sigset.h>
 #endif /* defined(__GLIBC__) */

_C_STD_BEGIN

		/* SIGNAL CODES */

 #if defined(_LINUX_C_LIB)
  #define SIGHUP	1
  #define SIGINT	2
  #define SIGQUIT	3
  #define SIGILL	4
  #define SIGTRAP	5
  #define SIGABRT	6
  #define SIGIOT	6
  #define SIGBUS	7
  #define SIGFPE	8
  #define SIGKILL	9
  #define SIGUSR1	10
  #define SIGSEGV	11
  #define SIGUSR2	12
  #define SIGPIPE	13
  #define SIGALRM	14
  #define SIGTERM	15
  #define SIGSTKFLT	16
  #define SIGCLD	17
  #define SIGCHLD	17
  #define SIGCONT	18
  #define SIGSTOP	19
  #define SIGTSTP	20
  #define SIGTTIN	21
  #define SIGTTOU	22
  #define SIGURG	23
  #define SIGXCPU	24
  #define SIGXFSZ	25
  #define SIGCTALRM	26
  #define SIGPROF	27
  #define SIGWINCH	28
  #define SIGPOLL	29
  #define SIGIO		29
  #define SIGPWR	30
  #define SIGSYS	31
  #define SIGUNUSED	31

  #define __sigmask(sig)	\
	((unsigned long)1 << (((sig) - 1) % (8 * sizeof (unsigned long))))
  #define sigmask(sig)	__sigmask(sig)

int sigsetmask(int) /* __THROW */;

 #else /* _LINUX_C_LIB */
  #define SIGABRT	_SIGABRT
  #define SIGINT	2
  #define SIGILL	4
  #define SIGFPE	8
  #define SIGSEGV	11
  #define SIGALRM	14
  #define SIGTERM	15
 #endif /* _LINUX_C_LIB */

#define _NSIG	_SIGMAX	/* one more than last code */

		/* SIGNAL RETURN VALUES */
#define SIG_DFL	((_CSTD _Sigfun *)0)
#define SIG_ERR	((_CSTD _Sigfun *)-1)
#define SIG_IGN	((_CSTD _Sigfun *)1)

_EXTERN_C
		/* TYPE DEFINITIONS */

 #if !defined(__APPLE__)
typedef int sig_atomic_t;

 #elif !defined(_BSD_MACHINE_SIGNAL_H_)
  #define _BSD_MACHINE_SIGNAL_H_

typedef int sig_atomic_t;

struct sigcontext {	/* define jump buffer */

  #if defined(__ppc__)
	int _Ints[5];
	void *_Ptr;

  #elif defined(__i386__)
	int _Ints[18];

  #else	/* unknown machine type */
   #error unknown machine type
  #endif /* machine type */

	};
 #endif /* !defined(__APPLE__) */

typedef void _Sigfun(int);

		/* LOW-LEVEL FUNCTIONS */
_Sigfun * signal(int, _Sigfun *) _NO_THROW;
_END_EXTERN_C

_C_LIB_DECL		/* DECLARATIONS */
int raise(int) _NO_THROW;
_END_C_LIB_DECL
_C_STD_END
#endif /* _SIGNAL */

#ifdef _STD_USING
using _CSTD sig_atomic_t; using _CSTD raise; using _CSTD signal;
#endif /* _STD_USING */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
