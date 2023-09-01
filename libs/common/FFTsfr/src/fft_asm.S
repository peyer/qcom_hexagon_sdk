/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

    .file   "fft_asm.S"

    /*[*****************************************************************************]*/
    /*[  Function   : void FFTsfc()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform N-point FFT transform on a complex number array       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - complex number: interleaved real/image with real part           ]*/
    /*[             at low memory address                                           ]*/
    /*[           - input  is aligned by size of the array                          ]*/
    /*[           - output is aligned by 8bytes                                     ]*/
    /*[           - N >= 4                                                          ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : cfloat  *input                                           ]*/
    /*[               R1 : int         N                                            ]*/
    /*[               R2 : cfloat  *twiddles                                        ]*/
    /*[               R3 : cfloat  *output                                          ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 56                                ]*/
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
    .globl FFTsfc
    .type	FFTsfc, @function
FFTsfc:
    { P0 = CMP.GT(R1,#3)                    //[ N > 3 ?                             ]
      IF !P0.new JUMPR:nt R31               //[ if !(N>=4) return                   ]
    }
    { R29 = ADD(R29, #-7*8)                 //[ allocate stack                      ]
      R7 = CL0(R1)                          //[ used to calculate log2(N)           ]
      R4 = #0                               //[ R4 = 0                              ]
      R5 = BREV(R0)                         //[ to bit reverse R0.L                 ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R0 = COMBINE(R0.H,R5.H)               //[ bit reverse R0.L (bit-reversed addr)]
      R7 = ADD(R7,#-18)                     //[ 16-log2(8N) = cl0(N)-18             ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R4 = SETBIT(R4,R7)                    //[ R4= 1<<(15-log2(4N))= 1<<(13-LOG2N) ]
      R6 = ASR(R1, #2)                      //[ N/4 (for FFTsfc_1stStageLoop)     ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      M0 = R4                               //[ set M0 for bit-reversed addressing  ]
      R4 = R2                               //[ set R6 = twiddle                    ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      MEMD(R29+#48) = R31:30                //[ callee-saved registers              ]
      R30 = ADD(R1,R1)                      //[ R30 = 2*N                           ]
      R1 = R3                               //[ set R1=  Output                     ]
    }
    { P3=SP1LOOP0(.FFTsfc_1stStageLoop,R6)  //[ setup loop0: lc0 = N/4              ]
    }
    
    .falign
.FFTsfc_1stStageLoop:
    { R11:10 = MEMD(R0++M0:brev)            //[[1] load A                           ]
      R18 = SFMPY(R8,R16)                   //[[2]Wc.r * D".r                       ]
      R19 = SFMPY(R8,R17)                   //[[2]Wc.r * D".i                       ]
    }
    { R13:12 = MEMD(R0++M0:brev)            //[[1]load B                            ]
      R18 -= SFMPY(R9,R17)                  //[[2]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]
      R19 += SFMPY(R9,R16)                  //[[2]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]
    }
    { R15:14 = MEMD(R0++M0:brev)            //[[1]load C                            ]
      R6 = SFADD(R10,R12)                   //[[1]A'= A + B                         ]
      R7 = SFADD(R11,R13)                   //[[1]A'= A + B                         ]
      IF P3 MEMD(R1++#8) = R15:14           //[[2]save C"                           ]
    }
    { R17:16 = MEMD(R0++M0:brev)            //[[1]load D                            ]
      R12 = SFSUB(R10,R12)                  //[[1]B'= A - B                         ]
      R13 = SFSUB(R11,R13)                  //[[1]B'= A - B                         ]
      IF P3 MEMD(R1++#8) = R19:18           //[[2]save D"                           ]
    }
    { R8 = SFADD(R14,R16)                   //[[1]C'= C + D                         ]
      R9 = SFADD(R15,R17)                   //[[1]C'= C + D                         ]
    }
    { R16 = SFSUB(R14,R16)                  //[[1]D'= C - D                         ]
      R17 = SFSUB(R15,R17)                  //[[1]D'= C - D                         ]
    }
    { R10 = SFADD(R6,R8)                    //[[1]A"= A'+ C'                        ]
      R11 = SFADD(R7,R9)                    //[[1]A"= A'+ C'                        ]
    }
    { R8 = SFSUB(R6,R8)                     //[[1]C"= A'- C'                        ]
      R9 = SFSUB(R7,R9)                     //[[1]C"= A'- C'                        ]
      R7:6 = MEMD(R4+#8)                    //[[1]load Wa                           ]
    }
    { R18 = SFADD(R12,R17)                  //[[1]B"= B' - j*D'                     ]
      R19 = SFSUB(R13,R16)                  //[[1]B"= B' - j*D'                     ]
      MEMD(R1++#8) = R11:10                 //[[1]save A"
    }
    { R16 = SFSUB(R12,R17)                  //[[1]D"= B'+ j*D'                      ]
      R17 = SFADD(R13,R16)                  //[[1]D"= B'+ j*D'                      ]
    }
    { R10 = SFMPY(R6,R18)                   //[[1]Wa.r * B".r                       ]
      R11 = SFMPY(R6,R19)                   //[[1]Wa.r * B".i                       ]
    }
    { R10 -= SFMPY(R7,R19)                  //[[1]Wa*B" = Wa.r*B".r-Wa.i*B".i       ]
      R11 += SFMPY(R7,R18)                  //[[1]Wa*B" = Wa.r*B".i+Wa.i*B".r       ]
      R7:6 = MEMD(R4++#16)                  //[[1]load Wb                           ]
    }
    { R14 = SFMPY(R6,R8)                    //[[1]Wb.r * C".r                       ]
      R15 = SFMPY(R6,R9)                    //[[1]Wb.r * C".i                       ]
      MEMD(R1++#8) = R11:10                 //[[1]save B"
    }
    { R14 -= SFMPY(R7,R9)                   //[[1]Wb*C" = Wb.r*C".r-Wb.i*C".i       ]
      R15 += SFMPY(R7,R8)                   //[[1]Wb*C" = Wb.r*C".i+Wb.i*C".r       ]
      R9:8 = MEMD(R4++#8)                   //[[1]load Wc                           ]
    }:endloop0


.FFTsfc_Stage1_END:
    { R18 = SFMPY(R8,R16)                   //[[e]Wc.r * D".r                       ]
      R19 = SFMPY(R8,R17)                   //[[e]Wc.r * D".i                       ]
      MEMD(R1++#8) = R15:14                 //[[e]save C"                           ]
      R21:20 = COMBINE(#-3*4*8,#4*8)        //[ -3*k1*8 :k1*8, where initial k1=4   ]
    }
    { R18 -= SFMPY(R9,R17)                  //[[e]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]
      R19 += SFMPY(R9,R16)                  //[[e]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]
      P0 = CMP.GT(R30,#8)                   //[ P0 = (N > 4)                        ]
      P1 = CMP.GT(R30,#32)                  //[ P1 = (N>16)                         ]
    }
    { MEMD(R1++#8) = R19:18                 //[[e]save D"                           ]
      R5 = ASR(R30,#5)                      //[ initial k2 = N/16                   ]
      IF !P0 JUMP .FFTsfc_DONE              //[ If !(N>4) then return               ]
      IF !P0 R31:30 = MEMD(R29+#48)         //[ and restore callee-saved registers  ]
    }
    { M0 = R20                              //[ set M0 = 8*k1                       ]
      IF !P1 JUMP .FFTsfc_LastStage         //[ if !(N>16) go to last stage         ]
    }

    .falign
.FFTsfc_StagesLOOP:
    { R31 = ASR(R20,#3)                     //[ R31 = k1                            ]
      R15 = ADD(R21,#8)                     //[ -3*k1*8 + 8                         ]
      LOOP1(.FFTsfc_mainLOOP,R5)            //[ setup loop1: lc1 = k2               ]
    }
    { M1 = R15                              //[ set M1 = -3*k1*8 + 8                ]
      R4 = R2                               //[ set R4 = *Wtwiddles                 ]
      R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
      P2 = CMP.GT(R0,R0)                    //[ clean P2                            ]
    }

    .falign
.FFTsfc_mainLOOP:
    { R18 -= SFMPY(R27,R17)                 //[[e1]Wc*D" = Wc.r*D".r-Wc.i*D".i      ]
      R19 += SFMPY(R27,R16)                 //[[e1]Wc*D" = Wc.r*D".i+Wc.i*D".r      ]
      R27:26 = MEMD(R4+#16)                 //[ load Wc                             ]
      R23:22 = MEMD(R4++#8)                 //[ load Wb                             ]
    }
    { IF P2 MEMD(R1++#8) = R19:18           //[[e1]save D"                          ]
      R25:24 = MEMD(R4++#16)                //[ load Wa                             ]
      P3=SP1LOOP0(.FFTsfc_innerLOOP,R31)    //[ setup loop0 with LC= k1             ]
      P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
    }

    .falign
.FFTsfc_innerLOOP:
    { R11:10 = MEMD(R0++M0)                 //[[1]load A                            ]
      R18 = SFMPY(R26,R16)                  //[[2]Wc.r * D".r                       ]
      R19 = SFMPY(R26,R17)                  //[[2]Wc.r * D".i                       ]
    }
    { R13:12 = MEMD(R0++M0)                 //[[1]load B                            ]
      R18 -= SFMPY(R27,R17)                 //[[2]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]
      R19 += SFMPY(R27,R16)                 //[[2]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]
    }
    { R15:14 = MEMD(R0++M0)                 //[[1]load C                            ]
      R6 = SFADD(R10,R12)                   //[[1]A'= A + B                         ]
      R7 = SFADD(R11,R13)                   //[[1]A'= A + B                         ]
      IF P3 MEMD(R1+#0) = R15:14            //[[2]save C"                           ]
    }
    { R17:16 = MEMD(R0++M1)                 //[[1]load D                            ]
      R12 = SFSUB(R10,R12)                  //[[1]B'= A - B                         ]
      R13 = SFSUB(R11,R13)                  //[[1]B'= A - B                         ]
      IF P3 R1 = ADD(R1,R20)                //[[2]update write pointer              ]
    }
    { R8 = SFADD(R14,R16)                   //[[1]C'= C + D                         ]
      R9 = SFADD(R15,R17)                   //[[1]C'= C + D                         ]
      IF P3 MEMD(R1++#8) = R19:18           //[[2]save D"                           ]
    }
    { R16 = SFSUB(R14,R16)                  //[[1]D'= C - D                         ]
      R17 = SFSUB(R15,R17)                  //[[1]D'= C - D                         ]
      IF P3 R1 = ADD(R1,R21)                //[[2]update write pointer              ]
    }
    { R10 = SFADD(R6,R8)                    //[[1]A"= A'+ C'                        ]
      R11 = SFADD(R7,R9)                    //[[1]A"= A'+ C'                        ]
    }
    { R8 = SFSUB(R6,R8)                     //[[1]C"= A'- C'                        ]
      R9 = SFSUB(R7,R9)                     //[[1]C"= A'- C'                        ]
    }
    { R18 = SFADD(R12,R17)                  //[[1]B"= B'- j*D'                      ]
      R19 = SFSUB(R13,R16)                  //[[1]B"= B'- j*D'                      ]
      MEMD(R1++M0) = R11:10                 //[[1]save A"
    }
    { R16 = SFSUB(R12,R17)                  //[[1]D"= B'+ j*D'                      ]
      R17 = SFADD(R13,R16)                  //[[1]D"= B'+ j*D'                      ]
    }
    { R10 = SFMPY(R24,R18)                  //[[1]Wa.r * B".r                       ]
      R11 = SFMPY(R24,R19)                  //[[1]Wa.r * B".i                       ]
    }
    { R10 -= SFMPY(R25,R19)                 //[[1]Wa*B" = Wa.r*B".r-Wa.i*B".i       ]
      R11 += SFMPY(R25,R18)                 //[[1]Wa*B" = Wa.r*B".i+Wa.i*B".r       ]
    }
    { R14 = SFMPY(R22,R8)                   //[[1]Wb.r * C".r                       ]
      R15 = SFMPY(R22,R9)                   //[[1]Wb.r * C".i                       ]
      MEMD(R1++M0) = R11:10                 //[[1]save B"
    }
    { R14 -= SFMPY(R23,R9)                  //[[1]Wb*C" = Wb.r*C".r-Wb.i*C".i       ]
      R15 += SFMPY(R23,R8)                  //[[1]Wb*C" = Wb.r*C".i+Wb.i*C".r       ]
    }:endloop0

    { R18 = SFMPY(R26,R16)                  //[[e]Wc.r * D".r                       ]
      R19 = SFMPY(R26,R17)                  //[[e]Wc.r * D".i                       ]
      MEMD(R1++M0) = R15:14                 //[[e]save C"                           ]
      R0 = SUB(R0,R21)                      //[ R0 = R0 + 3*M0                      ]
    }:endloop1

    { R18 -= SFMPY(R27,R17)                 //[[e]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]
      R19 += SFMPY(R27,R16)                 //[[e]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]
    }
    { MEMD(R1) = R19:18                     //[[e]save D"                           ]
      R21:20 = VASLW(R21:20,#2)             //[ update -3*8*k1 : 8*k1               ]
      R5 = ASR(R5,#2)                       //[ k2 >>= 2                            ]
      P0 = CMP.GT(R5,#4)                    //[ P0 = ((k2 >>2)> 1)                  ]
    }
    { M0 = R20                              //[ update M0 = 8*k1                    ]
      IF P0 JUMP .FFTsfc_StagesLOOP         //[ IF P0 continue next radix-4 stage   ]
    }

.FFTsfc_LastStage:
    { P0 = CMP.EQ(R5,#0)                    //[ P0=(k2==0),i.e.,last stage is Radix2]
      R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
      R15 = ADD(R21,#8)                     //[ -3*k1*8 + 8                         ]
      IF P0.new JUMP:nt  .FFTsfc_RADIX2_LastStage    
                                            //[ If P0 do Radix2                     ]
    }
    /*-----------------------------------------------------------------------------*/
    /*                       Last Radix-4 stage                                    */
    /*-----------------------------------------------------------------------------*/
    { R2 = ADD(R1,R30)                      //[ R2 = &output[1*N/4]                 ]
      R5 = ASR(R30,#3)                      //[ N/4                                 ]
      M1 = R15                              //[ set M1
    }
    { R3 = ADD(R2,R30)                      //[ R3 = &output[2*N/4]                 ]
      R4 = ADDASL(R2,R30,#1)                //[ R4 = &output[3*N/4]                 ]
      P3 = SP1LOOP0(.FFTsfc_RADIX4_LastStageLoop,R5)
                                            //[ setup loop0: lc0 = N/4              ]
    }

    .falign
.FFTsfc_RADIX4_LastStageLoop:
    { R11:10 = MEMD(R0++M0)                 //[[1] load A                           ]
      R18 = SFADD(R12,R17)                  //[[2]B"= B'- j*D'                      ]
      R19 = SFSUB(R13,R16)                  //[[2]B"= B'- j*D'                      ]
    }
    { R13:12 = MEMD(R0++M0)                 //[[1]load B                            ]
      R16 = SFSUB(R12,R17)                  //[[2]D"= B'+ j*D'                      ]
      R17 = SFADD(R13,R16)                  //[[2]D"= B'+ j*D'                      ]
      IF P3 MEMD(R2++#8) = R19:18           //[[2]save B"                           ]
    }
    { R15:14 = MEMD(R0++M0)                 //[[1]load C                            ]
      R6 = SFADD(R10,R12)                   //[[1]A'= A + B                         ]
      R7 = SFADD(R11,R13)                   //[[1]A'= A + B                         ]
      IF P3 MEMD(R3++#8) = R9:8             //[[2]save C"                           ]
    }
    { R17:16 = MEMD(R0++M1)                 //[[1]load D                            ]
      R12 = SFSUB(R10,R12)                  //[[1]B'= A - B                         ]
      R13 = SFSUB(R11,R13)                  //[[1]B'= A - B                         ]
      IF P3 MEMD(R4++#8) = R17:16           //[[2]save D"                           ]
    }
    { R8 = SFADD(R14,R16)                   //[[1]C'= C + D                         ]
      R9 = SFADD(R15,R17)                   //[[1]C'= C + D                         ]
    }
    { R16 = SFSUB(R14,R16)                  //[[1]D'= C - D                         ]
      R17 = SFSUB(R15,R17)                  //[[1]D'= C - D                         ]
    }
    { R10 = SFADD(R6,R8)                    //[[1]A"= A'+ C'                        ]
      R11 = SFADD(R7,R9)                    //[[1]A"= A'+ C'                        ]
    }
    { R8 = SFSUB(R6,R8)                     //[[1]C"= A'- C'                        ]
      R9 = SFSUB(R7,R9)                     //[[1]C"= A'- C'                        ]
      MEMD(R1++#8) = R11:10                 //[[1]save A"                           ]
    }:endloop0

    { R18 = SFADD(R12,R17)                  //[[e]B"= B'- j*D'                      ]
      R19 = SFSUB(R13,R16)                  //[[e]B"= B'- j*D'                      ]
      MEMD(R3) = R9:8                       //[[e]save C"                           ]
    }
    { R16 = SFSUB(R12,R17)                  //[[e]D"= B'+ j*D'                      ]
      R17 = SFADD(R13,R16)                  //[[e]D"= B'+ j*D'                      ]
      MEMD(R2) = R19:18                     //[[e]save B"                           ]
    }
    { MEMD(R4) = R17:16                     //[[e]save D"                           ]
      R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
      JUMP .FFTsfc_DONE                     //[ go to return                        ]
    }

    /*-----------------------------------------------------------------------------*/
    /*                       Last Radix-2 stage                                    */
    /*-----------------------------------------------------------------------------*/
    .falign
.FFTsfc_RADIX2_LastStage:
    { R0 += ASL(R30,#1)                     //[ R0 = &output[N/2] , R1=&output[0]   ]
      R5 = ASR(R30,#3)                      //[ N/4                                 ]
    }
    { R2 = R0                               //[ R2 points to output[N/2]            ]
      P3 = SP1LOOP0(.FFTsfc_RADIX2_LOOP,R5) //[ setup loop0: lc0 = N/4              ]
      R7:6 = MEMD(R1++#8)                   //[[p]load A                            ]
    }

    .falign
.FFTsfc_RADIX2_LOOP:
    { R9:8 = MEMD(R0++#8)                   //[[2]load B      (1)                   ]
      R10 = SFADD(R10,R12)                  //[[3]A'= A+B     (2)                   ]
      R11 = SFADD(R11,R13)                  //[[3]A'= A+B                           ]
      IF P3 MEMD(R2++#8) = R9:8             //[[3]save B'     (2)                   ]
    }
    { R11:10 = MEMD(R1++#8)                 //[[2]load A      (2)                   ]
      R12 = SFSUB(R6,R8)                    //[[2]B'= A-B     (1)                   ]
      R13 = SFSUB(R7,R9)                    //[[2]B'= A-B                           ]
      IF P3 MEMD(R3++#8) = R11:10           //[[3]save A'     (2)                   ]
    }
    { MEMD(R2++#8) = R13:12                 //[[2]save B'     (1)                   ]
      R13:12 = MEMD(R0++#8)                 //[[2]load B                            ]
      R6 = SFADD(R6,R8)                     //[[2]A'= A+B     (1)                   ]
      R7 = SFADD(R7,R9)                     //[[2]A'= A+B                           ]
    }
    { R7:6 = MEMD(R1++#8)                   //[[1]load A       (1)                  ]
      MEMD(R3++#8) = R7:6                   //[[2]save A'      (1)                  ]
      R8 = SFSUB(R10,R12)                   //[[2]B'= A-B      (2)                  ]
      R9 = SFSUB(R11,R13)                   //[[2]B'= A-B                           ]
    }:endloop0

    { R10 = SFADD(R10,R12)                  //[[e]A'= A+B     (2)                   ]
      R11 = SFADD(R11,R13)                  //[[e]A'= A+B                           ]
      MEMD(R2) = R9:8                       //[[e]save B'     (2)                   ]
    }
    { MEMD(R3) = R11:10                     //[[e]save A'     (2)                   ]
      R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
    }

    .falign
.FFTsfc_DONE:
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
    .size	FFTsfc, .-FFTsfc

    /*[*****************************************************************************]*/
    /*[  Function   : void FFTsfr()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: performs N-point real-valued FFT transform                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - complex number: interleaved real/image with real part           ]*/
    /*[             at low memory address                                           ]*/
    /*[           - input  is aligned by size of the array                          ]*/
    /*[           - output is aligned by 8bytes                                     ]*/
    /*[           - N >= 8                                                          ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : float *input                                             ]*/
    /*[               R1 : int       N                                              ]*/
    /*[               R2 : cfloat *Wt1    -- twiddle factors for N/2-point cxFFT    ]*/
    /*[               R3 : cfloat *Wt2    -- twiddel factors for last stage         ]*/
    /*[               R4 : cfloat *output                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R19, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 16+16                             ]*/
    /*[=============================================================================]*/
    /*[  Implementation:                                                            ]*/
    /*[           - Radix-4 DIF                                                     ]*/
    /*[           - INPUT: LOAD IN BIT-REVERSE ORDER                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl FFTsfr
    .type    FFTsfr, @function
FFTsfr:
    { P0 = CMP.GT(R1,#7)                    //[ if !(N>=8)                          ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
    }
    { ALLOCFRAME(#2*8)                      //[ reserve stack                       ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R19:18 = COMBINE(R3,R1)               //[ R19 = wt2,  R18 = N                 ]
      R3 = R4                               //[ func call: R3 = output              ]
    }
    /* --------------------------------------------------------------------------- */
    /*                    Call N/2-point FFT                                       */
    /* --------------------------------------------------------------------------- */
    { R1 = LSR(R1,#1)                       //[ func call: R1 = N/2                 ]
      R16 = R4                              //[ R16 = output                        ]
      CALL FFTsfc                           //[ FFTsfc()                            ]
    }
    .falign
.FFTsfr_LastStage:
    /* --------------------------------------------------------------------------- */
    /*                    Calcuate last stage butterflies                          */
    /* --------------------------------------------------------------------------- */
    { R0 = R16                              //[ R0 = &Z[0]                          ]
      R1 = ADDASL(R16,R18,#2)               //[ &Z[N/2]                             ]
      R6 = #0                               //[ constant 0                          ]
      R7 = ASR(R18,#2)                      //[ N/4
    }
    { R3:2 = R1:0                           //[ R2 = &Z[0]; R3 = &Z[N/2]            ]
      R1 = ADD(R1,#-8)                      //[ R1 = &Z[N/2-1]                      ]
      R5:4 = MEMD(R0++#8)                   //[ load Z[0]                           ]
      R28 = ADDASL(R16,R18,#3)              //[ R28 = &output[N]                    ]
    }
    { R9:8   = COMBINE(R6,R4)               //[ X = Z.r + 1i*0                      ]
      R15:14 = COMBINE(R6,R5)               //[ Y = Z.i + 1i*0                      ]
      R17 = ##0x3F000000                    //[ constant 0.5                        ]
    }
    { LOOP0(.FFTsfr_LastStage_LOOP,R7)     //[ setup loop0: lc0 = N/4              ]
      R18 = ASR(R18,#1)                     //[ N/2                                 ]
    }

    .falign
.FFTsfr_LastStage_LOOP:
    { R5:4 = MEMD(R0++#8)                   //[[1]load Z[i]                         ]
      R10 = SFADD(R8,R14)                   //[[2]X + Y                             ]
      R11 = SFADD(R9,R15)                   //[[2]                                  ]
    }
    { R7:6 = MEMD(R1++#-8)                  //[[1]load Z[N/2-i]                     ]
      R14 = SFSUB(R8,R14)                   //[[2]conj(X-Y)                         ]
      R15 = SFSUB(R15,R9)                   //[[2]                                  ]
    }
    { R8 = SFADD(R4,R6)                     //[[1]X= Z[i] + Z*[N/2-i]               ]
      R9 = SFSUB(R5,R7)                     //[[1]                                  ]
      MEMD(R2++#8) = R11:10                 //[[2]save output[i]                    ]
    }
    { R10= SFSUB(R4,R6)                     //[[1]Y=Z[i] - Z*[N/2-i]                ]
      R11= SFADD(R5,R7)                     //[[1]                                  ]
      MEMD(R3++#-8)= R15:14                 //[[2]save output[N/2-i]                ]
    }
    { R8 = SFMPY(R8,R17)                    //[[1]X = 0.5*X                         ]
      R9 = SFMPY(R9,R17)                    //[[1]                                  ]
      R13:12 = MEMD(R19++#8)                //[[1]load W                            ]
    }
    { R14 = SFMPY(R12,R10)                  //[[1]W.r * Y.r                         ]
      R15 = SFMPY(R12,R11)                  //[[1]W.r * Y.i                         ]
    }
    { R14 -= SFMPY(R13,R11)                 //[[1]Y = W*Y = W.r*Y.r-W.i*Y.i         ]
      R15 += SFMPY(R13,R10)                 //[[1]Y = W*Y = W.r*Y.i+W.i*Y.r         ]
    }:endloop0

    { R10 = SFADD(R8,R14)                   //[[e]X + Y                             ]
      R11 = SFADD(R9,R15)                   //[[e]                                  ]
      R0 = ADD(R16,#8)                      //[ R0 =&output[1]                      ]
    }
    { MEMD(R2+#0) = R11:10                  //[[e]save output[N/4]                  ]
      R1 = ADD(R28,#-8)                     //[ R1 =&output[N-1]                    ]
      R13 = ASL(R17,#7)                     //[ 0x80000000                          ] 
      P3 =SP2LOOP0(.FFTsfr_CONJ_LOOP,R18)  //[ setup loop0 with lc0 = N/2          ]
    }

    .falign
.FFTsfr_CONJ_LOOP: 
    { R5:4 = MEMD(R0++#8)                   //[[1]load output[i]                    ]
      R6 = R4                               //[[2]conj(output[i]                    ]
      R7 = XOR(R5,R13)                      //[[2]                                  ]
      IF P3 MEMD(R1++#-8) = R7:6            //[[3]save output[N-i]                  ]
    }:endloop0

    { MEMD(R1+#0) = R7:6                    //[[e]save output[N-i]                  ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      DEALLOC_RETURN                        //[ pop stack  and return               ]
    }
    .size    FFTsfr, .-FFTsfr


