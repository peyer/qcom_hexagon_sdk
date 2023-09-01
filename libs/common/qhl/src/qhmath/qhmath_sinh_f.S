/**=============================================================================
 @file
    qhmath_sinh_f.S

 @brief
    Computes the hyperbolic sine of single-precision floating-point argument.

    Function prototype

        float qhmath_sinh_f(float x);

    Reference C code
        // k is such that k*ln2 has minimal relative error and x - kln2 > log(FLT_MIN)
        static const int k = 235;
        static const float kln2 = 0x1.45c778p+7f;

        // expf(x)/2 for x >= log(FLT_MAX), slightly better than 0.5f*expf(x/2)*expf(x/2)
        float __expo2f(float x)
        {
            float scale;

            // note that k is odd and scale*scale overflows
            SET_FLOAT_WORD(scale, (uint32_t)(0x7f + k/2) << 23);
            // exp(x - k ln2) * 2**(k-1)
            return expf(x - kln2) * scale * scale;
        }

        float sinhf(float x)
        {
            union {float f; uint32_t i;} u = {.f = x};
            uint32_t w;
            float t, h, absx;

            h = 0.5;
            if (u.i >> 31)
                h = -h;
            // |x|
            u.i &= 0x7fffffff;
            absx = u.f;
            w = u.i;

            // |x| < log(FLT_MAX)
            if (w < 0x42b17217)
            {
                t = expm1f(absx);
                if (w < 0x3f800000)
                {
                    if (w < 0x3f800000 - (12<<23))
                        return x;
                    return h*(2*t - t*t/(t+1));
                }
                return h*(t + t/(t+1));
            }

            // |x| > logf(FLT_MAX) or nan
            t = 2*h*__expo2f(absx);
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
    .global qhmath_sinh_f
    .type qhmath_sinh_f, @function

/*============================================================================*/

#define   h         r17
#define   x         r0
#define   signx     r3:2
#define   sign      r3
#define   absx      r2
#define   w         r2
#define   one       r16
#define   SCALE     0x7A000000
#define   KLN2      0xC322E3BC
#define   LOGMAX_M1 0x42b17216
#define   ONEM1     0x3f7fffff
#define   SMALLRM1  0x397fffff

/*============================================================================*/

qhmath_sinh_f:
{
    allocframe(#8)
    memd(sp+#-16) = r17:16
    signx = bitsplit(x, #31)
    h = sfmake(#320):pos
}
{
    h |= asl(sign, #31)
    p0 = cmp.gtu(w,##LOGMAX_M1)
    if (p0.new) jump:nt .L_range0
}
{
    p0 = cmp.gtu(w,##SMALLRM1)
    if (!p0.new) jump:nt .L_range1
}
{
    r0 = absx;
    r16 = absx                                 // prevent absx from being clobbered
    call expm1f
}
{
    one = sfmake(#384):pos
    p0 = cmp.gtu(r16,##ONEM1)
    if (!p0.new) jump:t .L_range2
}
{
    r16 = r0                                   // prevent expm1f result from being clobbered
    r1 = sfadd(r0, one)
    call qhmath_div_f
}
{
    r2 = sfadd(r16,r0)
    jump .L_ret
}
    .falign
.L_range2:
{
    r2 = r0
    r0 = sfmpy(r0,r0)
    r1 = sfadd(r0,one)
}
{
    r16 = sfadd(r2,r2)
    call qhmath_div_f
}
{
    r2 = sfsub(r16,r0)
    jump .L_ret
}
    .falign
.L_range1:
{
    r0 = absx;
    r16 = x                                    // prevent x from being clobbered
    call expm1f
}
{
    r0 = r16
    r17:16 = memd(sp+#0)
    dealloc_return
}
    .falign
.L_range0:
{
    r17 = sfadd(h,h)
    r1 = ##KLN2
}
{
    r0 = sfadd(absx,r1)
    r16 = ##SCALE
    call qhmath_exp_f
}
{
    r2 = sfmpy(r0,r16)
    r17 = sfmpy(h,r16)
}
    .falign
.L_ret:
{
    r0 = sfmpy(r17,r2)
    r17:16 = memd(sp+#0)
    dealloc_return
}
    .size qhmath_sinh_f, .-qhmath_sinh_f
