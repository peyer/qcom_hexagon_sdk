/**=============================================================================
@file
    qhmath_floor_f.S

@brief
    Computes the largest integer value not greater than given argument.

    Function prototype
        
        float qhmath_floor_f(float x)

    Reference C code
        
        float floorf(float x)
        {
            union {float f; uint32_t i;} u = {x};
            int e = (int)(u.i >> 23 & 0xff) - 0x7f;
            uint32_t m;

            if (e >= 23)
                return x;
            if (e >= 0) {
                m = 0x007fffff >> e;
                if ((u.i & m) == 0)
                    return x;
                FORCE_EVAL(x + 0x1p120f);
                if (u.i >> 31)
                    u.i += m;
                u.i &= ~m;
            } else {
                FORCE_EVAL(x + 0x1p120f);
                if (u.i >> 31 == 0)
                    u.i = 0;
                else if (u.i << 1)
                    u.f = -1.0;
            }
            return u.f;
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_floor_f
    .type qhmath_floor_f, @function

/*============================================================================*/

#define  x            r0
#define  e            r1
#define  m            r2
#define  inexact      r3
#define  INEXACT_VAL  0x7B800000
#define  MMASK_H      0x7F
#define  MMASK_L      0xFFFF
#define  MONE         0xBF800000

/*============================================================================*/

qhmath_floor_f:
{
    e = extractu(x,#8,#23)
    inexact = ##INEXACT_VAL
    m.l = #MMASK_L
}
{
    m.h = #MMASK_H
    p0 = cmp.gtu(e,#149)
    if (p0.new) jumpr:t r31
    if (!p0.new) e = add(e,#-127)
}
{
    m = lsr(m,e)
    p0 = cmp.gt(e,#-1)
    if (!p0.new) jump:t .L_neg_exp
}
{
    p0 = bitsclr(m,x)
    if (p0.new) jumpr:nt r31
}
{
    inexact = sfadd(x,inexact)         // FORCE_EVAL(x + 0x1p120f)
    p0 = cmp.gt(x,#-1)
    if (!p0.new) x = add(x,m)
}
{
    x = and(x,~m)
    jumpr r31
}
    .falign
.L_neg_exp:
{
    inexact = sfadd(x,inexact)         // FORCE_EVAL(x + 0x1p120f)
    p0 = cmp.gt(x,#-1)
    if (p0.new) x = #0
    if (p0.new) jumpr:nt r31
}
{
    p0 = sfclass(x,#1)
    if (!p0.new) x = ##MONE
    jumpr r31
}
    .size qhmath_floor_f, .-qhmath_floor_f
