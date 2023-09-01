/**=============================================================================
@file
    qhmath_div_f.S

@brief
    Division of single-precision floating-point values.
    Divides numerator by denominator.

    Function prototype

        float qhmath_div_f(float n, float d);

    Reference C code

        float qhmath_div_f(float n, float d)
        {
            return n/d;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_div_f
    .type qhmath_div_f, @function

/*============================================================================*/

#define in_N      r0                            // input numerator (n)
#define in_D      r1                            // input denominator (d)

#define N         r2                            // numerator after fixup (n')
#define D         r3                            // denominator after fixup (d')

#define r_0       r4
#define r_1       r4
#define r_2       r4

#define e_0       r5
#define e_1       r5

#define d_0       r6
#define d_1       r6

#define q_0       r0
#define q_1       r0

#define NEG       r6
#define NEG_ZERO  0x80000000                    // -0.0
#define C_ONE     0x3f800000                    // 1.0

/*============================================================================*/

qhmath_div_f:
{
    r_0,p0 = sfrecipa(in_N,in_D)
    D = sffixupd(in_N,in_D)
    e_0 = ##C_ONE
}
{
    N = sffixupn(in_N,in_D)
    e_0 -= sfmpy(r_0,D):lib
    NEG = ##NEG_ZERO
}
{
    r_1 += sfmpy(e_0,r_0):lib
    q_0 = and(NEG,N)
    e_1 = ##C_ONE
    d_0 = N
}
{
    e_1 -= sfmpy(r_1,D):lib
    q_0 += sfmpy(N,r_1):lib
}
{
    r_2 += sfmpy(e_1,r_1):lib
    d_0 -= sfmpy(q_0,D):lib
}
{
    q_1 += sfmpy(d_0,r_2):lib
}
{
    N -= sfmpy(q_1,D):lib
}
{
    r0 += sfmpy(N,r_2,p0):scale
    jumpr r31
}
    .size qhmath_div_f, .-qhmath_div_f
