/**=============================================================================
@file
    qhmath_atan2_h.S

@brief
    Computes 4-quadrant inverse tangent for 16-bit fixed-point input arguments.
    x and y argument have to be in same Qi.f format where parameter i is integer part
    and f is fractional part.
    Output result is in Q15 signed format.

    Function prototype

        int16_t qhmath_atan2_h(int16_t y, int16_t x);

    7th degree polynomial approximation: proj_angle = atan(f)/pi,  where f in [0,1]
    proj_angle = 9.586300e-8 + 3.183e-1*f^1 -4.650e-5*f^2 -1.048e-1*f^3 -
                 1.024e-2*f^4 +1.011e-1*f^5 -7.122e-2*f^6 +1.685e-2*f^7

    Reference C code
        polynom coefficients
        static int32_t c[] = {
                               5340306, -3121, -28127105, -10989807,
                               434352049, -1223588557, 1157799985, 206
                              }; [c1:c7, c0] Qs (in the same order) =[(24:2:36) 31]

        int16_t qhmath_atan2_h(int16_t y, int16_t x)
        {
            int8_t is_x_ge_0, is_y_ge_0, is_ay_gt_ax;
            int32_t offset, xn;
            int64_t frac, sum;
            result_scale_t den;
            int32_t x_1, y_1;

            is_y_ge_0 = (y >= 0);
            is_x_ge_0 = (x >= 0);

            x_1 = Q6_R_abs_R(x);
            y_1 = Q6_R_abs_R(y);

            // is_ay_gt_ax = (|x|<|y|)?
            is_ay_gt_ax = ( y_1 > x_1 );

            // if |x|<|y|, swap the values to get the bigger value as the divisor.
            // This gives |f|<=1 & projects the angle into the 1st octant
            if (is_ay_gt_ax)
            {
                offset = x_1;
                x_1 = y_1;
                y_1 = offset;
            }

            // if max(|x|,|y|)==0 then actual angle =0
            if (x==0)
                return(0);

            // Approximate 1/|x| using approx_uinv_newt with 3 iterations
            // (for max accuracy)
            den = approx_uinv_newt(x_1 ,3);

            // frac = |y|* 1/|x|..Q32+scale(=32+scale-Qi+Qi).
            // Refer approx_uinv_newt() for details on formula
            // for returned Q used above
            frac = Q6_P_mpyu_RR(den.result, y_1);

            // scale+=2 to ensure unsigned product is within +/-1 in Q30.
            // Now, frac is in Q30+scale
            den.scale +=2;

            // f..Q30+scale --> f..Q30 (f=frac.w[0])
            x_1 = (Word32)Q6_P_asr_PR(frac, den.scale);

            xn = x_1;
            sum = 0;  //init sum of polynomial approximation

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[0]);   //sum += c(i+1)*f^(i+1)
            xn = Q6_R_mpy_RR(xn, x_1);                             //f^(i+2)..Q28-2i

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[1]);   //sum += c(i+1)*f^(i+1)
            xn = Q6_R_mpy_RR(xn, x_1);                              //f^(i+2)..Q28-2i

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[2]);   //sum += c(i+1)*f^(i+1)
            xn = Q6_R_mpy_RR(xn, x_1);                              //f^(i+2)..Q28-2i

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[3]);   //sum += c(i+1)*f^(i+1)
            xn = Q6_R_mpy_RR(xn, x_1);                              //f^(i+2)..Q28-2i

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[4]);   //sum += c(i+1)*f^(i+1)
            xn = Q6_R_mpy_RR(xn, x_1);                              //f^(i+2)..Q28-2i

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[5]);   //sum += c(i+1)*f^(i+1)
            xn = Q6_R_mpy_RR(xn, x_1);                              //f^(i+2)..Q28-2i

            sum += Q6_P_mpy_RR(xn, approx_atan2_poly7_coefs[6]);   //sum += c(i+1)*f^(i+1)

            // We have sum = c1*f + ..+c7*f^7. But proj_angle = c0+ sum.
            // Load c0 into frac & add to sum after Q-factor alignment i.e.
            // sum..Q54 --> sum..Q31
            y_1 = (Word32)(approx_atan2_poly7_coefs[7] + Q6_P_asr_PI(sum, 23 ));

            // Calculate the multiple of 90 deg. to re-rotate the projected angle in
            // 1st octant back to the original quadrant.
            // offset = (2*is_y_ge_0-1)*.5*(is_ay_gt_ax + 2*!(is_ay_gt_ax | is_x_ge_0))
            // NOTE: The conditional variables is_XXXX are treated as 0/1 above
            if (is_ay_gt_ax)
            {
                offset = 0x40000000;  //.5..Q31 (for +pi/2)
            }
            else if (!is_x_ge_0)
            {
                offset =(Word32)0x80000000; //-1..Q31 (for pi; pi=-pi due to wraparound)
            }else
            {
                offset = 0;
            }
            // Assign the correct sign to the pi/2 multiple
            if (!is_y_ge_0)
            {
                offset = -offset;
            }

            // actual_angle= (1-2*s3)*proj_angle + offset..Q31
            // s3 = is_y_ge_0^is_x_ge_0^is_ay_gt_ax
            // NOTE: The conditional variables are treated as 0/1 above
            if ((is_y_ge_0 ^ is_x_ge_0) ^is_ay_gt_ax)
            {
                y_1 = offset - y_1;
            }
            else
            {
                y_1 = offset + y_1;
            }

            y_1 = Q6_R_asrh_R(y_1);

            return (y_1);
        }

    NOTE: qhmath_atan2_h(y,x) ~ atan(y/x)/pi

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

#define ONE_Q16        65535
#define ONE_HALF_Q31   1073741824
#define C1_Q24         5340306
#define C2_Q26         -3121
#define C3_Q28         -28127105
#define C4_Q30         -10989807
#define C5_Q32         434352049
#define C6_Q34         -1223588557
#define C7_Q36         1157799985

/*============================================================================*/

    .globl    qhmath_atan2_h
    .balign 16
    .type    qhmath_atan2_h,@function
