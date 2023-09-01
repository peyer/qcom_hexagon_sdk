/**=============================================================================
@file
    qhmath_log_f.S

@brief
    Computes the natural (base-e) logarithm of single-precision floating-point argument.

    Function prototype

        float qhmath_log_f(float x);

   Reference C code

        float qhmath_log_f( float x)
        {
            int e;
            float fe, z, y;

            if(x ==0) return(-inf);
            if(x < 0) return(nan);

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
            y += x;          // + x

            //combine in the log of the exponent
            //cody and white extended precision modification, decomdpose lon(2) into 2 parts
            // log x ~ (log f -0.000212*k) + 0.6933*k (log(m.2^e) = log(m) + e*log(2))

            fe = (float) e;
            y += LOG21 * fe  ;
            y += LOG20 * fe; //+loge(2) * e
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_log_f
    .type qhmath_log_f, @function

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
#define   log20    0x3F318000
#define   log21    0xB95E8083
#define   mnnum    0x00200000
#define   mnlog    0xC2B17218
#define   mnan     0x7FC00000
#define   minf     0xFF800000
#define   inf      0x7F800000
#define   x        r0
#define   x2       r1
#define   MINNUM   r2
#define   x3       r14
#define   z        r0
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
#define   LOG21    r12
#define   LOG20    r13
#define   MINLOG   r15
#define   xx       r10
#define   INF      r14

/*============================================================================*/

qhmath_log_f:
{
    INF = ##inf
    MINNUM = ##mnnum
}
{
    e = lsr(x, #23)
    twox = #0
    p0 = sfcmp.eq(x,INF)
    if(p0.new) jumpr:nt r31
}
{
    xx = x
    e = add(e, #-126)
    p0 = sfcmp.gt(x, MINNUM)                   //x <= minnum   !(x > minnum)
    p2 = sfcmp.gt(x, twox)                     //x <= 0
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
    if(!p2) jump .L_trap
}
{
    L8 += sfmpy(L9, x)
    L7 = ##c7
    x3 = sfmpy(x2, x)
}
{
    L7 += sfmpy(L8, x)
    L6 = ##c6
    MINLOG.L = #0x7218
}
{
    L6 += sfmpy(L7, x)
    L5 = ##c5
    MINLOG.H = #0xC2B1
}
{
    L5 += sfmpy(L6, x)
    L4 = ##c4
    if(!p0) r0 = MINLOG
#if (__HEXAGON_ARCH__ == 61)
}
{
    L4 += sfmpy(L5, x)
    L3 = ##c3
}
{
    if(!p0) jumpr r31
}
{
#else
}
{
    L4 += sfmpy(L5, x)
    L3 = ##c3
    if(!p0) jumpr r31
}
{
#endif
    L3 += sfmpy(L4, x)
    L2 = ##c2
}
{
    L2 += sfmpy(L3, x)
    L1 = ##c1
}
{
    L1 += sfmpy(L2, x)
    L0 = ##c0
}
{
    L1 = sfmpy(L1, x3)
}
{
    L1 += sfmpy(L0, x2)
}
{
    x = sfadd(L1, x)
    LOG21 = ##log21
}
{
    x += sfmpy(e, LOG21)
    LOG20 = ##log20
#if (__HEXAGON_ARCH__ == 61)
}
{
    x += sfmpy(e, LOG20)
}
{
    jumpr r31                                  //return( y);
}
#else
}
{
    x += sfmpy(e, LOG20)
    jumpr r31                                  //return( y);
}
#endif
    .falign
.L_trap:
{
    allocframe (#8)
}
{
    memw(sp+#0) = xx
    call ##_Geterrno
}
{
    memw(r0) = #0x21 // EDOM
    xx = memw(sp+#0)
}
{
    deallocframe
    r1 = #0
    r0 = ##minf
}
#if (__HEXAGON_ARCH__ == 61)
{
    p0 = sfcmp.eq(xx, r1)
}
{
    if(p0) jumpr r31
}
#else
{
    p0 = sfcmp.eq(xx, r1)
    if(p0.new) jumpr:t r31
}
#endif
{
    r0 = sfsub(xx,xx)
    r1 = sfsub(xx,xx)
    jump qhmath_div_f
}

    .size qhmath_log_f, .-qhmath_log_f
