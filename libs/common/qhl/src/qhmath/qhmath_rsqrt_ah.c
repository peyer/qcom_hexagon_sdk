/**=============================================================================
@file
    qhmath_rsqrt_ah.c

@brief
    Computes the inverse square root over the elements of 16-bit input array
    and stores results to the output array.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_rsqrt_ah(uint16_t *input, uint16_t *output, uint32_t size)
{
    QHMATH_VECTORIZE_FUN (qhmath_rsqrt_h, output, size, input)
}