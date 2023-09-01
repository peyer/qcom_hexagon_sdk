/**=============================================================================
@file
    qhmath_abs_h.S

@brief
    Computes the absolute value of a 16-bit fixed-point argument.

    Function prototype

        int16_t qhmath_abs_h(int16_t x)

    Reference C code

        int16_t qhmath_abs_h(int16_t x)
        {
            int16_t result;

            if (x == (int16_t)0x8000)
            {
                result = 0x7fff;
            }
            else if (x < 0)
            {
                result = -x;
            }
            else
            {
                result = x;
            }

            return result;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_abs_h
    .type qhmath_abs_h, @function

/*============================================================================*/

qhmath_abs_h:
{
    r0 = zxth(r0)
}
{
    r1:0 = vabsh(r1:0):sat
    jumpr r31
}
    .size qhmath_abs_h, .-qhmath_abs_h
