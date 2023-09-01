/**=============================================================================
@file
    qhmath_atan2_f.S

@brief
    4-quadrant arctan.
    Returns the principal value of the arctan of y/x, expressed in radians.
    To compute the value, the function takes into account the sign of both
    arguments in order to determine the quadrant.

    Function prototype

        float qhmath_atan2_f(float y, float x)

    Reference C code

        static const float
        pi     = 3.1415927410e+00,  //0x40490fdb
        pi_lo  = -8.7422776573e-08; // 0xb3bbbd2e

        float atan2f(float y, float x)
        {
            float z;
            uint32_t m,ix,iy;

            if (isnan(x) || isnan(y))
                return x+y;
            GET_FLOAT_WORD(ix, x);
            GET_FLOAT_WORD(iy, y);
            if (ix == 0x3f800000)  // x=1.0
                return atanf(y);
            m = ((iy>>31)&1) | ((ix>>30)&2);  // 2*sign(x)+sign(y)
            ix &= 0x7fffffff;
            iy &= 0x7fffffff;

            // when y = 0
            if (iy == 0)
            {
                switch (m)
                {
                    case 0:
                    case 1: return y;   // atan(+-0,+anything)=+-0
                    case 2: return  pi; // atan(+0,-anything) = pi
                    case 3: return -pi; // atan(-0,-anything) =-pi
                }
            }
            // when x = 0
            if (ix == 0)
                return m&1 ? -pi/2 : pi/2;
            // when x is INF
            if (ix == 0x7f800000)
            {
                if (iy == 0x7f800000)
                {
                    switch (m)
                    {
                        case 0: return  pi/4;   // atan(+INF,+INF)
                        case 1: return -pi/4;   // atan(-INF,+INF)
                        case 2: return 3*pi/4;  //atan(+INF,-INF)
                        case 3: return -3*pi/4; //atan(-INF,-INF)
                    }
                }
                else
                {
                    switch (m)
                    {
                        case 0: return  0.0f;   // atan(+...,+INF)
                        case 1: return -0.0f;   // atan(-...,+INF)
                        case 2: return  pi;     // atan(+...,-INF)
                        case 3: return -pi;     // atan(-...,-INF)
                    }
                }

                // |y/x| > 0x1p26
                if (ix+(26<<23) < iy || iy == 0x7f800000)
                    return m&1 ? -pi/2 : pi/2;

                // z = atan(|y/x|) with correct underflow
                if ((m&2) && iy+(26<<23) < ix)  //|y/x| < 0x1p-26, x < 0
                    z = 0.0;
                else
                    z = atanf(fabsf(y/x));

                switch (m)
                {
                    case 0: return z;              // atan(+,+)
                    case 1: return -z;             // atan(-,+)
                    case 2: return pi - (z-pi_lo); // atan(+,-)
                    default:                       // case 3
                    return (z-pi_lo) - pi;         // atan(-,-)
                }
            }
        }

@note
    Reference C code obtained from musl library.

    Copyright from musl:

    ---------------------------------------------------------------------------
    origin: FreeBSD /usr/src/lib/msun/src/e_atan2f.c

    Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.

    Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
    
    Developed at SunPro, a Sun Microsystems, Inc. business.
    Permission to use, copy, modify, and distribute this
    software is freely granted, provided that this notice
    is preserved.
    ---------------------------------------------------------------------------

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_atan2_f
    .type qhmath_atan2_f, @function

/*============================================================================*/

