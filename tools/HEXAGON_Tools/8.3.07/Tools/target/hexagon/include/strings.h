/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#ifndef _STRINGS_H
#define _STRINGS_H

#include <sys/types.h>
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */


_EXTERN_C	/* declarations */
int bcmp(const void *, const void *, size_t) _NO_THROW;
void bcopy(const void *, void *, size_t) _NO_THROW;
void bzero(void *, size_t) _NO_THROW;
int ffs(int) _NO_THROW;
char *index (const char *, int) _NO_THROW;
char *rindex (const char *, int) _NO_THROW;
_END_EXTERN_C



#endif

