/**=============================================================================
@file
    qhcomplex_cimag_f.S

@brief
    Returns the imaginary part of z.

    Function prototype

        float qhcomplex_cimag_f(float complex z)

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_cimag_f
.type qhcomplex_cimag_f, @function

/*============================================================================*/

#define imag    r1

qhcomplex_cimag_f:
{
    r0 = imag
    jumpr r31
}
    .size qhcomplex_cimag_f, .-qhcomplex_cimag_f
