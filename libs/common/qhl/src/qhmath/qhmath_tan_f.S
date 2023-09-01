/**=============================================================================
@file
    qhmath_tan_f.S

@brief
    Computes the tangent of single-precision floating-point argument.

    Function prototype

        float qhmath_tan_f.S

    Reference C code

        float qhmath_tan_f(float thetai)
        {
            int sign, oct;
            float nf, x, x2, y, theta;
            double nd, thetad;

            sign = 0;
            theta = thetai;
            if(theta < 0.0)
            {
                theta = -thetai;
                sign = 1;
            }

            // determine which interval the theta is in
            if(theta > MTHETA)
            {
                thetad = theta;
                nd = thetad * FOPI;                     //how many pi/4's make up the angle
                oct = (int) nd;
                nd = oct;
                oct = oct & 3;

                if(oct & 1)
                {
                    nd = nd + 1.0;                      //remove extra pi/4 to reflect cos or sin
                }
                thetad = thetad - nd*DPIO40;
                thetad = thetad - nd*DPIO41;
                theta = thetad;
            }
            else
            {
                nf = theta * FOPI;                      //how many pi/4's make up the angle
                oct = (int) nf;
                nf = oct;
                oct = oct & 3;
                if(oct & 1)
                {
                    nf = nf + 1.0;                      //remove extra pi/4 to reflect cos or sin
                }
                if(nf > 0.0)
                {
                    theta = theta - nf*PIO40;
                    theta = theta - nf*PIO41;
                    theta = theta - nf*PIO42;
                }
            }

            if(oct & 2) sign = sign ^ 1;
            x = theta;  //make +ve due to reflection
            if(x < 0.0) x = -x;
            x2 = x * x;

            y = T5 + T6 * x2;
            y = T4 + y * x2;
            y = T3 + y * x2;
            y = T2 + y * x2;
            y = T1 + y * x2;
            y = T0 + y * x2;
            y = y * x;

            if(oct == 1 || oct == 2) y = 1.0 / y;
            if(sign) y = -y;
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_tan_f
    .type qhmath_tan_f, @function

/*============================================================================*/

#define   c6       0x3C19C53B
#define   c5       0x3B4C779C
#define   c4       0x3CC821B5
#define   c3       0x3D5AC5C9
#define   c2       0x3E0896DD
#define   c1       0x3EAAAA6F
#define   c0       0x3F800000
#define   pi_4_0   0x3F490000
#define   pi_4_1   0x397DA000
#define   pi_4_2   0x33222169
#define   maxtheta 0x48000000
#define   invpio4  0x3FA2F983
#define   signx    r1:0
#define   x        r0
#define   sign     r1
#define   MTHETA   r15
#define   theta    r0
#define   x_       r13
#define   FOPI     r2
#define   nf       r3
#define   ONE      r4
#define   ione     r14
#define   PIO40    r6
#define   PIO41    r6
#define   PIO42    r6
#define   oct      r9
#define   x2       r10
#define   T6       r11
#define   T5       r12
#define   T4       r11
#define   T3       r12
#define   T2       r11
#define   T1       r12
#define   T0       r11

/*============================================================================*/

