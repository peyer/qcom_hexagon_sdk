/**=============================================================================
@file
    qhmath_pow_f.S

@brief
    Computes the value of base raised to the power exponent.

    Function prototype

        float qhmath_pow_f(float x, float y);

    Reference C code

        static const float
        bp[] = {1.0, 1.5,},
        dp_h[] = { 0.0, 5.84960938e-01,}, // 0x3f15c000
        dp_l[] = { 0.0, 1.56322085e-06,}, // 0x35d1cfdc
        zero    =  0.0,
        one	=  1.0,
        two	=  2.0,
        two24	=  16777216.0, // 0x4b800000
        huge    =  1.0e30,
        tiny    =  1.0e-30,
        // poly coefs for (3/2)*(log(x)-2s-2/3*s**3
        L1  =  6.0000002384e-01,        // 0x3f19999a
        L2  =  4.2857143283e-01,        // 0x3edb6db7
        L3  =  3.3333334327e-01,        // 0x3eaaaaab
        L4  =  2.7272811532e-01,        // 0x3e8ba305
        L5  =  2.3066075146e-01,        // 0x3e6c3255
        L6  =  2.0697501302e-01,        // 0x3e53f142
        P1   =  1.6666667163e-01,       // 0x3e2aaaab
        P2   = -2.7777778450e-03,       // 0xbb360b61
        P3   =  6.6137559770e-05,       // 0x388ab355
        P4   = -1.6533901999e-06,       // 0xb5ddea0e
        P5   =  4.1381369442e-08,       // 0x3331bb4c
        lg2  =  6.9314718246e-01,       // 0x3f317218
        lg2_h  =  6.93145752e-01,       // 0x3f317200
        lg2_l  =  1.42860654e-06,       // 0x35bfbe8c
        ovt =  4.2995665694e-08,        // -(128-log2(ovfl+.5ulp))
        cp    =  9.6179670095e-01,      // 0x3f76384f =2/(3ln2)
        cp_h  =  9.6191406250e-01,      // 0x3f764000 =12b cp
        cp_l  = -1.1736857402e-04,      // 0xb8f623c6 =tail of cp_h
        ivln2    =  1.4426950216e+00,   // 0x3fb8aa3b =1/ln2
        ivln2_h  =  1.4426879883e+00,   // 0x3fb8aa00 =16b 1/ln2
        ivln2_l  =  7.0526075433e-06,   // 0x36eca570 =1/ln2 tail
        two25   =  3.355443200e+07,     // 0x4c000000
        twom25  =  2.9802322388e-08;    // 0x33000000

        inline float scalbnf (float x, int n)
        {
            int32_t k,ix;
            GET_FLOAT_WORD(ix,x);
            k = (ix&0x7f800000)>>23;                        // extract exponent
            if (k==0) {                                     // 0 or subnormal x
                if ((ix&0x7fffffff)==0) return x; // +-0
                x *= two25;
                GET_FLOAT_WORD(ix,x);
                k = ((ix&0x7f800000)>>23) - 25;
                if (n< -50000) return tiny*x;               // underflow
            }
            if (k==0xff) return x+x;                        // NaN or Inf
            k = k+n;
            if (k >  0xfe) return huge*copysignf(huge,x);   // overflow
            if (k > 0)                                      // normal result
                {SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23)); return x;}
            if (k <= -25) {
                if (n > 50000)                              // in case integer overflow in n+k
                    return huge*copysignf(huge,x);          // overflow
                else return tiny*copysignf(tiny,x);         // underflow
            }
            k += 25;                                        // subnormal result
            SET_FLOAT_WORD(x,(ix&0x807fffff)|(k<<23));
            return x*twom25;
        }

        OLM_DLLEXPORT float
        __ieee754_powf(float x, float y)
        {
            float z,ax,z_h,z_l,p_h,p_l;
            float y1,t1,t2,r,s,sn,t,u,v,w;
            int32_t i,j,k,yisint,n;
            int32_t hx,hy,ix,iy,is;

            GET_FLOAT_WORD(hx,x);
            GET_FLOAT_WORD(hy,y);
            ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

            // y==zero: x**0 = 1
            if(iy==0) return one;

            // x==1: 1**y = 1, even if y is NaN
            if (hx==0x3f800000) return one;

            // y!=zero: result is NaN if either arg is NaN
            if(ix > 0x7f800000 ||
               iy > 0x7f800000)
                    return x + y;

            // special value of y
            if (iy==0x7f800000) {                           // y is +-inf
                if (ix==0x3f800000)
                    return  one;                            // (-1)**+-inf is NaN
                else if (ix > 0x3f800000)                   // (|x|>1)**+-inf = inf,0
                    return (hy>=0)? y: zero;
                else                                        // (|x|<1)**-,+inf = inf,0
                    return (hy<0)?-y: zero;
            }

            // determine if y is an odd int when x < 0
            //  yisint = 0        ... y is not an integer
            //  yisint = 1        ... y is an odd int
            //  yisint = 2        ... y is an even int
            yisint  = 0;
            if(hx<0) {
                if(iy>=0x4b800000) yisint = 2;              // even integer y
                else if(iy>=0x3f800000) {
                    k = (iy>>23)-0x7f;                      // exponent
                    j = iy>>(23-k);
                    if((j<<(23-k))==iy) yisint = 2-(j&1);
                }
            }

            if(iy==0x3f800000) {                            // y is  +-1
                if(hy<0) return one/x; else return x;
            }
            if(hy==0x40000000) return x*x;                  // y is  2
            if(hy==0x40400000) return x*x*x;                // y is  3
            if(hy==0x40800000) {                            // y is  4
                u = x*x;
                return u*u;
            }
            if(hy==0x3f000000) {                            // y is  0.5
                if(hx>=0)        // x >= +0
                    return __ieee754_sqrtf(x);
            }

            ax   = fabsf(x);
            // special value of x
            if(ix==0x7f800000||ix==0||ix==0x3f800000){
                z = ax;                                     // x is +-0,+-inf,+-1
                if(hy<0) z = one/z;                         // z = (1/|x|)
                if(hx<0) {
                    if(((ix-0x3f800000)|yisint)==0) {
                        z = (z-z)/(z-z);                    // (-1)**non-int is NaN
                    } else if(yisint==1)
                        z = -z;                             // (x<0)**odd = -(|x|**odd)
                }
                return z;
            }

            n = ((u_int32_t)hx>>31)-1;

            // (x<0)**(non-int) is NaN
            if((n|yisint)==0) return (x-x)/(x-x);

            sn = one; // s (sign of result -ve**odd) = -1 else = 1
            if((n|(yisint-1))==0) sn = -one;                // (-ve)**(odd int)

            // |y| is huge
            if(iy>0x4d000000) {                             // if |y| > 2**27
            // over/underflow if x is not close to one
                if(ix<0x3f7ffff8) return (hy<0)? sn*huge*huge:sn*tiny*tiny;
                if(ix>0x3f800007) return (hy>0)? sn*huge*huge:sn*tiny*tiny;
            // now |1-x| is tiny <= 2**-20, suffice to compute
            // log(x) by x-x^2/2+x^3/3-x^4/4
                t = ax-1;                                   // t has 20 trailing zeros
                w = (t*t)*((float)0.5-t*((float)0.333333333333-t*(float)0.25));
                u = ivln2_h*t;                              // ivln2_h has 16 sig. bits
                v = t*ivln2_l-w*ivln2;
                t1 = u+v;
                GET_FLOAT_WORD(is,t1);
                SET_FLOAT_WORD(t1,is&0xfffff000);
                t2 = v-(t1-u);
            } else {
                float s2,s_h,s_l,t_h,t_l;
                n = 0;
            // take care subnormal number
                if(ix<0x00800000)
                    {ax *= two24; n -= 24; GET_FLOAT_WORD(ix,ax); }
                n  += ((ix)>>23)-0x7f;
                j  = ix&0x007fffff;
            // determine interval
                ix = j|0x3f800000;                          // normalize ix
                if(j<=0x1cc471) k=0;                        // |x|<sqrt(3/2)
                else if(j<0x5db3d7) k=1;                    // |x|<sqrt(3)
                else {k=0;n+=1;ix -= 0x00800000;}
                SET_FLOAT_WORD(ax,ix);

            // compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5)
                u = ax-bp[k];                               // bp[0]=1.0, bp[1]=1.5
                v = one/(ax+bp[k]);
                s = u*v;
                s_h = s;
                GET_FLOAT_WORD(is,s_h);
                SET_FLOAT_WORD(s_h,is&0xfffff000);
            // t_h=ax+bp[k] High
                is = ((ix>>1)&0xfffff000)|0x20000000;
                SET_FLOAT_WORD(t_h,is+0x00400000+(k<<21));
                t_l = ax - (t_h-bp[k]);
                s_l = v*((u-s_h*t_h)-s_h*t_l);
            // compute log(ax)
                s2 = s*s;
                r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
                r += s_l*(s_h+s);
                s2  = s_h*s_h;
                t_h = (float)3.0+s2+r;
                GET_FLOAT_WORD(is,t_h);
                SET_FLOAT_WORD(t_h,is&0xfffff000);
                t_l = r-((t_h-(float)3.0)-s2);
            // u+v = s*(1+...)
                u = s_h*t_h;
                v = s_l*t_h+t_l*s;
            // 2/(3log2)*(s+...)
                p_h = u+v;
                GET_FLOAT_WORD(is,p_h);
                SET_FLOAT_WORD(p_h,is&0xfffff000);
                p_l = v-(p_h-u);
                z_h = cp_h*p_h;                             // cp_h+cp_l = 2/(3*log2)
                z_l = cp_l*p_h+p_l*cp+dp_l[k];
            // log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l
                t = (float)n;
                t1 = (((z_h+z_l)+dp_h[k])+t);
                GET_FLOAT_WORD(is,t1);
                SET_FLOAT_WORD(t1,is&0xfffff000);
                t2 = z_l-(((t1-t)-dp_h[k])-z_h);
            }

            // split up y into y1+y2 and compute (y1+y2)*(t1+t2)
            GET_FLOAT_WORD(is,y);
            SET_FLOAT_WORD(y1,is&0xfffff000);
            p_l = (y-y1)*t1+y*t2;
            p_h = y1*t1;
            z = p_l+p_h;
            GET_FLOAT_WORD(j,z);
            if (j>0x43000000)                               // if z > 128
                return sn*huge*huge;                        // overflow
            else if (j==0x43000000) {                       // if z == 128
                if(p_l+ovt>z-p_h) return sn*huge*huge;      // overflow
            }
            else if ((j&0x7fffffff)>0x43160000)             // z <= -150
                return sn*tiny*tiny;                        // underflow
            else if (j==0xc3160000){                        // z == -150
                if(p_l<=z-p_h) return sn*tiny*tiny;         // underflow
            }

            // compute 2**(p_h+p_l)
            i = j&0x7fffffff;
            k = (i>>23)-0x7f;
            n = 0;
            if(i>0x3f000000) {                              // if |z| > 0.5, set n = [z+0.5]
                n = j+(0x00800000>>(k+1));
                k = ((n&0x7fffffff)>>23)-0x7f;              // new k for n
                SET_FLOAT_WORD(t,n&~(0x007fffff>>k));
                n = ((n&0x007fffff)|0x00800000)>>(23-k);
                if(j<0) n = -n;
                p_h -= t;
            }
            t = p_l+p_h;
            GET_FLOAT_WORD(is,t);
            SET_FLOAT_WORD(t,is&0xffff8000);
            u = t*lg2_h;
            v = (p_l-(t-p_h))*lg2+t*lg2_l;
            z = u+v;
            w = v-(z-u);
            t  = z*z;
            t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
            r  = (z*t1)/(t1-two)-(w+z*w);
            z  = one-(r-z);
            GET_FLOAT_WORD(j,z);
            j += (n<<23);
            if((j>>23)<=0) z = scalbnf(z,n);                // subnormal output
            else SET_FLOAT_WORD(z,j);
            return sn*z;
        }

@note
    Reference C code obtained from openlibm library.

    Copyright from openlibm:

    ---------------------------------------------------------------------------
    e_powf.c -- float version of e_pow.c.
    Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.

    Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
    
    Developed at SunPro, a Sun Microsystems, Inc. business.
    Permission to use, copy, modify, and distribute this
    software is freely granted, provided that this notice
    is preserved.
    ---------------------------------------------------------------------------

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

    .p2align 2
    .p2align 4,,15
    .global qhmath_pow_f
    .type qhmath_pow_f, @function

qhmath_pow_f:
{
    r2 = r0
    r0 = ##1065353216                                 // r0 = one
}
{
    p0 = sfclass(r1,#30)
    p0 = !cmp.eq(r2,r0)
    if (!p0.new) jumpr:nt r31                         // iy==0 || hx==0x3f800000
}
{
    p0 = sfclass(r2,#15)
    p0 = sfclass(r1,#15)
    if (p0.new) jump:t .L_ch_iy_is_inf                // x and y are not NaN
}
{
    r0 = sfadd(r2,r1)                                 // create NaN
    jumpr r31
}
    .falign
.L_ch_iy_is_inf:
{
    r4 = clrbit(r2,#31)                               // ix = hx&0x7fffffff
    p0 = sfclass(r1,#8)                               // iy==0x7f800000
    if (!p0.new) jump:t .L_ch_hx_is_neg
}
{
    r2 = togglebit(r1,#31)                            // calculate -y
    p1 = cmp.gt(r1,#-1)                               // hy>=0
    p0 = cmp.eq(r4,r0)                                // ix==0x3f800000
    if (p0.new) jumpr:t r31
}
{
    r0 = mux(p1,r1,#0)                                // (hy>=0)? y: zero
    r1 = mux(p1,#0,r2)                                // (hy>=0)? zero: -y
    p0 = cmp.gtu(r4,r0)                               // ix>0x3f800000
}
{
    if (!p0) r0 = r1
    jumpr r31
}
    .falign
.L_ch_hx_is_neg:
{
    r3 = clrbit(r1,#31)                               // iy = hy&0x7fffffff
    r17:16 = combine(r4,r1)
    memd(r29+#-16) = r17:16
    allocframe(#32)
}
{
    r19:18 = combine(#0,r0)
    memd(r29+#16) = r19:18
    memd(r29+#8) = r21:20
}
{
    p2 = cmp.gt(r2,#-1)
    if (p2.new) jump:t .L_ch_iy_eq_one                // hx>=0
    memd(r29+#0) = r23:22
}
{
    p1 = cmp.gtu(r3,##1266679807)
    if (p1.new) r19 = #2                              // yisint = 2
    if (p1.new) jump:t .L_ch_ix_is_spec_val           // iy>=0x4b800000
}
{
    p0 = cmp.gtu(r3,##1065353215)
    if (!p0.new) jump:t .L_ch_ix_is_spec_val          // iy<0x3f800000
    if (p0.new) r5 = r3                               // r5 = iy
}
{
    r5 = sub(#150,lsr(r5,#23))                        // k = 23-((iy>>23)-0x7f)
}
{
    r4 = lsr(r3,r5)                                   // j = iy>>k
}
{
    r4 = and(r4,#1)                                   // j&1
    r5 = asl(r4,r5)                                   // j<<k
    if (!cmp.eq(r5.new,r3)) jump:t .L_ch_iy_eq_one    // (j<<k)!=iy
}
{
    r19 = sub(#2,r4)                                  // yisint = 2-(j&1)
}
    .falign
.L_ch_iy_eq_one:
{
    p1 = cmp.eq(r3,r18)
    if (!p1.new) jump:t .L_ch_hy_eq_four              // iy!=0x3f800000
}
{
    p0 = cmp.gt(r16,#-1)
    if (p0.new) jump:t .L_ret                         // return x for hy>=0
    if (p0.new) r0 = r2
}
{
    r1 = r2
    call qhmath_div_f                                 // one/x
}
{
    jump .L_ret
}
    .falign
.L_ch_hy_eq_four:
{
    r0 = sfmpy(r2,r2)                                 // r0 = x*x
    p1 = cmp.eq(r16,##1082130432)
    if (!p1.new) jump:t .L_ch_hy_eq_three             // hy!=0x40800000
}
{
    r0 = sfmpy(r0,r0)
    jump .L_ret                                       // return x*x*x*x
}
    .falign
.L_ch_hy_eq_three:
{
    r7 = sfmake(#320):pos                             // r7 = 0.5f
    p1 = cmp.eq(r16,##1077936128)
    if (!p1.new) jump:t .L_ch_hy_eq_two               // hy!=0x40400000
}
{
    r0 = sfmpy(r0,r2)                                 // return x*x*x
    jump .L_ret
}
    .falign
.L_ch_hy_eq_two:
{
    p1 = cmp.eq(r16,##1073741824)
    if (p1.new) jump:t .L_ret                         // return x*x for hy==0x40000000
}
{
    p0 = cmp.gt(r2,#-1)
    p0 = cmp.eq(r16,r7)
    if (!p0.new) jump:t .L_ch_ix_is_spec_val          // !(hy==0x3f000000 && hx>=0)
    if (p0.new) r17:16 = memd(r29+#24)
    if (p0.new) r19:18 = memd(r29+#16)
}
{
    r21:20 = memd(r29+#8)
    r23:22 = memd(r29+#0)
}
{
    r0 = r2
    jump qhmath_sqrt_f
    deallocframe
}
    .falign
.L_ch_ix_is_spec_val:
{
    p0 = sfclass(r17,#22)                             // ix!=0x7f800000 && ix!=0
    p0 = !cmp.eq(r17,r18)                             // ix!=0x3f800000
    if (p0.new) jump:t .L_ix_not_spec_val
    if (p0.new) r5 = r2                               // r5 = hx
}
{
    p0 = cmp.gt(r16,#-1)
    if (p0.new) jump:nt .L_spec_ch_hx_lt_zero         // hy>=0
    if (p0.new) r0 = r17                              // z = ax
    r16 = r2                                          // r16 = hx
}
{
    r1:0 = combine(r17,r18)
    call qhmath_div_f                                 // one/z
}
    .falign
.L_spec_ch_hx_lt_zero:
{
    p0 = cmp.gt(r16,#-1)
    if (p0.new) jump:nt .L_ret                        // hx>=0
    if (!p0.new) r2 = add(r17,##-1065353216)          // ix-0x3f800000
}
{
    r3 = ##2147483648                                 // r3 = 0x80000000
    r2 = or(r19,r2)                                   // (ix-0x3f800000)|yisint
    if (cmp.eq(r2.new,#0)) jump:nt .L_ret_nan
}
{
    p0 = cmp.eq(r19,#1)
    if (p0.new) r0 = xor(r0,r3)                       // z = -z for yisint==1
    jump .L_ret
}
        .falign
.L_ix_not_spec_val:
{
    r1 = ##8388607                                    // 0x7fffff
    r5 = add(##-1,lsr(r5,#31))                        // n = ((u_int32_t)hx>>31)-1
    r4 = add(r19,#-1)                                 // yisint - 1
}
{
    r8 = ##8388608                                    // 0x00800000
    r6 = or(r19,r5)                                   // n|yisint
    r4 = or(r4,r5)                                    // n|(yisint-1)
}
{
    p0 = cmp.eq(r6,#0)
    if (p0.new) r0 = r2                               // r0 = x
    if (p0.new) jump:nt .L_ret_nan                    // (n|yisint)==0
    if (!p0.new) r2 = ##1266679808                    // two24 = 0x4b800000
}
{
    p0 = cmp.eq(r4,#0)
    if (p0.new) r19 = ##-1082130432                   // sn = -one
    if (!p0.new) r19 = r18                            // sn = one
}
{
    r22 = #0                                          // k = 0
    p0 = cmp.gtu(r3,##1291845632)
    if (!p0.new) jump:t .L_y_is_not_huge              // iy<=0x4d000000
}
{
    p0 = cmp.gtu(r17,##1065353207)
    if (p0.new) jump:t .L_ix_gt_close_to_one          // ix>=0x3f7ffff8
}
{
    p0 = cmp.gt(r16,#-1)                              // hy>=0
    if (p0.new) jump:t .L_ret_underflow               // return sn*tiny*tiny
    if (!p0.new) jump:nt .L_ret_overflow              // sn*huge*huge
}
    .falign
.L_ret_nan:
{
    r0 = sfsub(r0,r0)
    r1 = sfsub(r0,r0)
}
{
    call qhmath_div_f                                 // (x-x)/(x-x)
}
{
    jump .L_ret
}
    .falign
.L_ix_gt_close_to_one:
{
    r0 = sfsub(r17,r18)                               // t = ax-1
    p0 = cmp.gtu(r17,##1065353223)
    if (!p0.new) jump:t .L_compute_log_x              // ix<=0x3f800007
}
{
    p0 = cmp.gt(r16,#0)                              // hy>0
    if (p0.new) jump:t .L_ret_overflow               // return sn*huge*huge
    if (!p0.new) jump:nt .L_ret_underflow            // return sn*tiny*tiny
}
    .falign
.L_compute_log_x:
{
    r1 = sfmpy(r0,r0)                                 // (t*t)
    r2 = ##1048576000                                 // r2 = 0.25
}
{
    r3 = ##1051372203                                 // r3 = 0.333333
    r4 = ##1069066752                                 // ivln2_h = 0x3fb8aa00
}
{
    r4 = ##1056964608                                 // r4 = 0.5
    r3 -= sfmpy(r0,r2)                                // (float)0.333333333333-t*(float)0.25)
    r5 = sfmpy(r4,r0)                                 // u = ivln2_h*t
}
{
    r6 = ##921478512                                  // ivln2_l = 0x36eca570
    r4 -= sfmpy(r0,r3)                                // ((float)0.5-t*((float)0.333333333333-t*(float)0.25))
}
{
    r7 = ##1069066811                                 // ivln2 = 0x3fb8aa3b
    r2 = sfmpy(r0,r6)                                 // t*ivln2_l
    r3 = sfmpy(r4,r1)                                 // w = (t*t)*((float)0.5-t*((float)0.333333333333-t*(float)0.25))
}
{
    r2 -= sfmpy(r3,r7)                                // v = t*ivln2_l-w*ivln2;
}
{
    r4 = sfadd(r2,r5)                                 // t1 = u+v
}
{
    r4 = and(r4,##-4096)                              // t1 &= 0xfffff000
}
{
    r3 = sfsub(r4,r5)                                 // (t1-u)
    jump .L_split_up_y
}
    .falign
.L_y_is_not_huge:
{
    p0 = cmp.gtu(r17,r1)
    if (p0.new) jump:t .L_determine_interval          // ix>=0x00800000
    if (p0.new) r3 = r17                              // r3 = ax
    if (p0.new) r17 = #-127                           // n = -0x7f
}
{
    r3 = sfmpy(r17,r2)                                // ax *= two24
    r17 = #-151                                       // n = -24-0x7f
}
    .falign
.L_determine_interval:
{
    r4 = ##6140886                                    // r4 = 0x5db3d6
    r2 = extractu(r3,#23,#0)                          // j = ix&0x007fffff
    r17 += asr(r3,#23)                                // n += ((ix)>>23)
}
{
    r23 = or(r2,r18)                                  // ix = j|0x3f800000
    p0 = cmp.gtu(r2,##1885297)
    if (!p0.new) jump:nt .L_compute_s                 // j<=0x1cc471
}
{
    p0 = cmp.gtu(r2,r4)                               // j>=0x5db3d7
    if (p0.new) r17 = add(r17,#1)                     // n+=1
    if (p0.new) r23 = sub(r23,r8)                     // ix -= 0x00800000
    if (!p0.new) r22 = #1                             // k=1
}
    .falign
.L_compute_s:
{
    r2 = add(pc,##bp@PCREL)
}
{
    r20 = memw(r2+r22<<#2)                            // bp[k]
}
{
    r1 = sfadd(r20,r23)                               // ax+bp[k]
    r21 = sfsub(r23,r20)                              // u = ax-bp[k]
}
{
    r0 = r18
    call qhmath_div_f                                 // v = one/(ax+bp[k])
}
{
    r8 = asl(r22,#21)                                 // (k<<21)
    r1 = sfmpy(r21,r0)                                // s = u*v
    r3 = ##1045688642                                 // L6 = 0x3e53f142
}
{
    r5 = ##-4096                                      // r5 = 0xfffff000
    r6 = ##4194304                                    // r6 = 0x00400000
}
{
    r7 = and(r1,r5)                                   // s_h = s & 0xfffff000
    r2 = sfmpy(r1,r1)                                 // s2 = s*s
    r4 = ##1047278165                                 // L5 = 0x3e6c3255
}
{
    r5 &= asr(r23,#1)                                 // ((ix>>1)&0xfffff000)
    r4 += sfmpy(r2,r3)                                // (L5+s2*L6)
    r3 = ##1049338629                                 // L4 = 0x3e8ba305
}
{
    r5 = setbit(r5,#29)                               // is = ((ix>>1)&0xfffff000)|0x20000000
    r3 += sfmpy(r2,r4)                                // (L4+s2*(L5+s2*L6))
    r4 = ##1051372203                                 // L3 = 0x3eaaaaab
}
{
    r5 += add(r6,r8)                                  // t_h = is+0x00400000+(k<<21)
    r4 += sfmpy(r2,r3)                                // (L3+s2*(L4+s2*(L5+s2*L6)))
    r3 = ##1054567863                                 // L2 = 0x3edb6db7
}
{
    r21 -= sfmpy(r7,r5)                               // (u-s_h*t_h)
    r3 += sfmpy(r2,r4)                                // (L2+s2*(L3+s2*(L4+s2*(L5+s2*L6))))
    r4 = ##1058642330                                 // L1 = 0x3f19999a
}
{
    r4 += sfmpy(r2,r3)                                // (L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))))
    r3 = sfmpy(r2,r2)                                 // s2*s2
}
{
    r6 = sfsub(r5,r20)                                // t_h-bp[k]
    r3 = sfmpy(r3,r4)                                 // r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))))
}
{
    r5 = sfadd(r7,r1)                                 // (s_h+s)
    r6 = sfsub(r23,r6)                                // t_l = ax - (t_h-bp[k])
}
{
    r2 = sfmpy(r7,r7)                                 // s2  = s_h*s_h
    r21 -= sfmpy(r7,r6)                               // ((u-s_h*t_h)-s_h*t_l)
}
{
    r8 = ##1077936128                                 // r8 = 3.0f
    r4 = sfmpy(r0,r21)                                // s_l = v*((u-s_h*t_h)-s_h*t_l)
}
{
    r5 = sfadd(r2,r8)                                 // (float)3.0+s2
    r3 += sfmpy(r4,r5)                                // r += s_l*(s_h+s)
}
{
    r0 = convert_w2sf(r17)                            // t = (float)n
    r5 = sfadd(r3,r5)                                 // t_h = (float)3.0+s2+r
}
{
    r5 = and(r5,##-4096)                              // t_h = th&0xfffff000
}
{
    r8 = sfmpy(r7,r5)                                 // u = s_h*t_h
    r6 = sfsub(r5,r8)                                 // (t_h-(float)3.0)
}
{
    r7 = sfmpy(r4,r5)                                 // s_l*t_h
    r6 = sfsub(r6,r2)                                 // ((t_h-(float)3.0)-s2)
    r4 = ##1064714240                                 // cp_h = 0x3f764000
}
{
    r3 = sfsub(r3,r6)                                 // t_l = r-((t_h-(float)3.0)-s2)
    r5 = ##-1191828538                                // cp_l = 0xb8f623c6
}
{
    r7 += sfmpy(r3,r1)                                // v = s_l*t_h+t_l*s
    r6 = ##1064712271                                 // cp = 0x3f76384f
}
{
    r3 = add(pc,##dp_l@PCREL)
    r2 = sfadd(r8,r7)                                 // p_h = u+v
}
{
    r3 = memw(r3+r22<<#2)                             // dp_l[k]
    r2 = and(r2,##-4096)                              // p_h = p_h&0xfffff000
}
{
    r1 = sfmpy(r4,r2)                                 // z_h = cp_h*p_h
    r8 = sfsub(r2,r8)                                 // (p_h-u)
}
{
    r5 = sfmpy(r5,r2)                                 // cp_l*p_h
    r7 = sfsub(r7,r8)                                 // p_l = v-(p_h-u)
}
{
    r6 = add(pc,##dp_h@PCREL)
    r5 += sfmpy(r7,r6)                                // cp_l*p_h+p_l*cp
}
{
    r7 = memw(r6+r22<<#2)                             // dp_h[k]
    r2 = sfadd(r5,r3)                                 // z_l = cp_l*p_h+p_l*cp+dp_l[k]
}
{
    r4 = sfadd(r1,r2)                                 // (z_h+z_l)
}
{
    r4 = sfadd(r4,r7)                                 // ((z_h+z_l)+dp_h[k])
}
{
    r4 = sfadd(r4,r0)                                 // t1 = (((z_h+z_l)+dp_h[k])+t)
}
{
    r4 = and(r4,##-4096)                              // t1 = t1&0xfffff000
}
{
    r3 = sfsub(r4,r0)                                 // (t1-t)
}
{
    r3 = sfsub(r3,r7)                                 // ((t1-t)-dp_h[k])
}
{
    r3 = sfsub(r3,r1)                                 // (((t1-t)-dp_h[k])-z_h)
}
    .falign
.L_split_up_y:
{
    r0 = sfsub(r2,r3)                                 // compute t2
    r5 = and(r16,##-4096)                             // y1
}
{
    r6 = sfsub(r16,r5)                                // (y-y1)
    r3 = sfmpy(r4,r5)                                 // p_h = y1*t1
}
{
    r2 = sfmpy(r6,r4)                                 // (y-y1)*t1
}
{
    r2 += sfmpy(r0,r16)                               // p_l = (y-y1)*t1+y*t2
}
{
    r4 = sfadd(r3,r2)                                 // z = p_l+p_h
    r0 = ##859351612                                  // ovt = 0x3338aa3c
}
{
    p0 = cmp.gt(r4,##1124073472)
    if (p0.new) jump:nt .L_ret_overflow               // j>0x43000000
    if (!p0.new) r16 = #0                             // n = 0
}
{
    r5 = clrbit(r4,#31)                               // r5 = j&0x7fffffff
    p0 = cmp.eq(r4,##1124073472)
    if (!p0.new) jump:t .L_ch_z_le_mof                // j!=0x43000000
}
{
    r6 = sfsub(r4,r3)                                 // z-p_h
    r0 = sfadd(r2,r0)                                 // p_l+ovt
}
{
    p0 = sfcmp.gt(r0,r6)                              // p_l+ovt>z-p_h
    if (p0.new) jump:nt .L_ret_overflow               // return sn*huge*huge
    if (!p0.new) jump:t .L_i_gt_half                  // skip i>0x3f000000 check
}
    .falign
.L_ch_z_le_mof:
{
    p0 = cmp.gtu(r5,##1125515264)                     // (j&0x7fffffff)>0x43160000
    if (p0.new) jump:nt .L_ret_underflow              // return sn*tiny*tiny
}
{
    r6 = sfsub(r4,r3)                                 // z-p_h
    p0 = cmp.eq(r4,##-1021968384)
    if (!p0.new) jump:t .L_ch_i_gt_half               // j!=0xc3160000
}
{
    p0 = sfcmp.ge(r6,r2)                              // p_l<=z-p_h
    if (p0.new) jump:nt .L_ret_underflow              // return sn*tiny*tiny
    if (!p0.new) jump:t .L_i_gt_half                  // skip i>0x3f000000 check
}
    .falign
.L_ch_i_gt_half:
{
    p0 = cmp.gtu(r5,##1056964608)
    if (!p0.new) jump:t .L_compute_t                  // i<=0x3f000000
}
    .falign
.L_i_gt_half:
{
    r6 = ##8388608                                    // r6 = 0x00800000
    r5 = lsr(r5,#23)                                  // k = (i>>23)
}
{
    p0 = cmp.gt(r4,#-1)                               // j>=0
    r5 = add(r5,#-126)                                // k += (1-0x7f)
}
{
    r8 = ##-8388608                                   // r8 = 0xff800000
    r4 += lsr(r6,r5)                                  // n = j+(0x00800000>>(k+1));
    if (!p0) r0 = #0                                  // for if(j<0) n = -n
}
{
    r5 = extractu(r4,#8,#23)                          // k = ((n&0x7fffffff)>>23)
    r6 = insert(r4,#23,#0)                            // n = ((n&0x007fffff)|0x00800000)
}
{
    r7 = add(r5,#-127)                                // k -= 0x7f for t
    r5 = sub(#150,r5)                                 // (23+0x7f-k) for n
}
{
    r4 &= asr(r8,r7)                                  // t = n & (0xff800000 >> k)
    r16 = lsr(r6,r5)                                  // n = ((n&0x007fffff)|0x00800000)>>k
}
{
    r3 = sfsub(r3,r4)                                 // p_h -= t;
    if (!p0) r16 = sub(r0,r16)                        // if(j<0) n = -n
}
    .falign
.L_compute_t:
{
    r0 = sfadd(r2,r3)                                 // t = p_l+p_h
    r1 = ##1060205056                                 // lg2_h = 0x3f317200
}
{
    r0 = and(r0,##-32768)                             // t &= 0xffff8000
    r5 = ##1060205080                                 // lg2 = 0x3f317218
}
{
    r4 = sfmpy(r0,r1)                                 // u = t*lg2_h
    r1 = sfsub(r0,r3)                                 // (t-p_h)
}
{
    r3 = ##901758604                                  // lg2_l = 0x35bfbe8c
    r2 = sfsub(r2,r1)                                 // (p_l-(t-p_h))
}
{
    r6 = ##858897228                                  // P5 = 0x3331bb4c
    r2 = sfmpy(r2,r5)                                 // (p_l-(t-p_h))*lg2
}
{
    r7 = ##-1243747826                                // P4 = 0xb5ddea0e
    r2 += sfmpy(r0,r3)                                // v = (p_l-(t-p_h))*lg2+t*lg2_l
}
{
    r8 = ##948613973                                  // P3 = 0x388ab355
    r17 = sfadd(r2,r4)                                // z = u+v
}
{
    r3 = sfsub(r17,r4)                                // (z-u)
    r4 = sfmpy(r17,r17)                               // t = z*z
}
{
    r6 = ##-1154086047                                // P2 = 0xbb360b61
    r20 = sfsub(r2,r3)                                // w = v-(z-u)
    r7 += sfmpy(r4,r6)                                // (P4+t*P5)
}
{
    r7 = ##1042983595                                 // P1 = 0x3e2aaaab
    r8 += sfmpy(r4,r7)                                // P3+t*(P4+t*P5)
}
{
    r1 = r17                                          // r1 = z
    r8 = ##1073741824                                 // two = 0x40000000
    r6 += sfmpy(r4,r8)                                // (P2+t*(P3+t*(P4+t*P5)))
}
{
    r7 += sfmpy(r4,r6)                                // (P1+t*(P2+t*(P3+t*(P4+t*P5))))
}
{
    r1 -= sfmpy(r4,r7)                                // t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))))
}
{
    r0 = sfmpy(r17,r1)                                // (z*t1)
    r1 = sfsub(r1,r8)                                 // (t1-two)
}
{
    r20 += sfmpy(r17,r20)                             // (w+z*w)
    call qhmath_div_f                                 // (z*t1)/(t1-two)
}
{
    r0 = sfsub(r0,r20)                                // r  = (z*t1)/(t1-two)-(w+z*w)
}
{
    r4 = ##8388607                                    // 0x7fffff
    r2 = sfsub(r0,r17)                                // (r-z)
}
{
    r2 = sfsub(r18,r2)                                // z  = one-(r-z)
    r3 = sfsub(r18,r2)                                // z  = one-(r-z)
}
{
    r2 += asl(r16,#23)                                // j += (n<<23)
    if (cmp.gt(r2.new,r4)) jump:t .L_mpy_and_ret      // j>0x7fffff instead of (j>>23)>0
}
{                                                     // float scalbnf (float x, int n)
    r20 = and(r3,##-2147483648)                       // r20 = x&0x80000000
    r4 = extractu(r3,#8,#23)                          // k = (ix&0x7f800000)>>23
}
{
    r0 = ##1275068416                                 // two25 = 0x4c000000
    p0 = cmp.eq(r4,#255)
    if (!p0.new) jump:t .L_ch_k_eq_zero               // k!=0xff
}
{
    r2 = sfadd(r3,r3)
    jump .L_mpy_and_ret                               // return x+x
}
    .falign
.L_ch_k_eq_zero:
{
    r1 = ##228737632                                  // r1 = tiny
    p0 = cmp.eq(r4,#0)
    if (!p0.new) jump:nt .L_ch_k_gt_largest_exp       // k!=0
}
{
    r3 = sfmpy(r3,r0)                                 // x *= two25
    p0 = sfclass(r3,#1)
    if (p0.new) r2 = add(r3,#0)
    if (p0.new) jump:nt .L_mpy_and_ret                // return x for +-0
}
{
    r4 = extractu(r3,#8,#23)                          // k = ((ix&0x7f800000)>>23)
    p0 = cmp.gt(r16,##-50001)
    if (p0.new) jump:nt .L_adjust_k                   // n>=-50000
}
{
    r2 = sfmpy(r3,r1)
    jump .L_mpy_and_ret                               // return tiny*x
}
    .falign
.L_adjust_k:
{
    r4 = add(r4,#-25)                                 // k -= 25
}
    .falign
.L_ch_k_gt_largest_exp:
{
    r4 = add(r4,r16)                                  // k = k+n
}
{
    p1 = cmp.gt(r4,#254)
    if (p1.new) r19 = xor(r19,r20)                    // sn = sn^r20
    if (p1.new) jump:nt .L_ret_overflow               // k>0xfe
}
{
    r2 = and(r3,##-2139095041)                        // x = ix&0x807fffff
    p1 = cmp.gt(r4,#0)
    if (!p1.new) jump:t .L_ch_k_le_smallest_exp       // k<=0
}
{
    r2 |= asl(r4,#23)                                 // x |= (k<<23)
    jump .L_mpy_and_ret
}
    .falign
.L_ch_k_le_smallest_exp:
{
    p1 = cmp.gt(r4,#-25)
    if (p1.new) r2 = ##855638016                      // twom25 = 0x33000000
    if (!p1.new) jump:t .L_k_le_smallest_exp          // k<=-25
}
{
    r4 = add(##209715200,asl(r4,#23))                 // k = 0xc800000(k+=25) + (k<<23)
}
{
    r3 = or(r4,and(r3,##-2139095041))                 // x = (ix&0x807fffff)|k
}
{
    r2 = sfmpy(r3,r2)
    jump .L_mpy_and_ret                               // return x*twom25
}
    .falign
.L_k_le_smallest_exp:
{
    r19 = xor(r19,r20)                                // sn = sn^r20
    p1 = cmp.gt(r16,##50000)
    if (p1.new) jump:nt .L_ret_overflow               // return huge*copysignf(huge,x)
}
    .falign
.L_ret_underflow:
{
    r2 = ##228737632                                  // r2 = tiny
}
{
    r19 = sfmpy(r19,r2)
    jump .L_mpy_and_ret
}
    .falign
.L_ret_overflow:
{
    r2 = ##1900671690                                 // r2 = huge
}
{
    r19 = sfmpy(r19,r2)
}
    .falign
.L_mpy_and_ret:
{
    r0 = sfmpy(r19,r2)
}
    .falign
.L_ret:
{
    r17:16 = memd(r29+#24)
    r19:18 = memd(r29+#16)
}
{
    r21:20 = memd(r29+#8)
    r23:22 = memd(r29+#0)
}
{
    dealloc_return
}
.size qhmath_pow_f, .-qhmath_pow_f

  .type  bp,@object              // @bp
  .section  .rodata.cst8,"aM",@progbits,8
  .p2align  3
bp:
  .word  1065353216              // float 1
  .word  1069547520              // float 1.5
  .size  bp, 8

  .type  dp_l,@object            // @dp_l
  .p2align  3
dp_l:
  .word  0                       // float 0
  .word  902942684               // float 1.56322085E-6
  .size  dp_l, 8

  .type  dp_h,@object            // @dp_h
  .p2align  3
dp_h:
  .word  0                       // float 0
  .word  1058390016              // float 0.584960938
  .size  dp_h, 8