#define   at4      0x3DA4F0D1
#define   at3      0xBE0E1B85
#define   at2      0x3E4C925F
#define   at1      0xBEAAAA2A
#define   tan3pi_8 0x401A827A
#define   tanpi_8  0x3ED413CD
#define   pi_2     0x3FC90FDB
#define   pi_4     0x3F490FDB
#define   xx       r0
#define   signx    r13:12
#define   x        r12
#define   sign     r13
#define   TWO      r2
#define   TAN3PIO8 r2
#define   TANPIO8  r4
#define   ONE      r3
#define   d        r1
#define   n        r0
#define   dn       r1:0
#define   c        r5
#define   z        r0
#define   z2       r6
#define   AT4      r10
#define   AT3      r9
#define   AT2      r10
#define   AT1      r9
#define   AT0      r10
#define   pi       0x40490FDB
#define   pi_lo    0xB3BBBD2E
#define   PI       r6
#define   PI_LO    r5
#define   YY       r9:8
#define   XX       r11:10
#define   signY    r9
#define   signX    r11
#define   signYs   r7
#define   signXs   r8
#define   y_arg    r0
#define   x_arg    r1
#define   absY     r8
#define   absX     r10
#define   INFINITY r6
#define   inf_val  0x7F800000
#define   pi_3_4   0x4016CBE4

/*============================================================================*/