qhmath_tan_f:
{
    p0 = sfclass(r0,#24)
    if (p0.new) jump:nt .L_inf
}
{
    allocframe(#32)
    signx = bitsplit(x, #31)                   //x = abs x  if x < 0 sign = 1 else 0
    MTHETA = ##maxtheta
}
{
    p1 = sfcmp.gt(x, MTHETA)
    if(p1.new) jump:nt .L_dp_rr                //if x > MTHETA do double prec range reduction
    FOPI = ##invpio4
}
{
    nf = sfmpy(x, FOPI)
    ONE = sfmake(#384):pos                     //1.0
}
{
    oct = convert_sf2w(nf):chop
    ione = #1
}
{
    p1 = tstbit(oct, #0)
    nf = convert_w2sf(oct)
    if(!p1.new) ONE = #0
}
{
    nf = sfadd(nf, ONE)
    PIO40 = ##pi_4_0
    oct = and(oct, #3)
}
{
    x -= sfmpy(nf, PIO40)
    PIO41 = ##pi_4_1
    p0 = tstbit(oct, #1)
}
{
    x -= sfmpy(nf, PIO41)
    PIO42 = ##pi_4_2
    if(p0) sign = xor(sign, ione)
}
{
    x -= sfmpy(nf, PIO42)
    T6 = ##c6
}
    .falign
.L_poly:
{
    x_ = clrbit(x, #31)                        //fabs(x)
    x2 = sfmpy(x, x)                           // x * x
    T5 = ##c5
}
{
    T5 += sfmpy(T6, x2)
    T4 = ##c4
}
{
    T4 += sfmpy(T5, x2)
    T3 = ##c3
    p0 = cmp.eq(oct, #1)
}
{
    T3 += sfmpy(T4, x2)
    T2 = ##c2
    p1 = cmp.eq(oct, #2)
}
{
    T2 += sfmpy(T3, x2)
    T1 = ##c1
    p1 = or(p0, p1)
}
{
    T1 += sfmpy(T2, x2)
    T0 = ##c0                                  //==1
    sign = asl(sign, #31)
}
{
    T0 = sfmpy(T1, x2)
    r15 = sign                                 //prevent sign from being clobbered
    r0 = x_
}
{
    r0 += sfmpy(T0, x_)
    if(!p1) jump .L_ret
}
{
    r1 = r0
    r0 = sfmake(#384):pos                      //1.0
    call qhmath_div_f                          //1/y
}
    .falign
.L_ret:
{
    r0 = xor(r0, r15)                          //use 1/y
    dealloc_return                             //return( y);
}

/*============================================================================*/

#define  fopidH   0x3ff45f30
#define  fopidL   0x6dc9c883
#define  dpi04_0H 0xbfe921fb
#define  dpi04_0L 0x40000000
#define  dpi04_1H 0xbe64442d
#define  dpi04_1L 0x18469899
#define  thetad   r21:20
#define  FOPID    r3:2
#define  FOPIDH   r3
#define  FOPIDL   r2
#define  octd     r17
#define  nd       r23:22
#define  one      r18
#define  signd    r16
#define  DPIO40   r3:2
#define  DPIO40H  r3
#define  DPIO40L  r2
#define  DPIO41   r3:2
#define  DPIO41H  r3
#define  DPIO41L  r2

/* ================================================================= */

    .falign
.L_dp_rr:
{
    memd(sp+#0) = r17:16
    memd(sp+#16) = r21:20
    thetad = convert_sf2df(theta)
    signd = sign
}
{
    memd(sp+#8) = r19:18
    memd(sp+#24) = r23:22
    FOPIDH = ##fopidH
}
{
    FOPIDL = ##fopidL
    r1:0 = thetad
    call __hexagon_muldf3                      //FOPI4 * thetad
}
{
    octd = convert_df2w(r1:0):chop
}
{
    p3 = tstbit(octd, #0)
    if(p3.new) octd = add(octd, #1)
}
{
    nd = convert_w2df(octd)
    if(p3) octd = add(octd, #-1)
    DPIO40H = ##dpi04_0H
}
{
    r1:0 = nd
    DPIO40L = ##dpi04_0L
    call __hexagon_muldf3                      //nd * DPIO0
}
{
    r3:2 = thetad
    call __hexagon_adddf3                      //thetad = thetad + -DPIO40*nd
}
{
    thetad = r1:0
    DPIO41H = ##dpi04_1H
}
{
    DPIO41L = ##dpi04_1L
    r1:0 = nd
    call __hexagon_muldf3
}
{
    r3:2 = thetad
    call __hexagon_adddf3                      //thetad = thetad + -DPIO41*nd
    one = #1
}
{
    theta = convert_df2sf(r1:0)
    oct = and(octd, #3)
    p3 = tstbit(octd, #1)
    if(p3.new) signd = xor(signd, one)
}
{
    sign = signd
    r17:16 = memd(sp+#0)
    r19:18 = memd(sp+#8)
}
{
    T6 = ##c6
    r21:20 = memd(sp+#16)
    r23:22 = memd(sp+#24)
    jump  .L_poly
}
    .falign
.L_inf:
{
    r0 = sfsub(r0,r0)
    jumpr r31
}
    .size qhmath_tan_f, .-qhmath_tan_f
