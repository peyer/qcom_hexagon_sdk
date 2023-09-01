/**=============================================================================
@file
    qhmath_sqrt_f.S

@brief
    Computes the square root of single-precision floating-point argument.

    Function prototype

        float qhmath_sqrt_f(float x);

    Reference C code

        float qhmath_sqrt_f(float x);
        {
            return __hexagon_sqrtf(x);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_sqrt_f
    .type qhmath_sqrt_f, @function

/*============================================================================*/

#define RIN r0
#define S r0
#define H r1
#define D r2
#define E r3
#define HALF r4
#define R r5

#define r0_return_val       r0
#define p2_is_RIN_NEGATIVE  p2

/*============================================================================*/

qhmath_sqrt_f:
{
    p1 = sfclass(RIN, #0x6)                     // NORMAL | SUBNORMAL
    if (!p1.new) jump:nt .L_INPUT_SPEC
    p2_is_RIN_NEGATIVE = tstbit(RIN, #31)
    if (p2_is_RIN_NEGATIVE.new) jump:nt .L_INPUT_SPEC
}
{
    E,p0 = sfinvsqrta(RIN)
    R = sffixupr(RIN)
    HALF = ##0x3f000000
    r1:0 = combine(#0,#0)
}
{
    S += sfmpy(E,R):lib
    H += sfmpy(E,HALF):lib
    D = HALF
    E = R
}
{
    D -= sfmpy(S,H):lib
    p1 = sfclass(R,#1)
}
{
    S += sfmpy(S,D):lib
    H += sfmpy(H,D):lib
    D = HALF
    E = R
}
{
    D -= sfmpy(S,H):lib
    E -= sfmpy(S,S):lib
}
{
    S += sfmpy(H,E):lib
    H += sfmpy(H,D):lib
    D = HALF
    E = R
}
{
    E -= sfmpy(S,S):lib
    if (p1) r0 = or(r0,R)
}
{
    S += sfmpy(H,E,p0):scale
    jumpr r31
}
    .falign
.L_INPUT_SPEC:
{
    /* sqrt(+-0) = +-0 */
    p1 = sfclass(RIN,#1)
    if (p1.new) jumpr:t r31

    /* take care of Inf and NaN */
    /* sqrt(NaN)=NaN, sqrt(+inf)=+inf, sqrt(-inf)=sNaN */
    /* return x * x + x; */
    r2 = and(RIN,##0x7F800000)
}
{
    r0_return_val += sfmpy(RIN,RIN)

    p0 = cmp.eq(r2,##0x7F800000)
    if (p0.new) jumpr:t r31
}
    .falign
.L_INPUT_ERROR_NEGATIVE:
{
    jump qhmath_div_f
    r0 = sfsub(r0,r0)
    r1 = sfsub(r0,r0)
}
    .size qhmath_sqrt_f, .-qhmath_sqrt_f
