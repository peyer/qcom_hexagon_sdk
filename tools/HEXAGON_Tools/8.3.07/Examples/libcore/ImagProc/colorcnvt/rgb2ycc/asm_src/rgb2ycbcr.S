/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008
****************************************************************************/
    .file   "rgb2ycbcr.S"

#define  ENABLE_PREFETCH

#ifdef ENABLE_PREFETCH
#define PREFETCH_AHEAD    64
#endif


#define R2Y     9798
#define G2Y     19235
#define B2Y     3736
#define R2CB    0xEA67   //-5529
#define G2CB    0xD599   //-10855
#define B2CB    16384
#define R2CR    16384
#define G2CR    0xCA68   //-13720
#define B2CR    0xF598   //-2664


    /*[*****************************************************************************]*/
    /*[  Function   : void RGB2YCbCr()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Convert RGB color values to YCbCr color space                 ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - pRGB and pYCrCb are 2-bytes aligned                             ]*/
    /*[           - either width or nrows is a multiple of 2                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *pRGB                                             ]*/
    /*[               R1 : int    width                                             ]*/
    /*[               R2 : int    nrows                                             ]*/
    /*[               R3 : UWord8 *pYCbCr                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6*(width*nrows/2)  + 13                                         ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl RGB2YCbCr
    .type       RGB2YCbCr, @function
RGB2YCbCr:
    { P0 = CMP.GT(R2,#0)                    //[ if !(nrow>0)                        ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      R2 = MPYI(R1,R2)                      //[ n = width*nrows                     ]
      IF  P0.new R1 = R3                    //[ R1 = pYCrCb                         ]
    }
    { R29 = ADD(R29, #-5*8)                 //[ reserve stack                       ]
      MEMD(R29+#(0-5*8)) = R25:24           //[ callee-saved registers              ]
      R8.L = #R2CB                          //[ to set R8 = G2CB|R2CB               ]
      R9.L = #G2Y                           //[ to set R9 = B2Y |G2Y                ]
    }
    { MEMD(R29+#8) = R23:22                 //[ callee-saved registers              ]
      R8.H = #G2CB                          //[ set R9:8 = B2Y|G2Y|G2CB|R2CB        ]
      R9.H = #B2Y                           //[ set R9:8 = B2Y|G2Y|G2CB|R2CB        ]
      R28 = #0                              //[
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R10.L = #R2Y                          //[ to set R11:10 = B2CR|G2CR|G2Y|R2Y   ]
      R11.L = #G2CR                         //[ to set R11:10 = B2CR|G2CR|G2Y|R2Y   ]
      R28.H = #128                          //[ R28 = (128)<<16                     ]
    }
    { MEMD(R29+#24) = R19:18                //[ callee-saved registers              ]
      R10.H = #G2Y                          //[ R11:10 = B2CR|G2CR|G2Y|R2Y          ]
      R11.H = #B2CR                         //[ R11:10 = B2CR|G2CR|G2Y|R2Y          ]
      R24 = ASR(R2,#1)                      //[ LC = n/2                            ]
    }
    { MEMD(R29+#32) = R17:16                //[ callee-saved registers              ]
      R12.L = #R2CR                         //[ to set R12 = G2CR|R2CR              ]
      R13.L = #G2CB                         //[ to set R13 = B2CB|G2CB              ]
      R3 = ADD(R24,#15)                     //[
    }
    { R12.H = #G2CR                         //[ R13:12 = B2CB|G2CB|G2CR|R2CR        ]
      R13.H = #B2CB                         //[ R13:12 = B2CB|G2CB|G2CR|R2CR        ]
      R17 = #0                              //[ R17 = #0                            ]
      R25 = #-32                            //[                                     ]
    }
    { R3 = ASR(R3,#4)                       //[ ceil(LC/16)                         ]
      R5 = COMBINE(R10.L,R13.H)             //[ R5 = R2Y|B2CB                       ]
      R6 = COMBINE(R12.L,R9.H)              //[ R6 = R2CR|B2Y                       ]
      R25 += MPYI(R2,#3)                    //[ 3*n - 32                            ]
    }
#ifndef ENABLE_PREFETCH
    { R7 = COMBINE(R8.L,R11.H)              //[ R7 = R2CB|B2CR                      ]
      R21:20 = COMBINE(R28,R17)             //[[p]128 : 0                           ]
      R23:22 = COMBINE(R28,R28)             //[[p]128 : 128                         ]
      P3 = SP1LOOP0(.RGB2YCBCR_LOOP,R24)    //[ setup loop0:lc0=width*nrows/2       ]
    }
#else
    { R7 = COMBINE(R8.L,R11.H)              //[ R7 = R2CB|B2CR                      ]
      R25 = ADD(R2,R25)                     //[ pYCrCb + 3*n - 32 (dczero boundary) ]
      LOOP1(.RGB2YCBCR_PREFETCH_LOOP,R3)    //[ setup loop1:lc1= ceil((n/2)/16)     ]
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
    }
    { R21:20 = COMBINE(R28,R17)             //[[p]128 : 0                           ]
      R23:22 = COMBINE(R28,R28)             //[[p]128 : 128                         ]
      NOP; NOP
    }

    .falign
.RGB2YCBCR_PREFETCH_LOOP:
    { DCFETCH(R0+#PREFETCH_AHEAD)           //[ prefetch $line                      ]
      R16 = #16                             //[                                     ]
      R14 = ADD(R1,#31)                     //[                                     ]
    }
    { DCFETCH(R0+#(PREFETCH_AHEAD+32))      //[ prefetch $line                      ]
      R16 = MIN(R24,R16)                    //[ min(LC,16)                          ]
      R24 = ADD(R24,#-16)                   //[ LC -= 16                            ]
      R14 = AND(R14,#-32)                   //[ aligned to $line                    ]
    }
    { DCFETCH(R0+#(PREFETCH_AHEAD+64))      //[ prefetch $line                      ]
      LOOP0(.RGB2YCBCR_LOOP,R16)            //[setup loop
      P0 = CMP.GTU(R14,R25)                 //[ will dczero be out of boundary ?    ]
      R15 = ADD(R14,#32)                    //[ next output $line                   ]
    }
    { IF P0 JUMP .RGB2YCBCR_LOOP            //[ skip dczero                         ]
      P0 = CMP.GTU(R15,R25)                 //[ will dczero be out of boundary ?    ]
      R16 = ADD(R15,#32)                    //[                                     ]
    }
    { DCZEROA(R14)                          //[ dc zero output array                ]
      IF P0 JUMP .RGB2YCBCR_LOOP            //[ skip dczero                         ]
      P0 = CMP.GTU(R16,R25)                 //[ will dczero be out of boundary ?    ]
    }
    { DCZEROA(R15)                          //[ dc zero output array                ]
      IF P0 JUMP .RGB2YCBCR_LOOP            //[ skip dczero                         ]
    }
    { DCZEROA(R16)                          //[ dc zero output array                ]
    }
#endif

    .falign
.RGB2YCBCR_LOOP:
    { R16 = MEMUBH(R0+#2)                   //[[2]load b0|r1                        ]
      R15 = MEMUBH(R0+#4)                   //[[2]load g1|b1                        ]
      R2 = VSATHUB(R3:2)                    //[[3]clamp; R2=Cb(1)Cr(1)|Cr(0)Y(0)    ]
      R4 = VRNDWH(R19:18)                   //[[3]round                             ]
    }
    { R14 = MEMUBH(R0++#6)                  //[[2]load g0|r0                        ]
      R19:18 = COMBINE(R17,R28)             //[[2]0|128                             ]
      R21:20 += VMPYH(R16,R6):<<1:sat       //[[2]Cr1+=r1*R2Cr: Y0+=b0*B2Y          ]
      R4 = VSATHUB(R5:4)                    //[[3]clamp; R4.L=Y(1)Cb(0)             ]
    }
    { R19:18 += VMPYH(R16,R5):<<1:sat       //[[2]Y(1) +=r1*R2Y : Cb(0) +=b0*B2Cb   ]
      R23:22 += VMPYH(R16,R7):<<1:sat       //[[2]Cb(1)+=r1*R2CB: Cr(0) +=b0*B2Cr   ]
      IF P3 MEMH(R1+#2) = R4                //[[3]save                              ]
    }
    { R21:20 += VDMPY(R15:14,R11:10):<<1:sat//[[2]Cr(1) +=g1*G2CR+ b1*B2CR:         ]
                                            //[    Y(0) +=r0*R2Y+ g0*G2Y            ]
      R23:22 += VDMPY(R15:14,R13:12):<<1:sat//[[2]Cb(1) +=g1*G2Cb+ b1*B2Cb:         ]
                                            //[   Cr(0) +=r0*R2Cr+ g0*G2Cr          ]
      IF P3 MEMH(R1+#4) = R2.H              //[[3]save                              ]
    }
    { R21:20 = COMBINE(R28,R17)             //[[1]128|0                             ]
      R2 = VRNDWH(R21:20)                   //[[2]round                             ]
      R3 = VRNDWH(R23:22)                   //[[2]round                             ]
      IF P3 MEMH(R1++#6) = R2               //[[3]save                              ]
    }
    { R23:22 = COMBINE(R28,R28)             //[[1]128 : 128                         ]
      R19:18 += VDMPY(R15:14,R9:8):<<1:sat  //[[2]Y1  +=g1*G2Y + b1*B2Y :           ]
                                            //[   Cb0 +=r0*R2Cb + g0*G2Cb           ]
      R3:2 = PACKHL(R3,R2)                  //[[2]Cb(1)|Cr(1)|Cr(0)|Y(0)            ]
#ifdef ENABLE_PREFETCH
      P3 = CMP.EQ(R0,R0)                    //[ setup P3                            ]
    }:endloop0:endloop1
#else
    }:endloop0
#endif

    { R2 = VSATHUB(R3:2)                    //[[e]clamp; R2=Cb(1)Cr(1)|Cr(0)Y(0)    ]
      R4 = VRNDWH(R19:18)                   //[[e]round                             ]
      R25:24 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMH(R1+#4) = R2.H                    //[[e]save                              ]
      R4 = VSATHUB(R5:4)                    //[[e]clamp; R4.L=Y(1)Cb(0)             ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { MEMH(R1++#0) = R2                     //[[e]save                              ]
      R19:18 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { MEMH(R1+#2) = R4                      //[[e]save                              ]
      R17:16 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size       RGB2YCbCr, .-RGB2YCbCr


