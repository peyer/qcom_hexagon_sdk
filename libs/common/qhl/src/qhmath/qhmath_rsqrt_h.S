/**=============================================================================
@file
    qhmath_rsqrt_h.S

@brief
    Computes the reciprocal of square root of 16-bit fixed-point argument.
    Input argument is 16-bit fixed-point Q0 format.
    Output is 16-bit fixed-point Q15 format.

    Function prototype

        uint16_t qhmath_rsqrt_h(uint16_t x)

    Reference C code

        uint16_t qhmath_rsqrt_h(uin16_t x)
        {
            float x_ft = (float) x;
            float N = x_ft * 0.5f;
            float y;

            // Check infinitive condition
            if (x == 0)
            {
                return x;
            }

            y = sfinvsqrta(x_ft);

            // Newton-Raphson Method
            y  = y * ( 1.5f - ( N * y * y ) );   // 1st iteration

            y = y * 65535.0f; // Scale for uint16_t

            return (uint16_t) y;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

#define x                 r0
#define y                 r0
#define ft_x              r1
#define CONST_0_5         #0x3F000000
#define n_reg             r2
#define three_halfs_reg   r3
#define CONST_3_2         #0x3FC00000
#define fix               r4
#define y_2               r5
#define scale_fact        r6
#define CONST_HALF_RANGE  #0x477fff00

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_rsqrt_h
    .type qhmath_rsqrt_h, @function

/*============================================================================*/

qhmath_rsqrt_h:
{
    ft_x = convert_w2sf(x)
    n_reg = CONST_0_5
    p0 = cmp.eq(x, #0)
    if (p0.new) jump:nt .L_inf
}
{
    three_halfs_reg = CONST_3_2
    n_reg = sfmpy(ft_x,n_reg)
    y,p0 = sfinvsqrta(ft_x)
}
{
    fix = sffixupr(ft_x)
    y_2 = sfmpy(y,y)
    scale_fact = CONST_HALF_RANGE
}
{
    three_halfs_reg -= sfmpy(y_2, n_reg)
    y = sfmpy(y,scale_fact)
}
{
    y = sfmpy(y, three_halfs_reg)
}
{
    y = convert_sf2w(y)
    jumpr r31
}

.L_inf:
{
    jumpr r31
}
    .size qhmath_rsqrt_h, .-qhmath_rsqrt_h
