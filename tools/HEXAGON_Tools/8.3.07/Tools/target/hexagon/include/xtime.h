/* xtime.h internal header */
#ifndef _XTIME
#define _XTIME
#include <xtinfo.h>
_C_STD_BEGIN

		/* macros */
#define WDAY	1	/* to get day of week right */

		/* type definitions */
typedef struct
	{	/* rule for daylight savings time */
	unsigned char wday, hour, day, mon, year;
	} Dstrule;

		/* internal declarations */
_C_LIB_DECL
int _Daysto(int, int);
const char * _Gentime(const struct tm *, const _Tinfo *,
	char, char, int *, char *);
Dstrule * _Getdst(const char *);
const char * _Gettime(const char *, int, int *);
int _Isdst(const struct tm *);
const char * _Getzone(void);
struct tm * _Ttotm(struct tm *, time_t, int);
time_t _Tzoff(void);
_END_C_LIB_DECL
_C_STD_END
#endif /* _XTIME */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
