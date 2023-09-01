/**=============================================================================
 @file
    qhmath_tanh_f.S

 @brief
    Computes the hyperbolic tangent of single-precision floating-point argument.

    Function prototype

        float qhmath_tanh_f(float x)

    Reference C code

        float tanhf(float x)
        {
            union {float f; uint32_t i;} u = {.f = x};
            uint32_t w;
            int sign;
            float t;

            // x = |x|
            sign = u.i >> 31;
            u.i &= 0x7fffffff;
            x = u.f;
            w = u.i;

            if (w > 0x3f0c9f54) {
                // |x| > log(3)/2 ~= 0.5493 or nan
                if (w > 0x41200000) {
                    // |x| > 10
                    t = 1 + 0/x;
                } else {
                    t = expm1f(2*x);
                    t = 1 - 2/(t+2);
                }
            } else if (w > 0x3e82c578) {
                // |x| > log(5/3)/2 ~= 0.2554
                t = expm1f(2*x);
                t = t/(t+2);
            } else if (w >= 0x00800000) {
                // |x| >= 0x1p-126
                t = expm1f(-2*x);
                t = -t/(t+2);
            } else {
                // |x| is subnormal
                FORCE_EVAL(x*x);
                t = x;
            }
            return sign ? -t : t;
        }

 @note
    Reference C code obtained from musl library.

 Copyright (c) 2019 Qualcomm Technologies Incorporated.
 All Rights Reserved. Qualcomm Proprietary and Confidential.
 ============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_tanh_f
    .type qhmath_tanh_f, @function

/*============================================================================*/

#define   log_3_2  0x3F0C9F54                  // ln(3)/2 ~= 0.5493
#define   log5_3_2 0x3E82C578                  // ln(5/3)/2 ~= 0.2554
#define   min_lvl  0x00800000
#define   inf_val  0x7F800000
#define   mask     0x80000000
#define   TWO      r2                          // and TEN
#define   ONE      r3
#define   TEN      r5
#define   XX       r11:10
#define   INFINITY r9
#define   signX    r11
#define   x_arg    r0
#define   absX     r10
#define   LOG3_2   r6
#define   LOG5_3_2 r7
#define   MIN_LVL  r8
#define   T        r0
#define   MASK     r4
#define   T_2      r1
#define   NUM      r0
#define   DEM      r1

/*============================================================================*/

qhmath_tanh_f:
{
    allocframe(#8)
    XX = bitsplit(x_arg, #31)                  // split sign and |x| of second argument x
    LOG5_3_2 = ##log5_3_2                      // Load inf_val in r6. See #define for value
}
{
    MASK = ##mask
    MIN_LVL = ##min_lvl
}
{
    T = sfadd(absX,absX)
    TEN = sfmake(#592):pos                     // 10.0 NOT one sharing register. Later this register r2 will be ONE
    LOG3_2 = ##log_3_2
}
{
    p0 = sfcmp.ge(MIN_LVL, absX)               // |x| too small
    if(p0.new) jump:nt .L_low_limit
}
{
    p0 = sfcmp.gt(LOG5_3_2,absX)               // MIN< |x| < ln(5/3)/2
    p0 = sfcmp.ge(absX,MIN_LVL)                // MIN< |x| < ln(5/3)/2
    if(p0.new) T = or(T,MASK)
}
{
    p0 = sfcmp.gt(absX,TEN)                    // |X| > 10.0
    if(p0.new) jump:nt .L_high_limit
}
{
    memd(sp+#0) = r17:16                       // store sign and free element to r17:16 !!!
    r17:16 = combine(signX,absX)               // save signX in r17
    call expm1f
}
{
    r11:10 = combine(r17,r16)                  // load free element c from r16
    r17:16 = memd(sp+#0)
    TWO = sfmake(#448):pos
}
{
    LOG3_2 = ##log_3_2                         // load again for check
    T_2 = sfadd(T,TWO)                         // T_2 = T + 2.0
}
{
    p0 = sfcmp.gt(absX,LOG3_2)
    if(p0.new) NUM = TWO
}
{
    memd(sp+#0) = r17:16                       // store sign and free element to r17:16 !!!
    r17:16 = combine(signX,absX)               // save signX in r17
    call qhmath_div_f
}
{
    r11:10 = combine(r17,r16)                  // load sign from r17. Restore
    r17:16 = memd(sp+#0)
    ONE = sfmake(#384):pos                     // 1.0
}
{
    LOG5_3_2 = ##log5_3_2
    LOG3_2 = ##log_3_2
}
{
    DEM = ##0
    p1 = sfcmp.gt(absX,LOG3_2)
    signX = asl(signX,#31)
}
{
    p0 = sfcmp.gt(absX,LOG5_3_2)
    p0 = sfcmp.gt(LOG3_2,absX)
    if(p0.new) jump:nt .L_end
}
{
    if(p1) DEM = ONE
}
{
    x_arg = sfsub(DEM,x_arg)
}
    .falign                                    // Behind the real end
.L_end:
{
    r0 = xor(r0,signX)                         // corect the sign
    dealloc_return                             // Correct  return(x)
}
    .falign                                    // Behind the real end
.L_high_limit:                                 // Label for (+-zero,+-zero) input
{
    T = sfmake(#384):pos                       // r0 <- 1.0
    signX = asl(signX,#31)                     // prepare sign for correction with asl
}
{
    r0 = xor(r0,signX)                         // corect the sign
    dealloc_return                             // return (result)
}
    .falign
.L_low_limit:
{
    T = sfmpy(absX,absX)
    signX = asl(signX,#31)
}
{
    r0 = xor(r0,signX)                         // corect the sign
    dealloc_return                             // return (result)
}
    .size  qhmath_tanh_f, .-qhmath_tanh_f
