/**=============================================================================
@file
    qhmath_atan_f.S

@brief
    Computes the inverse tangent of single-precision floating-point argument.

    Function prototype

        float qhmath_atan_f(float x)

    Reference C code

        float qhmath_atan_f(float x)
        {
            int sign;
            float z2, y, z, c, n, d;

            sign = 0;
            if(x < 0.0) { sign = 1; x = -x; }
            // determine which interval the input is in
            n = x;
            d = 1.0;
            c = 0.0;
            if(x > tan3pio8) // tan(3pi/8)
            {
                c = PIO2;
                n = 1.0;
                d = x;
            }
            else if (x > tanpio8) // tan(pi/8)
            {
                c = PIO4;
                n = x - 1.0;
                d = x + 1.0;
            }
            z = n/d;

            z2 = z * z;
            y = AT3 + AT4 * z2;
            y = AT2 + y * z2;
            y = AT1 + y * z2;
            y = AT0 + y * z2;
            y = y * z + c;

            if(sign) y = - y;
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_atan_f
    .type qhmath_atan_f, @function

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

/*============================================================================*/

qhmath_atan_f:
{
    allocframe(#16)
    TWO = sfmake(#448):pos
    TANPIO8 = ##tanpi_8
}
{
    signx = bitsplit(xx, #31)          //x = abs x  if x < 0 range = 1 else range 0
    TAN3PIO8 = sfadd(TWO, TANPIO8)
    c = #0
    d.L = #0x0000
}
{
    d.H = #0x3f80                      // = sfmake(#384):pos
    n = x
    p0 = sfcmp.gt(TANPIO8, x)
    if(p0.new) jump:t .L_divide
}
{
    p1 = sfcmp.gt(TAN3PIO8, x)
    d = sfadd(x, d)
    ONE = d
}
{
    c = ##pi_4
    n = sfsub(x, ONE)
    if(p1) jump .L_divide
}
{
    c = ##pi_2
    n = setbit(ONE, #31)               //-1.0
    d = x
}
    .falign
.L_divide:
{
    memd(sp+#0) = r17:16
    r17 = sign
    r16 = c
    call qhmath_div_f
}
{
    sign = r17
    c = r16
    r17:16 = memd(sp+#0)
    z2 = sfmpy(z, z)                   // z * z
}
{
    AT4 = ##at4
    AT3 = ##at3
}
{
    AT3 += sfmpy(AT4, z2)
    AT2 = ##at2
}
{
    AT2 += sfmpy(AT3, z2)
    AT1 = ##at1
}
{
    AT1 += sfmpy(AT2, z2)
    AT0 = sfmake(#384):pos
}
{
    AT0 += sfmpy(AT1, z2)
}
{
    c += sfmpy(z, AT0)
    sign = asl(sign, #31)
}
{
    r0 = xor(c, sign)
    dealloc_return                     //return( y)
}
    .size qhmath_atan_f, .-qhmath_atan_f
