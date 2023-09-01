/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "fft32x16.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void fft32x16()                                               ]*/
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
    /*[  Inputs     : R0 : CWord2x32 *input                                         ]*/
    /*[               R1 : int       N                                              ]*/
    /*[               R2 : CWord2x16 *twiddles                                      ]*/
    /*[               R3 : CWord2x32 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48                                ]*/
    /*[=============================================================================]*/
    /*[  Implementation:                                                            ]*/
    /*[           - Radix-4 DIF                                                     ]*/
    /*[           - INPUT: LOAD IN BIT-REVERSE ORDER                                ]*/
    /*[           - TWIDDLE FACTOR ARRAY: IN BIT-REVERSE ORDER & INTERLEAVED        ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - N =    4^k    (9N/4+5)*K - (11N+4)/12 + 8                       ]*/
    /*[           - N = 2*(4^K)   (9N/4+5)*K + (13N-8)/12 + 14                      ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  Deqiang Chen     08/29/2008         created                                ]*/
    /*[  MZ               10/15/2008         updated                                ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl fft32x16
    .type	fft32x16, @function
fft32x16:
    { P0 = CMP.GT(R1,#3)                    //[ P0 = N > 3                          ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
    }
    { R29 = ADD(R29, #-6*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-6*8)) = R17:16           //[ callee-saved registers              ]
      R7 = CL0(R1)                          //[ used to calculate log2(N)           ]
      R5 = BREV(R0)                         //[ to bit reverse R0.L                 ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R0 = COMBINE(R0.H,R5.H)               //[ bit reverse R0.L (bit-reversed addr)]
      R4 = #0                               //[ R4 = 0                              ]
      R7 = ADD(R7,#-18)                     //[ 16-log2(8N) = cl0(N)-18             ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R4 = SETBIT(R4,R7)                    //[ R4= 1<<(15-log2(4N))= 1<<(13-LOG2N) ]
      R6 = ASR(R1, #2)                      //[ N/4 (for fft32x16_1stStageLoop)     ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      M0 = R4                               //[ set M0 for bit-reversed addressing  ]
      R4 = R2                               //[ set R6 = twiddle                    ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R28 = R1                              //[ save R1, i.e., N, in R28            ]
      R1 = R3                               //[ set R1=  Output                     ]
      P3=SP1LOOP0(.fft32x16_1stStageLoop,R6)//[ setup loop0: lc0 = N/4              ]
    }
    
    .falign
.fft32x16_1stStageLoop:
    { R11:10 = MEMD(R0++M0:brev)            //[[1] load A                           ]
      R15:14 = VMPYWEH(R15:14,R13:12):<<1:rnd:sat  
                                            //[[2]Im(C")*Re(Wb): Re(C")*Re(Wb)      ]
      R7:6 = VMPYWEH(R17:16,R9:8):<<1:rnd:sat
                                            //[[2]Im(D")*Re(Wc): Re(D")*Re(Wc)      ]
      IF P3 MEMD(R1++#8 ) = R11:10          //[[2]Store B"                          ]
    }
    { R13:12 = MEMD(R0++M0:brev)            //[[1]load B                            ]
      R15:14 +=VMPYWOH(R19:18,R13:12):<<1:rnd:sat 
                                            //[[2]C"*Wb= Im(C")*Re(Wb)+Re(C")*Im(Wb)]
                                            //[         : Re(C")*Re(Wb)-Im(C)*Im(Wb)]
      R18 = NEG(R17):sat                    //[[2]R19:18= Re(D"): -Im(D")           ]
      R19 = R16                             //[[2]R19:18= Re(D"): -Im(D")           ]
    }
    { R15:14 = MEMD(R0++M0:brev)            //[[1]load C                            ]
      R11:10 = VADDW(R11:10,R13:12):sat     //[[1]A'= A+B                           ]
      R13:12 = VSUBW(R11:10,R13:12):sat     //[[1]B'= A-B                           ]
      IF P3 MEMD(R1++#8) = R15:14           //[[2]Store C"                          ]
    }
    { R17:16 = MEMD(R0++M0:brev)            //[[1]load D                            ]
      R12 = NEG(R13):sat                    //[[1]B'= j*B'                          ]
      R13 = R12                             //[[1]B'= j*B'                          ]
      R7:6+=VMPYWOH(R19:18,R9:8):<<1:rnd:sat//[[2]D"*Wc= Im(D")*Re(Wc)+Re(D")*Im(Wc)]
                                            //[         :Re(D")*Re(Wc)-Im(D")*Im(Wc)]
    }
    { R15:14 = VADDW(R15:14,R17:16):sat     //[[1]C'= C+D                           ]
      R17:16 = VSUBW(R15:14,R17:16):sat     //[[1]D'= C-D                           ]
      R8 = MEMW(R4+#4)                      //[[1]Load Wa                           ]
      IF P3 MEMD(R1++#8) = R7:6             //[[2]Store D"                          ]
    }
    { R11:10 = VADDW(R11:10,R15:14):sat     //[[1]A"= A'+C'                         ]
      R15:14 = VSUBW(R11:10,R15:14):sat     //[[1]C"= A'-C'                         ]
      R9 = R8                               //[[1]R9:8 = Wa : Wa                    ]
      R5 = MEMW(R4+#8)                      //[[1]Load Wc                           ]
    }
    { R13:12 = VADDW(R13:12,R17:16):sat     //[[1]B"= B'+D'                         ]
      R17:16 = VSUBW(R13:12,R17:16):sat     //[[1]D"= B'-D'                         ]
      MEMD(R1++#8) = R11:10                 //[[1]Store A"                          ]
      R10 = MEMW(R4++#12)                   //[[1]Load Wb                           ]
    }
    { R11:10=VMPYWEH(R13:12,R9:8):<<1:rnd:sat//[[1]Im(B")*Re(Wa): Re(B")*Re(Wa)     ]
      R18 = NEG(R13):sat                    //[[1]R19:18= Re(B"): -Im(B")           ]
      R19 = R12                             //[[1]R19:18= Re(B"): -Im(B")           ]
      R13:12 = COMBINE(R10,R10)             //[[1]R13:12 = Wb : Wb                  ]
    }
    { R11:10 += VMPYWOH(R19:18,R9:8):<<1:rnd:sat 
                                            //[[1]B"*Wa= Im(B")*Re(Wa)+Re(B")*Im(Wa)]
                                            //[        : Re(B")*Re(Wa)-Im(B")*Im(Wa)]
      R18 = NEG(R15):sat                    //[[1]R19:18= Re(C"): -Im(C")           ]
      R19 = R14                             //[[1]R19:18= Re(C"): -Im(C")           ]
      R9:8 = COMBINE(R5,R5)                 //[[1]R9:8 = Wc : Wc                    ]
    }:endloop0

.fft32x16_Stage1_END:
    { MEMD(R1++#8 ) = R11:10                //[[e]Store B"                          ]
      R15:14 = VMPYWEH(R15:14,R13:12):<<1:rnd:sat  
                                            //[[e]Im(C")*Re(Wb): Re(C")*Re(Wb)      ]
      R7:6   = VMPYWEH(R17:16,R9:8):<<1:rnd:sat
                                            //[[e]Im(D")*Re(Wc): Re(D")*Re(Wc)      ]
      R23:22 = COMBINE(#-3*4*8,#4*8)        //[ -3*k1*8 :k1*8, where initial k1=4   ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R15:14 += VMPYWOH(R19:18, R13:12):<<1:rnd:sat 
                                            //[[e]C"*Wb= Im(C")*Re(Wb)+Re(C")*Im(Wb)]
                                            //[        : Re(C")*Re(Wb)-Im(C")*Im(Wb)]
      R18 = NEG(R17):sat                    //[[e]
      R19 = R16                             //[[e]R21:20= Re(D"): -Im(D")           ]
    }
    { MEMD(R1++#8) = R15:14                 //[[e]Store C""                         ]
      R7:6+=VMPYWOH(R19:18,R9:8):<<1:rnd:sat//[[e]D"*Wc= Im(D")*Re(Wc)+Re(D")*Im(Wc)]
                                            //[        : Re(D")*Re(Wc)-Im(D")*Im(Wc)]
      M0 = R22                              //[ set M0 = 8*k1                       ]
      P0 = CMP.GT(R28,#4)                   //[ P0 = (N > 4)                        ]
    }
    { MEMD(R1) = R7:6                       //[[e]Store D"                          ]
      IF !P0 JUMP .fft32x16_DONE            //[ If !(N>4) then return               ]
      P1 = CMP.GT(R28,#16)                  //[ P1 = (N>16)                         ]
    }
    { R5 = ASR(R28,#4)                      //[ initial k2 = N/16                   ]
      R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
      R15 = ADD(R23,#8)                     //[ -3*k1*8 + 8                         ]
      IF !P1 JUMP .fft32x16_LastStage       //[ if !(N>16) go to last stage         ]
    }

    .falign
.fft32x16_StagesLOOP:
    { R4 = ASR(R22,#3)                      //[ R4 = k1                             ]
      LOOP1(.fft32x16_mainLOOP,R5)          //[ setup loop1: lc1 = k2               ]
      R11:10 = MEMD(R0++M0)                 //[ [p0]load A                          ]
    }
    { M1 = R15                              //[ set M1 = -3*k1*8 + 8                ]
      R4 = ADD(R4,#-1)                      //[ R4 = k1-1                           ]
      R6 = R2                               //[ set R6 = *Wtwiddles
      R13:12 = MEMD(R0++M0)                 //[[p0]load B                           ]
    }
    { R15:14 = MEMD(R0++M0)                 //[[p0]load C                           ]
      R11:10 = VADDW(R11:10,R13:12):sat     //[[p0]A'= A+B                          ]
      R13:12 = VSUBW(R11:10,R13:12):sat     //[[p0]B'= A-B                          ]
      R24 = #1                              //[ R24=i+1, initial i=0                ]
    }

    .falign
.fft32x16_mainLOOP:
    { R17:16 = MEMD(R0++M1)                 //[[p0]load D                           ]
      R12 = NEG(R13):sat                    //[[p0]B'= j*B'                         ]
      R13 = R12                             //[[p0]B'= j*B'                         ]
      LOOP0(.fft32x16_innerLOOP,R4)         //[ setup loop0 with LC= k2-1           ]
    }
    { R15:14 = VADDW(R15:14,R17:16):sat     //[[p0]C'= C+D                          ]
      R17:16 = VSUBW(R15:14,R17:16):sat     //[[p0]D'= C-D                          ]
      R7 = MEMW(R6+#4)                      //[Load Wa                              ]
      R9 = MEMW(R6+#8)                      //[Load Wc                              ]
    }
    { R8 = MEMW( R6++#12)                   //[ Load Wb                             ]
      R11:10 = VADDW(R11:10,R15:14):sat     //[[p]A"= A'+C'                         ]
      R15:14 = VSUBW(R11:10,R15:14):sat     //[[p]C"= A'-C'                         ]
      R19:18 = COMBINE(R7,R7)               //[[p]R19:18 = Wa : Wa                  ]
    }
    { P1 = CMP.EQ(R24,R5)                   //[ P1 = (i+1 == K2)?, i.e. last iter   ]
      R13:12 = VADDW(R13:12,R17:16):sat     //[[p]B"= B'+D'                         ]
      R17:16 = VSUBW(R13:12,R17:16):sat     //[[p]D"= B'-D'                         ]
      MEMD(R1++M0) = R11:10                 //[[p]Store A"                          ]
    }
    { R11:10 = VMPYWEH(R13:12,R19:18):<<1:rnd:sat  
                                            //[[p]Im(B")*Re(Wa): Re(B")*Re(Wa)      ]
      R21 = R12                             //[[p]R21:20= Re(B"): -Im(B")           ]
      R20 = NEG(R13):sat                    //[[p]                                  ]
      R13:12 = COMBINE(R8,R8)               //[[p]R13:12 = Wb : Wb                  ]
    }
    { R11:10 += VMPYWOH(R21:20,R19:18):<<1:rnd:sat 
                                            //[[p]B"*Wa= Im(B")*Re(Wa)+Re(B")*Im(Wa)]
                                            //[        : Re(B")*Re(Wa)-Im(B")*Im(Wa)]
      R21 = R14                             //[[p]R21:20= Re(C"): -Im(C")           ]
      R20 = NEG(R15):sat                    //[[p]                                  ]
      R19:18 = COMBINE(R9,R9)               //[[p]R19:18 = Wc : Wc                  ]
    }

    .falign
.fft32x16_innerLOOP:
    { R11:10 = MEMD(R0++M0)                 //[[1] load A                           ]
      R15:14 = VMPYWEH(R15:14,R13:12):<<1:rnd:sat  
                                            //[[2]Im(C")*Re(Wb): Re(C")*Re(Wb)      ]
      R27:26 = VMPYWEH(R17:16,R19:18):<<1:rnd:sat  
                                            //[[2]Im(D")*Re(Wc): Re(D")*Re(Wc)      ]
      MEMD( R1++M0 ) = R11:10               //[[2]Store B"
    }
    { R13:12 = MEMD(R0++M0)                 //[[1]load B                            ]
      R15:14 += VMPYWOH(R21:20, R13:12):<<1:rnd:sat 
                                            //[[2]C"*Wb= Im(C")*Re(Wb)+Re(C")*Im(Wb)]
                                            //[        : Re(C")*Re(Wb)-Im(C")*Im(Wb)]
      R21 = R16                             //[[2]R21:20= Re(D"): -Im(D")           ]
      R20 = NEG(R17):sat                    //[[2]                                  ]
    }
    { R15:14 = MEMD(R0++M0)                 //[[1]load C                            ]
      R11:10 = VADDW(R11:10,R13:12):sat     //[[1]A'= A+B                           ]
      R13:12 = VSUBW(R11:10,R13:12):sat     //[[1]B'= A-B                           ]
      MEMD( R1++M0) = R15:14                //[[2]Store C"                          ]
    }
    { R17:16 = MEMD(R0++M1)                 //[[1]load D                            ]
      R12 = NEG(R13):sat                    //[[1]j*B'                              ]
      R13 = R12                             //[[1]j*B'                              ]
      R27:26 += VMPYWOH(R21:20, R19:18):<<1:rnd:sat 
                                            //[[2]D"*Wc= Im(D")*Re(Wc)+Re(D")*Im(Wc)]
                                            //[        : Re(D")*Re(Wc)-Im(D")*Im(Wc)]
    }
    { R15:14 = VADDW(R15:14,R17:16):sat     //[[1]C'= C+D                           ]
      R17:16 = VSUBW(R15:14,R17:16):sat     //[[1]D'= C-D                           ]
      MEMD(R1++M1) = R27:26                 //[[2]Store D"                          ]
    }
    { R11:10 = VADDW(R11:10,R15:14):sat     //[[1]A"= A'+C'                         ]
      R15:14 = VSUBW(R11:10,R15:14):sat     //[[1]C"= A'-C'                         ]
      R19:18 = COMBINE(R7,R7)               //[[1]R19:18 = Wa : Wa                  ]
    }
    { R13:12 = VADDW(R13:12,R17:16):sat     //[[1]B"= jB'+D'                        ]
      R17:16 = VSUBW(R13:12,R17:16):sat     //[[1]D"= jB'-D'                        ]
      MEMD(R1++M0) = R11:10                 //[[1]Store A"                          ]
    }
    { R11:10 = VMPYWEH(R13:12, R19:18):<<1:rnd:sat  
                                            //[[1]Im(B")*Re(Wa): Re(B")*Re(Wa)      ]
      R21 = R12                             //[[1]R21:20= Re(B"): -Im(B")           ]
      R20 = NEG(R13):sat                    //[[1]                                  ]
      R13:12 = COMBINE(R8,R8)               //[[1]R13:12 = Wb : Wb                  ]
    }
    { R11:10 += VMPYWOH(R21:20, R19:18):<<1:rnd:sat 
                                            //[[1]B"*Wa= Im(B")*Re(Wa)+Re(B")*Im(Wa)]
                                            //[        : Re(B")*Re(Wa)-Im(B")*Im(Wa)]
      R21 = R14                             //[[1]R21:20= Re(C"): -Im(C")           ]
      R20 = NEG(R15):sat                    //[[1]                                  ]
      R19:18 = COMBINE(R9,R9)               //[[1]R19:18 = Wc : Wc                  ]
    }:endloop0

    { MEMD( R1++M0 ) = R11:10               //[[e]Store B"                          ]
      R15:14 = VMPYWEH(R15:14, R13:12):<<1:rnd:sat  
                                            //[[e]Im(C")*Re(Wb): Re(C")*Re(Wb)      ]
      R27:26 = VMPYWEH(R17:16, R19:18):<<1:rnd:sat  
                                            //[[e]Im(D")*Re(Wc): Re(D")*Re(Wc)      ]
      IF (!P1) R0 = SUB(R0,R23)             //[ R0 = R0+3*M0                        ]
                                            //[ if it's last iteration, no update to]
                                            //[ control out-of-range load)          ]
    }
    { R15:14 += VMPYWOH(R21:20, R13:12):<<1:rnd:sat 
                                            //[[e]C"*Wb= Im(C")*Re(Wb)+Re(C")*Im(Wb)]
                                            //[        : Re(C")*Re(Wb)-Im(C")*Im(Wb)]
      R21 = R16                             //[[e]R21:20= Re(D"): -Im(D")           ]
      R20 = NEG(R17):sat                    //[[e]                                  ]
      R11:10 = MEMD(R0++M0)                 //[[p0] load A                          ]
    }
    { MEMD( R1++M0) = R15:14                //[[e]Store C"                          ]
      R27:26 += VMPYWOH(R21:20, R19:18):<<1:rnd:sat 
                                            //[[e]D"*Wc= Im(D")*Re(Wc)+Re(D")*Im(Wc)]
                                            //[        : Re(D")*Re(Wc)-Im(D")*Im(Wc)]
      R13:12 = MEMD(R0++M0)                 //[[p0]load B                           ]
      R24 = ADD(R24,#1)                     //[ i++                                 ]
    }
    { MEMD(R1++#8) = R27:26                 //[[e]Store D"                          ]
      R11:10 = VADDW(R11:10,R13:12):sat     //[[p0]A'= A+B                          ]
      R13:12 = VSUBW(R11:10,R13:12):sat     //[[p0]B'= A-B                          ]
      R15:14 = MEMD(R0++M0)                 //[[p0]load C 
    }:endloop1 
    

    { R23:22 = VASLW(R23:22,#2)             //[ update -3*8*k1 : 8*k1               ]
      R1:0 = COMBINE(R3,R3)                 //[ R1:0 = output                       ]
      R5 = ASR(R5,#2)                       //[ k2 >>= 2                            ]
      P0 = CMP.GT(R5,#4)                    //[ P0 = ((k2 >>2)> 1)                  ]
    }
    { M0 = R22                              //[ update M0 = 8*k1                    ]
      R15 = ADD(R23,#8)                     //[ -3*k1*8 + 8                         ]
      IF P0 JUMP .fft32x16_StagesLOOP       //[ IF P0 continue next radix-4 stage   ]
    }


.fft32x16_LastStage:
    { P0 = CMP.EQ(R5,#0)                    //[ P0=(k2==0),i.e.,last stage is Radix2]
      IF P0.new JUMP:nt  .fft32x16_RADIX2_LastStage    
                                            //[ If P0 do Radix2                     ]
    }
    /*-----------------------------------------------------------------------------*/
    /*                       Last Radix-4 stage                                    */
    /*-----------------------------------------------------------------------------*/
    { R2 = ADD(R1,R22)                      //[ R2 = &output[1*N/4]                 ]
      R5 = ASR(R22,#3)                      //[ N/4                                 ]
      M1 = R15                              //[ set M1
    }
    { R3 = ADD(R2,R22)                      //[ R3 = &output[2*N/4]                 ]
      R4 = ADDASL(R2,R22,#1)                //[ R4 = &output[3*N/4]                 ]
      P3 = SP1LOOP0(.fft32x16_RADIX4_LastStageLoop,R5)
                                            //[ setup loop0: lc0 = N/4              ]
      R7:6 = MEMD(R0++M0)                   //[[p]load A                            ]
    }

    .falign
.fft32x16_RADIX4_LastStageLoop:
    { R9:8 = MEMD(R0++M0)                   //[[2]load B                            ]
      R17:16 = VADDW(R9:8,R11:10):sat       //[[3]A" = A' + C'                      ]
      R11:10 = VSUBW(R9:8,R11:10):sat       //[[3]C" = A' - C'                      ]
    }
    { R11:10 = MEMD(R0++M0)                 //[[2]load C                            ]
      IF P3 MEMD(R3++#8) = R11:10           //[[3]save C"                           ]
      R19:18 = VSUBW(R15:14,R13:12):sat     //[[3]B" = B'- jD'                      ]
      R13:12 = VADDW(R15:14,R13:12):sat     //[[3]D" = B'+ jD'                      ]
    }
    { R13:12 = MEMD(R0++M1)                 //[[2]load D                            ]
      R9:8   = VADDW(R7:6,R9:8):sat         //[[2]A' = A + B                        ]
      R15:14 = VSUBW(R7:6,R9:8):sat         //[[2]B' = A - B                        ]
      IF P3 MEMD(R4++#8) = R13:12           //[[3]save D"                           ]
    }
    { R11:10 = VADDW(R11:10,R13:12):sat     //[[2]C' = C + D                        ]
      R13:12 = VSUBW(R11:10,R13:12):sat     //[[2]D' = C - D                        ]
      IF P3 MEMD(R2++#8) = R19:18           //[[3]save B"                           ]
    }
    { R7:6 = MEMD(R0++M0)                   //[[1]load A                            ]
      R12 = NEG(R13):sat                    //[[2] j*D'                             ]
      R13 = R12                             //[[2] j*D'                             ]
      IF P3 MEMD(R1++#8) = R17:16           //[[3]save A"                           ]
    }:endloop0

    { R17:16 = VADDW(R9:8,R11:10):sat       //[[e]A" = A' + C'                      ]
      R11:10 = VSUBW(R9:8,R11:10):sat       //[[e]C" = A' - C'                      ]
      R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { MEMD(R1) = R17:16                     //[[e]save A"                           ]
      R19:18 = VSUBW(R15:14,R13:12):sat     //[[e]B" = B'- jD'                      ]
      R13:12 = VADDW(R15:14,R13:12):sat     //[[e]D" = B'+ jD'                      ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { MEMD(R2) = R19:18                     //[[e]save B"                           ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { MEMD(R3) = R11:10                     //[[e]save C"                           ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMD(R4) = R13:12                     //[[e]save D"                           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }

    /*-----------------------------------------------------------------------------*/
    /*                       Last Radix-2 stage                                    */
    /*-----------------------------------------------------------------------------*/
    .falign
.fft32x16_RADIX2_LastStage:
    { R0 += ASL(R28,#2)                     //[ R0 = &output[N/2] , R1=&output[0]   ]
      R4 = ASR(R28,#1)                      //[ N/2                                 ]
    }
    { R2 = R0                               //[ R2 points to output[N/2]            ]
      P3 = SP1LOOP0(fft32x16_RADIX2_LOOP,R4)//[ setup loop0: lc0 = N/2              ]
      R7:6 = MEMD(R1++#8)                   //[p]load A                             ]
    }

    .falign
fft32x16_RADIX2_LOOP:
    { R9:8 = MEMD(R0++#8)                   //[[2]load B                            ]
      IF (P3) MEMD(R2++#8) = R13:12         //[[3]save B'                           ]
    }
    { R7:6 = MEMD(R1++#8)                   //[[1]load A                            ]
      R11:10 = VADDW(R7:6,R9:8):sat         //[[2]A'= A+B                           ]
      R13:12 = VSUBW(R7:6,R9:8):sat         //[[2]B'= A-B                           ]
      IF (P3) MEMD(R3++#8) = R11:10         //[[3]save A'                           ]
    }:endloop0

    { MEMD(R2) = R13:12                     //[[e]save results                      ]
    }
    { MEMD(R3) = R11:10                     //[[e]save results                      ]
    }
.fft32x16_DONE:
    { R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	fft32x16, .-fft32x16

