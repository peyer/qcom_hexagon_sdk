/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008
****************************************************************************/

    .file   "sfpFFT.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void sfpFFT()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform N-point FFT transform on a complex number array       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - complex number: interleaved real/image with real part           ]*/
    /*[             at low memory address                                           ]*/
    /*[           - input  is aligned by size of the array                          ]*/
    /*[             * This alignment can be removed with ~2% penalty                ]*/
    /*[           - output is aligned by 8bytes                                     ]*/
    /*[           - N >= 4                                                          ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : cfloat    *input                                         ]*/
    /*[               R1 : int       N                                              ]*/
    /*[               R2 : cfloat    *twiddles                                      ]*/
    /*[               R3 : cfloat    *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R31                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 64                                ]*/
    /*[=============================================================================]*/
    /*[  Implementation:                                                            ]*/
    /*[           - Radix-4 DIF                                                     ]*/
    /*[           - INPUT: LOAD IN BIT-REVERSE ORDER                                ]*/
    /*[           - TWIDDLE FACTOR ARRAY: IN BIT-REVERSE ORDER & INTERLEAVED        ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - N =    4^k    (7N/2+5)*K - 5N/4 + 6                             ]*/
    /*[           - N = 2*(4^K)   (7N/2+5)*K + 5N/4 + 12                            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  MZ               01/05/2012         created                                ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl sfpFFT
    .type	sfpFFT, @function
/* ================================================================================= */
sfpFFT:
    { P0 = CMP.GT(R1,#3)                    //[ N > 3 ?                             ]
      IF !P0.new JUMPR:nt R31               //[ if !(N>=4) return                   ]
      R5 = BREV(R0)                         //[ to bit reverse R0.L                 ]
      R28 = ADD(R1,R1)                      //[ R28 = 2*N                           ]
    }
    { R29 = ADD(R29, #-8*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-8*8)) = R17:16           //[ callee-saved registers              ]
      MEMD(R29+#(8-8*8)) = R19:18           //[ callee-saved registers              ]
      R7 = ADD(CLB(R1),#-18)                //[ 16-log2(8N) = cl0(N)-18             ] 
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R4 = LSL(#1,R7)                       //[ R4= 1<<(15-log2(4N))= 1<<(13-LOG2N) ]
      R6 = ASR(R1, #2)                      //[ N/4 (for sfpFFT_1stStageLoop)     ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R0 = COMBINE(R0.H,R5.H)               //[ bit reverse R0.L (bit-reversed addr)]
      M0 = R4                               //[ set M0 for bit-reversed addressing  ]
    }
    { MEMD(R29+#48) = R31:30                //[ callee-saved registers              ]
      MEMW(R29+#56) = R2                    //[ save twiddle on stack               ]
      R1 = R3                               //[ set R1=  Output                     ]
      P3 = SP1LOOP0(.sfpFFT_1stStageLoop,R6)//[ setup loop0: lc0 = N/4              ]
    }
/* ================================================================================= */
    .falign
.sfpFFT_1stStageLoop:
    { R13:12 = MEMD(R0++M0:brev)            //[[1]load A                            ]
      R22 -= SFMPY(R7,R21)                  //[[2]Wb*B" = Wb.r*B".r-Wb.i*B".i       ]
      R23 += SFMPY(R7,R20)                  //[[2]Wb*B" = Wb.r*B".i+Wb.i*B".r       ]
    }
    { R15:14 = MEMD(R0++M0:brev)            //[[1]load B                            ]
      R24 -= SFMPY(R9,R17)                  //[[2]Wc*C" = Wc.r*C".r-Wc.i*C".i       ]
      R25 += SFMPY(R9,R16)                  //[[2]Wc*C" = Wc.r*C".i+Wc.i*C".r       ]
    }
    { R17:16 = MEMD(R0++M0:brev)            //[[1]load C                            ]
      R26 -= SFMPY(R11,R19)                 //[[2]Wd*D" = Wd.r*D".r-Wd.i*D".i       ]
      R27 += SFMPY(R11,R18)                 //[[2]Wd*D" = Wd.r*D".i+Wd.i*D".r       ]
    }
    { R19:18 = MEMD(R0++M0:brev)            //[[1]load D                            ]
      R12 = SFADD(R12,R14)                  //[[1]A'= A + B   (r)                   ]
      R14 = SFSUB(R12,R14)                  //[[1]B'= A - B   (r)                   ]
    }
    { R13 = SFADD(R13,R15)                  //[[1]A'= A + B   (i)                   ]
      R15 = SFSUB(R13,R15)                  //[[1]B'= A - B   (i)                   ]
      IF P3 MEMD(R1++#8) = R23:22           //[[2]save B"                           ]
    }
    { R16 = SFADD(R16,R18)                  //[[1]C'= C + D   (r)                   ]
      R18 = SFSUB(R16,R18)                  //[[1]D'= C - D   (r)                   ]
    }
    { R17 = SFADD(R17,R19)                  //[[1]C'= C + D   (i)                   ]
      R19 = SFSUB(R17,R19)                  //[[1]D'= C - D   (i)                   ]
      IF P3 MEMD(R1++#8) = R25:24           //[[2]save C"                           ]
    }
    { R12 = SFADD(R12,R16)                  //[[1]A"= A' + C' (r)                   ]
      R16 = SFSUB(R12,R16)                  //[[1]C"= A' - C' (r)                   ]
      R7:6 = MEMD(R2+#8)                    //[[1]load Wb                           ]
    }
    { R20 = SFADD(R14,R19)                  //[[1]B"= B' - j*D' (r)                 ]
      R21 = SFSUB(R15,R18)                  //[[1]B"= B' - j*D' (i)                 ]
      R9:8 = MEMD(R2++#24)                  //[[1]load Wc                           ]
    }
    { R13 = SFADD(R13,R17)                  //[[1]A"= A' + C'  (i)                  ]
      R17 = SFSUB(R13,R17)                  //[[1]C"= A' - C'  (i)                  ]
      IF P3 MEMD(R1++#8) = R27:26           //[[2]save D"                           ]
    }
    { R18 = SFSUB(R14,R19)                  //[[1]D"= B'+ j*D'  (r)                 ]
      R19 = SFADD(R15,R18)                  //[[1]D"= B'+ j*D'  (i)                 ]
      R11:10 = MEMD(R2+#-8)                 //[[1]load Wd                           ]
    }
    { R22 = SFMPY(R6,R20)                   //[[1]Wb.r * B".r                       ]
      R23 = SFMPY(R6,R21)                   //[[1]Wb.r * B".i                       ]
      MEMD(R1++#8) = R13:12                 //[[1]save A"
    }
    { R24 = SFMPY(R8,R16)                   //[[1]Wc.r * C".r                       ]
      R25 = SFMPY(R8,R17)                   //[[1]Wc.r * C".i                       ]
    }
    { R26 = SFMPY(R10,R18)                  //[[1]Wd.r * D".r                       ]
      R27 = SFMPY(R10,R19)                  //[[1]Wd.r * D".i                       ]
    }:endloop0
/* ================================================================================= */
.sfpFFT_Stage1_end:
    { R22 -= SFMPY(R7,R21)                  //[[e]Wb*B" = Wb.r*B".r-Wb.i*B".i       ]
      R23 += SFMPY(R7,R20)                  //[[e]Wb*B" = Wb.r*B".i+Wb.i*B".r       ]
      P0 = CMP.EQ(R28,#8)                   //[ P0 = (N ==4 )                       ]
    }
    { R24 -= SFMPY(R9,R17)                  //[[e]Wc*C" = Wc.r*C".r-Wc.i*C".i       ]
      R25 += SFMPY(R9,R16)                  //[[e]Wc*C" = Wc.r*C".i+Wc.i*C".r       ]
      P1 = CMP.GT(R28,#32)                  //[ P1 = (N>16)                         ]
    }
    { R26 -= SFMPY(R11,R19)                 //[[e]Wd*D" = Wd.r*D".r-Wd.i*D".i       ]
      R27 += SFMPY(R11,R18)                 //[[e]Wd*D" = Wd.r*D".i+Wd.i*D".r       ]
      MEMD(R1+#8 ) = R25:24                 //[[e]save C"                           ]
      R31:30 = COMBINE(#-3*4*8,#4*8)        //[ -3*k1*8 :k1*8, where initial k1=4   ]
    }
    { MEMD(R1+#0 ) = R23:22                 //[[e]save B"                           ]
      MEMD(R1+#16) = R27:26                 //[[e]save D"                           ]
      R5 = ASR(R28,#5)                      //[ initial k2 = N/16                   ]
      IF P0 JUMP .sfpFFT_Done               //[ If !(N>4) then return               ]
    }
    { M0 = R30                              //[ set M0 = 8*k1                       ]
      IF !P1 JUMP .sfpFFT_LastStage         //[ if !(N>16) go to last stage         ]
    }
/* ================================================================================= */
    .falign
.sfpFFT_StagesLoop:
    { R2 = MEMW(R29+#56)                    //[ set R2 = *Wtwiddles                 ]
      R15 = ADD(R31,#8)                     //[ -3*k1*8 + 8                         ]
      P2 = CMP.GT(R0,R0)                    //[ clean P2                            ]
      LOOP1(.sfpFFT_mainLoop,R5)            //[ setup loop1: lc1 = k2               ]
    }
    { R4 = ASR(R30,#3)                      //[ R4 = k1                             ]
      M1 = R15                              //[ set M1 = -3*k1*8 + 8                ]
      R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
    }
/* ================================================================================= */
    .falign
.sfpFFT_mainLoop:
    { P3 = SP1LOOP0(.sfpFFT_innerLoop,R4)   //[ setup loop0 with LC= k1             ]
      IF P2 R0 = SUB(R0,R31)                //[ R0 = R0 + 3*M0                      ]
      IF P2 R2 = ADD(R2,#24)                //[ update weights                      ]
    }
    .falign
.sfpFFT_innerLoop:
    { R13:12 = MEMD(R0++M0)                 //[[1]load A                            ]
      R22 -= SFMPY(R7,R21)                  //[[2]Wb*B" = Wb.r*B".r-Wb.i*B".i       ]
      R23 += SFMPY(R7,R20)                  //[[2]Wb*B" = Wb.r*B".i+Wb.i*B".r       ]
    }
    { R15:14 = MEMD(R0++M0)                 //[[1]load B                            ]
      R24 -= SFMPY(R9,R17)                  //[[2]Wc*C" = Wc.r*C".r-Wc.i*C".i       ]
      R25 += SFMPY(R9,R16)                  //[[2]Wc*C" = Wc.r*C".i+Wc.i*C".r       ]
    }
    { R17:16 = MEMD(R0++M0)                 //[[1]load C                            ]
      R26 -= SFMPY(R11,R19)                 //[[2]Wd*D" = Wd.r*D".r-Wd.i*D".i       ]
      R27 += SFMPY(R11,R18)                 //[[2]Wd*D" = Wd.r*D".i+Wd.i*D".r       ]
    }
    { R19:18 = MEMD(R0++M1)                 //[[1]load D                            ]
      R12 = SFADD(R12,R14)                  //[[1]A'= A + B   (r)                   ]
      R14 = SFSUB(R12,R14)                  //[[1]B'= A - B   (r)                   ]
    }
    { R13 = SFADD(R13,R15)                  //[[1]A'= A + B   (i)                   ]
      R15 = SFSUB(R13,R15)                  //[[1]B'= A - B   (i)                   ]
      IF P2 MEMD(R1+#0) = R23:22            //[[2]save B"                           ]
      IF P2 R1 = ADD(R1,R30)                //[[2]update write pointer              ]
    }
    { R16 = SFADD(R16,R18)                  //[[1]C'= C + D   (r)                   ]
      R18 = SFSUB(R16,R18)                  //[[1]D'= C - D   (r)                   ]
      R7:6 = MEMD(R2+#8)                    //[load Wb                              ]
    }
    { R17 = SFADD(R17,R19)                  //[[1]C'= C + D   (i)                   ]
      R19 = SFSUB(R17,R19)                  //[[1]D'= C - D   (i)                   ]
      IF P2 MEMD(R1+#0) = R25:24            //[[2]save C"                           ]
      IF P2 R1 = ADD(R1,R30)                //[[2]update write pointer              ]
    }
    { R12 = SFADD(R12,R16)                  //[[1]A"= A' + C' (r)                   ]
      R16 = SFSUB(R12,R16)                  //[[1]C"= A' - C' (r)                   ]
      R9:8 = MEMD(R2+#0)                    //[[1]load Wc                           ]
      R24 = MUX(P3,R31,#0)                  //[[2]                                  ]
    }
    { R20 = SFADD(R14,R19)                  //[[1]B"= B' - j*D' (r)                 ]
      R21 = SFSUB(R15,R18)                  //[[1]B"= B' - j*D' (i)                 ]
      IF P2 MEMD(R1++#8) = R27:26           //[[2]save D"                           ]
    }
    { R13 = SFADD(R13,R17)                  //[[1]A"= A' + C'  (i)                  ]
      R17 = SFSUB(R13,R17)                  //[[1]C"= A' - C'  (i)                  ]
      R11:10 = MEMD(R2+#16)                 //[[1]load Wd                           ]
    }
    { R18 = SFSUB(R14,R19)                  //[[1]D"= B'+ j*D'  (r)                 ]
      R19 = SFADD(R15,R18)                  //[[1]D"= B'+ j*D'  (i)                 ]
      IF P2 R1 = ADD(R1,R24)                //[[2]update write pointer              ]
    }
    { R22 = SFMPY(R6,R20)                   //[[1]Wb.r * B".r                       ]
      R23 = SFMPY(R6,R21)                   //[[1]Wb.r * B".i                       ]
      P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
    }
    { R24 = SFMPY(R8,R16)                   //[[1]Wc.r * C".r                       ]
      R25 = SFMPY(R8,R17)                   //[[1]Wc.r * C".i                       ]
      MEMD(R1++M0) = R13:12                 //[[1]save A"
    }
    { R26 = SFMPY(R10,R18)                  //[[1]Wd.r * D".r                       ]
      R27 = SFMPY(R10,R19)                  //[[1]Wd.r * D".i                       ]
    }:endloop0:endloop1
/* ================================================================================= */
    { R22 -= SFMPY(R7,R21)                  //[[e]Wb*B" = Wb.r*B".r-Wb.i*B".i       ]
      R23 += SFMPY(R7,R20)                  //[[e]Wb*B" = Wb.r*B".i+Wb.i*B".r       ]
      P0 = CMP.GT(R5,#4)                    //[ P0 = ((k2 >>2)> 1)                  ]
    }
    { R24 -= SFMPY(R9,R17)                  //[[e]Wc*C" = Wc.r*C".r-Wc.i*C".i       ]
      R25 += SFMPY(R9,R16)                  //[[e]Wc*C" = Wc.r*C".i+Wc.i*C".r       ]
      MEMD(R1+#0) = R23:22                  //[[e]save B"                           ]
    }
    { R26 -= SFMPY(R11,R19)                 //[[e]Wd*D" = Wd.r*D".r-Wd.i*D".i       ]
      R27 += SFMPY(R11,R18)                 //[[e]Wd*D" = Wd.r*D".i+Wd.i*D".r       ]
      MEMD(R1+R30<<#0) = R25:24             //[[e]save C"                           ]
    }
    { MEMD(R1+R30<<#1) = R27:26             //[[e]save D"                           ]
      R5 = ASR(R5,#2)                       //[ k2 >>= 2                            ]
      R31:30 = VASLW(R31:30,#2)             //[ update -3*8*k1 : 8*k1               ]
    }
    { M0 = R30                              //[ update M0 = 8*k1                    ]
      IF P0 JUMP .sfpFFT_StagesLoop         //[ IF P0 continue next radix-4 stage   ]
    }
/* ================================================================================= */
.sfpFFT_LastStage:
    { R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
      R15 = ADD(R31,#8)                     //[ -3*k1*8 + 8                         ]
      P0 = CMP.EQ(R5,#0)                    //[ P0=(k2==0),i.e.,last stage is Radix2]
      IF P0.new JUMP:nt  .sfpFFT_radix2_LastStage
                                            //[ If P0 do Radix2                     ]
    }
/* ================================================================================= */
.sfpFFT_radix4_LastStage:
    { R2 = ADD(R1,R28)                      //[ R2 = &output[1*N/4]                 ]
      R5 = ASR(R28,#3)                      //[ N/4                                 ]
      M1 = R15                              //[ set M1
    }
    { R3 = ADD(R2,R28)                      //[ R3 = &output[2*N/4]                 ]
      R4 = ADDASL(R2,R28,#1)                //[ R4 = &output[3*N/4]                 ]
      P3 = SP2LOOP0(.sfpFFT_radix4_LastStageLoop,R5)
                                            //[ setup loop0: lc0 = N/4              ]
    }
/* ================================================================================= */
    .falign
.sfpFFT_radix4_LastStageLoop:
    { R14 = SFADD(R6,R8)                    //[[2]A'= A + B   (r)                   ]
      R15 = SFADD(R7,R9)                    //[[2]A'= A + B   (i)                   ]
      IF P3 MEMD(R1++#8) = R15:14           //[[3]save A"                           ]
    }
    { R7:6 = MEMD(R0++M0)                   //[[1]load A                            ]
      R16 = SFSUB(R6,R8)                    //[[2]B'= A - B   (r)                   ]
      R17 = SFSUB(R7,R9)                    //[[2]B'= A - B   (i)                   ]
    }
    { R18 = SFADD(R10,R12)                  //[[2]C'= C + D   (r)                   ]
      R19 = SFADD(R11,R13)                  //[[2]C'= C + D   (i)                   ]
      IF P3 MEMD(R2++#8) = R23:22           //[[3]save B"                           ]
    }
    { R9:8 = MEMD(R0++M0)                   //[[1]load B                            ]
      R20 = SFSUB(R10,R12)                  //[[2]D'= C - D   (r)                   ]
      R21 = SFSUB(R11,R13)                  //[[2]D'= C - D   (i)                   ]
    }
    { R24 = SFSUB(R14,R18)                  //[[2]C"= A'- C'  (r)                   ]
      R25 = SFSUB(R15,R19)                  //[[2]C"= A'- C'  (i)                   ]
      IF P3 MEMD(R3++#8) = R25:24           //[[3]save C"                           ]
    }
    { R11:10 = MEMD(R0++M0)                 //[[1]load C                            ]
      R22 = SFADD(R16,R21)                  //[[2]B"= B'- j*D' (r)                  ]
      R23 = SFSUB(R17,R20)                  //[[2]B"= B'- j*D' (i)                  ]
    }
    { R14 = SFADD(R14,R18)                  //[[1]A"= A'+ C'  (r)                   ]
      R15 = SFADD(R15,R19)                  //[[1]A"= A'+ C'  (i)                   ]
      IF P3 MEMD(R4++#8) = R27:26           //[[3]save D"                           ]
    }
    { R13:12 = MEMD(R0++M1)                 //[[1]load C                            ]
      R26 = SFSUB(R16,R21)                  //[[2]D"= B'+ j*D' (r)                  ]
      R27 = SFADD(R17,R20)                  //[[2]D"= B'+ j*D' (i)                  ]
    }:endloop0
/* ================================================================================= */
    { R14 = SFADD(R6,R8)                    //[[e]A'= A + B   (r)                   ]
      R15 = SFADD(R7,R9)                    //[[e]A'= A + B   (i)                   ]
      IF P3 MEMD(R1++#8) = R15:14           //[[e]save A"                           ]
    }
    { R16 = SFSUB(R6,R8)                    //[[e]B'= A - B   (r)                   ]
      R17 = SFSUB(R7,R9)                    //[[e]B'= A - B   (i)                   ]
      IF P3 MEMD(R2++#8) = R23:22           //[[e]save B"                           ]
    }
    { R18 = SFADD(R10,R12)                  //[[e]C'= C + D   (r)                   ]
      R19 = SFADD(R11,R13)                  //[[e]C'= C + D   (i)                   ]
      IF P3 MEMD(R3++#8) = R25:24           //[[e]save C"                           ]
    }
    { R20 = SFSUB(R10,R12)                  //[[e]D'= C - D   (r)                   ]
      R21 = SFSUB(R11,R13)                  //[[e]D'= C - D   (i)                   ]
      IF P3 MEMD(R4++#8) = R27:26           //[[e]save D"                           ]
    }
    { R24 = SFSUB(R14,R18)                  //[[e]C"= A'- C'  (r)                   ]
      R25 = SFSUB(R15,R19)                  //[[e]C"= A'- C'  (i)                   ]
    }
    { R22 = SFADD(R16,R21)                  //[[e]B"= B'- j*D' (r)                  ]
      R23 = SFSUB(R17,R20)                  //[[e]B"= B'- j*D' (i)                  ]
      MEMD(R3+#0) = R25:24                  //[[e]save C"                           ]
    }
    { R14 = SFADD(R14,R18)                  //[[e]A"= A'+ C'  (r)                   ]
      R15 = SFADD(R15,R19)                  //[[e]A"= A'+ C'  (i)                   ]
      MEMD(R2+#0) = R23:22                  //[[e]save B"                           ]
    }
    { R26 = SFSUB(R16,R21)                  //[[e]D"= B'+ j*D' (r)                  ]
      R27 = SFADD(R17,R20)                  //[[e]D"= B'+ j*D' (i)                  ]
      MEMD(R1+#0) = R15:14                  //[[e]save A"                           ]
    }
    { MEMD(R4+#0) = R27:26                  //[[e]save D"                           ]
      JUMP .sfpFFT_Done                     //[ go to return                        ]
    }
/* ================================================================================= */
    .falign
.sfpFFT_radix2_LastStage:
    { R0 += ASL(R28,#1)                     //[ R0 = &output[N/2] , R1=&output[0]   ]
      R5 = ASR(R28,#3)                      //[ N/4                                 ]
    }
    { R2 = R0                               //[ R2 points to output[N/2]            ]
      P3 = SP1LOOP0(.sfpFFT_radix2_loop,R5) //[ setup loop0: lc0 = N/4              ]
    }
/* ================================================================================= */
    .falign
.sfpFFT_radix2_loop:
    { R7:6 = MEMD(R1++#8)                   //[[1]load A0                           ]
      R16 = SFSUB(R6,R8)                    //[[2]B0'= A0 - B0    (r)               ]
      R17 = SFSUB(R7,R9)                    //[[2]B0'= A0 - B0    (i)               ]
      IF P3 MEMD(R3++#8) = R15:14           //[[2]save A0'                          ]
    }
    { R9:8 = MEMD(R0++#8)                   //[[1]load B0                           ]
      R14 = SFADD(R10,R12)                  //[[2]A1'= A1 + B1    (r)               ]
      R15 = SFADD(R11,R13)                  //[[2]A1'= A1 + B1    (i)               ]
      IF P3 MEMD(R2++#8) = R17:16           //[[2]save B0'                          ]
    }
    { R11:10 = MEMD(R1++#8)                 //[[1]load A1                           ]
      R16 = SFSUB(R10,R12)                  //[[2]B1'= A1 - B1    (r)               ]
      R17 = SFSUB(R11,R13)                  //[[2]B1'= A1 - B1    (i)               ]
      IF P3 MEMD(R3++#8) = R15:14           //[[2]save A1'                          ]
    }
    { R13:12 = MEMD(R0++#8)                 //[[1]load B1                           ]
      R14 = SFADD(R6,R8)                    //[[1]A0'= A0 + B0    (r)               ]
      R15 = SFADD(R7,R9)                    //[[1]A0'= A0 + B0    (i)               ]
      IF P3 MEMD(R2++#8) = R17:16           //[[2]save B0'                          ]
    }:endloop0

    { R16 = SFSUB(R6,R8)                    //[[e]B0'= A0 - B0    (r)               ]
      R17 = SFSUB(R7,R9)                    //[[e]B0'= A0 - B0    (i)               ]
      MEMD(R3++#8) = R15:14                 //[[e]save A0'                          ]
    }
    { R14 = SFADD(R10,R12)                  //[[e]A1'= A1 + B1    (r)               ]
      R15 = SFADD(R11,R13)                  //[[e]A1'= A1 + B1    (i)               ]
      MEMD(R2++#8) = R17:16                 //[[e]save B0'                          ]
    }
    { R16 = SFSUB(R10,R12)                  //[[e]B1'= A1 - B1    (r)               ]
      R17 = SFSUB(R11,R13)                  //[[e]B1'= A1 - B1    (i)               ]
      MEMD(R3+#0) = R15:14                  //[[e]save A1'                          ]
    }
    { MEMD(R2+#0) = R17:16                  //[[e]save B0'                          ]
    }
/* ================================================================================= */
.sfpFFT_Done:
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
      R29 = ADD(R29,#8*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
/* ================================================================================= */
    .size	sfpFFT, .-sfpFFT

