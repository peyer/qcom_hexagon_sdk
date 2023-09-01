/**=============================================================================
@file
    qhcomplex_cabs_f.S

@brief
    Computes the complex absolute value of z.

    Function prototype

        float qhcomplex_cabs_f(float complex z)

    Reference C code

        float cabsf(float complex z)
        {
            return hypotf(crealf(z), cimagf(z));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_cabs_f
.type qhcomplex_cabs_f, @function

/*============================================================================*/

qhcomplex_cabs_f:
{
    jump qhmath_hypot_f
}
    .size qhcomplex_cabs_f, .-qhcomplex_cabs_f
