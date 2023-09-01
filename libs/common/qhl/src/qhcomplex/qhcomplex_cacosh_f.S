/**=============================================================================
@file
    qhcomplex_cacosh_f.S

@brief
    Computes the complex arccos hyperbolic of z.

    Function prototype

        float complex qhcomplex_cacosh_f(float complex x)

    Reference C code

        float complex cacoshf(float complex z)
        {
                float complex w;
                w = cacosf(z);
                return CMPLXF(-cimagf(z), crealf(z));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_cacosh_f
.type qhcomplex_cacosh_f, @function

/*============================================================================*/

#define Re_cacos r0
#define Im_cacos r1
#define Re_ r0
#define Im_ r1

qhcomplex_cacosh_f:
{
    call qhcomplex_cacos_f
    allocframe(#0)
}
{
    Re_ = togglebit(Im_cacos,#31)
    Im_ = Re_cacos
    dealloc_return
}
    .size   qhcomplex_cacosh_f, .-qhcomplex_cacosh_f
