/**=============================================================================
@file
   qhdsp_internal.h

@brief
   Header file for C implementation of QHDSP routines.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHDSP_INTERNAL_H
#define _QHDSP_INTERNAL_H

#include <stdint.h>

#define MAX_BYTES_ON_STACK          2048
#define MAX_COMPLEX_FLOATS_ON_STACK (MAX_BYTES_ON_STACK / sizeof(float complex))
#define MAX_FLOATS_ON_STACK         (MAX_BYTES_ON_STACK / sizeof(float))
#define MAX_COMPLEX_FXPs_ON_STACK   (MAX_BYTES_ON_STACK / sizeof(int32_t))
#define MAX_FXPs_ON_STACK           (MAX_BYTES_ON_STACK / sizeof(int16_t))

#endif /* _QHDSP_INTERNAL_H */
