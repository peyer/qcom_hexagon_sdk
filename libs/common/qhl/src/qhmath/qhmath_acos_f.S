/**=============================================================================
@file
    qhmath_acos_f.S

@brief
    Computes the inverse cosine of single-precision floating-point argument.

    Function prototype

        float qhmath_acos_f(float x);

    Reference C code

        float qhmath_acos_f(float x)
        {
            int range;
            float z, x2, y;

            z = x;
            if(x < 0.0)
            {
                x = -x;
                range = 1;
            }
            else
                range  = 0;

            if(x > 1.0)
                return(0.0);

            if(x > 0.5)
            {
                x = 0.5 - 0.5*x;
                x = sqrtf(x);
            }
            else
            {
                x = z;
                range = 2;
            }

            x2 = x * x;

            y = AS4 + AS5 * x2;
            y = AS3 + y * x2;
            y = AS2 + y * x2;
            y = AS1 + y * x2;
            y = AS0 + y * x2;
            y = y * x;

            if(range ==  1) y = 2.0 * PIO2 - 2.0 * y;
            if(range ==  2) y = PIO2 - y;
            if(range ==  0) y = 2.0 * y;

            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_acos_f
    .type qhmath_acos_f, @function

/*============================================================================*/

#define   as5      0x3D2CB352
#define   as4      0x3CC617E3
#define   as3      0x3D3A3EC7
#define   as2      0x3D9980F6
#define   as1      0x3E2AAAE4
#define   pi_2     0x3FC90FDB
#define   nan      0x7FC00000
#define   rangex   r1:0
#define   x        r0
#define   range    r1
#define   HALF     r2
#define   ONE      r3
#define   z        r6
#define   PIO2     r7
#define   x2       r8
#define   AS5      r9
#define   AS4      r10
#define   AS3      r9
#define   AS2      r10
#define   AS1      r9

/*============================================================================*/

qhmath_acos_f:
{
    allocframe(#8)
    rangex = bitsplit(x, #31)                  //x = abs x  if x < 0 range = 1 else range 0
    z = x
    ONE = sfmake(#384):pos                     //1.0
}
{
    p0 = sfcmp.gt(x, ONE)
    if(p0.new) jump:nt .L_trap
    HALF = sfmake(#320):pos
}
{
    p0 = sfcmp.gt(x, HALF)
    if(p0.new) jump:nt .L_adjust
    if(!p0.new) x = z
    if(!p0.new) range = #2
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
}
{
    AS1 = sfmpy(AS1, x2)
    p0 = cmp.eq(range, #0)
}
{
    x += sfmpy(x, AS1)
    if(p0) jump .L_range0
    p1 = cmp.eq(range, #2)
}
{
    x = sfsub(PIO2,x)
    if(p1) dealloc_return                      //return( y)
}
    .falign
.L_range0:
{
    x = sfadd(x, x)
    dealloc_return                             //return( y)
}
    .falign
.L_adjust:
{
    memd(sp+#0) = r17:16
    x = sfsub(ONE, x)
}
{
    x = sfmpy(x, HALF)
    r17 = range
    call qhmath_sqrt_f
}
{
    r17:16 = memd(sp+#0)
    range = r17
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
    .size qhmath_acos_f, .-qhmath_acos_f
