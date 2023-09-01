/**=============================================================================
@file
    qhmath_abs_f.S

@brief
    Computes the absolute value of single-precision floating-point argument.

    Function prototype

        float qhmath_abs_f(float x);

    Reference C code

        float qhmath_abs_f(float x)
        {
            return((x<0) ? -x : x );
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_abs_f
    .type qhmath_abs_f, @function

/*============================================================================*/

qhmath_abs_f:
{
    r0 = extractu(r0,#0x1f,#0x0)
    jumpr r31
}
    .size qhmath_abs_f, .-qhmath_abs_f
