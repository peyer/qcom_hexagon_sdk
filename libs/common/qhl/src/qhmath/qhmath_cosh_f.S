/**=============================================================================
@file
    qhmath_cosh_f.S

@brief
    Computes the hyperbolic cosine of single-precision floating-point argument.

    Function prototype

        float qhmath_cosh_f(float x)

    Reference C code

        float coshf(float x)
        {
            union {float f; uint32_t i;} u = {.f = x};
            uint32_t w;
            float t;

            // |x|
            u.i &= 0x7fffffff;
            x = u.f;
            w = u.i;

            // |x| < log(2)
            if (w < 0x3f317217) {
            if (w < 0x3f800000 - (12<<23)) {
                FORCE_EVAL(x + 0x1p120f);
                return 1;
            }
            t = expm1f(x);
            return 1 + t*t/(2*(1+t));
            }

            // |x| < log(FLT_MAX)
            if (w < 0x42b17217) {
            t = expf(x);
            return 0.5f*(t + 1/t);
            }

            // |x| > log(FLT_MAX) or nan
            t = __expo2f(x);
            return t;
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_cosh_f
    .type qhmath_cosh_f, @function

/*============================================================================*/

#define   x         r0
#define   w         r0
#define   kln2      r2
#define   scale     r16
#define   one       r17
#define   SCALE     0x7A000000
#define   KLN2      0xC322E3BC
#define   RIN       0x7b800000
#define   LOG2_M1   0x3F317216
#define   LOGMAX_M1 0x42B17216
#define   SMALLRM1  0x397fffff
#define   HALF      0x3f000000

/*============================================================================*/

qhmath_cosh_f:
{
    allocframe(#8)
    memd(sp+#-16) = r17:16
    x = extractu(x,#31,#0)
    one = sfmake(#384):pos
}
{
    p0 = cmp.gtu(w,##LOG2_M1)
    p1 = cmp.gtu(w,##SMALLRM1)
}
{
    r3 = ##RIN
    if (p0) jump:nt .L_range1
    if (!p1) jump:t .L_range2
}
{
    call expm1f
}
{
    r0 = sfmpy(r0,r0)
    r3 = sfadd(r0,one)
}
{
    call qhmath_div_f
    r1 = sfadd(r3,r3)
}
{
    r0 = sfadd(r0,one)
    jump .L_ret
}
    .falign
.L_range2:
{
    r0 = one
    r3 = sfadd(x,r3)
    jump .L_ret
}
    .falign
.L_range1:
{
    p0 = cmp.gtu(w,##LOGMAX_M1)
    if (p0.new) jump:t .L_range0
}
{
    call qhmath_exp_f
}
{
    r0 = one
    r1 = r0
    r16 = r0
    call qhmath_div_f
}
{
    r2 = sfadd(r16,r0)
    r3 = ##HALF
}
{
    r0 = sfmpy(r2,r3)
    jump .L_ret
}
    .falign
.L_range0:
{
    kln2 = ##KLN2
    scale = ##SCALE
}
{
    call qhmath_exp_f
    r0 = sfadd(x,kln2)
}
{
    r3 = sfmpy(r0,scale)
}
{
    r0 = sfmpy(r3,r16)
}
    .falign
.L_ret:
{
    r17:16 = memd(sp+#0)
    dealloc_return
}
    .size   qhmath_cosh_f, .-qhmath_cosh_f
