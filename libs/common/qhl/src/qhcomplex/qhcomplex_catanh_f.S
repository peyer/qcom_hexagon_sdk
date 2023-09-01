/**=============================================================================
@file
    qhcomplex_catanh_f.S

@brief
    Computes the complex arctan hyperbolic of z.

    Function prototype

        float complex qhcomplex_catanh_f(float complex z)

    Reference C code

        float complex catanhf(float complex z)
        {
            z = catanf(CMPLXF(-cimagf(z), crealf(z)));
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
.global qhcomplex_catanh_f
.type qhcomplex_catanh_f, @function

/*============================================================================*/

#define Re r0
#define Im r1
#define Re_catan r0
#define Im_catan r1

qhcomplex_catanh_f:
{
    allocframe(#0)
    Re = togglebit(Im,#31)
    Im = Re
    call qhcomplex_catan_f
}
{
    Im = togglebit(Re_catan,#31)
    Re = Im_catan
    dealloc_return
}
    .size  qhcomplex_catanh_f, .-qhcomplex_catanh_f
