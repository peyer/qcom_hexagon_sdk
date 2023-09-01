/**=============================================================================
@file
    qhmath_hypot_f.S

@brief
    Computes the hypotenuse of right-angled triangle.

    Function prototype
        
        float qhmath_hypot_f(float x, float y)

    Reference C code
    
        float hypotf(float x, float y)
        {
            union {float f; uint32_t i;} ux = {x}, uy = {y}, ut;
            float_t z;

            ux.i &= -1U>>1;
            uy.i &= -1U>>1;
            if (ux.i < uy.i) {
                ut = ux;
                ux = uy;
                uy = ut;
            }

            x = ux.f;
            y = uy.f;
            if (uy.i == 0xff<<23)
                return y;
            if (ux.i >= 0xff<<23 || uy.i == 0 || ux.i - uy.i >= 25<<23)
                return x + y;

            z = 1;
            if (ux.i >= (0x7f+60)<<23) {
                z = 0x1p90f;
                x *= 0x1p-90f;
                y *= 0x1p-90f;
            } else if (uy.i < (0x7f-60)<<23) {
                z = 0x1p-90f;
                x *= 0x1p90f;
                y *= 0x1p90f;
            }
            return z*sqrtf((double)x*x + (double)y*y);
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_hypot_f
    .type qhmath_hypot_f, @function

/*============================================================================*/

#define  x          r0
#define  y          r1
#define  absx       r0
#define  absy       r1
#define  uy         r0
#define  ux         r1
#define  scale      r2
#define  z          r3
#define  zscale     r3:2
#define  one        r4
#define  INF_VAL    0x7F800000
#define  INFM1_VAL  0x7F7FFFFF

/*============================================================================*/

qhmath_hypot_f:
{
    absx = clrbit(x,#31)
    absy = clrbit(y,#31)
    scale = ##310378496
}
{
    uy = minu(absx,absy)
    ux = maxu(absx,absy)
    z = ##1820327936
}
{
    one = sfmake(#384):pos
    p0 = cmp.eq(uy,##INF_VAL)
    if (p0.new) jumpr:nt r31           // uy.i == 0xff<<23
}
{
    r5 = sub(ux,uy)
    p0 = cmp.gtu(ux,##INFM1_VAL)
    if (p0.new) jump:nt .L_ret         // ux.i >= 0xff<<23
}
{
    r6 = ##562036735
    p0 = cmp.eq(uy,#0)
    if (p0.new) jump:nt .L_ret         // uy.i == 0
}
{
    p0 = cmp.gtu(r5,##209715199)       // ux.i - uy.i >= 25<<23
    if (p0.new) jump:nt .L_ret
}
{
    allocframe(#16)
    p0 = cmp.gtu(ux,##1568669695)      // ux.i >= (0x7f+60)<<23
    if (p0.new) jump:t .L_adjust
}
{
    p0 = cmp.gtu(uy,r6)                // uy.i < (0x7f-60)<<23
    if (p0.new) jump:t .L_sqrtf
    if (p0.new) z = one
    if (!p0.new) zscale = combine(scale,z)
}
    .falign
.L_adjust:
{
    ux = sfmpy(ux,scale)
    uy = sfmpy(uy,scale)
}
    .falign
.L_sqrtf:
{
    memd(sp+#0) = r17:16
    r17:16 = combine(z,uy)             // prevent uy and z from being clobbered
    r1:0 = convert_sf2df(ux)
    r3:2 = convert_sf2df(ux)
}
{
    call __hexagon_muldf3
}
{
    memd(sp+#8) = r19:18
    r19:18 = combine(r1,r0)            // prevent muldf3 result from being clobbered
    r1:0 = convert_sf2df(r16)
    r3:2 = convert_sf2df(r16)
}
{
    call __hexagon_muldf3
}
{
    r19:18 = memd(sp+#8)
    r1:0 = dfadd(r19:18,r1:0)
}
{
    call qhmath_sqrt_f
    r0 = convert_df2sf(r1:0)
}
{
    r0 = sfmpy(r17,r0)
    r17:16 = memd(sp+#0)
    dealloc_return
}
    .falign
.L_ret:
{
    r0 = sfadd(r0,r1)
    jumpr r31
}
    .size qhmath_hypot_f, .-qhmath_hypot_f
