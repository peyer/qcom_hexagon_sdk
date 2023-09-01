/**=============================================================================
@file
    qhcomplex_cacos_f.S

@brief
    Computes the complex arccos of z.

    Function prototype

        float complex qhcomplex_cacos_f(float complex x)

    Reference C code
        
        float complex cacosf(float complex z)
        {
            float complex w;
            w = casinf(z);

            return CMPLXF((float)M_PI_2 - crealf(w), -cimagf(w));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_cacos_f
.type qhcomplex_cacos_f, @function

/*============================================================================*/

#define M_PI_2 0x3FC90FDB
#define pi_2  r2

qhcomplex_cacos_f:
{
    call qhcomplex_casin_f
    allocframe(#0)
} 
{
    r1 = togglebit(r1,#31)
    pi_2 = ##M_PI_2
}
{
    r0 = sfsub(pi_2,r0)
    dealloc_return
}
    .size  qhcomplex_cacos_f, .-qhcomplex_cacos_f
