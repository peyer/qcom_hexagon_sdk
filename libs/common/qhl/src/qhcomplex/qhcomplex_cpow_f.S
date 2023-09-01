/**=============================================================================
@file
    qhcomplex_cpow_f.S

@brief
    Computes the complex power function z^(c).

    Function prototype

        float complex cpowf(float complex z, float complex c)

    Reference C code

        float complex cpowf(float complex z, float complex c)
        {
            return cexpf(c * clogf(z));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_cpow_f
.type qhcomplex_cpow_f, @function

/*============================================================================*/

#define x       r0                             // real part of base complex z = x + y*I
#define y       r1                             // imaginary part of base complex z = x+y*I
#define x_c     r16                            // real part of base complex z = x + y*I
#define y_c     r17                            // imaginary part of base complex z = x+y*I

qhcomplex_cpow_f:
{
    call qhcomplex_clog_f
    r17:16 = combine(r3,r2)
    memd(r29+#-16) = r17:16
    allocframe(#8)
}
{
    r2 = sfmpy(r17,r1)
    r3 = sfmpy(r16,r1)
}
{
    r4 = sfmpy(r17,r0)
    r5 = sfmpy(r16,r0)
    r17:16 = memd(r29+#0)
    deallocframe
}
{
    r1 = sfadd(r4,r3)
    r0 = sfsub(r5,r2)
    jump qhcomplex_cexp_f
}
    .size  qhcomplex_cpow_f, .-qhcomplex_cpow_f
