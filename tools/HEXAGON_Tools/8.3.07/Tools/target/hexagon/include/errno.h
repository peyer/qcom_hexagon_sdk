/* errno.h standard header */
#ifndef _ERRNO
#define _ERRNO
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

/* C11 */
typedef int errno_t;

		/* ERROR CODES */
#define EDOM           0x0021
#define EFAULT         0x000E
#define EFBIG          0x001B
#define EFPOS          0x0098
#define EMLINK         0x001F
#define ENFILE         0x0017
#define ENOENT         0x0002
#define ENOSPC         0x001C
#define ENOTTY         0x0019
#define EPIPE          0x0020
#define ERANGE         0x0022
#define EROFS          0x001E
#define ESPIPE         0x001D

 #if defined(__BORLANDC__)
  #define E2BIG         0x0014
  #define EACCES        0x0005
  #define EAGAIN        0x002A
  #define EBADF         0x0006
  #define EBUSY         0x002C
  #define ECHILD        0x0018
  #define EEXIST        0x0023
  #define EINTR         0x0027
  #define EINVAL        0x0013
  #define EIO           0x0028
  #define EISDIR        0x002E
  #define EMFILE        0x0004
  #define ENODEV        0x000F
  #define ENOEXEC       0x0015
  #define ENOMEM        0x0008
  #define ENOTDIR       0x002D
  #define ENXIO         0x0029
  #define EPERM         0x0025
  #define ESRCH         0x0026
  #define EXDEV         0x0016

 #else /* defined(__BORLANDC__) */
  #define E2BIG         0x0007
  #define EACCES        0x000D
  #define EAGAIN        0x000B
  #define EBADF         0x0009
  #define EBUSY         0x0010
  #define ECHILD        0x000A
  #define EEXIST        0x0011
  #define EINTR         0x0004
  #define EINVAL        0x0016
  #define EIO           0x0005
  #define EISDIR        0x0015
  #define EMFILE        0x0018
  #define ENODEV        0x0013
  #define ENOEXEC       0x0008
  #define ENOMEM        0x000C
  #define ENOTDIR       0x0014
  #define ENXIO         0x0006
  #define EPERM         0x0001
  #define ESRCH         0x0003
  #define EXDEV         0x0012
 #endif /* defined(__BORLANDC__) */

 #if defined(__BORLANDC__)
  #define EBADMSG       0x004A
  #define ECANCELED     0x007D
  #define EDEADLK       0x0024
  #define EILSEQ        0x0058
  #define EINPROGRESS   0x0073
  #define EMSGSIZE      0x005A
  #define ENAMETOOLONG  0x0030
  #define ENOLCK        0x006F
  #define ENOSYS        0x0059
  #define ENOTEMPTY     0x0031
  #define ENOTSUP       0x005F
  #define ETIMEDOUT     0x006E

 #elif defined(_MSL_WCHAR_T_TYPE)
  #define EBADMSG       0x004A
  #define ECANCELED     0x007D
  #define EDEADLK       0x0023
  #define EILSEQ        0x0058
  #define EINPROGRESS   0x0073
  #define EMSGSIZE      0x005A
  #define ENAMETOOLONG  0x0024
  #define ENOLCK        0x004D
  #define ENOSYS        0x0026
  #define ENOTEMPTY     0x0027
  #define ENOTSUP       0x005F
  #define ETIMEDOUT     0x006E

 #elif defined(_WIN32_C_LIB)
  #define EBADMSG       0x004A
  #define ECANCELED     0x007D
  #define EDEADLK       0x0024
  #define EILSEQ        0x002A
  #define EINPROGRESS   0x0073
  #define EMSGSIZE      0x005A
  #define ENAMETOOLONG  0x0026
  #define ENOLCK        0x0027
  #define ENOSYS        0x0028
  #define ENOTEMPTY     0x0029
  #define ENOTSUP       0x005F
  #define ETIMEDOUT     0x006E

 #elif defined(_LINUX_C_LIB)
  #define EBADMSG       0x004A
  #define ECANCELED     0x007D
  #define EDEADLK       0x0023
  #define EILSEQ        0x0054
  #define EINPROGRESS   0x0073
  #define EMSGSIZE      0x005A
  #define ENAMETOOLONG  0x0024
  #define ENOLCK        0x0025
  #define ENOSYS        0x0026
  #define ENOTEMPTY     0x0027
  #define ENOTSUP       0x005F
  #define ETIMEDOUT     0x006E

  #define EWOULDBLOCK	EAGAIN
  #define EAFNOSUPPORT	0x0061

 #else /* library type */
  #define EBADMSG       0x004D
  #define ECANCELED     0x002F
  #define EDEADLK       0x002D
  #define EILSEQ        0x0058
  #define EINPROGRESS   0x0096
  #define EMSGSIZE      0x0061
  #define ENAMETOOLONG  0x004E
  #define ENOLCK        0x002E
  #define ENOSYS        0x0059
  #define ENOTEMPTY     0x005D
  #define ENOTSUP       0x0030
  #define ETIMEDOUT     0x0091
 #endif /* library type */

		/* DECLARATIONS */
_C_LIB_DECL

 #if !_MULTI_THREAD || _COMPILER_TLS && !_GLOBAL_LOCALE
extern int _TLS_QUAL _Errno;

  #define errno	(_CSTD _Errno)

 #else /* !_MULTI_THREAD || _COMPILER_TLS && !_GLOBAL_LOCALE */
int *_Geterrno(void);

  #define errno	(*_CSTD _Geterrno())
 #endif /* !_MULTI_THREAD || _COMPILER_TLS && !_GLOBAL_LOCALE */

/*  Open std specific errnos */
#define EOVERFLOW 0x0092 /* Value too large to be stored in data type. */

_END_C_LIB_DECL
_C_STD_END
#endif /* _ERRNO */

 #if defined(_STD_USING)
 #endif /* defined(_STD_USING) */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
