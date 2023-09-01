/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 


    .file   "corr.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void CorrS16()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Compute correlations between two real number sequences        ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - output correlations is signed 16-bits                           ]*/
    /*[           - 64-bit accumualation is used                                    ]*/
    /*[           - length is a multiple of 4                                       ]*/
    /*[           - input  x[]   array is aligned by 4-bytes                        ]*/
    /*[           - input  y[]   array is aligned by 8-bytes                        ]*/
    /*[           - output cor[] array is aligned by 8-bytes                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *x                                                ]*/
    /*[               R1 : Word16 *y                                                ]*/
    /*[               R2 : int    length                                            ]*/
    /*[               R3 : int    nc     - number of correlations                   ]*/
    /*[               R4 : int    Qshift - shift factor                             ]*/
    /*[               R5 : Word16 *corr                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R24, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - [2*(length/4)-ceil(nc/4)+1]*ceil(nc/4)*3/2 + 7*ceil(nc/4) + 15  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl CorrS16
    .type	CorrS16, @function
CorrS16:
    { P0 = CMP.GT(R3,#0)                    //[ if !(nc >0)                         ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      R2 = ASR(R2,#2)                       //[ length/4                            ]
      R6 = ADD(R3,#3)                       //[ nc+3                                ]
    }
    { P0 = CMP.GT(R2,#0)                    //[ if (length/4 <=0)                   ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      R6 = ASR(R6,#2)                       //[ ceil(nc/4)                          ]
    }
    { P0 = CMP.GT(R6,R2)                    //[ if ( ceil(nc/4) > length/4)         ]
      IF P0.new JUMPR:nt R31                //[ then return                         ]
    }
    { R29 = ADD(R29,#-5*8)                  //[ reserve stacks                      ]
      MEMD(R29+#(-5*8+0)) = R17:16          //[ callee-saved registers              ]
      R28 = ADD(R4,#16)                     //[ Qshift + 16                         ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R4 = ADD(R2,#-1)                      //[ length/4-1                          ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      P0 = CMP.GT(R4,#0)                    //[ P0 = (length/4-1) > 0               ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      LOOP1(.CorrS16_mainLOOP,R6)           //[ setup outer loop: lc1= ceil(nc/4)   ]
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
    }
    { MEMW(R29+#32) = R24                   //[ callee-saved registers              ]
      R24 = AND(R3,#3)                      //[ nc%4                                ]
    }

    .falign
.CorrS16_mainLOOP:
    { R21:20 = COMBINE(R20,R16)             //[[O2]pack to sum1:sum0                ]
      R23:22 = COMBINE(R22,R18)             //[[O2]pack to sum3:sum2                ]
      R17:16 = #0                           //[ sum0 = 0                            ]
      R19:18 = #0                           //[ sum2 = 0                            ]
    }
    { R2 = VSATWH(R21:20)                   //[[O2]saturate to 16-bit               ]
      R3 = VSATWH(R23:22)                   //[[O2]saturate to 16-bit               ]
      R9 = MEMW(R0++#8)                     //[ load R9 =x[0-1]                     ]
      R12 = MEMW(R0+#4)                     //[ load R12=x[2-3]                     ]
    }
    { IF P3 MEMD(R5++#8) = R3:2             //[[O2]save correlations                ]                             
      R23:22 = #0                           //[ sum1 = 0                            ]
      R21:20 = #0                           //[ sum3 = 0                            ]
      LOOP0(.CorrS16_innerLOOP,R4)          //[ setup inner loop:lc0 = length/4-i   ]
    }
    { R15:14 = #0                           //[ 0                                   ]
      R10 = R9                              //[ x[4*(i+k)+0-1]                      ]
      R3:2 = R1:0                           //[ R3 = y, R2 = &x[4*i+4]              ]
      IF !P0 JUMP .CorrS16_innerLPEND       //[ if (length/4-1)<=0 skip loop        ]
    }

    .falign
.CorrS16_innerLOOP:
    { R11:10 = VALIGNB(R11:10,R7:6,#2)      //[[2]x[4*(i+k)+1-4]                    ]
      R13:12 = VALIGNB(R13:12,R9:8,#2)      //[[2]x[4*(i+k)+3-6]                    ]
      R7:6 = COMBINE(R12,R10)               //[[1]x[4*(i+k+1)+0-3]                  ]
      R9 = MEMW(R2++#4)                     //[[1]load x[4*(i+k+1)+4-5]             ]
    }
    { R21:20 += VRMPYH(R11:10,R15:14)       //[[2]sum1 += x[4*i+4*k+1-4]*y[4*k+0-3] ]
      R23:22 += VRMPYH(R13:12,R15:14)       //[[2]sum3 += x[4*i+4*k+3-6]*y[4*k+0-3] ]
      R8 = R7                               //[[1]R9:8 = x[4*(i+k+1)+2-5]           ]
      R15:14 = MEMD(R3++#8)                 //[[1]load y[4*k+0-3]                   ]
    }
    { R17:16 += VRMPYH(R7:6,R15:14)         //[[1]sum0 += x[4*(i+k)+0-3]*y[4*k+0-3] ]
      R19:18 += VRMPYH(R9:8,R15:14)         //[[1]sum2 += x[4*(i+k)+2-5]*y[4*k+0-3] ]
      R10 = R9                              //[[1]x[4*(i+k)+4-5]                    ]
      R12 = MEMW(R2++#4)                    //[[1]load x[4*(i+k+1)+6-7]             ]
    }:endloop0

.CorrS16_innerLPEND:
    { R11:10 = VALIGNB(R11:10,R7:6,#2)      //[[e]x[4*(i+k)+1-4]                    ]
      R13:12 = VALIGNB(R13:12,R9:8,#2)      //[[e]x[4*(i+k)+3-6]                    ]
      R7:6 = COMBINE(R12,R10)               //[ x[4*(i+k+1)+0-3]                    ]
      R9 = #0                               //[                                     ]
    }
    { R21:20 += VRMPYH(R11:10,R15:14)       //[[e]sum1 += x[4*i+4*k+1-4]*y[4*k+0-3] ]
      R23:22 += VRMPYH(R13:12,R15:14)       //[[e]sum3 += x[4*i+4*k+3-6]*y[4*k+0-3] ]
      R8 = R7                               //[ R9:8 = x[4*(i+k+1)+2-5]             ]
      R15:14 = MEMD(R3++#8)                 //[ load y[4*k+0-3]                     ]
    }
    { R17:16 += VRMPYH(R7:6,R15:14)         //[ sum0 += x[4*(i+k)+0-3] * y[4*k+0-3] ]
      R19:18 += VRMPYH(R9:8,R15:14)         //[ sum2 += x[4*(i+k)+2-5] * y[4*k+0-3] ]
      R6 = COMBINE(R7.L,R6.H)               //[ R7:6 = x[4*(i+k)+1-4]               ]
      R7 = COMBINE(R9.L,R7.H)               //[ R7:6 = x[4*(i+k)+1-4]               ]
    }
    { R21:20 += VRMPYH(R7:6,R15:14)         //[ sum1 += x[4*i+4*k+1-4] * y[4*k+0-3] ]
      R23:22 += MPY(R7.L,R14.L)             //[ sum3 += x[4*i+4*k+3-6] * y[4*k+0-3] ]
      R4 = ADD(R4,#-1)                      //[ LC0--                               ]
    }
    { R17:16 = ASR(R17:16,R28)              //[ sum0>>= Qshift                      ]
      R21:20 = ASR(R21:20,R28)              //[ sum1>>= Qshift                      ]
      P0 = CMP.GT(R4,#0)                    //[ P0 = (length/4-i)> 0                ]
    }
    { R19:18 = ASR(R19:18,R28)              //[ sum2>>= Qshift                      ]
      R23:22 = ASR(R23:22,R28)              //[ sum3>>= Qshift                      ]
      P3 = CMP.EQ(R0,R0)                    //[ set P3                              ]
    }:endloop1


    { R21:20 = COMBINE(R20,R16)             //[[e]pack to sum1:sum0                 ]
      R23:22 = COMBINE(R22,R18)             //[[e]pack to sum3:sum2                 ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { R2 = VSATWH(R21:20)                   //[[e]saturate to 16-bit                ]
      R3 = VSATWH(R23:22)                   //[[e]saturate to 16-bit                ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { R24 = MEMW(R29+#32)                   //[ restore callee-saved registers      ]
      P0 = CMP.GT(R24,#0)                   //[ (nc%4!=0)                           ]
      P1 = CMP.EQ(R24,#1)                   //[ (nc%4 ==1)                          ]
      P2 = CMP.EQ(R24,#2)                   //[ (nc%4 ==2)                          ]
    }
    { R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      IF !P0 MEMD(R5) = R3:2                //[[e]save correlations                 ]
      IF !P0 JUMPR R31                      //[ return                              ]
    }
    { MEMH(R5) = R2                         //[[e] save one correlation             ]
      IF P1 JUMPR R31                       //[ if (nc%4==1) return                 ]
    }
    { MEMH(R5+#2) = R2.H                    //[[e] save one more correlation        ]
      IF P2 JUMPR R31                       //[ if (nc%4==2) return                 ]
    }
    { MEMH(R5+#4) = R3                      //[[e]save  one more correlation        ]
      JUMPR R31                             //[ return                              ]
    }
    .size	CorrS16, .-CorrS16




    /*[*****************************************************************************]*/
    /*[  Function   : void CorrS32()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Compute correlations between two real number sequences        ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - output correlations is signed 32-bits                           ]*/
    /*[           - 64-bit accumualation is used                                    ]*/
    /*[           - length is a multiple of 4                                       ]*/
    /*[           - input  x[]   array is aligned by 4-bytes                        ]*/
    /*[           - input  y[]   array is aligned by 8-bytes                        ]*/
    /*[           - output cor[] array is aligned by 8-bytes                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *x                                                ]*/
    /*[               R1 : Word16 *y                                                ]*/
    /*[               R2 : int    length                                            ]*/
    /*[               R3 : int    nc     - number of correlations                   ]*/
    /*[               R4 : int    Qshift - shift factor                             ]*/
    /*[               R5 : Word32 *corr                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R24, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - [2*(length/4)-ceil(nc/4)+1]*ceil(nc/4)*3/2 + 7*ceil(nc/4) + 15  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl CorrS32
    .type	CorrS32, @function
CorrS32:
    { P0 = CMP.GT(R3,#0)                    //[ if !(nc>0)                          ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      R2 = ASR(R2,#2)                       //[ length/4                            ]
      R6 = ADD(R3,#3)                       //[ nc+3                                ]
    }
    { P0 = CMP.GT(R2,#0)                    //[ if (length/4 <=0)                   ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      R6 = ASR(R6,#2)                       //[ ceil(nc/4)                          ]
    }
    { P0 = CMP.GT(R6,R2)                    //[ if ( ceil(nc/4) > length/4)         ]
      IF P0.new JUMPR:nt R31                //[ then return                         ]
    }
    { R29 = ADD(R29,#-5*8)                  //[ reserve stacks                      ]
      MEMD(R29+#(-5*8+0)) = R17:16          //[ callee-saved registers              ]
      R28 = R4                              //[ R28 = Qshift                        ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R4 = ADD(R2,#-1)                      //[ LC0 = lenght/4-1                    ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      P0 = CMP.GT(R4,#0)                    //[[p]LC0 > 0                           ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
    }
    { MEMW(R29+#32) = R24                   //[ callee-saved registers              ]
      R24 = AND(R3,#3)                      //[ nc%4                                ]
      LOOP1(.CorrS32_mainLOOP,R6)           //[ setup outer loop: lc1= ceil(nc/4)   ]
      R15:14 = #0                           //[[p]0                                 ]
    }

    .falign
.CorrS32_mainLOOP:
    { R2 = SAT(R17:16)                      //[[O2]L_sat(sum0)                      ]
      R3 = SAT(R21:20)                      //[[O2]L_sat(sum1)                      ]
      R9 = MEMW(R0++#8)                     //[ load R9 =x[0-1]                     ]
      R12 = MEMW(R0+#4)                     //[ load R12=x[2-3]                     ]
    }
    { IF P3 MEMD(R5++#8) = R3:2             //[[O2]save                             ]
      R2 = SAT(R19:18)                      //[[O2]L_sat(sum2)                      ]
      R3 = SAT(R23:22)                      //[[O2]L_sat(sum3)                      ]
      R17:16 = #0                           //[ sum0 = 0                            ]
    }
    { IF P3 MEMD(R5++#8) = R3:2             //[[O2]save                             ]
      R19:18 = #0                           //[ sum2 = 0                            ]
      R21:20 = #0                           //[ sum3 = 0                            ]
      LOOP0(.CorrS32_innerLOOP,R4)          //[ setup inner loop:lc0 = length/4-i   ]
    }
    { R23:22 = #0                           //[ sum1 = 0                            ]
      R10 = R9                              //[ x[4*(i+k)+0-1]                      ]
      R3:2 = R1:0                           //[ R3 = y, R2 = &x[4*i+4]              ]
      IF !P0 JUMP .CorrS32_innerLPEND       //[ if LC= 0 skip loop                  ]
    }

    .falign
.CorrS32_innerLOOP:
    { R11:10 = VALIGNB(R11:10,R7:6,#2)      //[[2]x[4*(i+k)+1-4]                    ]
      R13:12 = VALIGNB(R13:12,R9:8,#2)      //[[2]x[4*(i+k)+3-6]                    ]
      R7:6 = COMBINE(R12,R10)               //[[1]x[4*(i+k+1)+0-3]                  ]
      R9 = MEMW(R2++#4)                     //[[1]load x[4*(i+k+1)+4-5]             ]
    }
    { R21:20 += VRMPYH(R11:10,R15:14)       //[[2]sum1 += x[4*i+4*k+1-4]*y[4*k+0-3] ]
      R23:22 += VRMPYH(R13:12,R15:14)       //[[2]sum3 += x[4*i+4*k+3-6]*y[4*k+0-3] ]
      R8 = R7                               //[[1]R9:8 = x[4*(i+k+1)+2-5]           ]
      R15:14 = MEMD(R3++#8)                 //[[1]load y[4*k+0-3]                   ]
    }
    { R17:16 += VRMPYH(R7:6,R15:14)         //[[1]sum0 += x[4*(i+k)+0-3]*y[4*k+0-3] ]
      R19:18 += VRMPYH(R9:8,R15:14)         //[[1]sum2 += x[4*(i+k)+2-5]*y[4*k+0-3] ]
      R10 = R9                              //[[1]x[4*(i+k)+4-5]                    ]
      R12 = MEMW(R2++#4)                    //[[1]load x[4*(i+k+1)+6-7]             ]
    }:endloop0

.CorrS32_innerLPEND:
    { R11:10 = VALIGNB(R11:10,R7:6,#2)      //[[e]x[4*(i+k)+1-4]                    ]
      R13:12 = VALIGNB(R13:12,R9:8,#2)      //[[e]x[4*(i+k)+3-6]                    ]
      R7:6 = COMBINE(R12,R10)               //[ x[4*(i+k+1)+0-3]                    ]
      R9 = #0                               //[                                     ]
    }
    { R21:20 += VRMPYH(R11:10,R15:14)       //[[e]sum1 += x[4*i+4*k+1-4]*y[4*k+0-3] ]
      R23:22 += VRMPYH(R13:12,R15:14)       //[[e]sum3 += x[4*i+4*k+3-6]*y[4*k+0-3] ]
      R8 = R7                               //[ R9:8 = x[4*(i+k+1)+2-5]             ]
      R15:14 = MEMD(R3++#8)                 //[ load y[4*k+0-3]                     ]
    }
    { R17:16 += VRMPYH(R7:6,R15:14)         //[ sum0 += x[4*(i+k)+0-3] * y[4*k+0-3] ]
      R19:18 += VRMPYH(R9:8,R15:14)         //[ sum2 += x[4*(i+k)+2-5] * y[4*k+0-3] ]
      R6 = COMBINE(R7.L,R6.H)               //[ R7:6 = x[4*(i+k)+1-4]               ]
      R7 = COMBINE(R9.L,R7.H)               //[ R7:6 = x[4*(i+k)+1-4]               ]
    }
    { R21:20 += VRMPYH(R7:6,R15:14)         //[ sum1 += x[4*i+4*k+1-4] * y[4*k+0-3] ]
      R23:22 += MPY(R7.L,R14.L)             //[ sum3 += x[4*i+4*k+3-6] * y[4*k+0-3] ]
      R4 = ADD(R4,#-1)                      //[ LC0--                               ]
    }
    { R17:16 = ASR(R17:16,R28)              //[ sum0>>= Qshift                      ]
      R21:20 = ASR(R21:20,R28)              //[ sum1>>= Qshift                      ]
      P0 = CMP.GT(R4,#0)                    //[ LC0 > 0                             ]
    }
    { R19:18 = ASR(R19:18,R28)              //[ sum2>>= Qshift                      ]
      R23:22 = ASR(R23:22,R28)              //[ sum3>>= Qshift                      ]
      P3 = CMP.EQ(R0,R0)                    //[ set P3                              ]
      R15:14 = #0                           //[ 0                                   ]
    }:endloop1

    { R2 = SAT(R17:16)                      //[[e]L_sat(sum0)                       ]
      R3 = SAT(R21:20)                      //[[e]L_sat(sum1)                       ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R6 = SAT(R19:18)                      //[[e]L_sat(sum2)                       ]
      R7 = SAT(R23:22)                      //[[e]L_sat(sum3)                       ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { R24 = MEMW(R29+#32)                   //[ restore callee-saved registers      ]
      P0 = CMP.EQ(R24,#1)                   //[ nc%4==1                             ]
      P1 = CMP.EQ(R24,#2)                   //[ nc%4==2                             ]
      P2 = CMP.EQ(R24,#3)                   //[ nc%4==3                             ]
    }
    { R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      MEMW(R5++#4) = R2                     //[[e]save one correlations             ]
      IF P0 JUMPR R31                       //[ return                              ]
    }
    { MEMW(R5++#4) = R3                     //[[e]save one more correlations        ]
      IF P1 JUMPR R31                       //[ return                              ]
    }
    { MEMW(R5++#4) = R6                     //[[e]save one more correlations        ]
      IF P2 JUMPR R31                       //[ return                              ]
    }
    { MEMW(R5) = R7                         //[[e]save one more correlations        ]
      JUMPR R31                             //[ return                              ]
    }
    .size	CorrS32, .-CorrS32



    /*[*****************************************************************************]*/
    /*[  Function   : void cxCorr()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Compute correlations between two complex number sequences     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - ONLY 32-bit accumualation is used. Overflow must be considered  ]*/
    /*[           - x, y, corr are sc16                                             ]*/
    /*[           - length is a multiple of 2                                       ]*/
    /*[           - x[], y[] and corr[]  arrays are aligned by 8-bytes              ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *x                                             ]*/
    /*[               R1 : CWord2x16 *y                                             ]*/
    /*[               R2 : int       length                                         ]*/
    /*[               R3 : int       nc     - number of correlations                ]*/
    /*[               R4 : int       Qshift - shift factor                          ]*/
    /*[               R5 : CWord2x16 *corr                                          ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (length - ceil(nc/2) + 4)*ceil(nc/2) + 7                        ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl cxCorr
    .type	cxCorr, @function
cxCorr:
    { P0 = CMP.GT(R3,#0)                    //[ if !(nc>0)                          ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      IF  P0.new R28 = R4                   //[ R28 = Qshift                        ]
      R2 = ASR(R2,#1)                       //[ length/2                            ]
    }
    { P0 = CMP.GT(R2,#0)                    //[ if (length/2 <=0)                   ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      R6 = ASRRND(R3,#1)                    //[ ceil(nc/2)                          ]
      R4 = ADD(R2,#-1)                      //[ LC0 = lenght/2-1                    ]
    }
    { P0 = CMP.GT(R6,R2)                    //[ if ( ceil(nc/2) > length/2)         ]
      IF  P0.new JUMPR:nt R31               //[ then return                         ]
      IF !P0.new R2 = ADD(R0,#8)            //[[p]R2 = &x[2]                        ]
      P2 = TSTBIT(R3,#0)                    //[ nc is odd                           ]
    }
    { LOOP1(.cxCorr_mainLOOP,R6)            //[ setup outer loop: lc1 = ceil(nc/2)  ]
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
      P0 = CMP.GT(R4,#0)                    //[[p]LC0 > 0                           ]
      R3 = R1                               //[[p]R3 = y                            ]
    }


    .falign
.cxCorr_mainLOOP:
    { R14 = VSATWH(R13:12)                  //[[O2]cor[2i]                          ]
      R7:6 = MEMD(R0++#8)                   //[load x[2*(i+k)+0-1]                  ]
      R13:12 = #0                           //[ sum0 = 0                            ]
      LOOP0(.cxCorr_innerLOOP,R4)           //[ setup inner loop                    ]
    }
    { IF P3 MEMD(R5++#8) = R15:14           //[[O2]save                             ]
      R15:14 = #0                           //[ sum1 = 0                            ]
      R11:10 = MEMD(R3++#8)                 //[[p]load y[2*k+0-1]                   ]
      IF !P0 JUMP .cxCorr_innerLPEND        //[ if LC= 0 skip loop                  ]
    }

    .falign
.cxCorr_innerLOOP:
    { R9:8 = MEMD(R2++#8)                   //[[2]load x[2*(i+k)+2-3]               ]
      R13:12 += CMPY(R6,R10*):<<1:sat       //[[2]sum0+= (x[2*(i+k)+0]*y[2*k])<<1   ]
      R15:14 += CMPY(R7,R10*):<<1:sat       //[[2]sum1+= (x[2*(i+k)+1]*y[2*k])<<1   ]
      NOP
    }
    { R13:12 += CMPY(R7,R11*):<<1:sat       //[[2]sum0+= (x[2*(i+k)+1]*y[2*k+1])<<1 ]
      R15:14 += CMPY(R8,R11*):<<1:sat       //[[2]sum1+= (x[2*(i+k)+2]*y[2*k+1])<<1 ]
      R7:6 = R9:8                           //[[2]R7:6 = x[2*(i+k)+2-3]             ]
      R11:10 = MEMD(R3++#8)                 //[[1]load y[2*k+0-1]                   ]
    }:endloop0

.cxCorr_innerLPEND:
    { R13:12 += CMPY(R6,R10*):<<1:sat       //[[e]sum0+= (x[2*(i+k)+0]*y[2*k])<<1   ]
      R15:14 += CMPY(R7,R10*):<<1:sat       //[[e]sum1+= (x[2*(i+k)+1]*y[2*k])<<1   ]
      R4 = ADD(R4,#-1)                      //[ LC0--                               ]
    }
    { R13:12 += CMPY(R7,R11*):<<1:sat       //[[e]sum0+= (x[2*(i+k)+1]*y[2*k+1])<<1 ]
      R15:14 = VASRW(R15:14,R28)            //[ sum0>>Qshift                        ]
      P0 = CMP.GT(R4,#0)                    //[ LC0 > 0                             ]
      R2 = ADD(R0,#8)                       //[ R2 = &x[2]                          ]
    }
    { R13:12 = VASRW(R13:12,R28)            //[ sum0>>Qshift                        ]
      R15 = VSATWH(R15:14)                  //[ cor[2i+1]                           ]
      R3 = R1                               //[ R3 = y                              ]
      P3 = CMP.EQ(R0,R0)                    //[ set P3                              ]
    }:endloop1


    { R14 = VSATWH(R13:12)                  //[[e]cor[2i]                           ]
    }
    { MEMW(R5++#4) = R14                    //[[e]save                              ]
    }
    { IF !P2 MEMW(R5) = R15                 //[[e]if nc is even then save           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	cxCorr, .-cxCorr
