/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "tc.S"

#define INITIAL_VALUE     1024
#define THRESHOLD         22

    /*[*****************************************************************************]*/
    /*[  Function   : void turbo_dec_alpha()                                        ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calculate alpha-loop of MAP decoder                           ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - alpha[] is aligned by 8-bytes                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *xLin                                             ]*/
    /*[               R1 : Word16 *p                                                ]*/
    /*[               R2 : Word16 *alpha                                            ]*/
    /*[               R3 : int nFrameSize                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 13 * (nFrameSize-1) + 11                                        ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl turbo_dec_alpha
    .type	turbo_dec_alpha, @function
turbo_dec_alpha:
    { R29 = ADD(R29,#-6*8)                  //[ allocate stack                      ]
      MEMD(R29+#(40-6*8)) = R27:26          //[ callee-saved registers              ]
      R14 = ADD(R3,#-1)                     //[ nFrameSize-1                        ]
      R8 = #THRESHOLD                       //[ THRESHOLD                           ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R5:4 = #0                             //[ initialize alpha0-3                 ]
      R7:6 = #0                             //[ initialize alph4-7                  ]
      R10 = MEMH(R0++#2)                    //[ [p]xLin[j]                          ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R4.L = #INITIAL_VALUE                 //[ initialize alpha0                   ]
      R25:24 = VSPLATH(R8)                  //[ R25:24 = CONSTANTS THRESHOLD        ]
      R11 = MEMUH(R1++#2)                   //[ [p]load p[j]                        ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R28.H = #HI(TurboTable)               //[ R28 = TurboTable                    ]
      R12 = ADD(R10.L,R11.L):sat            //[ [p]saturate( xLin[j] + p[j])        ]
      R3.H = #1                             //[ R3 = 1| 1                           ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R10 = COMBINE(R10.L,R12.L)            //[[p]R11:10 = lambd3|2|1|0             ]
      R12 = COMBINE(R11.L,R11.H)            //[[p]R13:12 = lambd0|1|2|3             ]
      R13 = COMBINE(R12.L,R10.L)            //[[p]R13:12 = lambd0|1|2|3             ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R28.L = #LO(TurboTable)               //[ R28 = TurboTable                    ]
      R3.L = #1                             //[ R3 = 1| 1                           ]
      R27:26 = #0                           //[ R27:26 = CONSTANT 0                 ]
    }
    { LOOP0(.alpha_loop, R14)               //[ setup loop0: lc0 = nFrameSize-1     ]
    }

    .falign
.alpha_loop:
    { MEMD(R2++#8) = R5:4                   //[ save alpha0-3[i]                    ]
      R15:14 = VADDH(R5:4,R11:10)           //[ alpha[3|2|1|0] + lambda[3|2|1|0]    ]
      R17:16 = VADDH(R7:6,R13:12)           //[ alpha[7|6|5|4] + lambda[0|1|2|3]    ]
    }
    { MEMD(R2++#8) = R7:6                   //[ save alpha4-7[i]                    ]
      R9:8   = VSUBH(R15:14,R17:16)         //[ max_start(alpha[3-0] + lambda[3-0], ]
      R19:18 = VABSDIFFH(R15:14,R17:16)     //[           alpha[7-4] + lambda[0-3]) ]
      R21:20 = COMBINE(R28,R28)             //[ R21=R20 = TurboTable                ]
    }
    { R19:18 = VMINH(R19:18,R25:24)         //[ min(z, threshold)                   ]
      P0 = VCMPH.GT(R9:8,R27:26)            //[ sxy > 0                             ]
      R23:22 = COMBINE(R28,R28)             //[ R23=R22 = TurboTable                ]
      R6 = VADDH(R6,R10)                    //[ alpha[7|6|5|4] + lambda[3|2|1|0]    ]
    }
    { R21:20 += VMPYH(R18,R3)               //[ address of correction factor in LUT ]
      R23:22 += VMPYH(R19,R3)               //[ address of correction factor in LUT ]
      R10 = MEMH(R0++#2)                    //[[1]load xLin[j]                      ]
      R7 = VADDH(R7,R11)                    //[ alpha[7|6|5|4] + lambda[3|2|1|0]    ]
    }
    { R20 = MEMB(R20)                       //[ correction factor,(alpha0)          ]
      R21 = MEMB(R21)                       //[ correction factor,(alpha2)          ]
      R5:4 = VADDH(R5:4,R13:12)             //[ alpha[3|2|1|0] + lambda[0|1|2|3]    ]
      R15:14 = VMUX(P0,R15:14,R17:16)       //[ max() (alpha[0,2,4,6])              ]
    }
    { R22 = MEMB(R22)                       //[ correction factor, (alpha4)         ]
      R23 = MEMB(R23)                       //[ correction factor, (alpha6)         ]
      R19:18 = VABSDIFFH(R5:4,R7:6)         //[ max_start(alpha[3-0] + lambda[0-3], ]
      R9:8 = VSUBH(R5:4,R7:6)               //[           alpha[7-4] + lambda[3-0]) ]
    }
    { R19:18 = VMINH(R19:18,R25:24)         //[ min(z, threshold)                   ]
      R21:20 = VTRUNEWH(R23:22,R21:20)      //[ pack correction factors             ]
      R17:16 = COMBINE(R28,R28)             //[ R17=R16 = TurboTable                ]
      R23:22 = COMBINE(R28,R28)             //[ R23=R22 = TurboTable                ]
    }
    { R17:16 += VMPYH(R18,R3)               //[ address of correction factor in LUT ]
      R23:22 += VMPYH(R19,R3)               //[ address of correction factor in LUT ]
      R11 = MEMUH(R1++#2)                   //[ [1]load p[j]                        ]
    }
    { R16 = MEMB(R16)                       //[ correction factor, (alpha1)         ]
      R17 = MEMB(R17)                       //[ correction factor, (alpha3)         ]
      P0 = VCMPH.GT(R9:8,R27:26)            //[ sxy > 0                             ]
      R15:14 = VADDH(R15:14,R21:20)         //[ alpha6|alpha4|alpha2|alpha0         ]
    }
    { R18 = MEMB(R22)                       //[ correction factor, (alpha5)         ]
      R19 = MEMB(R23)                       //[ correction factor, (alpha7)         ]
      R5:4 = VMUX(P0,R5:4,R7:6)             //[ max() (alpha[1,3,5,7])              ]
      R12 = ADD(R10.L,R11.L):sat            //[ saturate( xLin[j] + [j])            ]
    }
    { R17:16 = VTRUNEWH(R19:18,R17:16)      //[
      R10 = COMBINE(R10.L,R12.L)            //[[1]R11:10 = lambda3|2|1|0            ]
      R12 = COMBINE(R11.L,R11.H)            //[[1]R13:12 = lambda0|1|2|3            ]
      R13 = COMBINE(R12.L,R10.L)            //[[1]R13:12 = lambda0|1|2|3            ]
    }
    { R5:4 = VADDH(R5:4,R17:16)             //[ alpha7|alpha5|alpha3|alpha1         ]
    }
    { R5:4 = PACKHL(R4,R14)                 //[ R5:4 = alpha3|alpha2|alpha1|alpha0  ]
      R7:6 = PACKHL(R5,R15)                 //[ R7:6 = alpha7|alpha6|alpha5|alpha4  ]
    }:endloop0

    { MEMD(R2++#8) = R5:4                   //[ save alpha0-3[]                     ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { MEMD(R2++#8) = R7:6                   //[ save alpha4-7[]                     ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
      R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	turbo_dec_alpha, .-turbo_dec_alpha





    /*[*****************************************************************************]*/
    /*[  Function   : void turbo_dec_beta_extrinsic()                               ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calculate beta-loop and extrinsics of MAP decoder             ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - alpha[] and beta[] are aligned by 8-bytes                       ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *xLin                                             ]*/
    /*[               R1 : Word16 *p                                                ]*/
    /*[               R2 : Word16 *alpha                                            ]*/
    /*[               R3 : Word16 *L_write                                          ]*/
    /*[               R4 : Word16 *beta                                             ]*/
    /*[               R5 : int nFrameSize                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28, R30-R31                                       ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 56                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 35 * nFrameSize + 11                                            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl turbo_dec_beta_extrinsic
    .type	turbo_dec_beta_extrinsic, @function
turbo_dec_beta_extrinsic:
    { R29 = ADD(R29,#-7*8)                  //[ allocate stack                      ]
      MEMD(R29+#(48-7*8)) = R31:30          //[ callee-saved registers              ]
      P3 =SP1LOOP0(.beta_extrinsic_loop, R5)//[ setup loop0: lc0=FramSize           ]
      R5 += ADD(R5,#-2)                     //[ 2*(nFrameSize-1)
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R0 = ADD(R0,R5)                       //[ &xLin[FRAME_SIZE-1]                 ]
      R1 = ADD(R1,R5)                       //[ &p[FRAME_SIZE-1]                    ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R24 = #THRESHOLD                      //[ THRESHOLD                           ]
      R2 += ASL(R5,#3)                      //[ &alpha[8*(FRAME_SIZE-1)]            ]
      R3 = ADD(R3,R5)                       //[ L_write[FRAME_SIZE-1]               ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R25:24 = VSPLATH(R24)                 //[ R25:24 = THRESHOLD                  ]
      R28.H = #HI(TurboTable)               //[ R28 = TurboTable                    ]
      R31:30 = #0
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R28.L = #LO(TurboTable)               //[ R28 = TurboTable                    ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R15:14 = MEMD(R4+#0)                  //[ load beta3|beta2|beta1|beta0        ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R17:16 = #0                           //[ set R17:16 = 0 (correction factors) ]
      R5:4   = MEMD(R4+#8)                  //[ load beta7|beta6|beta5|beta4        ]
    }

    .falign
.beta_extrinsic_loop:
    { R11:10 = MEMD(R2++#-16)               //[ load alpha0-3                       ]
      R13:12 = MEMD(R2+#8)                  //[ load alpha4-7                       ]
      R5:4 = VADDH(R5:4,R17:16)             //[[2]beta7|beta6|beta5|beta4           ]
    }
    { R11 = R13                             //[ R11:10 = alpha7|alpha6|alpha1|alpha0]
      R13 = R11                             //[ R13:12 = alpha3|alpha2|alpha5|alpha4]
      R5:4 = VTRUNEWH(R5:4,R15:14)          //[ beta6|beta4|beta2|beta0             ]
      R7:6 = VTRUNOWH(R5:4,R15:14)          //[ beta7|beta5|beta3|beta1             ]
    }
    { R15:14 = VADDH(R5:4,R11:10)           //[ beta[6|4|2|0] + alpha[7|6|1|0]      ]
      R17:16 = VADDH(R7:6,R13:12)           //[ beta[7|5|3|1] + alpha[3|2|5|4]      ]
      R27 = VADDH(R5,R13)                   //[ beta[6|4|2|0] + alpha[3|2|5|4]      ]
      R26 = VADDH(R4,R12)                   //[ beta[6|4|2|0] + alpha[3|2|5|4]      ]
    }
    { R9:8 = VSUBH(R15:14,R17:16)           //[ max*( beta[6|4|2|0]+alpha[7|6|1|0], ]
      R19:18 = VABSDIFFH(R15:14,R17:16)     //[       beta[7|5|3|1]+alpha[3|2|5|4]  ]
      R12 = VADDH(R6,R10)                   //[ beta[7|5|3|1] + alpha[7|6|1|0]      ]
      R10 = MEMH(R0++#-2)                   //[ lambda1 = xLin[j]                   ]
    }
    { R19:18 = VMINH(R19:18,R25:24)         //[ min(z,threshold)                    ]
      P0 = VCMPH.GT(R9:8,R31:30)            //[ sxy > 0                             ]
      R13 = VADDH(R7,R11)                   //[ beta[7|5|3|1] + alpha[7|6|1|0]      ]
      R11 = MEMUH(R1++#-2)                  //[ lambda3|lambda2 = 0|p[j]            ]
    }
    { R21:20 = VZXTHW(R18)                  //[                                     ]
      R22 = ZXTH(R19)                       //[                                     ]
      R23 = ASRH(R19)                       //[                                     ]
      R19:18 = VABSDIFFH(R27:26,R13:12)     //[ max*( beta[6|4|2|0]+alpha[3|2|5|4], ]
                                            //[       beta[7|5|3|1]+alpha[7|6|1|0]  ]
    }
    { R15:14 = VMUX(P0,R15:14,R17:16)       //[ max() (tmp_d|c|b|a, "1")            ]
      R19:18 = VMINH(R19:18,R25:24)         //[                                     ]
      R20 = ADD(R28,R20)                    //[ address of correction factor in LUT ]
      R21 = ADD(R28,R21)                    //[ address of correction factor in LUT ]
    }
    { R20 = MEMB(R20)                       //[ correction factor, (tmp_a, "1")     ]
      R21 = MEMB(R21)                       //[ correction factor, (tmp_b, "1")     ]
      R22 = ADD(R28,R22)                    //[ address of correction factor in LUT ]
      R23 = ADD(R28,R23)                    //[ address of correction factor in LUT ]
    }
    { R9:8 = VSUBH(R27:26,R13:12)           //[ max*( beta[6|4|2|0]+alpha[3|2|5|4], ]
                                            //[       beta[7|5|3|1]+alpha[7|6|1|0]  ]
      R17:16 = VZXTHW(R18)                  //[                                     ]
      R22 = MEMB(R22)                       //[ correction factor, (tmp_c, "1")     ]
      R23 = MEMB(R23)                       //[ correction factor, (tmp_d, "1")     ]
    }
    { R21:20 = VTRUNEWH(R23:22,R21:20)      //[ pack correction factors             ]
      R19:18 = VZXTHW(R19)                  //[                                     ]
      R16 = ADD(R28,R16)                    //[ address of correction factor in LUT ]
      R17 = ADD(R28,R17)                    //[ address of correction factor in LUT ]
    }
    { R16 = MEMB(R16)                       //[ correction factor, (tmp_a, "0")     ]
      R17 = MEMB(R17)                       //[ correction factor, (tmp_b, "0")     ]
      R18 = ADD(R28,R18)                    //[ address of correction factor in LUT ]
      R19 = ADD(R28,R19)                    //[ address of correction factor in LUT ]
    }
    { P0 = VCMPH.GT(R9:8,R31:30)            //[ sxy > 0                             ]
      R23:22 = VADDH(R15:14,R21:20)         //[ tmp_d|c|b|a ("1")
      R18 = MEMB(R18)                       //[ correction factor, (tmp_c, "0")     ]
      R19 = MEMB(R19)                       //[ correction factor, (tmp_d, "0")     ]
    }
    { R27:26 = VMUX(P0,R27:26,R13:12)       //[ max() (tmp_d|c|b|a, "0")            ]
      R16 = COMBINE(R17.L,R16.L)            //[ pack correction factors             ]
      R17 = COMBINE(R19.L,R18.L)            //[ pack correction factors             ]
      R12 = ADD(R10.L,R11.L):sat            //[ lambda[0]=saturate( xLin[j] + [j])  ]
    }
    { R27:26 = VADDH(R27:26,R17:16)         //[ tmp_d|c|b|a ("0")                   ]
      R10 = COMBINE(R10.L,R12.L)            //[ R11:10 = lambda[3|2|1|0]            ]
      R12 = COMBINE(R11.L,R11.H)            //[ R13:12 = lambda[0|1|2|3]            ]
      R13 = COMBINE(R12.L,R10.L)
    }
    { R23 = COMBINE(R26.L,R26.H)            //[ R23:22 = tmp_a|b("0"): tmp_b|a("1") ]
      R26 = COMBINE(R23.L,R23.H)            //[ R27:26 = tmp_d|c("0"): tmp_c|d("1") ]
      R15:14 = VADDH(R5:4,R11:10)           //[ beta[6|4|2|0] + lambda[3|2|1|0]     ]
      R17:16 = VADDH(R7:6,R13:12)           //[ beta[7|5|3|1] + lambda[0|1|2|3]     ]
    }
    { R9:8 = VSUBH(R27:26,R23:22)           //[                                     ]
      R19:18 = VABSDIFFH(R27:26,R23:22)     //[                                     ]
      R5 = VADDH(R5,R13)                    //[ beta[6|4|2|0] + lambda[0|1|2|3]     ]
      R4 = VADDH(R4,R12)                    //[ beta[6|4|2|0] + lambda[0|1|2|3]     ]
    }
    { R19:18 = VMINH(R19:18,R25:24)         //[                                     ]
      P0 = VCMPH.GT(R9:8,R31:30)            //[                                     ]
      R7 = VADDH(R7,R11)                    //[ beta[7|5|3|1] + lambda[3|2|1|0]     ]
      R6 = VADDH(R6,R10)                    //[ beta[7|5|3|1] + lambda[3|2|1|0]     ]
    }
    { R27:26 = VMUX(P0,R27:26,R23:22)       //[ max() (alpha0,2,4,6)                ]
      R21:20 = VZXTHW(R18)                  //[                                     ]
      R22 = ZXTH(R19)                       //[                                     ]
      R23 = ASRH(R19)                       //[                                     ]
    }
    { R20 = ADD(R28,R20)                    //[ address of correction factor in LUT ]
      R21 = ADD(R28,R21)                    //[ address of correction factor in LUT ]
      R22 = ADD(R28,R22)                    //[ address of correction factor in LUT ]
      R23 = ADD(R28,R23)                    //[ address of correction factor in LUT ]
    }
    { R20 = MEMB(R20)                       //[ correction factor of max* (alpha0)  ]
      R21 = MEMB(R21)                       //[ correction factor of max* (alpha2)  ]
      R9:8 = VSUBH(R15:14,R17:16)           //[ for new beta[3|2|1|0]               ]
      R19:18 = VABSDIFFH(R15:14,R17:16)     //[ for new beta[3|2|1|0]               ]
    }
    { R22 = MEMB(R22)                       //[ correction factor of max* (alpha4)  ]
      R23 = MEMB(R23)                       //[ correction factor of max* (alpha6)  ]
      R19:18 = VMINH(R19:18,R25:24)         //[ for new beta[3|2|1|0]               ]
      P0 = VCMPH.GT(R9:8,R31:30)            //[ for new beta[3|2|1|0]               ]
    }
    { R13:12 = VTRUNEWH(R23:22,R21:20)      //[ pack correction factors for         ]
                                            //[        tmp_a|c("0") : tmp_c|a("1")  ]
      R21:20 = VZXTHW(R18)                  //[ for new beta[3|2|1|0]               ]
      R22 = ZXTH(R19)                       //[ for new beta[3|2|1|0]               ]
      R23 = ASRH(R19)                       //[ for new beta[3|2|1|0]               ]
    }
    { R27:26 = VADDH(R27:26,R13:12)         //[ new tmp_a|c("0"): tmp_c|a("1")      ]
      R15:14 = VMUX(P0,R15:14,R17:16)       //[ max() (beta[0,1,2,3])               ]
      R20 = ADD(R28,R20)                    //[ address of correction factor in LUT ]
      R21 = ADD(R28,R21)                    //[ address of correction factor in LUT ]
    }
    { R20 = MEMB(R20)                       //[ correction factor, (beta0)          ]
      R21 = MEMB(R21)                       //[ correction factor, (beta1)          ]
      R22 = ADD(R28,R22)                    //[ address of correction factor in LUT ]
      R23 = ADD(R28,R23)                    //[ address of correction factor in LUT ]
    }
    { R27:26 = VADDH(R27:26,R11:10)         //[R27=tmp_a(0)+lambda3|tmp_c(0)+lambda2]
                                            //[R26=tmp_c(1)+lambda1|tmp_a(1)+lambda0]
      R23 = MEMB(R23)                       //[ correction factor, (beta3)          ]
      R22 = MEMB(R22)                       //[ correction factor, (beta2)          ]
      R19:18 = VABSDIFFH(R5:4,R7:6)         //[ for new beta[7|6|5|4]               ]
    }
    { R23 = ASRH(R27)                       //[ R23 = X | tmp_a(0)+lambda3          ]
      R22 = ASRH(R26)                       //[ R22 = X | tmp_c(1)+lambda1          ]
      R21:20 = VTRUNEWH(R23:22,R21:20)      //[ correction factor for beta[3|2|1|0] ]
      R19:18 = VMINH(R19:18,R25:24)         //[ for new beta[7|6|5|4]               ]
    }
    { R9 = VSUBH(R27,R23)                   //[ max*- max_over_zeros:max_over_ones  ]
      R8 = VSUBH(R26,R22)                   //[ max*- max_over_zeros:max_over_ones  ]
      R13:12 = VABSDIFFH(R27:26,R23:22)     //[ max*- max_over_zeros:max_over_ones  ]
      R15:14 = VADDH(R15:14,R21:20)         //[ beta3|beta2|beta1|beta0             ]
    }
    { R13:12 = VMINH(R13:12,R25:24)         //[ max_over_zeros : max_over_ones      ]
      P0 = VCMPH.GT(R9:8,R31:30)            //[ max_over_zeros : max_over_ones      ]
      R16 = ZXTH(R18)                       //[ for new beta[4]                     ]
      R17 = ASRH(R18)                       //[ for new beta[5]                     ]
    }
    { R12 = ZXTH(R12)                       //[ max_over_zeros : max_over_ones      ]
      R13 = ZXTH(R13)                       //[ max_over_zeros : max_over_ones      ]
      R16 = ADD(R28,R16)                    //[ address of correction factor in LUT ]
      R17 = ADD(R28,R17)                    //[ address of correction factor in LUT ]
    }
    { R16 = MEMB(R16)                       //[[2]correction factor, (beta4)        ]
      R17 = MEMB(R17)                       //[[2]correction factor, (beta5)        ]
      R12 = ADD(R28,R12)                    //[ address of correction factor in LUT ]
      R13 = ADD(R28,R13)                    //[ address of correction factor in LUT ]
    }
    { R27:26 = VMUX(P0,R27:26,R23:22)       //[ max()                               ]
      R12 = MEMB(R12)                       //[ correction factor                   ]
      R13 = MEMB(R13)                       //[ correction factor                   ]
      R19:18 = VZXTHW(R19)                  //[ for new beta[7|6|5|4]               ]
    }
    { R27:26 = VADDH(R27:26,R13:12)         //[ X|max_over_zeros|X|max_over_ones    ]
      R9:8 = VSUBH(R5:4,R7:6)               //[ for new beta[7|6|5|4]               ]
      R18 = ADD(R28,R18)                    //[ address of correction factor in LUT ]
      R19 = ADD(R28,R19)                    //[ address of correction factor in LUT ]
    }
    { R18 = MEMB(R18)                       //[ correction factor, (beta6)          ]
      R19 = MEMB(R19)                       //[ correction factor, (beta7)          ]
      R26 = VSUBH(R26,R27)                  //[ max_over_ones - max_over_zeros      ]
      P0 = VCMPH.GT(R9:8,R31:30)            //[
    }
    { R26 = SUB(R26.L,R10.H)                //[ max_over_ones-max_over_zeros-xLin[j]]
      R5:4 = VMUX(P0,R5:4,R7:6)             //[ max() (beta[4,5,6,7])               ]
    }
    { MEMH(R3++#-2) = R26                   //[ save L_writ[j]                      ]
      R17:16 = VTRUNEWH(R19:18,R17:16)      //[ correction factor for beta[7|6|5|4] ]
    }:endloop0

    { R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
    }
    { R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#7*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	turbo_dec_beta_extrinsic, .-turbo_dec_beta_extrinsic





    /*[*****************************************************************************]*/
    /*[  Function   : void turbo_dec_beta_tail()                                    ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calculate beta-loop of MAP decoder for tail bits              ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - alpha[] and beta[] are aligned by 8-bytes                       ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *xLin                                             ]*/
    /*[               R1 : Word16 *p                                                ]*/
    /*[               R2 : Word16 *beta                                             ]*/
    /*[               R3 : int ntails                                               ]*/
    /*[               R4 : int nFrameSize                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 13 * (nTail-1) + 11                                             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl turbo_dec_beta_tail
    .type	turbo_dec_beta_tail, @function
turbo_dec_beta_tail:
    { R29 = ADD(R29,#-6*8)                  //[ allocate stack                      ]
      MEMD(R29+#(40-6*8)) = R27:26          //[ callee-saved registers              ]
      R3 = ADD(R3,#-1)                      //[ ntails-1                            ]
      R4 += ADD(R3,#-2)                     //[ nFrameSize + nTails-2               ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R0 += ASL(R4,#1)                      //[ &xLin[nFrameSize+nTail-2]           ]
      R1 += ASL(R4,#1)                      //[ &p[nFrameSize+nTail-2]              ]
      R24 = #THRESHOLD                      //[ THRESHOLD                           ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R5:4 = #0                             //[ initialize beta[7|6|5|4]            ]
      R15:14 = #0                           //[ initialize beta[3|2|1|0]            ]
      R25:24 = VSPLATH(R24)                 //[ R25:24 = THRESHOLD                  ] 
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R14.L = #INITIAL_VALUE                //[ initialize beta[0]=INITIAL_VALUE    ]
      R10 = MEMH(R0++#-2)                   //[ [p]load xLin[j]                     ]
      R28.H = #HI(TurboTable)
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R11 = MEMUH(R1++#-2)                  //[ [p]load p[j]                        ]
      R28.L = #LO(TurboTable)
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R12 = ADD(R10.L,R11.L):sat            //[ [p]saturate( xLin[j] + [j])         ]
      R27:26 = #0
    }
    { R10 = COMBINE(R10.L,R12.L)            //[ [p]R11:10 = lambda[3|2|1|0]         ]
      R12 = COMBINE(R11.L,R11.H)            //[ [p]R13:12 = lambda[0|1|2|3]         ]
      R13 = COMBINE(R12.L,R10.L)
      LOOP0(.beta_tail_loop, R3)
    }

    .falign
.beta_tail_loop:
    { R5:4 = VTRUNEWH(R5:4,R15:14)          //[ R5:4 = beta6|beta4|beta2|beta0      ]
      R7:6 = VTRUNOWH(R5:4,R15:14)          //[ R7:6 = beta7|beta5|beta3|beta1      ]
    }
    { R15:14 = VADDH(R5:4,R11:10)           //[ beta[6|4|2|0] + lambda[3|2|1|0]     ]
      R17:16 = VADDH(R7:6,R13:12)           //[ beta[7|5|3|1] + lambda[0|1|2|3]     ]
      R7 = VADDH(R7,R11)                    //[ beta[7|5|3|1] + lambda[3|2|1|0]     ]
      R6 = VADDH(R6,R10)                    //[ beta[7|5|3|1] + lambda[3|2|1|0]     ]
    }
    { R9:8 = VSUBH(R15:14,R17:16)           //[ max*(beta[6|4|2|0]+lambda[3|2|1|0], ]
      R19:18 = VABSDIFFH(R15:14,R17:16)     //[      beta[7|5|3|1]+lambda[0|1|2|3]) ]
      R5 = VADDH(R5,R13)                    //[ beta[6|4|2|0] + lambda[0|1|2|3]     ]
      R10 = MEMH(R0++#-2)                   //[[1]load xLin[j]                      ]
    }
    { R19:18 = VMINH(R19:18,R25:24)         //[ min(z,THRESHOLD)                    ]
      P0 = VCMPH.GT(R9:8,R27:26)            //[                                     ]
      R4 = VADDH(R4,R12)                    //[ beta[6|4|2|0] + lambda[0|1|2|3]     ]
      R11 = MEMUH(R1++#-2)                  //[[1]load p[j]                         ]
    }
    { R21:20 = VZXTHW(R18)                  //[                                     ]
      R22 = ZXTH(R19)                       //[                                     ]
      R23 = ASRH(R19)                       //[                                     ]
      R19:18 = VABSDIFFH(R5:4,R7:6)         //[ max*(beta[6|4|2|0]+lambda[0|1|2|3], ]
                                            //[      beta[7|5|3|1]+lambda[3|2|1|0]) ]
    }
    { R15:14 = VMUX(P0,R15:14,R17:16)       //[ max() (beta[0,1,2,3])               ]
      R19:18 = VMINH(R19:18,R25:24)         //[ min(z, THRESHOLD)                   ]
      R20 = ADD(R28,R20)                    //[ address of correction factor in LUT ]
      R21 = ADD(R28,R21)                    //[ address of correction factor in LUT ]
    }
    { R20 = MEMB(R20)                       //[ correction factor, (beta0)          ]
      R21 = MEMB(R21)                       //[ correction factor, (beta1)          ]
      R22 = ADD(R28,R22)                    //[ address of correction factor in LUT ]
      R23 = ADD(R28,R23)                    //[ address of correction factor in LUT ]
    }
    { R9:8 = VSUBH(R5:4,R7:6)               //[ max*                                ]
      R17:16 = VZXTHW(R18)                  //[                                     ]
      R22 = MEMB(R22)                       //[ correction factor, (beta2)          ]
      R23 = MEMB(R23)                       //[ correction factor, (beta3)          ]
    }
    { R21:20 = VTRUNEWH(R23:22,R21:20)      //[ pack correction factors             ]
      R19:18 = VZXTHW(R19)                  //[                                     ]
      R16 = ADD(R28,R16)                    //[ address of correction factor in LUT ]
      R17 = ADD(R28,R17)                    //[ address of correction factor in LUT ]
    }
    { R16 = MEMB(R16)                       //[ correction factor, (beta4)          ]
      R17 = MEMB(R17)                       //[ correction factor, (beta5)          ]
      R18 = ADD(R28,R18)                    //[ address of correction factor in LUT ]
      R19 = ADD(R28,R19)                    //[ address of correction factor in LUT ]
    }
    { P0 = VCMPH.GT(R9:8,R27:26)            //[                                     ]
      R15:14 = VADDH(R15:14,R21:20)         //[ beta3|beta2|beta1|beta0             ]
      R18 = MEMB(R18)                       //[ correction factor, (beta6)          ]
      R19 = MEMB(R19)                       //[ correction factor, (beta7)          ]
    }
    { R5:4 = VMUX(P0,R5:4,R7:6)             //[ max() (beta[4,5,6,7])               ]
      R16 = COMBINE(R17.L,R16.L)            //[ pack corrections factors            ]
      R17 = COMBINE(R19.L,R18.L)            //[ for  beta7|beta6|beta5|beta4        ]
      R12 = ADD(R10.L,R11.L):sat            //[ saturate( xLin[j] + [j])            ]
    }
    { R5:4 = VADDH(R5:4,R17:16)             //[ beta7|beta6|beta5|beta4
      R10 = COMBINE(R10.L,R12.L)            //[[1]R11:10 = lambda[3|2|1|0]          ]
      R12 = COMBINE(R11.L,R11.H)            //[[1]R13:12 = lambda[0|1|2|3]          ]
      R13 = COMBINE(R12.L,R10.L)            //[[1]                                  ]
    }:endloop0


    { MEMD(R2+#0) = R15:14                  //[ save beta[0-3]                      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved reigsters      ]
    }
    { MEMD(R2+#8) = R5:4                    //[ save beta[4-7]                      ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved reigsters      ]
    }
    { R21:20 = MEMD(R29+#16)                //[ restore callee-saved reigsters      ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved reigsters      ]
    }
    { R25:24 = MEMD(R29+#32)                //[ restore callee-saved reigsters      ]
      R27:26 = MEMD(R29+#40)                //[ restore callee-saved reigsters      ]
      R29 = ADD(R29,#48)                    //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	turbo_dec_beta_tail, .-turbo_dec_beta_tail
