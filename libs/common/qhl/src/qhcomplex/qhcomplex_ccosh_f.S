/**=============================================================================
@file
    qhcomplex_ccosh_f.S

@brief
    Computes the complex hyperbolic cosine of z.

    Function prototype

        float complex ccoshf(float complex x)

    Reference C code

        static const float huge = 0x1p127;

        static const uint32_t k = 235; // constant for reduction
        static const float kln2 = 162.88958740F; // k * ln2


        // See __cexp.c for details.

        // Input:  ln(FLT_MAX) <= x < ln(2 * FLT_MAX / FLT_MIN_DENORM) ~= 192.7
        // Output: 2**127 <= y < 2**128

        static float __frexp_expf(float x, int *expt)
        {
            float exp_x;
            uint32_t hx;

            exp_x = expf(x - kln2);
            GET_FLOAT_WORD(hx, exp_x);
            *expt = (hx >> 23) - (0x7f + 127) + k;
            SET_FLOAT_WORD(exp_x, (hx & 0x7fffff) | ((0x7f + 127) << 23));
            return exp_x;
        }

        static float complex local__ldexp_cexpf(float complex z, int expt)
        {
            float x, y, exp_x, scale1, scale2;
            int ex_expt, half_expt;

            x = crealf(z);
            y = cimagf(z);
            exp_x = __frexp_expf(x, &ex_expt);
            expt += ex_expt;

            half_expt = expt / 2;
            SET_FLOAT_WORD(scale1, (0x7f + half_expt) << 23);
            half_expt = expt - half_expt;
            SET_FLOAT_WORD(scale2, (0x7f + half_expt) << 23);

            return CMPLXF(cosf(y) * exp_x * scale1 * scale2,
            sinf(y) * exp_x * scale1 * scale2);
        }

        float complex ccoshf(float complex z)
        {
            float x, y, h;
            int32_t hx, hy, ix, iy;

            x = crealf(z);
            y = cimagf(z);

            GET_FLOAT_WORD(hx, x);
            GET_FLOAT_WORD(hy, y);

            ix = 0x7fffffff & hx;
            iy = 0x7fffffff & hy;

            if (ix < 0x7f800000 && iy < 0x7f800000)
            {
                if (iy == 0)  //LB12 IY0
                    return CMPLXF(coshf(x), x * y);
                if (ix < 0x41100000)    // small x: normal case
                    return CMPLXF(coshf(x) * cosf(y), sinhf(x) * sinf(y));

                // |x| >= 9, so cosh(x) ~= exp(|x|)
                if (ix < 0x42b17218) { //LBB0_18
                    // x < 88.7: expf(|x|) won't overflow
                    h = expf(fabsf(x)) * 0.5f;
                    return CMPLXF(h * cosf(y), copysignf(h, x) * sinf(y));
                } else if (ix < 0x4340b1e7) { //LB 18
                    // x < 192.7: scale to avoid overflow
                    z = local__ldexp_cexpf(CMPLXF(fabsf(x), y), -1);
                    return CMPLXF(crealf(z), cimagf(z) * copysignf(1, x));
                } else { // LB26 pretpostavka
                    // x >= 192.7: the result always overflows L_overflow
                    h = huge * x;
                    return CMPLXF(h * h * cosf(y), h * sinf(y));
                }
            }

            if (ix == 0 && iy >= 0x7f800000) // LD5 SPECIAL CASE 1
                return CMPLXF(y - y, copysignf(0, x * (y - y)));

            if (iy == 0 && ix >= 0x7f800000) { //LD 8 SPECIAL CASE 2
                if ((hx & 0x7fffff) == 0)
                    return CMPLXF(x * x, copysignf(0, x) * y);
                return CMPLXF(x * x, copysignf(0, (x + x) * y)); // LDBB 24 CASE3
            }

            if (ix < 0x7f800000 && iy >= 0x7f800000) //CASE 4
                return CMPLXF(y - y, x * (y - y));

            if (ix >= 0x7f800000 && (hx & 0x7fffff) == 0) { // CASE 5
                if (iy >= 0x7f800000)
                    return CMPLXF(x * x, x * (y - y)); //CASE 5
                return CMPLXF((x * x) * cosf(y), x * sinf(y)); // CASE 6
            }

                return CMPLXF((x * x) * (y - y), (x + x) * (y - y)); // L 26
        }

@note
    Reference C code obtained from musl library.

    Copyright from musl:

    ---------------------------------------------------------------------------
    Copyright (c) 2005 Bruce D. Evans and Steven G. Kargl
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
    notice unmodified, this list of conditions, and the following
    disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------------------

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_ccosh_f
.type qhcomplex_ccosh_f, @function

/*============================================================================*/

