/**=============================================================================
@file
    qhmath_atan2_ah.c

@brief
    Computes the 4-quadrant arctan over the elements of 16-bit input array.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_atan2_ah(int16_t *input_x, int16_t *input_y, int16_t *output, uint32_t size)
{
    QHMATH_VECTORIZE_FUN(qhmath_atan2_h, output, size, input_x, input_y)
}