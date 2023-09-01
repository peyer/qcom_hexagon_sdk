/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#ifndef _UNISTD_H
#define _UNISTD_H
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#include <sys/types.h>


/*
 * For access, from:
 * http://www.opengroup.org/onlinepubs/009695399/basedefs/unistd.h.html
 */
#ifndef R_OK
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif 


_EXTERN_C	/* declarations */
int access (const char *, int) _NO_THROW;
unsigned int alarm (unsigned int) _NO_THROW;
char *getcwd (char *, size_t) _NO_THROW;
pid_t getpid (void) _NO_THROW;
pid_t fork (void) _NO_THROW;
int isatty (int) _NO_THROW;
void swab (const void *, void *, size_t) _NO_THROW;
/* sysconf implementation will be provided by the OS library */
long int sysconf (int) _NO_THROW;
int unlink (const char *) _NO_THROW;
unsigned int alarm (unsigned int) _NO_THROW;
int close (int) _NO_THROW;
int execve(const char *, char *const[], char *const[]) _NO_THROW;
size_t read(int, void *, size_t) _NO_THROW;
off_t lseek(int, off_t, int) _NO_THROW;
int ftruncate(int fd, off_t length) _NO_THROW;
#if _PROVIDE_POSIX_TIME_DECLS
/* sleep implementation will be provided by the OS library */
unsigned sleep(unsigned seconds) _NO_THROW;
#endif /* _PROVIDE_POSIX_TIME_DECLS */

#define _SC_NPROCESSORS_ONLN (1)

_END_EXTERN_C

#endif
