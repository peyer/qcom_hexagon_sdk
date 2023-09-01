/**=============================================================================
@file
    qhmath_atan_h.S

@brief
    Computes inverse tangent for 32-bit fixed-point input argument.
    Output result is in Q15 signed format.

    Function prototype

        int16_t qhmath_atan_h(int32_t x);

    7th degree polynomial approximation: proj_angle = atan(f)/pi,  where f in [0,1]
    proj_angle = 9.586300e-8 + 3.183e-1*f^1 -4.650e-5*f^2 -1.048e-1*f^3 -
                 1.024e-2*f^4 +1.011e-1*f^5 -7.122e-2*f^6 +1.685e-2*f^7

    Reference C code
        polynom coefficients
        static int32_t c[] = {
                               5340306, -3121, -28127105, -10989807,
                               434352049, -1223588557, 1157799985, 206
                              }; [c1:c7, c0] Qs (in the same order) =[(24:2:36) 31]

        int16_t qhmath_atan_h(int32_t x)
        {
              int32_t x_real, offset, xn;
              int64_t frac, sum;
              result_scale_t den;
              int32_t y = 0;

              // Compute absolute value of x
              x_real = x;

              if (x == 0x80000000)
              {
                  x = 0x80000001;
              }

              x = Q6_R_abs_R(x);

              if (x > 0x0000FFFF)
              {
                  // Inline function newton inverse
                  den = approx_uinv_newt(x ,3);
                  frac = den.result;

                  offset =  den.scale;
                  x = (Word32) (frac >> offset);
                  offset = 0x40000000;
              }
              else
              {
                  offset = 0;
              }

              x = x << 14;
              y = 0;

              xn = x;
              sum = 0;

              sum += Q6_P_mpy_RR(xn, c[0]);   //sum += c[0] * x
              xn = Q6_R_mpy_RR(xn, x);        //x^2..Q28

              sum += Q6_P_mpy_RR(xn, c[1]);   //sum += c[1]*x^2
              xn = Q6_R_mpy_RR(xn, x);        //x^3..Q26

              sum += Q6_P_mpy_RR(xn, c[2]);   //sum += c[2]*x^3
              xn = Q6_R_mpy_RR(xn, x);        //x^4..Q24

              sum += Q6_P_mpy_RR(xn, c[3]);   ///sum += c[3]*x^4
              xn = Q6_R_mpy_RR(xn, x);         //x^5..Q22

              sum += Q6_P_mpy_RR(xn, c[4]);   ///sum += c[4]*x^4
              xn = Q6_R_mpy_RR(xn, x);         //x^6..Q20

              sum += Q6_P_mpy_RR(xn, c[5]);   //sum += c[5]*x^5
              xn = Q6_R_mpy_RR(xn, x);        //x^7..Q18

              sum += Q6_P_mpy_RR(xn, c[5]);   //sum += c[6]*x^6
              sum += Q6_P_mpy_RR(xn, c[6]);   //sum += c[7]*x^7

              y = (Word32)(c[7] + Q6_P_asr_PI(sum, 23));

              if (offset != 0)
              {
                  y = offset - y;
              }

              y = Q6_R_asrh_R(y);

              if (x_real < 0)
              {
                  y = -y;
              }

              return Q6_R_asrh_R(y);
        }

    NOTE: qhmath_atan_h ~ atan(x)/pi
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

    .globl    qhmath_atan_h
    .balign 16
    .type    qhmath_atan_h,@function
qhmath_atan_h:
{
    p0 = cmp.eq(r0, ##0x80000000) // Fixup input argument in order to get valid absolute value of x
    if (p0.new) r0 = ##0x80000001
}
{
    allocframe(#8)
    r3 = abs(r0) // abs(x)
    memd(sp+#-16) = r17:16
}
{
    r17 = r0   // Save signed value of x
    p0 = cmp.gt(r3, ##ONE_Q16) // Jump to label L_X_LT_1 if the input value less then 2^16 in Q16 fixed-point
    r16 = #0     // Store information about (x<0x0000FFFF)
    if (!p0.new) jump:t .L_X_LT_1
}
// Inline Newton inverse
{
    r5 = r3                    // r5 = x, store input argument in r5
    r6 = cl0(r3)               //[ #leading 0s in input                ]
    r3.h = #HI(1963229551)     //[ x.uw[1]  .9142     Q31              ]
    r1 = #3                    // Make 3 iteration in Newton method
}
{
    r3.l = #LO(1963229551)    //[ x.uw[1]                             ]
    r2 = #0                   //[ x.uw[0]                             ]
    loop0(.NEWTONS_LOOP, r1)  //[ loop setup                          ]
}
{
    r16 = #1                  // Store information about (x>0x0000FFFF) this x is related for atan input
    r0 = asl(r5,r6)           //[ r0=d=norm. input in range [.5,1)    ]
    r3-= asl(r5,r6)           //[ d in Q32 can be interpreted as 2*d  ]
    r8 = #0                   //[ if d were in Q31. Use the trick     ]
                              //[  2-x=-x for x in Q31. Thus,         ]
                              //[  r3= x= 2.9142 -2*d                 ]
                              //[       = 0.9142-2*d in Q31           ]
                              //[ init. loop counter n                ]
}
    .falign
.NEWTONS_LOOP:
{
    r7 = mpyu(r0,r3)          //[ Calculate a= (d *x.uw[1])>> 32  ..  ]
    r5:4 = asl(r3:2,r8)       //[           in Q31-n(=32 +31-n -32)   ]
                              //[ xp = x<<n     Q31(=31-n +n)         ]
}
{
    r3:2-= mpyu(r7,r5)        //[ x=2*x-d*x*xp  Q31-n-1(=31-n+31-32)  ]
                              //[ n++                                 ]
                              //[(n-1)++                              ]
                              //[  start=newtons_LOOP                 ]
    r8 = add(r8, #1)
    r9 = r8
}:endloop0
{
    r1 = sub(#30,r6)         //[ scale_factor=30-cl0 ..in [-1,30]    ]
    r0 = asl(r3,r9):sat
}
{
    r3 = asr(r0,r1) //      [r3 = r0 >> scale_factor]
}
.L_X_LT_1:
{
    p0 = cmp.gt(r17, #0)  // Check is the input greater than 0
    r1 = #0
    r3 = asl(r3, #14)     // [x = x << 14;]
    r5 = ##C1_Q24         // [c[0] = 5340306;]
}
{
    if (!p0) r17 = #1     // Set flag to 1 if the input parameter is negative
    r6 = ##C2_Q26         // [c[1] = -3121;]
    r7 = mpy(r3, r3)      // x_n = Q6_R_mpy_RR(x_n, x)
}
{
    r11:10 = mpy(r3, r5)  // [sum_odd_poly = x_n * c[0];]
    r6 = ##C3_Q28         // [c[2] = -28127105;]
    r13:12 = mpy(r7, r6)  // [sum_even_poly = x_n * c[1];]
}
{
    r5 = mpy(r7, r3)   // [x^3 = x^2 * x;]
    r8 = mpy(r7, r7)   // [x^4 = x^2 * x^2;]
    r9 = ##C4_Q30      // [c[3] = -10989807;]
}
{
    r6 = ##C5_Q32          // [c[4] = 434352049;]
    r11:10 += mpy(r5, r6)  // [sum_odd_poly += x^3*c[2];]
    r13:12 += mpy(r8, r9)  // [sum_even_poly += x^4*c[3];]
}
{
    r5 = mpy(r8, r3)       // [x^5 = x^4 * x;]
    r8 = mpy(r8, r7)       // [x^6 = x^4 * x^2;]
    r9 = ##C6_Q34          // [c[5] = -1223588557;]
}
{
    r11:10 += mpy(r5, r6)          // sum_odd_poly += x^5*c[4];
    r13:12 += mpy(r8, r9)          // sum_even_poly += x^6*c[3];
    r9 = ##C7_Q36
}
{
    if (p0) r17 = #0               // Reset flag is the input parameter is positiv
    r11:10 = add(r11:10, r13:12)   // sum_odd_poly = sum_odd_poly + sum_even_poly;
    r8 = mpy(r8, r3)               // [x^7 = x^6 * x;]
    p0 = cmp.eq(r16, #1)
}
{
    r11:10 += mpy(r8, r9) // sum_odd_poly += X^7 * c[6]
    r0 = #206
    if (p0) r2 = ##ONE_HALF_Q31 // offset = 1073741824;(0.5 Q31)
}
{
    p1 = cmp.eq(r17, #1)
    r1:0 += asr(r11:10, #23) // y = sum_odd_poly + c[8];
    r17:16 = memd(sp+#0)
}
{
    if(p0) r0 = sub(r2, r0) // Check condition: x < 65535, Q16 format, if true r0 = 0.5 - r0
    r3 = #0
}
{
    r0 = asrh(r0) // y = y >> 16; back to Q16 format
}
{
    if (p1) r0 = sub(r3, r0) // Check condition: x < 0 if (x < 0) y = -y
    dealloc_return
}
    .size qhmath_atan_h, .-qhmath_atan_h
