/**=============================================================================
@file
    qhmath_rsqrt_f.S

@brief
    Computes the reciprocal of square root of single-precision floating-point argument.

    Function prototype

        float qhmath_rsqrt_f(float x);

    Reference C code
        
        float qhmath_rsqrt_f( float x )
        {
            const float threehalfs = 1.5F;
            float N = x * 0.5F;
            float y = sfinvsqrta(x);

            // Newton-Raphson Method
            y  = y * ( threehalfs - ( N * y * y ) );   // 1st iteration
            y  = y * ( threehalfs - ( N * y * y ) );   // 2nd iteration

            return y;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_rsqrt_f
    .type qhmath_rsqrt_f, @function

/*============================================================================*/

#define HALF_VAL        0x3F000000
#define THREEHALFS_VAL  0x3FC00000
#define NaN_VAL         0x7FC00000
#define INFINITY_VAL    0x7F800000

#define X               r0
#define FIX             r1
#define N               r2
#define Y               r3
#define TMP             r4
#define THREEHALFS      r5

#define ERROR_INPUT     r1
#define INFINITY        r2

#define RESULT          r0

/*============================================================================*/

qhmath_rsqrt_f:
{
    p0 = sfclass(X, #6)                // FP_SUBNORMAL | FP_NORMAL
    p0 = cmp.gt(X, #0)
    if (!p0.new) jump:nt .L_input_error

    N = ##HALF_VAL
}
{
    N = sfmpy(X,N)

    THREEHALFS = ##THREEHALFS_VAL

    Y,p0 = sfinvsqrta(X)
}
{
    FIX = sffixupr(X)

    TMP = sfmpy(Y,Y)
}
{
    THREEHALFS -= sfmpy(TMP, N)
}
{
    Y = sfmpy(Y,THREEHALFS)

    THREEHALFS = ##THREEHALFS_VAL
}
{
    TMP = sfmpy(Y,Y)
}
{
    THREEHALFS -= sfmpy(TMP, N)

    RESULT = ##0
}
{
    RESULT += sfmpy(Y,THREEHALFS, p0):scale

    jumpr r31
}

    /* --- Errors: */
    .falign
.L_input_error:
{
    ERROR_INPUT = X
    RESULT = ##NaN_VAL
    INFINITY = ##INFINITY_VAL
}
{
    p1 = cmp.eq(ERROR_INPUT, INFINITY)
    if (p1.new) RESULT = #0
    p2 = cmp.eq(ERROR_INPUT, #0)
    if (p2.new) RESULT = INFINITY
}
{
    RESULT = sfadd(RESULT, RESULT)
    jumpr r31
}
    .size qhmath_rsqrt_f, .-qhmath_rsqrt_f
