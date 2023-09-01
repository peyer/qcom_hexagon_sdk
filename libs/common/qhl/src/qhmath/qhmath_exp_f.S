/**=============================================================================
@file
    qhmath_exp_f.S

@brief
    Computes the e (Euler's number) raised to the given argument.

    Function prototype
        
        float qhmath_exp_f(float x)

    Reference C code
    
        float qhmath_exp_f(float x)
        {
            float f, z, y ;
            int k;

            if(x > MAXLOG) return(MAXNUM);
            if(x < MINLOG) return(0.0);

            k = Q6_R_convert_sf2w_R(LOG2E * x); //f = floorf( LOG2E * x + 0.5);
            f = Q6_R_convert_w2sf_R(k);         //k = (int)f;
            x = x - f*LOG20;
            x = x - f*LOG21;

            z = x * x;

            y = E4 + E5 * x;
            y = E3 + y * x;
            y = E2 + y * x;
            y = E1 + y * x;
            y = E0 + y * x;
            y = x + y * z;
            y = y + 1.0;

            //insert exponents
            y = ldexpf(y, k);
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_exp_f
    .type qhmath_exp_f, @function

/*============================================================================*/

#define  xx         r0
#define  x          r1
#define  z          r2
#define  MAXLOG     r3
#define  MINLOG     r13
#define  MAXNUM     r4
#define  LOG2E      r5
#define  LOG20      r6
#define  LOG21      r7
#define  f          r8
#define  k          r9
#define  ONE        r10
#define  E5         r11
#define  E4         r12
#define  E3         r11
#define  E2         r12
#define  E1         r11
#define  E0         r12
#define  INFINITY   r1
#define  NAN        r2
#define  inf_val    0x7F800000
#define  nan_val    0x7FC00000
#define  e5         0x39506967
#define  e4         0x3AB743CE
#define  e3         0x3C088908
#define  e2         0x3D2AA9C1
#define  e1         0x3E2AAAAA
#define  e0         0x3F000000
#define  log20      0x3F318000
#define  log21      0xB95E8083
#define  log2e      0x3FB8AA3B
#define  mxlog      0x42B00F34
#define  mnlog      0xC2B00F34
#define  mxnum      0x3FD9C7DC

/*============================================================================*/

qhmath_exp_f:
{
    INFINITY = ##inf_val
    NAN = ##nan_val
}
{
    p0 = cmp.eq(r0,INFINITY)
    if(p0.new) jump:nt .L_inf
    p1 = sfclass(r0,#16)
    if(p1.new) jump:nt .L_inf
}
{
    LOG2E = ##log2e
    MAXLOG = ##mxlog
}
{
    k = sfmpy(LOG2E, xx)
    MINLOG = setbit(MAXLOG, #31)
    MAXNUM = ##mxnum
}
{
    k = convert_sf2w(k)
    LOG20 = ##log20
    x = xx
}
{
    f = convert_w2sf(k)
    p1 = sfcmp.gt(xx, MINLOG)          //if x <= MINLOG
    LOG21 = ##log21
}
{
    x -= sfmpy(f, LOG20)               //x = x-f*log20
    E5 = ##e5
    if(!p1) r0 = #0
#if (__HEXAGON_ARCH__ == 61)
}
{
    x -= sfmpy(f, LOG21)               //x = x-f*log21
    E4 = ##e4
}
{
    if(!p1) jumpr r31                  //if x < MINLOG
}
{
#else
}
{
    x -= sfmpy(f, LOG21)               //x = x-f*log21
    E4 = ##e4
    if(!p1) jumpr r31                  //if x < MINLOG
}
{
#endif
    z = sfmpy(x, x)                    //x^2
    E4 += sfmpy(E5, x)
    E3 = ##e3
}
{
    E3 += sfmpy(E4, x)
    E2 = ##e2
    p2 = sfcmp.gt(xx, MAXLOG)
}
{
    E2 += sfmpy(E3, x)
    E1 = ##e1
    if(p2) r0 = MAXNUM
#if (__HEXAGON_ARCH__ == 61)
}
{
    E1 += sfmpy(E2, x)
    E0 = ##e0
}
{
    if(p2) jumpr r31                   //if x > MAXLOG
}
{
#else
}
{
    E1 += sfmpy(E2, x)
    E0 = ##e0
    if(p2) jumpr r31                   //if x > MAXLOG
}
{
#endif
    E0 += sfmpy(E1, x)
}
{
    x += sfmpy(z, E0)
    ONE = sfmake(#384):pos             //1.0
}
{
    x = sfadd(x, ONE)
    k = asl(k, #23)
}
{
    r0 = add(k, x)
    jumpr r31
}
    .falign
.L_inf:
{
    r0 = sfadd(r0,r0)
    jumpr r31
}
    .size qhmath_exp_f, .-qhmath_exp_f
