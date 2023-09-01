/**=============================================================================
@file
    qhmath_log10_h.S

@brief
    Computes base 10 logarithm for 16-bit fixed-point type as input argument.
    Input argument is 16-bit fixed-point Q0 format. Output is 16-bit fixed-point
    Q5.11 unsigned format.

    Function prototype

        uint16_t qhmath_log10_h(uint16_t x)

    Reference C code

        uint16_t qhmath_log10_h(uint16_t x)
        {
            int16_t y;
            // inline qhmath_log2(x)
            y = qhmath_log2(x);
            // Scale with base 10 logarithm of 2
            y = y * LOG10_2;
            return  y >> 11;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_log10_h
    .type qhmath_log10_h, @function

/*============================================================================*/

#define x_sht_11      r2
#define x             r0
#define fract         r3
#define FRACT_BITS    #11
#define norm          r4
#define msb_pos       r5
#define b             r4
#define b_sht_11      r2
#define fract_2       r0
#define fract_2_sht   r4
#define ret_value     r0
#define v1            r3
#define v2            r4
#define V0            #-3453
#define ONE_Q11_FMT   #2048
#define POLY_1_CONST  #4155
#define POLY_2_CONST  #702
#define LOG10_2       #617
#define log10_2       r4

/*============================================================================*/

qhmath_log10_h:
{
    x_sht_11 = asl(x, FRACT_BITS)
    fract = ONE_Q11_FMT
    norm = normamt(x)
    p0 = cmp.eq(x, #0)
}
{
    msb_pos = sub(#41, norm)
    b = sub(#30, norm)
    if (p0) jump:nt .L_ret_error
}
{
    x_sht_11 = togglebit(x_sht_11, msb_pos)
}
{
    fract += lsr(x_sht_11, b)
    b_sht_11 = asl(b, FRACT_BITS)
}
{
    fract_2 = mpyi(fract, fract)
    v1 = +mpyi(fract, POLY_1_CONST)
}
{
    v2 = lsr(fract_2, FRACT_BITS)
    v1 = lsr(v1, FRACT_BITS)
}
{
    v2 = +mpyi(v2, POLY_2_CONST)
    ret_value = add(b_sht_11,add(v1,V0))
}
{
    ret_value -= lsr(v2, FRACT_BITS)
    log10_2 = LOG10_2
}
{
    ret_value = mpyi(ret_value, log10_2)
}
{
    ret_value = asr(ret_value, FRACT_BITS)
    jumpr r31
}
.L_ret_error:
{
    jumpr r31
}

    .size    qhmath_log10_h, .-qhmath_log10_h
