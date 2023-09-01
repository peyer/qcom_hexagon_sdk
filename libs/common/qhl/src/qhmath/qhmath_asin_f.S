/**=============================================================================
@file
    qhmath_asin_f.S

@brief
    Computes the inverse sine of single-precision floating-point argument.

    Function prototype

        float qhmath_asin_f(float x)

    Reference C code

        float qhmath_asin_f(float x)
        {
            int sign, range;
            float x2, y;

            sign = 0;
            if(x < 0.0)
            {
                x = -x;
                sign = 1;
            }

            if(x > 1.0) return(0.0);

            range = 0;

            if(x > 0.5)
            {
                range = 1;
                x = 0.5 - 0.5*x;
                x = sqrtf(x);
            }

            x2 = x * x;
            y = AS4 + AS5 * x2;
            y = AS3 + y * x2;
            y = AS2 + y * x2;
            y = AS1 + y * x2;
            y = AS0 + y * x2;
            y = y * x;

            if(range)
            {
                y = PIO2 - 2.0 * y;
            }
            if(sign) y = -y;
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_asin_f
    .type qhmath_asin_f, @function

/*============================================================================*/

#define  as5     0x3D2CB352
#define  as4     0x3CC617E3
#define  as3     0x3D3A3EC7
#define  as2     0x3D9980F6
#define  as1     0x3E2AAAE4
#define  pi_2    0x3FC90FDB
#define  nan     0x7FC00000
#define  signx   r1:0
#define  HALF    r2
#define  x       r0
#define  sign    r1
#define  ONE     r3
#define  TWO     r4
#define  range   r5
#define  PIO2    r9
#define  x2      r10
#define  AS5     r11
#define  AS4     r12
#define  AS3     r11
#define  AS2     r12
#define  AS1     r11

/*============================================================================*/

qhmath_asin_f:
{
    allocframe(#8)
    signx = bitsplit(x, #31)                   //x = abs x  if x < 0 sign = 1 else 0
    ONE = sfmake(#384):pos                     //1.0
}
{
    p0 = sfcmp.gt(x, ONE)
    range = #0
}
{
    if(p0) jump .L_trap
    HALF = sfmake(#320):pos
}
{
    p0 = sfcmp.gt(x, HALF)
    if(p0.new) jump:nt .L_adjust
}
    .falign
.L_poly:
{
    x2 = sfmpy(x, x)                           // x * x
    AS5 = ##as5
}
{
    AS4 = ##as4
    PIO2 = ##pi_2
}
{
    AS4 += sfmpy(AS5, x2)
    AS3 = ##as3
    sign = asl(sign, #31)
}
{
    AS3 += sfmpy(AS4, x2)
    AS2 = ##as2
}
{
    AS2 += sfmpy(AS3, x2)
    AS1 = ##as1
}
{
    AS1 += sfmpy(AS2, x2)
    TWO = sfmake(#448):pos
}
{
    AS1 = sfmpy(AS1, x2)
    p0 = cmp.eq(range, #1)
}
{
    x += sfmpy(x, AS1)
    if(!p0) jump .L_ret
}
{
    PIO2 -= sfmpy(x, TWO)
}
{
    x = xor(PIO2, sign)
    dealloc_return                             //return( y)
}
    .falign
.L_ret:
{
    x = xor(x, sign)
    dealloc_return                             //return( y)
}
    .falign
.L_adjust:
{
    memd(sp+#0) = r17:16
    HALF -= sfmpy(x, HALF)
}
{
    r17 = sign
    r0 = HALF
    call qhmath_sqrt_f
}
{
    r17:16 = memd(sp+#0)
    range = #1
    sign = r17
    jump .L_poly
}
    .falign
.L_trap:
    call ##_Geterrno
    memw(r0) = #0x21                           // EDOM
{
    r0 = ##nan
    dealloc_return
}
    .size qhmath_asin_f, .-qhmath_asin_f