qhmath_atan2_h:
{
    r5 = abs(r0)                    // [y_1 = Q6_R_abs_R(y);]
    r4 = abs(r1)                    // [x_1 = Q6_R_abs_R(x);]
    p0 = cmp.eq(r1,#0)
    if (p0.new) jump:nt .L_input_error // if (x == 0) jump to .L_input_error
}
{
    r2 = and(r1,##32768)            // (x < 0) << 15
    p0 = cmp.gt(r5,r4)              // abs(y) > abs(x)
    allocframe(r29,#16):raw
}
{
    r0 = max(r5,r4)               // r0 = max(abs(x), abs(y))
    r3 = and(r0,##32768)          // (y < 0) << 15
    r1 = #3                       // 3 iteration in Newthon algorithm
}
{
    r18 = min(r5,r4)
    memd(r29+#8) = r19:18
    memd(r29+#0) = r17:16
}
{
    r17:16 = vlsrw(r3:2,#15)              // r16 = (x < 0); r17 = (y < 0)
    r6 = cl0(R0)                          //[ #leading 0s in input                ]
    r3.h = #HI(1963229551)                //[ x.uw[1]  .9142     Q31              ]
}
{
    r19 = p0
    r3.l = #LO(1963229551)                //[ x.uw[1]                             ]
    r2 = #0                               //[ x.uw[0]                             ]
    loop0(.L_newtons_loop, r1)            //[ loop setup                          ]
}
{
    r0 = asl(r0,r6)                       //[ r0=d=norm. input in range [.5,1)    ]
    r3 -= asl(r0,r6)                      //[ d in Q32 can be interpreted as 2*d  ]
                                          //[ if d were in Q31. Use the trick     ]
                                          //[  2-x=-x for x in Q31. Thus,         ]
                                          //[  r3= x= 2.9142 -2*d                 ]
                                          //[       = 0.9142-2*d in Q31           ]
    r8 = #0                               //[ init. loop counter n                ]
}
    .falign
.L_newtons_loop:
{
    r7 = mpyu(r0,r3)                      //[ Calculate a= (d *x.uw[1])>> 32  ..  ]
                                            //[           in Q31-n(=32 +31-n -32)   ]
    r5:4 = asl(r3:2,r8)                   //[ xp = x<<n     Q31(=31-n +n)         ]
}
{
    r3:2-= mpyu(r7,r5)                    //[ x=2*x-d*x*xp  Q31-n-1(=31-n+31-32)  ]
    r8 = add(r8, #1)                      //[ n++                                 ]
    r9 = r8                               //[(n-1)++                              ]
}:endloop0                                //[  start=newtons_LOOP                 ]

{
    r1 = sub(#30, r6)                      //[ scale_factor=30-cl0 ..in [-1,30]    ]
    r0 = asl(r3,r9):sat
                                            //[ For explanation of use of asl:sat op]
                                            //[ refer prototype details under       ]
                                            //[ loop instability                 ]
}
{
    r4 = add(r1,#2)        // scale += 2;
    r3:2 = mpyu(r0,r18)    // frac = Q6_P_mpyu_RR(den.result, y_1);
}
{
    r1 = #0
    r3:2 = asr(r3:2, r4)  // x_1 = (Word32)Q6_P_asr_PR(frac, den.scale);
    r5 = ##C1_Q24         // [c[0] = 5340306;]
}
{
    r4 = asr(r19, #7)     // Shift right variable (is_ay_gt_ax >> 7) in order to set is_ay_gt_ax on
                          // 0 or 1 depndes of (y_1 > x_1) if (y_1 > x_1) => is_ay_gt_ax = 0xFF
    r6 = ##C2_Q26         // [c[1] = -3121;]
    r7 = mpy(r2, r2)      // x_n = Q6_R_mpy_RR(x_n, x)
}
{
    r11:10 = mpy(r2, r5)  // [sum_odd_poly = x_n * c[0];]
    r6 = ##C3_Q28         // [c[2] = -28127105;]
    r13:12 = mpy(r7, r6)  // [sum_even_poly = x_n * c[1];]
}
{
    r5 = mpy(r7, r2)   // [x^3 = x^2 * x;]
    r8 = mpy(r7, r7)   // [x^4 = x^2 * x^2;]
    r9 = ##C4_Q30      // [c[3] = -10989807;]
}
{
    r6 = ##C5_Q32          // [c[4] = 434352049;]
    r11:10 += mpy(r5, r6)  // [sum_odd_poly += x^3*c[2];]
    r13:12 += mpy(r8, r9)  // [sum_even_poly += x^4*c[3];]
}
{
    r5 = mpy(r8, r2)       // [x^5 = x^4 * x;]
    r8 = mpy(r8, r7)       // [x^6 = x^4 * x^2;]
    r9 = ##C6_Q34          // [c[5] = -1223588557;]
}
{
    r11:10 += mpy(r5, r6)          // sum_odd_poly += x^5*c[4];
    r13:12 += mpy(r8, r9)          // sum_even_poly += x^6*c[3];
    r9 = ##C7_Q36
}
{
    r11:10 = add(r11:10, r13:12)   // sum_odd_poly = sum_odd_poly + sum_even_poly;
    r8 = mpy(r8, r2)               // [x^7 = x^6 * x;]
    p0 = cmp.eq(r19, #0)
    r6 = #0
}
{
    r11:10 += mpy(r8, r9)        // sum_odd_poly += x^7 * c[6]
    r0 = #206                    // c[7] = #206
    if (!p0) r6 = ##0x40000000   // offset = 0.5 in Q31
}
{
    p1 = cmp.eq(r19, #0)     // [if ((!is_ay_gt_ax) && (!is_x_ge_0))]
    p1 = cmp.eq(r16, #1)
    if (p1.new) r6 = ##0x80000000 // offset = ##0x80000000; -1.0 in Q31
}
{
    p0 = cmp.eq(r17, #1)     // r17 = !is_y_ge_0 [if (!is_y_ge_0)]
    r1:0 += asr(r11:10, #23) // y = sum_odd_poly + c[8];
    r19:18 = memd(r29+#8)
    r5 = #0
}
{
    r7 = neg(r16)  //  r7 = is_x_ge_0
    r8 = neg(r17)  //  r8 = is_y_ge_0
}
{
    if (p0) r6 = sub(r5, r6)   // if (!is_y_ge_0) offset = -offset;
    r4 ^= xor(r7, r8)          // ((is_y_ge_0 ^ is_x_ge_0) ^is_ay_gt_ax)
}

{
    p0 = cmp.eq(r4, #1)           // [((is_y_ge_0 ^ is_x_ge_0) ^is_ay_gt_ax) == 1]
    if (!p0.new) r0 = add(r6, r0) //[y_1 = offset + y_1;]
}
{
    if (p0) r0 = sub(r6, r0) // [y_1 = offset - y_1;]
}
{
    r17:16 = memd(r29+#0)
    r0 = asrh(r0) // y = y >> 16; back to Q15 format
    dealloc_return
}
.L_input_error:
{
    r0 = r1     // Store #0 in r0 register during invalid input (x = 0)
    jumpr r31
}

    .size   qhmath_atan2_h, .-qhmath_atan2_h
