/**=============================================================================
@file
    qhmath_exp10_ah.c

@brief
    Computes base-10 exponential over the elements of 16-bit input array.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_exp10_ah(int16_t *input, int16_t *output, uint32_t size)
{
    QHMATH_VECTORIZE_FUN (qhmath_exp10_h, output, size, input)
}