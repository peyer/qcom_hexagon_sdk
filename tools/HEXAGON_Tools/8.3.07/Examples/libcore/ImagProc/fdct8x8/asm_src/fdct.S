/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 


    .file "fdct.S"

// DEFINE CONSTANTS
#define  COS1_16_Q15               32138
#define  COS2_16_Q15               30274
#define  COS3_16_Q15               27246
#define  COS4_16_Q15               23170
#define  COS5_16_Q15               18205
#define  COS6_16_Q15               12540
#define  COS7_16_Q15                6393

    /*[*****************************************************************************]*/
    /*[  Function   : void fDCT()                                                   ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform forwad DCT transform on 8x8 blocks                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and output are aligned by 8bytes                          ]*/
    /*[           - can be used for inplaced-inplementation, i.e., output = input   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *input                                            ]*/
    /*[               R1 : int nblocks                                              ]*/
    /*[               R2 : Word16 *output                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 96*n + 19                                                       ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl fDCT
    .type	fDCT, @function
fDCT:
    { P0 = CMP.GT(R1,#0)                    //[ if !(num_blks>0)                    ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      IF  P0.new R3 = R2                    //[ setup write pointer                 ]
      R6 = ASL(R1,#3)                       //[ 8*NumofBlocks for horizontal loop   ]
    }
    { R29 = ADD(R29, #-4*8)                 //[ reserve stack                       ]
      MEMD(R29+#(24-4*8)) = R23:22          //[ callee-saved registers              ]
      R22.H = #COS1_16_Q15                  //[ C1                                  ]
      R23.H = #COS5_16_Q15                  //[ C5                                  ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R22.L = #COS7_16_Q15                  //[ R22 = C1||C7                        ]
      R23.L = #COS3_16_Q15                  //[ R23 = C5||C3                        ]
      R1 = ASL(R1,#2)                       //[ LC = 4*blocks for vertical loop     ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R20.H = #COS4_16_Q15                  //[ C4                                  ]
      R21.H = #COS2_16_Q15                  //[ C2                                  ]
      P3 = SP1LOOP0(.fDCT_horizontalLOOP,R6)//[ setup horizontal loop lc0=8*nblock  ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R20.L = #COS4_16_Q15                  //[ R20 = C4||C4                        ]
      R21.L = #COS6_16_Q15                  //[ R21 = C2||C6                        ]
      R5:4 = MEMD(R0++#8)                   //[p]load x0-3                          ]
    }

    .falign
.fDCT_horizontalLOOP:
    { R7:6 = MEMD(R0++#8)                   //[[2]load x4-7                         ]
      R5 = COMBINE(R5.L,R5.H)               //[[2]R5:4 = x2|x3|x1|x0                ]
      R8 = CMPY(R8,R20):<<1:rnd:sat         //[[3] w0|w4                            ]
      R9 = CMPY(R9,R22):<<1:rnd:sat         //[[3] w1|w7                            ]
    }
    { R7 = R6                               //[[2]R7:6 = x5|x4|x6|x7                ]
      R6 = COMBINE(R7.L,R7.H)               //[[2]R7:6 = x5|x4|x6|x7                ]
      R10 = CMPY(R10,R21):<<1:rnd:sat       //[[3] w2|w6                            ]
      R11 = CMPY(R11,R23):<<1:rnd:sat       //[[3] w5|w3                            ]
    }
    { R5:4 = VADDH(R5:4,R7:6)               //[[2]R5:4 = s3|s2|s1|s0                ]
      R7:6 = VSUBH(R5:4,R7:6)               //[[2]R7:6 = s7|s6|s5|s4                ]
      R12 = COMBINE(R9.H,R8.H)              //[[3]pack y1|y0                        ]
      R13 = COMBINE(R11.L,R10.H)            //[[3]pack y3|y2                        ]
    }
    { R5 = COMBINE(R7.L,R6.L)               //[[2]R9:8 = t5|t4|t1|t0                ]
      R6 = R5                               //[[2]R6 = t3|t2                        ]
      R7 = COMBINE(R6.H,R7.H)               //[[2]R7 = s5|s7                        ]
      IF P3 MEMD(R3++#8) = R13:12           //[[3]save                              ]
    }
    { R7 = CMPY(R7,R20*):<<1:rnd:sat        //[[2]R7 = t7|t6                        ]
      R12 = COMBINE(R11.H,R8.L)             //[[3]pack y5|y4                        ]
      R13 = COMBINE(R9.L,R10.L)             //[[3]pack y7|y6                        ]
    }
    { R5:4 = MEMD(R0++#8)                   //[[1]load x0-3                         ]
      R9:8   = VADDH(R5:4,R7:6)             //[[2]R9:8   = u5|u4|u1|u0              ]
      R11:10 = VSUBH(R5:4,R7:6)             //[[2]R11:10 = u7|u6|u3|u2              ]
      IF P3 MEMD(R3++#8) = R13:12           //[[3]save                              ]
    }:endloop0

    { R8 = CMPY(R8,R20):<<1:rnd:sat         //[[e] w0|w4                            ]
      R9 = CMPY(R9,R22):<<1:rnd:sat         //[[e] w1|w7                            ]
      R4 = MEMW(R2+#0*8*2)                  //[[p]load x0                           ]
      R5 = MEMW(R2+#1*8*2)                  //[[p]load x1                           ]
    }
    { R14 = CMPY(R10,R21):<<1:rnd:sat       //[[e] w2|w6                            ]
      R15 = CMPY(R11,R23):<<1:rnd:sat       //[[e] w5|w3                            ]
      R7 = MEMW(R2+#2*8*2)                  //[[p]load x2                           ]
      R6 = MEMW(R2+#3*8*2)                  //[[p]load x3                           ]
    }
    { R12 = COMBINE(R9.H,R8.H)              //[[e]pack y1|y0                        ]
      R13 = COMBINE(R15.L,R14.H)            //[[e]pack y3|y2                        ]
      R10 = MEMW(R2+#4*8*2)                 //[[p]load x4                           ]
      R11 = MEMW(R2+#5*8*2)                 //[[p]load x5                           ]
    }
    { MEMD(R3++#8) = R13:12                 //[[e]save                              ]
      R12 = COMBINE(R15.H,R8.L)             //[[e]pack y5|y4                        ]
      R13 = COMBINE(R9.L,R14.L)             //[[e]pack y7|y6                        ]
      LOOP0(.fDCT_verticalLOOP,R1)          //[ setup vertical loop                 ]
    }
    { MEMD(R3++#8) = R13:12                 //[[e]save                              ]
      R28 = R20                             //[ R28 = C4|C4 in Q15                  ]
      R21:20 = VASRH(R21:20,#2)             //[ Ck in Q13                           ]
      R23:22 = VASRH(R23:22,#2)             //[ Ck in Q13                           ]
    }
    { R9 = MEMW(R2+#6*8*2)                  //[[p]load x6                           ]
      R8 = MEMW(R2+#7*8*2)                  //[[p]load x7                           ]
      R7:6 = VADDH(R7:6,R11:10)             //[[p]s3 : s2                           ]
      R11:10 = VSUBH(R7:6,R11:10)           //[[p]S7 : s6                           ]
    }
    { R5:4 = VADDH(R5:4,R9:8)               //[[p]s1 : s0                           ]
      R9:8 = VSUBH(R5:4,R9:8)               //[[p]s5 : s4                           ]
      R0 = ADD(R2,#4)                       //[ set input pointer for vertical pass ]
      R1 = ADD(R2,#8*2)                     //[ pointer to next row
    }
    { R5:4 = VADDH(R5:4,R7:6)               //[[p]u1 : 0                            ]
      R7:6 = VSUBH(R5:4,R7:6)               //[[p]u3 : u2                           ]
      R12 = COMBINE(R9.L,R11.L)             //[[p]s5|s7                             ]
      R13 = COMBINE(R9.H,R11.H)             //[[p]s5|s7                             ]
    }
    { R8 = COMBINE(R10.L,R8.L)              //[[p]t5|t4                             ]
      R9 = COMBINE(R10.H,R8.H)              //[[p]t5|t4                             ]
      R10 = CMPY(R12,R28*):<<1:rnd:sat      //[[p]t7|t6                             ]
      R11 = CMPY(R13,R28*):<<1:rnd:sat      //[[p]t7|t6                             ]
    }
    { R9:8   = VADDH(R9:8,R11:10)           //[[p]u5|u4                             ]
      R11:10 = VSUBH(R9:8,R11:10)           //[[p]u7|u6                             ]
      R4 = COMBINE(R5.L,R4.L)               //[[p]u1|u0                             ]
      R5 = COMBINE(R5.H,R4.H)               //[[p]u1|u0                             ]
    }
    { R6 = COMBINE(R7.L,R6.L)               //[[p]u3|u2                             ]
      R7 = COMBINE(R7.H,R6.H)               //[[p]u3|u2                             ]
      R12 = CMPY(R4,R20):<<1:rnd:sat        //[[p]w0|w4                             ]
      R13 = CMPY(R5,R20):<<1:rnd:sat        //[[p]w0|w4                             ]
    }
    { R2 = ADD(R2,#4)                       //[ setup write pointer                 ]
      R14 = CMPY(R8,R22):<<1:rnd:sat        //[[p]w1|w7                             ]
      R15 = CMPY(R9,R22):<<1:rnd:sat        //[[p]w1|w7                             ]
    }

    .falign 
.fDCT_verticalLOOP:
    { R9 = MEMW(R0+#6*8*2)                  //[[1]load x6                           ]
      R8 = MEMW(R0+#7*8*2)                  //[[1]load x7                           ]
      R16 = CMPY(R6,R21):<<1:rnd:sat        //[[2]w2|w6                             ]
      R17 = CMPY(R7,R21):<<1:rnd:sat        //[[2]w2|w6                             ]
    }
    { R10 = MEMW(R0+#4*8*2)                 //[[1]load x4                           ]
      R11 = MEMW(R0+#5*8*2)                 //[[1]load x5                           ]
      R18 = CMPY(R10,R23):<<1:rnd:sat       //[[2]w5|w3                             ]
      R19 = CMPY(R11,R23):<<1:rnd:sat       //[[2]w5|w3                             ]
    }
    { R7 = MEMW(R0+#2*8*2)                  //[[1]load x2                           ]
      R6 = MEMW(R0+#3*8*2)                  //[[1]load x3                           ]
      R13:12 = PACKHL(R13,R12)              //[[2]w0 : w4                           ]
      R15:14 = PACKHL(R15,R14)              //[[2]w1 : w7                           ]
    }
    { R4 = MEMW(R0++#4)                     //[[1]load x0                           ]
      R5 = MEMW(R0+#1*8*2)                  //[[1]load x1                           ]
      R7:6 = VADDH(R7:6,R11:10)             //[[1]s3 : s2                           ]
      R11:10 = VSUBH(R7:6,R11:10)           //[[1]S7 : s6                           ]
    }
    { R5:4 = VADDH(R5:4,R9:8)               //[[1]s1 : s0                           ]
      R9:8 = VSUBH(R5:4,R9:8)               //[[1]s5 : s4                           ]
      MEMW(R2+#(0*8*2-4)) = R13             //[[2]save y0                           ]
      R13 = COMBINE(R17.H,R16.H)            //[[2]w2                                ]
    }
    { R5:4 = VADDH(R5:4,R7:6)               //[[1]u1 : 0                            ]
      R7:6 = VSUBH(R5:4,R7:6)               //[[1]u3 : u2                           ]
      MEMW(R2+#(1*8*2-4)) = R15             //[[2]save y1                           ]
      R15 = COMBINE(R19.L,R18.L)            //[[2]w3                                ]
    }
    { R9:8 = PACKHL(R10,R8)                 //[[1]t5|t4                             ]
      R11:10 = PACKHL(R9,R11)               //[[1]s5|s7                             ]
      MEMW(R2+#(2*8*2-4)) = R13             //[[2]save y2                           ]
      R18 = COMBINE(R19.H,R18.H)            //[[2]w5                                ]
    }
    { R10 = CMPY(R10,R28*):<<1:rnd:sat      //[[1]t7|t6                             ]
      R11 = CMPY(R11,R28*):<<1:rnd:sat      //[[1]t7|t6                             ]
      MEMW(R2+#(3*8*2-4)) = R15             //[[2]save y3                           ]
      R16 = COMBINE(R17.L,R16.L)            //[[2]w6                                ]
    }
    { R9:8 = VADDH(R9:8,R11:10)             //[[1]u5|u4                             ]
      R11:10 = VSUBH(R9:8,R11:10)           //[[1]u7|u6                             ]
      MEMW(R2+#(4*8*2-4)) = R12             //[[2]save y4                           ]
      R3:2 = COMBINE(R2,R0)                 //[[2]update write pointer              ]
    }
    { R5:4 = PACKHL(R5,R4)                  //[[1]u1|u0                             ]
      R7:6 = PACKHL(R7,R6)                  //[[1]u3|u2                             ]
      P0 = CMP.EQ(R0,R1)                    //[[1]complete a row ?                  ]
      MEMW(R3+#(5*8*2-4)) = R18             //[[2]save y5                           ]
    }
    { R12 = CMPY(R4,R20):<<1:rnd:sat        //[[1]w0|w4                             ]
      R13 = CMPY(R5,R20):<<1:rnd:sat        //[[1]w0|w4                             ]
      IF P0 R0 = ADD(R0,#7*8*2)             //[[1] update to next block             ]
      MEMW(R3+#(6*8*2-4)) = R16             //[[2]save y6                           ]
    }
    { R14 = CMPY(R8,R22):<<1:rnd:sat        //[[1]w1|w7                             ]
      R15 = CMPY(R9,R22):<<1:rnd:sat        //[[1]w1|w7                             ]
      IF P0 R1 = ADD(R0,#8*2)               //[[1] update to next block             ]
      MEMW(R3+#(7*8*2-4)) = R14             //[[2]save y7                           ]
    }:endloop0

    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore allee-saved registers       ]
      R29 = ADD(R29,#4*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	fDCT, .-fDCT


