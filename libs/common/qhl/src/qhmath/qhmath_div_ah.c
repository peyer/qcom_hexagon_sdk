/**=============================================================================
@file
    qhmath_div_ah.c

@brief
    Divides each element in input_num array by corresponding element in
    input_den array.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_div_ah(int16_t *input_num, int16_t *input_den, int16_t *output, uint32_t size)
{
    QHMATH_VECTORIZE_FUN (qhmath_div_h, output, size, input_num, input_den)
}