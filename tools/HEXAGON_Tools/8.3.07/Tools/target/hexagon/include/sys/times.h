/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

/*
 * Qualcomm:
 * Structure from the Open Group Base Specification.
 * Used by times call.
 *
 * Notes:
 * Deviations from the Standard:
 *
 *     tms_utime and tms_stime are returned in units of 
 *     cycles executed.
 *
 *     The child elements (cstime, cutime) are not valid in
 *     standalone mode.
 */
#ifndef _times_h
#define _times_h

#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#include <time.h>

struct tms {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
};

_EXTERN_C	/* declarations */
clock_t times (struct tms *buffer) _NO_THROW;
_END_EXTERN_C

#endif

