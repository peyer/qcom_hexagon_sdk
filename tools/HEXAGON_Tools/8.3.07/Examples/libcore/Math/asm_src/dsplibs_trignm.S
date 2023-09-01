/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "dsplibs_trignm.S"

    /*[*****************************************************************************]*/
    /*[  Function   : Word32 dsp_sin()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  Description: calculate sin(2*PI*x) using 512-LUT and linear interpolation  ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input Q24 in range [-1,1]                                       ]*/
    /*[           - output in Q31                                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 x                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R3                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6 (x<0),  5(x>=0)                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl dsplib_sin
    .type    dsplib_sin, @function
dsplib_sin:
    { R1 = ABS(R0)                          //[ xa = abs(x)                         ]
      P0 = TSTBIT(R0,#31)                   //[ P0 = (x<0)                          ]
      R2 = CONST32(#COSSINTAB)              //[ R2 = COSSINTAB                      ]
      R3 = #0x7FFF                          //[ R3 = 0x7fff                         ]
    }
    { P1 = TSTBIT(R1,#23)                   //[ (xa>>15)&(LUT_SIZE/2)               ]
      R3 = AND(R1,R3)                       //[ frac = (xa&0x07fff)                 ]
      R2 = TABLEIDXW(R1,#8,#15)             //[ index = (xa >> 15) & 0xFF           ]
    }                                       //[ R2 = COSSINTAB[2*index]             ]
    { R0 = MEMH(R2+#2)                      //[ load SINETAB[2*index+1]             ]
      R1 = MEMH(R2+#6)                      //[ load SINETAB[2*index+3]             ]
    }
    { R0 = ASLH(R0)                         //[ SINETAB[index] << 16                ]
      R1 = SUB(R1,R0)                       //[ SINETAB[index+1]-SINTAB[index]      ]
      P0 = XOR(P0,P1)                       //[ P0^P1                               ]
    }
    { R0 += MPY(R1.L,R3.L):<<1:sat          //[ interpolation                       ]
      IF !P0 JUMPR R31                      //[ return                              ]
    }
    { R0 = NEG(R0):sat                      //[ if (x<0) y = L_negate(y)            ]
      JUMPR R31                             //[ return                              ]
    }
    .size    dsplib_sin, .-dsplib_sin


    /*[*****************************************************************************]*/
    /*[  Function   : Word32 dsp_cos()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  Description: calculate cos(2*PI*x) using 512-LUT and linear interpolation  ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input Q24 in range [-1,1]                                       ]*/
    /*[           - output in Q31                                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 x                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R3                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6 (x<0),  5(x>=0)                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .p2align 2
    .p2align 4,,15
    .globl dsplib_cos
    .type    dsplib_cos, @function
dsplib_cos:
    { R1 = ABS(R0)                          //[ xa = abs(x)                         ]
      R2 = CONST32(#COSSINTAB)              //[ R2 = COSSINTAB                      ]
      R3 = #0x7FFF                          //[ R3 = 0x7fff                         ]
    }
    { P0 = TSTBIT(R1,#23)                   //[ (xa>>15)&(LUT_SIZE/2)               ]
      R3 = AND(R1,R3)                       //[ frac = (xa&0x07fff)                 ]
      R2 = TABLEIDXW(R1,#8,#15)             //[ index = (xa >> 15) & 0xFF           ]
    }                                       //[ R2 = &COSSINTAB[2*index]            ]
    { R0 = MEMH(R2)                         //[ load SINETAB[2*index]               ]
      R1 = MEMH(R2+#4)                      //[ load SINETAB[2*index+2]             ]
    }
    { R0 = ASLH(R0)                         //[ SINETAB[index] << 16                ]
      R1 = SUB(R1,R0)                       //[ SINETAB[index+1]-SINTAB[index]      ]
    }
    { R0 += MPY(R1.L,R3.L):<<1:sat          //[ interpolation                       ]
      IF !P0 JUMPR R31                      //[ return                              ]
    }
    { R0 = NEG(R0):sat                      //[ if (x<0) y = L_negate(y)            ]
      JUMPR R31                             //[ return                              ]
    }
    .size    dsplib_cos, .-dsplib_cos




    /*[*****************************************************************************]*/
    /*[  Function   : Word32 dsp_atan()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: calculate arctan(x)/PI using polynomial approximation         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input Q15                                                       ]*/
    /*[           - output in Q31                                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 x                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R4                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 4                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .p2align 2
    .p2align 4,,15
    .globl dsplib_atan
    .type    dsplib_atan, @function
dsplib_atan:
    { R1 = MPY(R0.L,R0.L):<<1:sat           //[ x2 = x*x                            ]
      R2 = CONST32(#(-3049)<<16)            //[ C2                                  ]
      R3 = #863                             //[ C3                                  ]
      R4 = #10388                           //[ C1                                  ]
    }
    { R2 += MPY(R3.L,R1.H):<<1:sat          //[ C2 + C3*x2                          ]
      R4 = ASLH(R4)                         //[ C1 << 16                            ]
    }
    { R4 += MPY(R2.H,R1.H):<<1:sat          //[ C1 + (C2 + C3*x2)*x2                ]
    }
    { R0 = MPY(R4.H,R0.L):<<1:sat           //[ x*(C1 +(C2+C3*x2)*x2)               ]
      JUMPR R31                             //[ return                              ]
    }
    .size    dsplib_atan, .-dsplib_atan



    /*[*****************************************************************************]*/
    /*[  Function   : Word32 approx_atan2_poly7()                                   ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate atan2(y,x)/pi using 7th degree polynomial         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 y                                                 ]*/
    /*[               R1 : Word32 x                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R11                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 22                                                              ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_atan2_poly7
    .type approx_atan2_poly7, @function
approx_atan2_poly7:
    { R7:6 = VABSW(R1:0):sat                //[ r7:6= ax:ay= abs(x:y)               ]
      LOOP0(.atan2_newtonsLOOP,#2)          //[ loop setup                          ]
      P0 = CMP.GE(R0, #0)                   //[ is_y_ge_0 = (y>=0)                  ]
      P1 = CMP.GE(R1, #0)                   //[ is_x_ge_0 = (x>=0)                  ]
    }
    { R3.H = #HI(1963229551)                //[ z.uw[1]  .9142     Q31              ]
      P2 = CMP.GT(R6, R7)                   //[ is_ay_gt_ax = (|x|<|y|)             ]
      IF P2.new R7 = R6                     //[ if |x|<|y| swap x, y                ]
      IF P2.new R6 = R7                     //[ same as above                       ]
    }
    /*--------------------------------------*/
    /* Calculate 1/|x| using the            */
    /* approx_uinv_newt(). The code is      */
    /* inlined below with the 1st loop      */
    /* unrolled to interleave some          */
    /* approx_atan2() related code.         */
    /* upto the point stated in the comments*/
    /*--------------------------------------*/
    { R8 = CL0(R7)                          //[ #leading 0s in |x|                  ]
      P3 = CMP.EQ(R7, #0)                   //[ |x|==0 ?                            ]
      R3.L = #LO(1963229551)                //[ z.uw[1]                             ]
      R10.H= #0x4000                        //[ r10=.5..Q31 ( for +/-pi/2 )         ]
    }
    { R2 = #0                               //[ z.uw[0]                             ]
      R10.L =#0                             //[ r10=.5..Q31                         ]
      R9 = ASL(R7,R8)                       //[ r9=d=|x| normed to range [.5,1)     ]
      R3-= ASL(R7,R8)                       //[ z=aprox for 1/d using Newton-Raphson]
                                            //[ iterative method..the initial value ]
                                            //[ is a linear approx. z_init d in Q32 ]
                                            //[ can be interpreted as 2*d if d were ]
                                            //[ in Q31. Use the trick  2-z=-z for z ]
                                            //[ in Q31.Thus,                        ]
                                            //[ r3= z_init= 2.9142 -2*d             ]
                                            //[           = 0.9142-2*d in Q31       ]
    }
    { R1 = MPYU(R9,R3)                      //[ Calculate a= (d *z.uw[1])>> 32      ]
                                            //[             in Q31-n(=32 +31-n -32) ]
      R0 =#0                                //[ return angle=0                      ]
      IF P3 JUMPR R31                       //[ max(|y|,|x|) is 0 => angle=0        ]
      R8 = SUB(#32,R8)                      //[ scale_factor=sf=32-cl0 ..in [1,32]  ]
    }
    { R3:2-= MPYU(R1,R3)                    //[ z=2*z-d*z^2..Q31-n-1(=31-n+31-32)   ]
      P3 = XOR(P0, P1)                      //[ p3=is_y_ge_0 ^ is_x_ge_0            ]
      R7 = #1                               //[ init. loop counter n                ]
      R11.H=#HI(0x80000000)                 //[ r11=-1..Q31 (for pi; pi = -pi)      ]
    }
    .falign
.atan2_newtonsLOOP:
    { R1 = MPYU(R9,R3)                      //[ Calculate a= (d *z.uw[1])>> 32      ]
                                            //[             in Q31-n(=32 +31-n -32) ]
      R5:4 = ASL(R3:2,R7)                   //[ zp = z<<n     Q31(=31-n +n)         ]
    }
    { R3:2-= MPYU(R1,R5)                    //[ z=2*z-d*z*zp  Q31-n-1(=31-n+31-32)  ]
      R7 = ADD(R7, #1)                      //[ n++                                 ]
    }:endloop0                              //[ start=newtons_LOOP                  ]

    { R3 = ASL(R3, #2):sat                  //[ The approx_uinv_newt() inlined code ]
                                            //[ ends after this cycle. We have      ]
                                            //[ R3= z ~= 1/d at this cycle's end.   ]
                                            //[ For explanation of use of asl:sat op]
                                            //[ refer approx_uinv_newt() prototype  ]
                                            //[ details under loop instability      ]
      P3 = XOR(P3, P2)                      //[ p3=is_y_ge_0^is_x_ge_0^is_ay_gt_ax  ]
      R4.H = #HI(5340306)                   //[ r4 = c1..Q24                        ]
      R11.L=#LO(0x80000000)                 //[ r11=-1..Q31 (for pi; as pi=-pi)     ]
    }
    { R1:0 = MPYU(R3, R6)                   //[ r1:0=y/x=y*z..Q30+sf(=30+sf-Qi+Qi)  ]
      P1 = OR(P2, P1)                       //[ p1=(is_ay_gt_ax|is_x_ge_0)          ]
      R6 = #206                             //[ r6 = c0..Q31                        ]
      R4.L = #LO(5340306)                   //[ r4 = c1..Q24                        ]
    }
    { R1:0 = LSR(R1:0, R8)                  //[ f=y/x..Q30+sf--> f..Q30             ]
      R5.H = #HI(-3121)                     //[ r5 = c2..Q26                        ]
      R7 = MUX(P1,#0,R11)                   //[ IF P1, r7 = 0, otherwise r7=-pi=pi  ]
    }
    { R3:2 = MPY(R0, R4)                    //[ r3:2 = sum =c1*f^1..Q54(=24+30)     ]
      R1 = MPY(R0, R0)                      //[ r1 = f^2..Q28(=30+30-32)            ]
      R5.L = #LO(-3121)                     //[ r5 = c2..Q26                        ]
      R4.H = #HI(-28127105)                 //[ r4 = c3..Q28                        ]
    }
    { R3:2+= MPY(R1, R5)                    //[ sum+=c2*f^2..Q54(=26+28)            ]
      R1 = MPY(R1, R0)                      //[ r1 = f^3..Q26(=28+30-32)            ]
      R4.L = #LO(-28127105)                 //[ r4 = c3..Q28                        ]
      R5.H = #HI(-10989807)                 //[ r5 = c4..Q30                        ]
    }
    { R3:2+= MPY(R1, R4)                    //[ sum+=c3*f^3..Q54(=28+26)            ]
      R1 = MPY(R1, R0)                      //[ r1 = f^4..Q24(=26+30-32)            ]
      R5.L = #LO(-10989807)                 //[ r5 = c4..Q30                        ]
      R4.H = #HI(434352049)                 //[ r4 = c5..Q32                        ]
    }
    { R3:2+= MPY(R1, R5)                    //[ sum+=c4*f^4..Q54(=30+24)            ]
      R1 = MPY(R1, R0)                      //[ r1 = f^5..Q22(=24+30-32)            ]
      R4.L = #LO(434352049)                 //[ r4 = c5..Q32                        ]
      R5.H = #HI(-1223588557)               //[ r5 = c6..Q34                        ]
    }
    { R3:2+= MPY(R1, R4)                    //[ sum+=c5*f^5..Q54(=32+22)            ]
      R1 = MPY(R1, R0)                      //[ r1 = f^6..Q20(=22+30-32)            ]
      R5.L = #LO(-1223588557)               //[ r5 = c6..Q34                        ]
      R4.H = #HI(1157799985)                //[ r4 = c7..Q36                        ]
    }
    { R3:2+= MPY(R1, R5)                    //[ sum+=c6*f^6..Q54(=34+20)            ]
      R1 = MPY(R1, R0)                      //[ r1 = f^7..Q18(=20+30-32)            ]
      R4.L = #LO(1157799985)                //[ r4 = c7..Q36                        ]
      IF P2 R7 = R10                        //[ if (|x|<|y|) r7=.5..Q31 (for pi/2)  ]
    }
    { R3:2+= MPY(R1, R4)                    //[ sum+=c7*f^7..Q54(=36+18)            ]
      R1 =#0                                //[ clear r1 for use in lsr op          ]
      R0 = R6                               //[ r0=c0..Q31                          ]
    }
    {                                       //[ proj_angle = c0+ c1*f+ ..+c7*f^7    ]
                                            //[            = c0 + sum.              ]
                                            //[ First do the conversion             ]
      R1:0+= LSR(R3:2,#23)                  //[ sum..Q54 --> sum..Q31 before adding ]
                                            //[ Calculate the multiple of 90 deg. to]
                                            //[ re-rotate the projected angle in 1st]
                                            //[ octant back to the original quadrant]
                                            //[ Treating is_XXXX as 0/1,            ]
                                            //[ r7 = offset=                        ]
                                            //[ .5*(2*is_y_ge_0-1)(is_ay_gt_ax + 2* ]
                                            //[    !(is_ay_gt_ax|is_x_ge_0)).       ]
                                            //[ We already have offset in {0,.5,1}  ]
                                            //[ (in Q31), it just needs the correct ]
                                            //[ sign.                               ]
      IF !P0 R7 = SUB(R1, R7)               //[ if (y< 0) offset=-offset            ]
    }
    { IF  P3 R0 = SUB(R7, R0)               //[ r0 = actual_angle=                  ]
      IF !P3 R0 = ADD(R7, R0)               //[    (1-2*s3)*proj_angle + offset..Q31]
                                            //[ NOTE:                               ]
                                            //[ s3=(is_y_ge_0^is_x_ge_0)^is_ay_gt_ax]
                                            //[ considered as binary 0/1            ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_atan2_poly7, .-approx_atan2_poly7

