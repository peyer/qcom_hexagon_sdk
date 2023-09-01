/**=============================================================================
@file
    qhmath_log2_f.S

@brief
    Computes the base-2 logarithm of single-precision floating-point argument.

    Function prototype

        float qhmath_log2_f(float x);

    Reference C code

        float qhmath_log2_f( float x)
        {
            int e;
            float fe, z, y;

            if(x <=0) return(-170.0);

            x = frexp(x, &e);  //extract exponent leave mantissa bare

            if( x < SQRTH)
            {
                e = e-1;     //adjust if < 0.707
                x = 2.0*x;
            }
            x = x - 1.0;  //adjust for log(1+x) approximation
            z = x * x;

            y = L8 + L9 * x;
            y = L7 + y * x;
            y = L6 + y * x;
            y = L5 + y * x;
            y = L4 + y * x;
            y = L3 + y * x;
            y = L2 + y * x;
            y = L1 + y * x;

            y = y * x * z;
            y += L0 * z;     // y - x^2/2

            //combine in the log of the exponent
            //cody and white extended precision modification, decomdpose into 2 parts
            //to bring sensitive calculation correctly to answer
            z = y * LOG2E1;          //log2(x) = loge x / loge2 = loge x * log2 e
            z += x * LOG2E1;
            z += y  ;
            z += x  ;
            fe = (float) e;
            z += fe;

            return(z);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_log2_f
    .type qhmath_log2_f, @function

/*============================================================================*/

#define   c9       0x3D9021BB
#define   c8       0xBDEBD1B8
#define   c7       0x3DEF251A
#define   c6       0xBDFE5D4F
#define   c5       0x3E11E9BF
#define   c4       0xBE2AAE50
#define   c3       0x3E4CCEAC
#define   c2       0xBE7FFFFC
#define   c1       0x3EAAAAAA
#define   c0       0xBF000000
#define   sqrt0p5  0x3F3504F3
#define   log2e1   0x3EE2A8ED
#define   mnnum    0x00200000
#define   mnlog2   0xC3000000
#define   minf     0xff800000
#define   mnan     0x7fc00000
#define   inf      0x7F800000
#define   xx       r0
#define   xxx      r13
#define   x        r11
#define   x2       r1
#define   MINNUM   r2
#define   x3       r14
#define   z        r0
#define   y        r10
#define   SQRTH    r5
#define   e        r3
#define   ONE      r4
#define   twox     r4
#define   L9       r4
#define   L8       r5
#define   L7       r4
#define   L6       r5
#define   L5       r6
#define   L4       r7
#define   L3       r6
#define   L2       r7
#define   L1       r8
#define   L0       r9
#define   LOG2E1   r12
#define   MINLOG2  r15
#define   zero     r12
#define   INF      r14

/*============================================================================*/

qhmath_log2_f:
{
    INF = ##inf
    xxx = r0
}
{
    p0 = sfcmp.eq(xxx,INF)
    if(p0.new) jumpr:nt r31
}
{
    e = lsr(xx, #23)
    MINNUM = ##mnnum
    x = xx
}
{
    e = add(e, #-126)
    twox = #0
    p0 = sfcmp.gt(x, MINNUM)                   //x <= minnum   !(x > minnum)
}
{
    SQRTH = ##sqrt0p5
    x -= asl(e, #23)
    twox = setbit(twox, #23)
}
{
    p1 = sfcmp.gt(SQRTH, x)
    if(p1.new) x = add(x, twox)
    if(p1.new) e = add(e, #-1)
    ONE = sfmake(#384):pos                     //1.0
}
{
    x = sfsub(x, ONE)                          //x = x-1
    L9 = ##c9
    e = convert_w2sf(e)
}
{
    L8 = ##c8
    x2 = sfmpy(x, x)
    zero = #0
}
{
    L8 += sfmpy(L9, x)
    L7 = ##c7
    x3 = sfmpy(x2, x)
}
{
    L7 += sfmpy(L8, x)
    L6 = ##c6
    MINLOG2.L = #0x0000
}
{
    L6 += sfmpy(L7, x)
    L5 = ##c5
    MINLOG2.H = #0xC300
}
{
    L5 += sfmpy(L6, x)
    L4 = ##c4
    p2 = sfcmp.gt(xx, zero)
}
{
    L4 += sfmpy(L5, x)
    L3 = ##c3
    if(!p2) jump .L_trap
}
{
    L3 += sfmpy(L4, x)
    L2 = ##c2
    if(!p0) r0 = MINLOG2
#if (__HEXAGON_ARCH__ == 61)
}
{
    L2 += sfmpy(L3, x)
    L1 = ##c1
}
{
    if(!p0) jumpr r31
}
{
#else
}
{
    L2 += sfmpy(L3, x)
    L1 = ##c1
    if(!p0) jumpr r31
}
{
#endif
    L1 += sfmpy(L2, x)
    L0 = ##c0
}
{
    L1 = sfmpy(L1, x3)
}
{
    L1 += sfmpy(L0, x2)
    LOG2E1 = ##log2e1
}
{
    z = sfmpy(L1, LOG2E1)
}
{
    z += sfmpy(x, LOG2E1)
}
{
    z = sfadd(z, L1)
}
{
    z = sfadd(z, x)
#if (__HEXAGON_ARCH__ == 61)
}
{
    z = sfadd(z, e)
}
{
    jumpr r31                                  //return( y);
}
#else
}
{
    z = sfadd(z, e)
    jumpr r31                                  //return( y);
}
#endif
    .falign
.L_trap:
{
    allocframe (#8)
}
{
    memw(sp+#0) = xxx
    call ##_Geterrno
}
{
    memw(r0) = #0x21 // EDOM
    xxx = memw(sp+#0)
}
{
    deallocframe
    zero = #0
}
#if (__HEXAGON_ARCH__ == 61)
{
    p2 = sfcmp.eq(xxx, zero)
    r0 = ##minf
}
{
    if(p2) jumpr r31
}
#else
{
    p2 = sfcmp.eq(xxx, zero)
    r0 = ##minf
    if(p2.new) jumpr:t r31
}
#endif
{
    r0 = ##mnan
    jumpr r31
}
    .size qhmath_log2_f, .-qhmath_log2_f
