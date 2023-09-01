/**=============================================================================
@file
    qhcomplex_creal_f.S

@brief
    Returns the real part of z.

    Function prototype

        float qhcomplex_creal_f(float complex z)

    Reference C code
    
        float qhcomplex_creal_f(float complex z)
        {
            return (float)(z);
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_creal_f
.type qhcomplex_creal_f, @function

/*============================================================================*/

qhcomplex_creal_f:
{
    jumpr r31
}
    .size qhcomplex_creal_f, .-qhcomplex_creal_f
