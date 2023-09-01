/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

/*
 dirent.h

 Contains definitions for directory traversal functions.

 NOTE: struct dirent is only required to contain d_ino and d_name[].

 The type DIR will index into a host-maintained struct __DIR array - i.e.,
   it is used as a "handle" to the correct DIR structure maintained on the host.
   This gives us the flexibility to define the structure in the most efficient
   form for the host.

 */

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#ifndef _DIRENT_H_
#error "Never include <sys/dirent.h> - include <dirent.h>"
#endif

struct dirent
{
  long	d_ino;
  char	d_name[MAXNAMLEN]; /* Should be variable size */
};

typedef struct {
	int		index;
	struct dirent	entry;
} DIR;

_EXTERN_C	/* declarations */
DIR *opendir (const char *) _NO_THROW;
struct dirent *readdir (DIR *) _NO_THROW;
int closedir (const DIR *) _NO_THROW;

int mkdir(char *, int) _NO_THROW;
int rmdir(char *) _NO_THROW;
_END_EXTERN_C

#endif /*_SYS_DIRENT_H*/
