/**=============================================================================
@file
    qhcomplex_ctan_f.S

@brief
    Computes the complex tangent of z.

    Function prototype

        float complex qhcomplex_ctan_f(float complex z)

    Reference C code

        float complex ctanf(float complex z)
        {
            z = ctanhf(CMPLXF(-cimagf(z), crealf(z)));
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
.global qhcomplex_ctan_f
.type qhcomplex_ctan_f, @function

/*============================================================================*/

#define  real   r0
#define  imag   r1

qhcomplex_ctan_f:
{
    real = togglebit(imag,#31)
    imag = real
    allocframe(#0)
}
{
    call qhcomplex_ctanh_f
}
{
    r0 = r1
    r1 = togglebit(r0,#31)
    dealloc_return
}
    .size  qhcomplex_ctan_f, .-qhcomplex_ctan_f
