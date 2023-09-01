/**=============================================================================
@file
    qhmath_copysign_ah.c

@brief
    Composes output array of 16-bit values with the absolute value of inputs
    and the sign of k.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_copysign_ah(int16_t *input, int16_t k, int16_t *output, uint32_t size)
{
    if ((input == NULL) || (output == NULL) || (size == 0))
    {
        return -1;
    }

    for (uint32_t i = 0; i < size; i++)
    {
        output[i] = qhmath_copysign_h(input[i], k);
    }

    return 0;
}