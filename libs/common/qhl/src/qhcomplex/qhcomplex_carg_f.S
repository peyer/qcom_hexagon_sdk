/**=============================================================================
@file
    qhcomplex_carg_f.S

@brief
    Computes the argument of z.

    Function prototype

        float qhcomplex_carg_f(float complex z)

     Reference C code
     
        float cargf(float complex z)
        {
            return atan2f(cimagf(z), crealf(z));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_carg_f
.type qhcomplex_carg_f, @function

/*============================================================================*/

#define  real  r0
#define  imag  r1

qhcomplex_carg_f:
{
    r1:0 = combine(real,imag)
    jump qhmath_atan2_f
}
    .size qhcomplex_carg_f, .-qhcomplex_carg_f
