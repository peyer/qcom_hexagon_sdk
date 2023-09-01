/**=============================================================================
@file
    qhcomplex_cproj_f.S

@brief
    Computes projection of z on the Riemann sphere.

    Function prototype

        float complex qhcomplex_cproj_f(float complex z)

    Reference C code

        float complex qhcomplex_cproj_f(float complex z)
        {
            if (!isinf(crealf(z)) && !isinf(cimagf(z)))
                return (z);
            else
                return (CMPLXF(INFINITY, copysignf(0.0, cimagf(z))));
        }

@note
    Reference C code obtained from openlibm library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_cproj_f
.type qhcomplex_cproj_f, @function

/*============================================================================*/

#define  real       r0
#define  imag       r1
#define  mask       r2
#define  INF_VAL    0x7F800000
#define  SIGN_MASK  0x80000000

qhcomplex_cproj_f:
{
    p0 = sfclass(real, #23)
    p0 = sfclass(imag, #23)
    mask = #SIGN_MASK
}
{
    if (!p0) r0 = #INF_VAL
    if (!p0) r1 = and(imag, mask)
    jumpr r31
}
    .size  qhcomplex_cproj_f, .-qhcomplex_cproj_f
