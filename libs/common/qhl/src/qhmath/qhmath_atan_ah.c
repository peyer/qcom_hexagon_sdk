/**=============================================================================
@file
    qhmath_atan_ah.c

@brief
    Computes arctan over the elements of 16-bit input array.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_atan_ah(int32_t *input, int16_t *output, uint32_t size)
{
    QHMATH_VECTORIZE_FUN (qhmath_atan_h, output, size, input)
}