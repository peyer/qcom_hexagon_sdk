/**=============================================================================
@file
    qhcomplex_ctanh_f.S

@brief
    Computes the complex hyperbolic tangent of z.

    Function prototype

        float complex qhcomplex_ctanh_f(float complex z)

    Reference C code

        float complex ctanhf(float complex z)
        {
            float x, y;
            float t, beta, s, rho, denom;
            uint32_t hx, ix;

            x = crealf(z);
            y = cimagf(z);

            GET_FLOAT_WORD(hx, x);
            ix = hx & 0x7fffffff;

            if (ix >= 0x7f800000) {
                if (ix & 0x7fffff)
                    return CMPLXF(x, (y == 0 ? y : x * y));
                SET_FLOAT_WORD(x, hx - 0x40000000);
                return CMPLXF(x, copysignf(0, isinf(y) ? y : sinf(y) * cosf(y)));
            }

            if (!isfinite(y))
                return CMPLXF(ix ? y - y : x, y - y);

            if (ix >= 0x41300000) { // x >= 11
                float exp_mx = expf(-fabsf(x));
                    return CMPLXF(copysignf(1, x), 4 * sinf(y) * cosf(y) * exp_mx * exp_mx);
            }

            t = tanf(y);
            beta = 1.0 + t * t;
            s = sinhf(x);
            rho = sqrtf(1 + s * s);
            denom = 1 + beta * s * s;
            return CMPLXF((beta * rho * s) / denom, t / denom);
        }

@note
    Reference C code obtained from musl library.

    Copyright from musl:

    ---------------------------------------------------------------------------
    Copyright (c) 2011 David Schultz <das@FreeBSD.ORG>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
    ---------------------------------------------------------------------------

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_ctanh_f
.type qhcomplex_ctanh_f, @function

/*============================================================================*/

#define  ONE        0x3f800000
#define  TWO        0x40000000
#define  ELEVENM1   0x412fffff
#define  SIGN_MASK  0x80000000

qhcomplex_ctanh_f:
{
    r17:16 = combine(r1,r0)
    memd(r29+#-16) = r17:16
    allocframe(#16)
}
{
    r2 = clrbit(r0,#31)
    memd(r29+#0) = r19:18
    p0 = sfclass(r0,#7)                        // isfinite(x)
    if (p0.new) jump:t .L_isfinite_y
}
{
    r3 = #0
    p0 = sfclass(r0,#8)                        // isinf(x)
    if (p0.new) jump:nt .L_inf_x
}
{
    p0 = sfcmp.eq(r1,r3)
    if (p0.new) jump:nt .L_ret
}
{
    r1 = sfmpy(r0,r1)
    jump .L_ret
}
.falign
.L_inf_x:
{
    p0 = sfclass(r1,#8)                        // isinf(y)
    if (p0.new) jump:t .L_inf_y
    r16 = add(r0,##-TWO)
}
{
    r0 = r1
    call qhmath_sin_f
}
{
    call qhmath_cos_f
    r18 = r0
    r0 = r17
}
{
    r1 = sfmpy(r18,r0)
}
.falign
.L_inf_y:
{
    r1 = and(r1,##SIGN_MASK)
    jump .L_ret
}
.falign
.L_isfinite_y:
{
    r1 = sfsub(r17,r17)
    p0 = sfclass(r1,#7)                        // isfinite(y)
    if (p0.new) jump:t .L_finite_y
}
{
    p0 = cmp.eq(r2,#0)
    if (!p0.new) r16 = r1
    jump .L_ret
}
.falign
.L_finite_y:
{
    p0 = cmp.gtu(r2,##ELEVENM1)
    if (!p0.new) jump:t .L_smaller_range
}
{
    call qhmath_exp_f
    r0 = setbit(r0,#31)
}
{
    call qhmath_sin_f
    r18 = r0
    r0 = r17
    r19 = sfmake(#512):pos                     // 4.0
}
{
    call qhmath_cos_f
    r19 = sfmpy(r0,r19)
    r0 = r17
    r17 = ##ONE
}
{
    r16 = insert(r17,#31,#0)
    r2 = sfmpy(r19,r0)
}
{
    r2 = sfmpy(r18,r2)
}
{
    r1 = sfmpy(r18,r2)
    jump .L_ret
}
.falign
.L_smaller_range:
{
    r0 = r17
    call qhmath_tan_f
    r18 = sfmake(#384):pos // 1.0
}
{
    r2 = sfmpy(r0,r0)
    r17 = r0
    r0 = r16
}
{
    call qhmath_sinh_f
    r19 = sfadd(r2,r18)
}
{
    r2 = sfmpy(r0,r0)
    r16 = r0
}
{
    call qhmath_sqrt_f
    r0 = sfadd(r2,r18)
}
{
    r2 = sfmpy(r16,r19)
    r3 = sfmpy(r19,r0)
}
{
    r2 = sfmpy(r16,r2)
    r0 = sfmpy(r16,r3)
}
{
    r18 = sfadd(r2,r18)
}
{
    call qhmath_div_f
    r1 = r18
}
{
    call qhmath_div_f
    r16 = r0
    r1:0 = combine(r18,r17)
}
{
    r1 = r0
}
.falign
.L_ret:
{
    r0 = r16
    r17:16 = memd(r29+#8)
    r19:18 = memd(r29+#0)
}
{
    dealloc_return
}
    .size  qhcomplex_ctanh_f, .-qhcomplex_ctanh_f
