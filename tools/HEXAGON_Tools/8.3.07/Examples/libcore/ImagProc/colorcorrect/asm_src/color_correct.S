/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 


    .file    "color_correct.S"
#define    OFFSET_C0          0
#define    OFFSET_C1          2
#define    OFFSET_C2          4
#define    OFFSET_C3          6
#define    OFFSET_C4          8
#define    OFFSET_C5          10
#define    OFFSET_C6          12
#define    OFFSET_C7          14 
#define    OFFSET_C8          16
#define    OFFSET_K0          18
#define    OFFSET_K1          20 
#define    OFFSET_K2          22
#define    OFFSET_QF          24

    /*[*****************************************************************************]*/            
    /*[  Function   : void color_correct()                                          ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: perform color correction on RGB-components of image           ]*/ 
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - G, B, R array must be aligned by 4bytes                         ]*/
    /*[           - process two pixel per iteration                                 ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord16 *R                                               ]*/
    /*[               R1 : UWord16 *G                                               ]*/
    /*[               R2 : UWord16 *B                                               ]*/
    /*[               R3 : int width                                                ]*/
    /*[               R4 : int height                                               ]*/
    /*[               R5 : ColorCorrectParams *params                               ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28,R30,R31                                        ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 56                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 4*W*H + 19                                                      ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl color_correct
    .type    color_correct, @function
color_correct:
    { P0 = CMP.GT(R3,#0)                    //[ P0 = (width>0)                      ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
      P1 = CMP.GT(R4,#0)                    //[ P1 = (height>0)                     ]
      R4 = MPYI(R3,R4)                      //[ width * height                      ]
    }
    { IF !P1 JUMPR R31                      //[ IF !P1 return                       ]
      IF  P1 R12 = MEMH(R5+#OFFSET_C0)      //[ get params->C[0]                    ]
      IF  P1 R13 = MEMH(R5+#OFFSET_C1)      //[ get params->C[1]                    ]
    }
    { R29 = ADD(R29,#-7*8)                  //[ allocate stack                      ]
      MEMD(R29+#(0-7*8)) = R17:16           //[ callee-saved registers              ]
      R12 = COMBINE(R13.L,R12.L)            //[ c1|c0                               ]
      R14 = MEMH(R5+#OFFSET_C3)             //[ params->C[3]                        ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved reigsters              ]
      R13 = R12                             //[ R13 = R12 = c1|c0                   ]
      R15 = MEMH(R5+#OFFSET_C4)             //[ params->C[4]                        ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved reigsters              ]
      R14 = COMBINE(R15.L,R14.L)            //[ c4|c3                               ]
      R18 = MEMH(R5+#OFFSET_C2)             //[ params->C[2]                        ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved reigsters              ]
      R15 = R14                             //[ R15 = R14 = C4|c3                   ]
      R19 = MEMH(R5+#OFFSET_C5)             //[ params->C[5]                        ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved reigsters              ]
      R16 = MEMH(R5+#OFFSET_C6)             //[ params->C[6]                        ]
      R18 = COMBINE(R18.L,R18.L)            //[ R18 = c2|c2                         ]
      R19 = COMBINE(R19.L,R19.L)            //[ R19 = c5|c5                         ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved reigsters              ]
      R17 = MEMH(R5+#OFFSET_C7)             //[ params->C[7]                        ]
      R4 = ASR(R4,#1)                       //[ width*height/2                      ]
      R25 = #4095                           //[ R25 = 4095                          ]
    }
    { MEMD(R29+#48) = R31:30                //[ callee-saved reigsters              ]
      R20 = MEMH(R5+#OFFSET_C8)             //[ params->C[8]                        ]
      R16 = COMBINE(R17.L,R16.L)            //[ R16 = c7|c6                         ]
      R17 = COMBINE(R17.L,R16.L)            //[ R17 = c7|c6                         ]
    }
    { R21 = MEMW(R0++#4)                    //[ load R-component                    ]
      R20 = COMBINE(R20.L,R20.L)            //[ R20 = c8|c8                         ]
      R31 = R0                              //[ R31 = R                             ]
      P3 = SP1LOOP0(.color_correct_loop,R4) //[ setup loop with lc0 = width*height/2] 
    }
    { R22 = MEMW(R1++#4)                    //[ load G-component                    ]
      R23 = MEMW(R2++#4)                    //[ load B-component                    ]
      R7:6 = VMPYH(R18,R21):sat             //[ R[i]*C[2]                           ]
      R9:8 = VMPYH(R19,R21):sat             //[ R[i]*C[5]                           ]
    }
    { R3  = MEMH(R5+#OFFSET_QF)             //[ params->qfactor                     ]
      R28 = MEMH(R5+#OFFSET_K2)             //[ params->k[2]                        ]
      R23:22 = PACKHL(R23,R22)              //[ pack into B[i+1]G[i+1]|B[i]G[i]     ]
      R24 = R25                             //[ R24 = R25 = 4095                    ]
    }
    { R26 = MEMH(R5+#OFFSET_K0)             //[ params->k[0]                        ]
      R27 = MEMH(R5+#OFFSET_K1)             //[ params->k[1]                        ]
      R7:6 += VDMPY(R23:22,R13:12):sat      //[ lG = G[i]*C[0]+B[i]*C[1]+R[i]*C[2]  ]
      R9:8 += VDMPY(R23:22,R15:14):sat      //[ lB = G[i]*C[3]+B[i]*C[4]+R[i]*C[5]  ]
    }
    { R5:4 = #0                             //[ R5:4 = 0                            ]
      R7:6 = VASRW(R7:6,R3)                 //[ lG >> (qfactor+7)                   ]
      R9:8 = VASRW(R9:8,R3)                 //[ lB >> (qfactor+7)                   ]
    }
    { R6 = ADD(R6,R26)                      //[ lG = lG + k[0]                      ]
      R7 = ADD(R7,R26)                      //[ lG = lG + k[0]                      ]
      R11:10 = VMPYH(R20,R21):sat           //[ R[i]*C[8]                           ]
    }

    .falign
.color_correct_loop:
    { R22 = MEMW(R1++#4)                    //[[1]load G                            ]
      R8 = ADD(R8,R27)                      //[[2]lB = lB + k[1]                    ]
      R11:10 += VDMPY(R23:22,R17:16):sat    //[[2]lB = G[i]*C[6]+B[i]*C[7]+R[i]*C[8]]
      R7:6 = VMAXW(R7:6,R5:4)               //[[2]clip lG to (0,4095)               ]
    }
    { R23 = MEMW(R2++#4)                    //[[1]load B                            ]
      R9 = ADD(R9,R27)                      //[[2]lB = lB + k[1]                    ]
      R7:6 = VMINW(R7:6,R25:24)             //[[2]clip lG to (0,4095)               ]
      R11:10 = VASRW(R11:10,R3)             //[[2]lC >> (qfactor+7)                 ]
    }
    { R21 = MEMW(R0++#4)                    //[[1]load R                            ]
      R23:22 = PACKHL(R23,R22)              //[[1]pack into B[i+1]G[i+1]|B[i]G[i]   ]
      R30 = COMBINE(R7.L,R6.L)              //[[2]pack output G[i+1]|G[i]           ]
      R9:8 = VMAXW(R9:8,R5:4)               //[[2]clip lB to (0,4095)               ]
    }
    { R7:6 = VMPYH(R18,R21):sat             //[[1]R[i]*C[2]                         ]
      R10 = ADD(R10,R28)                    //[[2]lC = lC + k[2]                    ]
      R11 = ADD(R11,R28)                    //[[2]lC = lC + k[2]                    ]
      R9:8 = VMINW(R9:8,R25:24)             //[[2]clip lB to (0,4095)               ]
    }
    { R7:6 += VDMPY(R23:22,R13:12):sat      //[[1]lG = G[i]*C[0]+B[i]*C[1]+R[i]*C[2]]
      R8 = COMBINE(R9.L,R8.L)               //[[2]pack output B[i+1]/B[i]           ]
      R11:10 = VMAXW(R11:10,R5:4)           //[[2]clip lC to (0,4095)               ]
      IF P3 MEMW(R31++#4) = R15             //[[3]save corrected R[i]               ]
    }
    { R9:8 = VMPYH(R19,R21):sat             //[[1]R[i]*C[5]                         ]
      R7:6 = VASRW(R7:6,R3)                 //[[1]lG >> (qfactor+7)                 ]
      R15 = R14                             //[[1]                                  ]
      MEMW(R2+#-8) = R8                     //[[2]save corrected B[i]               ]
    }
    { R6 = ADD(R6,R26)                      //[[1]lG = lG + k[0]                    ]
      R7 = ADD(R7,R26)                      //[[1]lG = lG + k[0]                    ]
      R9:8 += VDMPY(R23:22,R15:14):sat      //[[1]lB = G[i]*C[3]+B[i]*C[4]+R[i]*C[5]]
      R11:10 = VMINW(R11:10,R25:24)         //[[2] clip lC to (0,4095)              ]
    }
    { R9:8 = VASRW(R9:8,R3)                 //[[1]lB >> (qfactor+7)                 ]
      R11:10 = VMPYH(R20,R21):sat           //[[1]R[i]*C[8]                         ]
      MEMW(R1+#-8) = R30                    //[[2]save corrected G[i]               ]
      R15 = COMBINE(R11.L,R10.L)            //[[2]                                  ]
    }:endloop0

    { MEMW(R31) = R15                       //[ [e]save corrected R[i]              ]
      R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
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
      JUMPR R31
    }
    .size    color_correct, .-color_correct
