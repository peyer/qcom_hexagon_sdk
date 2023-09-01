/**=============================================================================
@file
    qhcomplex_casin_f.S

@brief
    Computes the complex arcsin of z.

    Function prototype

        float complex qhcomplex_cabs_f(float complex z)

    Reference C code

        float complex casinf(float complex z)
        {
            float complex w;
            float x, y;

            x = crealf(z);
            y = cimagf(z);
            w = CMPLXF(1.0 - (x - y)*(x + y), -2.0*x*y);
            float complex r = clogf(CMPLXF(-y, x) + csqrtf(w));
            return CMPLXF(cimagf(r), -crealf(r));
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_casin_f
.type qhcomplex_casin_f, @function

/*============================================================================*/

#define ONE r4
#define Re r0
#define Im r1
#define w_Re r0
#define w_Im r1
#define Re_stored r16
#define Im_stored r17

qhcomplex_casin_f:
{
    r17:16 = combine(Im,Re)
    memd(r29+#-16) = r17:16
    allocframe(#16)
}
{
    r2 = sfsub(Re,Im)
    r3 = sfadd(Re,Im)
}
{
    ONE = sfmake(#384):pos
    r5 = sfmpy(r2,r3)
}
{
    r3 = sfmake(#448):neg           // -2.0f
    r2 = sfmpy(Im,Re)
}
{
    w_Im = sfmpy(r2,r3)
    w_Re = sfsub(ONE,r5)
}
{
    call qhcomplex_csqrt_f
}
{
    r0 = sfsub(r0,Im_stored)
    r1 = sfadd(Re_stored,r1)
}
{
    call qhcomplex_clog_f
}
{
    r3 = togglebit(r0,#31)
    r17:16 = memd(r29+#8)
}
{
    r1:0 = combine(r3,r1)
    dealloc_return
}
    .size  qhcomplex_casin_f, .-qhcomplex_casin_f
