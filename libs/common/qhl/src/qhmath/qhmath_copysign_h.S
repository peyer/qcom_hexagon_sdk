/**=============================================================================
@file
    qhmath_copysign_h.S

@brief
    Composes a 16-bit value with the absolute value of x and the sign of k.

    Function prototype

        int16_t qhmath_copysign_h(int16_t x, int16_t k);

    Reference C code

        int16_t qhmath_copysign_h(int16_t x, int16_t k)
        {
            int16_t change = (x ^ k) & 0x8000;

            if (change)
                x = -x; // invert

            return x;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_copysign_h
    .type qhmath_copysign_h,@function

/*============================================================================*/

qhmath_copysign_h:
{
    r2 = neg(r0)
    r3 = xor(r0,r1)
    p1 = cmph.eq(r0, #-32768)                   // check for saturation
}
{
    p0 = tstbit(r3, #15)
    if (p0.new) r0 = r2
    if (!p1) jumpr r31
}
.L_SATURATION:
{
    p0 = tstbit(r1, #15)
    if (!p0.new) r0 = #0x7fff                   // saturation !
    jumpr r31
}
    .size qhmath_copysign_h, .-qhmath_copysign_h