qhmath_atan2_f:
{
    p0 = sfclass(x_arg,#15)
    p0 = sfclass(y_arg,#15)
    if (!p0.new) jump:nt .L_nan                // handling NAN with signaling
}
{
    allocframe(#32)
    XX = bitsplit(x_arg, #31)                  // split sign and |x| of second argument x
    YY = bitsplit(y_arg, #31)                  // split sign and |y| of first argument y
    INFINITY = ##inf_val                       // Load inf_val in r6. See #define for value
}
{
    p0 = cmp.eq(absY,#0)                       // Check if absY is 0.0
    p0 = cmp.eq(absX,#0)                       // Check if absX is 0.0
    if(p0.new) jump:nt .L_zero                 // Because p.new is logicly ADD-ing previous two expresions
}
{                                              // jump to .L_zero only if absX = absY = 0.0
    p0 = cmp.eq(absY,INFINITY)                 // Check if absY is INF
    p0 = cmp.eq(absX,INFINITY)                 // Check if absX is INF
    if(p0.new) jump:nt .L_inf                  // jump to .L_inf only if absX = absY = INF
}
{
    memd(sp+#0) = r17:16                       // store sign and free element to r17:16 !!!
    r17 = signX                                // save signX in r17
    r16 = signY                                // save signY element in r 16
}
{
    r0 = absY                                  // r0 <- |y| numinator
    r1 = absX                                  // r1 <- |x| denuminator
    call qhmath_div_f                      // r0 <- |Y| / |X|
}
{
    signXs = r17                               // load sign from r17. Restore
    signYs = r16                               // loar free element c from r16
    r17:16 = memd(sp+#0)                       // load r17:r16
}
{
    TWO = sfmake(#448):pos                     // 2.0 float constant
    TANPIO8 = ##tanpi_8                        // tg(pi/8) = 0.41 float constant
}
{
    signx = bitsplit(xx, #31)                  //x = abs x  if x < 0 range = 1 else range 0
    TAN3PIO8 = sfadd(TWO, TANPIO8)             // tg(3pi/8) float constant is 2.41 derived from 2.0 + tg(pi/8)
    c = #0                                     // free element c is int 0 for now. Free element
    d.L = #0x0000                              // r1 lower register has only zeroes for mantisa. Why is this a spilt into 2
}
{
    sign = addasl(signYs,signXs,#1)            // sign will be all zeros except 2 last bits that hold signX on #1 bit and signY on #0 bit
}
{
    d.H = #0x3f80                              // = sfmake(#384):pos which means denuminator = 1.0
    n = x                                      // numinator is x
    p0 = sfcmp.gt(TANPIO8, x)                  // compare if tg(pi/8) > |x| Note: there is only gt, eq, gte for comparison
    if(p0.new) jump:t .L_divide                // p0 (if tg(pi/8) > |x|)  jump to .L_devide. First range
}
{
    p1 = sfcmp.gt(TAN3PIO8, x)                 // compare if(tg(3pi/8)>|x|)
    d = sfadd(x, d)                            // change denuminator to x+1
    ONE = d                                    // d is 1.0 float const. Packages are paralel in execution
}
{
    c = ##pi_4                                 // free element c is pi/4 float constant
    n = sfsub(x, ONE)                          // numinator is x - 1
    if(p1) jump .L_divide                      // p1 if (tg(3pi/8)>|x|) then jump to .L_devide
}
{
    c = ##pi_2                                 // free element c is pi/2 float const
    n = setbit(ONE, #31)                       // negate ONE to -1.0. Why not use sfmake(#384):neg?
    d = x                                      // denuminator is |x|
}
    .falign
.L_divide:                                     // polynom aprximation of atan((n/d)) + c
{
    memd(sp+#0) = r17:16                       // store sign and free element to r17:16 !!!
    r17 = sign                                 // save sign in r17
    r16 = c                                    // save c free element in r 16
    call qhmath_div_f                      // call hexagon div
}
{
    sign = r17                                 // load sign from r17. Why is not called memd first?
    c = r16                                    // loar free element c from r16
    r17:16 = memd(sp+#0)                       // load r17:r16
    z2 = sfmpy(z, z)                           // z^2 = z * z
}
{
    AT4 = ##at4                                // Load AT4 in r10. See #define for value
    AT3 = ##at3                                // Load AT3 in r9. See #define for value
}
{
    AT3 += sfmpy(AT4, z2)                      // r9 <- AT4 *z^2 + AT3
    AT2 = ##at2                                // Load AT2 in r10. See #define for value
}
{
    AT2 += sfmpy(AT3, z2)                      // r10 <- (AT4*z^2 + AT3)*z^2 + AT2
    AT1 = ##at1                                // Load AT1 in r9. See #define for value.
}
{
    AT1 += sfmpy(AT2, z2)                      // r9 <- ((AT4*z^2 + AT3)*z^2 + AT2)*z^2 + AT1
    AT0 = sfmake(#384):pos                     // r10 <- 1.0 float const
}
{
    AT0 += sfmpy(AT1, z2)                      // r10 <- 1.0 + (((AT4*z^2 + AT3)*z^2 + AT2)*z^2 + AT1)*z^2
}
{
    c += sfmpy(z, AT0)                         // add free element to aprximation after mul with z
}
{
    r0 = c                                     // adjust sign of y. If (y<0) {y = -y} r0 = xor(c, sign)
    p0 = tstbit(sign,#1)                       // extract only one bit
    if(!p0.new) jump:t .L_skip                 // if sign of X skip correction of phase
}
{
    PI = ##pi                                  // PI constant see #define for value
    PI_LO = ##pi_lo                            // PI_LO constant see #define for value
}
{
    r0 = sfsub(r0,PI_LO)                       // r0 <- r0 - PI_LO
}
{
    r0 = sfsub(PI,r0)                          // r0 <- PI - -r0 - PI_LO
}
    .falign
.L_skip:                                       // Label for skipping phase correction step
{
    signYs = asl(sign,#31)                     // Push last bit of sign to be correction
}
{
    r0 = xor(r0,signYs)                        // Correct the sign of return
    dealloc_return                             // return( result)
}
    .falign
.L_zero:                                       // Label for (+-zero,+-zero) input
{
    p0 = tstbit(signX,#0)                      // If signX = 0 assign r0 to 0.0
    if(!p0.new) r0 = #0                        // If signX = 1 assign r0 to PI
    signY = asl(signY,#31)                     // prepare sign for correction with asl
}
{
    if(p0) r0 = ##pi                           // assign PI value
}
{
    r0 = xor(r0,signY)                         // corect the sign
    dealloc_return                             // return (result)
}
    .falign
.L_inf:                                        // Label for (+-INF,+-INF) input
{
    p0 = tstbit(signX,#0)                      // If signX = 0 assign r0 to 0.0
    if(!p0.new) r0 = ##pi_4                    // If signX = 1 assign r0 to PI
    signY = asl(signY,#31)                     // prepare sign for correction with asl
}
{
    if(p0) r0 = ##pi_3_4                       // assign 3PI/4
}
{
    r0 = xor(r0,signY)                         // corect the sign
    dealloc_return                             // return(result)
}
    .falign
.L_nan:
{
    r0 = sfadd(x_arg,y_arg)
    jumpr r31
}
    .size qhmath_atan2_f, .-qhmath_atan2_f
