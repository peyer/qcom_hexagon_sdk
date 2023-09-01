/**=============================================================================
@file
    qhmath_sin_f.S

@brief
    Computes the sine of single-precision floating-point argument.

    Function prototype

        float qhmath_sin_f.S

    Reference C code

        float qhmath_sin_f(float thetai)
        {
            int sign, oct;
            float nf, x, x2, y, theta;
            double nd, thetad;

            sign = 0;
            theta = thetai;
            if(thetai < 0.0)
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
                if(oct & 1)
                {
                    nd = nd + 1.0;                      //remove extra pi/4 to reflect cos or sin
                }
                thetad = thetad - nd*DPIO40;
                thetad = thetad - nd*DPIO41;
                theta = thetad;
                if(oct & 4) sign = sign ^ 1;            //>pi so negate
                oct = oct & 3;                          //octant 1,2 cos 0,3 sin
            }
            else
            {
                nf = theta * FOPI;                      //how many pi/4's make up the angle
                oct = (int) nf;
                nf = oct;
                if(oct & 1)
                {
                    nf = nf + 1.0;                      //remove extra pi/4 to reflect cos or sin
                }
                theta = theta - nf*PIO40;
                theta = theta - nf*PIO41;
                theta = theta - nf*PIO42;
                if(oct & 4) sign = sign ^ 1;            //>pi so negate
                oct = oct & 3;                          //octant 1,2 cos 0,3 sin
            }
            x = theta;
            if(x < 0.0) x = -x;                         //make +ve due to reflection
            x2 = x * x;

            if(oct == 1 || oct == 2)                    //cosine
            {
                y = C3 + C4 * x2;
                y = C2 + y * x2;
                y = C1 + y * x2;
                y = C0 + y * x2;
            } else {                                    //sine
                y = S2 + S3 * x2;
                y = S1 + y * x2;
                y = y * x2;
                y = x + y * x;                          //big accuracy loss if not careful
            }
            if(sign) y = -y;
            return(y);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,1
    .global qhmath_sin_f
    .type qhmath_sin_f, @function

/*============================================================================*/

#define   sp3      0xB94D3E19
#define   sp2      0x3C08859F
#define   sp1      0xBE2AAAA8
#define   cp4      0x37CD1D28
#define   cp3      0xBAB60694
#define   cp2      0x3D2AAAA5
#define   cp1      0xBF000000
#define   pi_4_0   0x3F490000
#define   pi_4_1   0x397DA000
#define   pi_4_2   0x33222169
#define   maxtheta 0x48000000
#define   invpio4  0x3FA2F983
#define   signx    r1:0
#define   x        r0
#define   y        r0
#define   sign     r1
#define   MTHETA   r15
#define   theta    r0
#define   x_       r13
#define   FOPI     r2
#define   nf       r3
#define   ONE      r4
#define   ione     r14
#define   isix     r12
#define   PIO40    r11
#define   PIO41    r6
#define   PIO42    r6
#define   oct      r9
#define   x2       r10
#define   S3       r11
#define   S2       r12
#define   S1       r11
#define   S0       r12
#define   C4       r11
#define   C3       r12
#define   C2       r11
#define   C1       r12
#define   C0       r11

/*============================================================================*/

qhmath_sin_f:
{
    p0 = sfclass(r0,#1)
    if (p0.new) jumpr:nt r31
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
    PIO40 = ##pi_4_0
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
    p0 = tstbit(oct, #2)                       //is oct & 4
}
{
    x -= sfmpy(nf, PIO40)
    PIO41 = ##pi_4_1
    oct = and(oct, #3)                         //oct = oct & 3
}
{
    x -= sfmpy(nf, PIO41)
    PIO42 = ##pi_4_2
    if(p0) sign = xor(sign, ione)
}
{
    x -= sfmpy(nf, PIO42)
    p0 = !cmp.eq(oct, #1)
    p0 = !cmp.eq(oct, #2)
}
    .falign
.L_poly:
{
    x_ = clrbit(x, #31)                        //fabs(x)
    x2 = sfmpy(x, x)                           // x * x
    C4 = ##cp4
}
{
    if(!p0) jump .L_cosine
    C3 = ##cp3
}
{
    S3 = ##sp3
    S2 = ##sp2
}
{
    S2 += sfmpy(S3, x2)
    S1 = ##sp1
    sign = asl(sign, #31)
}
{
    S1 += sfmpy(S2, x2)
}
{
    S0 = sfmpy(S1, x2)
    x = xor(x_, sign)
}
{
    x += sfmpy(x, S0)
    dealloc_return                             //return( y)
}
    .falign
.L_cosine:
{
    C3 += sfmpy(C4, x2)
    C2 = ##cp2
}
{
    C2 += sfmpy(C3, x2)
    C1 = ##cp1
    sign = asl(sign, #31)
}
{
    C1 += sfmpy(C2, x2)
    C0 = sfmake(#384):pos                      //1.0
}
{
    C0 += sfmpy(C1, x2)
}
{
    y = xor(C0, sign)
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

/*============================================================================*/

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
    one = #1
}
{
    p0 = tstbit(octd, #2)
    if(p0.new) signd = xor(signd, one)
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
    octd = and(octd, #3)
    r3:2 = thetad
    call __hexagon_adddf3                      //thetad = thetad + -DPIO41*nd
}
{
    theta = convert_df2sf(r1:0)
    p0 = !cmp.eq(octd, #1)
    p0 = !cmp.eq(octd, #2)
}
{
    sign = signd
    r17:16 = memd(sp+#0)
    r19:18 = memd(sp+#8)
}
{
    r21:20 = memd(sp+#16)
    r23:22 = memd(sp+#24)
    jump  .L_poly                              //p0 is live out
}
    .size qhmath_sin_f, .-qhmath_sin_f
