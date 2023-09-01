/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008
****************************************************************************/
    
    .file   "conv3x3.S"


#define   ENABLE_PREFETCH

#ifdef ENABLE_PREFETCH
#define   PREFETCH_AHEAD    32
#endif

    /*[*****************************************************************************]*/
    /*[  Function   : void conv3x3()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: calculate convolution of image with 3x3 mask                  ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - img and output are aligned by 2-bytes                           ]*/
    /*[           - width is a multiple of 2                                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *img                                              ]*/
    /*[               R1 : Word8  *mask                                             ]*/
    /*[               R2 : int    width                                             ]*/
    /*[               R3 : int    shift                                             ]*/
    /*[               R4 : UWord8 *output                                           ]*/
    /*[               R5 : int    nrows   - number of rows to be computed           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R31                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8+56                              ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 4*(width*nrows/2) + 17  (ideal case)                            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl conv3x3
    .type       conv3x3, @function
conv3x3:
    { ALLOCFRAME(#7*8)                      //[ reserve stack                       ]
      R5 = MPYI(R5,R2)                      //[ n = nrows*width                     ]
      R28 = R4                              //[ R28 = output                        ]
      R8 = R1                               //[ R8 = mask                           ]
    }
    { MEMD(R29+#0) = R27:26                 //[ callee-saved registers              ]
      R4 = MEMB(R8+#0)                      //[ mask00                              ]
      R7 = ADD(R5,#-32)                     //[ n-32                                ]
      R31 = ASR(R5,#1)                      //[ LC = n/2                            ]
    }
    { MEMD(R29+#8) = R25:24                 //[ callee-saved registers              ]
      R5 = MEMB(R8+#1)                      //[ mask01                              ]
      R9 = ADD(R31,#15)                     //[                                     ]
      R1 = ADD(R0,R2)                       //[ ptr to row 1: input + width         ]
    }
    { MEMD(R29+#16) = R23:22                //[ callee-saved registers              ]
      R6 = MEMB(R8+#2)                      //[ mask02                              ]
      R9 = ASR(R9,#4)                       //[ ceil(LC/16)                         ]
      R10 = COMBINE(R5.L,R4.L)              //[ R10 = mask01|mask00                 ]
    }
    { MEMD(R29+#24) = R21:20                //[ callee-saved registers              ]
      R11 = ZXTH(R6)                        //[ R11:10 = 0|mask02|mask01|mask00     ]
      R4 = MEMB(R8+#3)                      //[ mask10                              ]
      R2 = ADD(R1,R2)                       //[ ptr to rwo 2: input + 2*width       ]
    }
    { MEMD(R29+#32) = R19:18                //[ callee-saved registers              ]
      R5 = MEMB(R8+#4)                      //[ mask11                              ]
      R18 = ASLH(R4)                        //[ mask10|0                            ]
#ifdef ENABLE_PREFETCH
      LOOP1(.conv3x3_PREFETCH_LOOP,R9)      //[ setup loop1: lc1=ceil((n/2)/16)     ]
#else
      P3=SP2LOOP0(.conv3x3_LOOP,R31)        //[ setup loop0: lc0 = n/2              ]
#endif
    }
    { MEMD(R29+#40) = R17:16                //[ callee-saved registers              ]
      R6 = MEMB(R8+#5)                      //[ mask12                              ]
      R12 = COMBINE(R5.L,R4.L)              //[ mask11|mask10                       ]
      R17:16 = ASL(R11:10,#16)              //[ R17:16 = mask02|mask01|mask00|0     ]
    }
    { R13 = ZXTH(R6)                        //[ R13:12 = 0|mask12|mask11|mask10     ]
      R19 = COMBINE(R6.L,R5.L)              //[ R19:18 = mask12|mask11|mask10|0     ]
      R4 = MEMB(R8+#6)                      //[ mask20                              ]
      R5 = MEMB(R8+#7)                      //[ mask21                              ]
    }
    { R6 = MEMB(R8+#8)                      //[ mask22                              ]
      R14 = COMBINE(R5.L,R4.L)              //[ mask21|mask20                       ]
      R23 = MEMUBH(R0++#2)                  //[ load 2 samples from row0            ]
      R20 = ASLH(R4)                        //[ mask20|0                            ]
    }
    { R25 = MEMUBH(R1++#2)                  //[ load 2 samples from row1            ]
      R27 = MEMUBH(R2++#2)                  //[ load 2 samples from row2            ]
      R15 = ZXTH(R6)                        //[ R15:14 = 0|mask22|mask21|mask20     ]
      R21 = COMBINE(R6.L,R5.L)              //[ R23:22 = mask22|mask21|mask20|0     ]
    }
    { MEMW(R29+#48) = R30                   //[ callee-saved registers              ]
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
      R30 = ADD(R28,R7)                     //[ &output[n-32]                       ]
      P2 = CMP.GT(R0,R0)                    //[ clean P2                            ]
    }

#ifdef ENABLE_PREFETCH
    .falign
.conv3x3_PREFETCH_LOOP:
    { DCFETCH(R0+#PREFETCH_AHEAD)           //[ prefetch from row0                  ]
      R7 = #16                              //[                                     ]
      R9 = ADD(R28,#31)                     //[ write PTR + 31                      ]
      NOP
    }
    { DCFETCH(R1+#PREFETCH_AHEAD)           //[ prefetch from row1                  ]
      R7 = MIN(R7,R31)                      //[                                     ]
      R9 = AND(R9,#-32)                     //[ align to $line                      ]
      NOP
    }
    { DCFETCH(R2+#PREFETCH_AHEAD)           //[ prefetch from row2                  ]
      R31 = ADD(R31,#-16)                   //[ LC -= 16                            ]
      P0 = CMP.GTU(R9,R30)                  //[ will dczero be out of boundary ?    ]
      P3=SP2LOOP0(.conv3x3_LOOP,R7)         //[ setup loop0: lc0 = min(16,LC)       ]
    }
    { IF P0 JUMP .conv3x3_LOOP              //[ if yes then skip dczero             ]
      P3 = OR(P3,P2)                        //[                                     ]
      P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
    }
    { DCZEROA(R9)                           //[ dc zero output array                ]
    }
#endif

    .falign
.conv3x3_LOOP:
    { R22 = R23                             //[[1]save cur two samples in R26       ]
      R23 = MEMUBH(R0++#2)                  //[[1]load next two samples from row0   ]
      R9:8 += VRMPYH(R19:18,R25:24)         //[[2]sum1+=mask10|11|12 dot* in10|11|12]
      R4 = VSATHUB(R5:4)                    //[[3]saturate to [0,255]               ]
    }
    { R24 = R25                             //[[1]                                  ]
      R7:6 += VRMPYH(R15:14,R27:26)         //[[2]sum0+=mask20|21|22 dot* in20|21|22]
      R9:8 += VRMPYH(R21:20,R27:26)         //[[2]sum1+=mask20|21|22 dot* in20|21|22]
      IF P3 MEMH(R28++#2) = R4              //[[3]save 2 output samples             ]
    }
    { R25 = MEMUBH(R1++#2)                  //[[1]load next two samples from row1   ]
      R7:6 = VRMPYH(R11:10,R23:22)          //[[1]sum0 =mask00|01|02 dot* in00|01|02]
      R9:8 = VRMPYH(R17:16,R23:22)          //[[1]sum1 =mask00|01|02 dot *in0a|01|02]
      R5:4 = COMBINE(R8,R6)                 //[[2]pack R5:4 = sum1 : sum0           ]
    }
    { R26 = R27                             //[[1]save cur two samples in R26       ]
      R27 = MEMUBH(R2++#2)                  //[[1]load next two samples from row1   ]
      R7:6 += VRMPYH(R13:12,R25:24)         //[[1]sum0+=mask10|11|12 dot* in10|11|12]
      R4 = VASRW(R5:4,R3)                   //[[2]shift and pack to 16bit sum1|sum0 ]
#ifdef ENABLE_PREFETCH
    }:endloop0:endloop1
#else
    }:endloop0
#endif

    { R9:8 += VRMPYH(R19:18,R25:24)         //[[e]sum1+=mask10|11|12 dot* in10|11|12]
      R4 = VSATHUB(R5:4)                    //[[e]saturate to [0,255]               ]
      R30 = MEMW(R29+#48)                   //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
    }
    { MEMH(R28++#2) = R4                    //[[e]save 2 output samples
      R7:6 += VRMPYH(R15:14,R27:26)         //[[e]sum0+=mask20|21|22 dot* in20|21|22]
      R9:8 += VRMPYH(R21:20,R27:26)         //[[e]sum1+=mask20|21|22 dot* in20|21|22]
      R19:18 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { R5:4 = COMBINE(R8,R6)                 //[[e]pack R5:4 = sum1 : sum0           ]
      R21:20 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R4 = VASRW(R5:4,R3)                   //[[e]shift and pack to 16bit sum1|sum0 ]
      R25:24 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R27:26 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { R4 = VSATHUB(R5:4)                    //[[e]saturate to [0,255]               ]
      DEALLOCFRAME                          //[ pop stack                           ]
    }
    { MEMH(R28) = R4                        //[[e]save 2 output samples             ]
      JUMPR R31                             //[ return                              ]
    }
    .size       conv3x3, .-conv3x3


