/**=============================================================================
@file
    qhcomplex_ccos_f.S

@brief
    Computes the complex cosine of z.

    Function prototype

        float complex qhcomplex_ccos_f(float complex z)

    Reference C code
        
        float complex qhcomplex_ccos_f(float complex z)
        {
            return qhcomplex_ccoshf((float complex) { -cimagf(z), crealf(z)) } );
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_ccos_f
.type qhcomplex_ccos_f, @function

/*============================================================================*/

#define Re r0
#define Im r1

qhcomplex_ccos_f:
{
  Re = togglebit(Im,#31)
  Im = Re
  jump qhcomplex_ccosh_f
}
    .size qhcomplex_ccos_f, .-qhcomplex_ccos_f
