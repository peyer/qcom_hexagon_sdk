/**=============================================================================
@file
    qhmath_exp2_f.S

@brief
    Computes the 2.0 raised to the given argument.

    Function prototype
        
        float qhmath_exp2_f(float x)

   Reference C code

        float qhmath_exp2_f(float x)
        {
            float f, y, y0, y1, x2 ;
            int k;
            if(x > MAXLOG2) return(MAXNUM);
            if(x < -MINLOG2) return(0.0);
            k = Q6_R_convert_sf2w_R(x); //f = floorf(x + 0.5);
            f = Q6_R_convert_w2sf_R(k); //k = (int)f;
            x = x - f;
            if(x > 0.5)
            {
                k += 1;
                x -= 1.0;
            }
            y = Q4 + Q5 * x;
            y = Q3 + y  * x;
            y = Q2 + y  * x;
            y = Q1 + y  * x;
            y = Q0 + y  * x;
            y = 1.0 + y*x;

            y = ldexpf(y, k);
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_exp2_f
    .type qhmath_exp2_f, @function

/*============================================================================*/

#define   x         r0
#define   z         r12
#define   MAXLOG2   r1
#define   MAXNUM    r2
#define   MINLOG2   r3
#define   f         r4
#define   k         r5
#define   OP5       r13
#define   ONE       r14
#define   Q5        r6
#define   Q4        r7
#define   Q3        r8
#define   Q2        r9
#define   Q1        r10
#define   Q0        r11
#define   INFINITY  r1
#define   NAN       r2
#define   inf_val   0x7F800000
#define   nan_val   0x7FC00000
#define   c5        0x3920FDDE
#define   c4        0x3AAF9F29
#define   c3        0x3C1D96A6
#define   c2        0x3D635774
#define   c1        0x3E75FDEE
#define   c0        0x3F317218
#define   mnum      0x3FD9C7DC
#define   mln2      0x42FE0000

/*============================================================================*/

qhmath_exp2_f:
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
    k = convert_sf2w(x)
    OP5 = sfmake(#320):pos             //0.5
}
{
    f = convert_w2sf(k)
    ONE = sfmake(#384):pos             //1.0
    MAXLOG2 = ##mln2
}
{
    z = sfsub(x, f)                    //x = x-f
    MINLOG2 = sfmake(#831):neg
    Q5 = ##c5
}
{
    p0 = sfcmp.gt(z, OP5)
    if(!p0.new) ONE = #0
    MAXNUM = ##mnum
}
{
    Q4 = ##c4
    z = sfsub(z, ONE)
    ONE = sfmake(#384):pos             //1.0
}
{
    Q4 += sfmpy(Q5, z)
    Q3 = ##c3
    p1 = sfcmp.gt(MINLOG2, x)          //if x < -MAXLOG2
}
{
    Q3 += sfmpy(Q4, z)
    Q2 = ##c2
    if(p1) r0 = #0
}
{
    Q2 += sfmpy(Q3, z)
    if(p0) k = add(k, #1)
    Q1 = ##c1
#if (__HEXAGON_ARCH__ == 61)
}
{
    Q1 += sfmpy(Q2, z)
    Q0 = ##c0
}
{
    if(p1) jumpr r31
}
{
#else
}
{
    Q1 += sfmpy(Q2, z)
    Q0 = ##c0
    if(p1) jumpr r31
}
{
#endif
    Q0 += sfmpy(Q1, z)
    p2 = sfcmp.gt(x, MAXLOG2)
}
{
    ONE += sfmpy(Q0, z)
    k = asl(k, #23)
    if(p2) r0 = MAXNUM
}
{
    if(!p2) r0 = add(k, ONE)
    jumpr r31                          //return( y);
}
    .falign
.L_inf:
{
    r0 = sfadd(r0,r0)
    jumpr r31
}
    .size qhmath_exp2_f, .-qhmath_exp2_f
