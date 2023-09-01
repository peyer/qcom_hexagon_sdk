/**=============================================================================
@file
    qhcomplex_catan_f.S

@brief
    Computes the complex arctan of z.

    Function prototype

        float complex qhcomplex_catan_f(float complex z)

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_catan_f
.type qhcomplex_catan_f, @function

/*============================================================================*/

qhcomplex_catan_f:
{
    jump catanf
}
    .size qhcomplex_catan_f, .-qhcomplex_catan_f
