/**=============================================================================
@file
    qhcomplex_csin_f.S

@brief
    Computes the complex sine of z.

    Function prototype

        float complex qhcomplex_csin_f(float complex z)

    Reference C code

        float complex csinf(float complex z)
        {
            z = csinhf(CMPLXF(-cimagf(z), crealf(z)));
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
.global qhcomplex_csin_f
.type qhcomplex_csin_f, @function

/*============================================================================*/

#define Re r0
#define Im r1

qhcomplex_csin_f:
{
    allocframe(#0)
    Re = togglebit(Im,#31)
    Im = Re
    call qhcomplex_csinh_f
}
{
    Re = Im
    Im = togglebit(Re,#31)
    dealloc_return
}
    .size qhcomplex_csin_f, .-qhcomplex_csin_f