#define C1 0x42b17217                          // 0x42B17217 88.7228
#define C2 0x4340B1E6                          // 0x4340B1E6 192.695
#define HUGE 0x7F000000                        // 0x7F000000 1.70141e+38
#define ONE 0x3F800000
#define NINE 0x410fffff
#define INF_1 0x7f7fffff
#define MASK 0x80000000
#define MASK2 0x7fffff
// special cases
// Range 0 is |x| < 9.0f
// Range 1 is 9.0f =< |x| =< 88.9

qhcomplex_ccosh_f:
{
    r17:16 = combine(r1,r0)
    memd(r29+#-16) = r17:16
    allocframe(#24)
}
{
    r4 = clrbit(r16,#31)
    r3 = clrbit(r17,#31)
    r2 = ##INF_1
}
{
    p0 = cmp.gtu(r4,r2); if (p0.new) jump:nt .L_case1
    memd(r29+#8) = r19:18
    memd(r29+#0) = r21:20
}
{
    p1 = cmp.gtu(r3,r2); if (p1.new) jump:nt .L_case1
}
{
    if (p0.new) r0 = add(r16,#0)
    p0 = cmp.eq(r3,#0); if (p0.new) jump:nt .L_iy0
}
{
    p0 = cmp.gtu(r4,##NINE)
    if (!p0.new) r0 = add(r16,#0)
    if (p0.new) jump:t .L_range1
}
{
    call qhmath_cosh_f
}
{
    call qhmath_cos_f
    r18 = r0
    r0 = r17
}
{
    call qhmath_sinh_f
    r18 = sfmpy(r18,r0)
    r0 = r16
}
{
    call qhmath_sin_f
    r16 = r0
    r0 = r17
}
{
    r19 = sfmpy(r16,r0)
    jump .L_end
}
.falign
.L_case1:
{
     p1 = cmp.eq(r4,#0); if (!p1.new) jump:nt .L_case2
}
{
    r18 = sfsub(r17,r17)
    p1 = cmp.gtu(r3,r2); if (!p1.new) jump:nt .L_case2
}
{
    r1 = sfmpy(r18,r16)
}
{
    r19 = and(r1,##MASK)
    jump .L_end
}
.falign
.L_case2:
{
    if (!p0) jump:nt .L_case5
}
{
    r18 = sfmpy(r16,r16)
    p1 = cmp.eq(r3,#0); if (!p1.new) jump:nt .L_case5
}
{
    r2 = ##MASK2
}
{
    p0 = !bitsclr(r16,r2)
    r2 = sfadd(r16,r16)
    if (!p0.new) jump:nt .L_case3
}
{
    r1 = sfmpy(r2,r17)
}
{
    r19 = and(r1,##MASK)
    jump .L_end
}
.falign
.L_iy0:                                        // iy == 0 is TRUE
{
    call qhmath_cosh_f
}
{
    r19 = sfmpy(r16,r17)
    r18 = r0 ; jump .L_end
}
.falign
.L_range1:
{
    p0 = cmp.gtu(r4,##C1)
    if (!p0.new) r0 = add(r16,#0)
    if (p0.new) jump:t .L_scale
}
{
    r0 = extractu(r0,#0x1f,#0x0)
    call qhmath_exp_f
}
{
    r2 = sfmake(#320):pos                      // 0.5f
}
{
    call qhmath_cos_f
    r19 = sfmpy(r0,r2)
    r0 = r17
}
{
    r16 = insert(r19,#0x1f,#0x0)
    r18 = sfmpy(r19,r0)
}
{
    call qhmath_sin_f
    r0 = r17
}
{
    r19 = sfmpy(r16,r0)
    jump .L_end
}
.falign
.L_case5:
{
    if (p0) jump:nt .L_case5_1
}
{
    r18 = sfsub(r17,r17)
    p1 = cmp.gtu(r3,r2); if (!p1.new) jump:nt .L_case5_1
}
{
    r19 = sfmpy(r18,r16)
    jump .L_end
}
.falign
.L_scale:
{
    p0 = cmp.gtu(r4,##C2)
    if (!p0.new) r0 = add(r16,#0)
    if (p0.new) jump:t .L_overflow
}
{
    r0 = extractu(r0,#0x1f,#0x0)
    r2 = ##-1021123652
}
{
    r19 = ##HUGE
    call qhmath_exp_f
    r0 = sfadd(r0,r2)
}
{
    r20 = sfmake(#384):pos
    r21 = sfmake(#384):pos
}
{
    r19 = insert(r0,#23,#0)
    r0 = add(##-20,lsr(r0,#23))
}
{
    r2 = r0
}
{
    r2 += lsr(r0,#31)
}
{
    r2 = asr(r2,#1)
}
{
    r20 += asl(r2,#23)
    r3 = sub(r0,r2)
    r0 = r17
}
{
    r21 += asl(r3,#23)
    call qhmath_cos_f
}
{
    r2 = sfmpy(r0,r19)
    r0 = r17
}
{
    r2 = sfmpy(r2,r20)
}
{
    call qhmath_sin_f
    r18 = sfmpy(r2,r21)
}
{
    r2 = sfmpy(r0,r19)
    r1:0 = combine(r16,##ONE)
}
{
    r2 = sfmpy(r2,r20)
}
{
    r1 = insert(r0,#0x1f,#0x0)
    r16 = sfmpy(r2,r21)
}
{
    r19 = sfmpy(r1,r16)
    jump .L_end
}
.falign
.L_case5_1:
{
    r4 = clrbit(r16,#31)
    r18 = sfmpy(r16,r16)
    if (!p0) jump:nt .L_last
}
{
    r4 = extractu(r4,#23,#0)
    if (!cmp.eq(r4.new,#0)) jump:nt .L_last
}
{
    if (!p0.new) r0 = add(r17,#0)
    p0 = cmp.gtu(r3,r2); if (!p0.new) jump:t .L_case6
}
{
    r2 = sfsub(r17,r17)
}
{
    r19 = sfmpy(r2,r16)
    jump .L_end
}
.falign
.L_case3:
{
    r0 = and(r16,##MASK)
}
{
    r19 = sfmpy(r0,r17)
    jump .L_end
}
.falign
.L_overflow:                                   // the result always overflows
{
    r2 = ##HUGE
    r0 = r17
}
{
    r16 = sfmpy(r16,r2)
}
{
    call qhmath_cos_f
    r18 = sfmpy(r16,r16)
}
{
    call qhmath_sin_f
    r18 = sfmpy(r18,r0)
    r0 = r17
}
{
    r19 = sfmpy(r16,r0)
    jump .L_end
}
.falign
.L_last:                                       // else in all other cases
{
    r2 = sfsub(r17,r17)
    r3 = sfadd(r16,r16)
}
{
    r18 = sfmpy(r18,r2)
    r19 = sfmpy(r3,r2)
    jump .L_end
}
.falign
.L_case6:
{
    call qhmath_cos_f
}
{
    call qhmath_sin_f
    r18 = sfmpy(r18,r0)
    r0 = r17
}
{
    r19 = sfmpy(r0,r16)
}
.falign
.L_end:
{
    r1:0 = combine(r19,r18)
    r17:16 = memd(r29+#16)
    r19:18 = memd(r29+#8)
}
{
    r21:20 = memd(r29+#0)
    dealloc_return
}
    .size  qhcomplex_ccosh_f, .-qhcomplex_ccosh_f
