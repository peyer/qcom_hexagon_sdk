/**=============================================================================
@file
    qhcomplex_conj_f.S

@brief
    Computes the complex conjugate of z.

    Function prototype

        float complex qhcomplex_conj_f(float complex z)

    Reference C code
        
        float complex conjf(float complex z)
        {
            return CMPLXF(crealf(z), -cimagf(z));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_conj_f
.type qhcomplex_conj_f, @function

/*============================================================================*/

#define  imag  r1

qhcomplex_conj_f:
{
    r1 = togglebit(imag,#31)
    jumpr r31
}
    .size qhcomplex_conj_f, .-qhcomplex_conj_f
