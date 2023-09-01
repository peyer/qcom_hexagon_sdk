/**=============================================================================
@file
    qhmath_log_ah.c

@brief
    Computes natural (base-e) logarithm over the elements of 16-bit input array.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdio.h>
#include <stdint.h>
#include "qhmath.h"
#include "qhmath_macros.h"

int32_t qhmath_log_ah(uint16_t *input, uint16_t *output, uint32_t size)
{
    QHMATH_VECTORIZE_FUN (qhmath_log_h, output, size, input)
}