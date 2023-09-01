/**=============================================================================
@file
    qhcomplex_csqrt_f.S

@brief
    Computes the complex square root of z.

    Function prototype

        float complex qhcomplex_csqrt_f(float complex z)

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_csqrt_f
.type qhcomplex_csqrt_f, @function

/*============================================================================*/

qhcomplex_csqrt_f:
{
    jump csqrtf
}
    .size qhcomplex_csqrt_f, .-qhcomplex_csqrt_f
