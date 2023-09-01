/**=============================================================================
@file
    qhmath_exp10_f.S

@brief
    Computes the 10.0 raised to the given argument.

    Function prototype
        
        float qhmath_exp10_f(float x)

    Reference C code
        
        float qhmath_exp10_f(float x)
        {
            float f, y ;
            int k;

            if(x > MAXLOG10) return(MAXNUM);
            if(x < MINLOG10) return(0.0);

            k = Q6_R_convert_sf2w_R(LOG210 * x); //f = floorf( LOG210 * x + 0.5);
            f = Q6_R_convert_w2sf_R(k);          //k = (int)f;
            x = x - f*LOG1020;
            x = x - f*LOG1021;

            y = P4 + P5 * x;
            y = P3 + y * x;
            y = P2 + y * x;
            y = P1 + y * x;
            y = P0 + y * x;
            y = 1.0 + y * x;

            //insert exponents
            y = ldexpf(y, k);
            return(y);
        }

@note
   Function is not defined in C99 standard.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_exp10_f
    .type qhmath_exp10_f, @function

/*============================================================================*/

#define  xx         r0
#define  x          r1
#define  z          r2
#define  MAXLOG10   r3
#define  MINLOG10   r13
#define  MAXNUM     r4
#define  LOG210     r5
#define  LOG1020    r6
#define  LOG1021    r7
#define  INFINITY   r1
#define  NAN        r2
#define  inf_val    0x7F800000
#define  nan_val    0x7FC00000
#define  f          r8
#define  k          r9
#define  ONE        r10
#define  P5         r11
#define  P4         r12
#define  P3         r11
#define  P2         r12
#define  P1         r11
#define  P0         r12
#define  pt5        0x3E5345FD
#define  pt4        0x3F0AC229
#define  pt3        0x3F95ECEB
#define  pt2        0x400237B4
#define  pt1        0x4029A925
#define  pt0        0x40135D8E
#define  log1020    0x3E9A0000
#define  log1021    0x39826A14
#define  log210     0x40549A78
#define  mxlog10    0x4218EC59
#define  mxnum      0x3FD9C7DC

/*============================================================================*/

qhmath_exp10_f:
{
    INFINITY = ##inf_val
    NAN = ##nan_val
}
{
    p0 = cmp.eq(r0,INFINITY)
    if(p0.new) jump:nt .L_inf
    p1 = cmp.eq(r0,NAN)
    if(p1.new) jump:nt .L_inf
}
{
    LOG210 = ##log210
    MAXLOG10.L = #0xec59               //mxlog10
    x = xx
}
{
    MAXLOG10.H = #0x4218               //mxlog10
    k = sfmpy(LOG210, x)
    MAXNUM = ##mxnum
}
{
    MINLOG10 = setbit(MAXLOG10, #31)
    k = convert_sf2w(k)
    LOG1020 = ##log1020
}
{
    f = convert_w2sf(k)
    p1 = sfcmp.gt(xx, MINLOG10)        //if x <= MINLOG10
    LOG1021 = ##log1021
}
{
    x -= sfmpy(f, LOG1020)             //x = x-f*log1020
    P5 = ##pt5
    if(!p1) r0 = #0
#if (__HEXAGON_ARCH__ == 61)
}
{
    x -= sfmpy(f, LOG1021)             //x = x-f*log1021
    P4 = ##pt4
}
{
    if(!p1) jumpr r31                  //if x < MINLOG10
}
{
#else
}
{
    x -= sfmpy(f, LOG1021)             //x = x-f*log1021
    P4 = ##pt4
    if(!p1) jumpr r31                  //if x < MINLOG10
}
{
#endif
    P4 += sfmpy(P5, x)
    P3 = ##pt3
}
{
    P3 += sfmpy(P4, x)
    P2 = ##pt2
    p2 = sfcmp.gt(xx, MAXLOG10)
}
{
    P2 += sfmpy(P3, x)
    P1 = ##pt1
    if(p2) r0 = MAXNUM
#if (__HEXAGON_ARCH__ == 61)
}
{
    P1 += sfmpy(P2, x)
    P0 = ##pt0
}
{
    if(p2) jumpr r31                   //if x > MAXLOG
}
{
#else
}
{
    P1 += sfmpy(P2, x)
    P0 = ##pt0
    if(p2) jumpr r31                   //if x > MAXLOG
}
{
#endif
    P0 += sfmpy(P1, x)
    ONE = sfmake(#384):pos             //1.0
}
{
    ONE += sfmpy(x, P0)
    k = asl(k, #23)
}
{
    r0 = add(k, ONE)
    jumpr r31
}
    .falign
.L_inf:
{
    r0 = sfadd(r0,r0)
    jumpr r31
}
    .size qhmath_exp10_f, .-qhmath_exp10_f
