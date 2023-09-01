/**=============================================================================
@file
    qhmath_sqrt_h.S

@brief
    Computes square root for 16-bit fixed-point type as input argument.
    Input argument is 16-bit fixed-point Q0 format. Output is 16-bit fixed-point
    Q8.8 unsigned format.


    Function prototype

        uint16_t qhmath_sqrt_h(uint16_t x)

    Reference C code

        uint16_t qhmath_sqrt_h(uin16_t x)
        {
            x_ft = (float) x;
            float N = x_ft * 0.5f;
            float y;

            y = sfinvsqrta(x_ft);

            // Newton-Raphson Method
            y  = y * ( 1.5f - ( N * y * y ) );   // 1st iteration

            y = y * 256.0f; // Scale for uint16_t

            y = x * y; // x * 1/sqrt(x) = sqrt(x)

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
#define CONST_256         #0x43800000
#define scale_q8_reg      r7

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_sqrt_h
    .type qhmath_sqrt_h, @function

/*============================================================================*/

qhmath_sqrt_h:
{
    ft_x = convert_w2sf(x)
    n_reg = CONST_0_5
}
{
    three_halfs_reg = CONST_3_2
    n_reg = sfmpy(ft_x,n_reg)
    y,p0 = sfinvsqrta(ft_x)
}
{
    fix = sffixupr(ft_x)
    scale_q8_reg = CONST_256
}
{
    y_2 = sfmpy(y,y)
    scale_fact = sfmpy(ft_x, scale_q8_reg)
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
    .size qhmath_sqrt_h, .-qhmath_sqrt_h
