/**=============================================================================
@file
    qhcomplex_casinh_f.S

@brief
    Computes the complex arcsin hyperbolic of z.

    Function prototype

        float complex qhcomplex_casinh_f(float complex z)

    Reference C code

        float complex casinhf(float complex z)
        {
            z = casinf(CMPLXF(-cimagf(z), crealf(z)));
            return CMPLXF(cimagf(z), -crealf(z));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_casinh_f
.type qhcomplex_casinh_f, @function

/*============================================================================*/

qhcomplex_casinh_f:
{
    r0 = togglebit(r1,#31)
    r1 = r0
    allocframe(#0)
    call qhcomplex_casin_f
}
{
    r1 = togglebit(r0,#31)
    r0 = r1
    dealloc_return
}
    .size  qhcomplex_casinh_f, .-qhcomplex_casinh_f
