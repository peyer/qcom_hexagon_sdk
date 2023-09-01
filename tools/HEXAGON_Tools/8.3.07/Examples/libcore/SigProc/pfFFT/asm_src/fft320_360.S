/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file    "fft320_360.S"

    /*-----------------------------------------------------------------------------*/
    /*                 Define Constants                                            */ 
    /*-----------------------------------------------------------------------------*/
#define   RCTRLM0320         0x09800500            // increment = 64
#define   RCTRLM1320         0x098A0500            // increment = 64 + 5
#define   RCTRLM0360         0x099005A0            // increment = 72
#define   RCTRLM1360         0x09EA05A0            // increment = 72 + 45 
#define   WCTRLM0320         0x09820500            // increment = 65
#define   WCTRLM1320         0x09020500            // increment = (65+256)%320

#define  WCTRLM0360          0x09F005A0            // increment = (3*280)%360
#define  WCTRLM1360          0x095005A0            // increment = (4*280)%360

#define   LEN360             (360*4)
#define   ZINC               (40*4)

// For 5-point FFT
#define   W15X2d5            0xCF4E0FD2            // 2/5*W1
#define   W25X2d5            0xE1E8D694            // 2/5*W2
#define   NW15X2d5           0x30B2F02E
#define   CONST1d5           0x199A
#define   CONST4d5           0x6666

// For 9-point FFT
#define  W39X1d2             0xC893E000
#define  W39X1d2c            0x376DE000
#define  W39                 0x9126C000
#define  W39c                0x6EDAC000
#define  W19X4d9             0xDB6F2B94
#define  W29X4d9             0xC7FA09E1
#define  W49X4d9             0xEC8BCA8B
#define  CONST4d9            0x000038E4

    /*[*****************************************************************************]*/
    /*[  Function   : void fft320()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 320-point FFT                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and output are aligned by 2048-bytes                      ]*/
    /*[             (due to circular addressing)                                    ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : CWord2x16 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5, R24-R27                                             ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 16+8                              ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 1756                                                            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl fft320
    .type    fft320, @function
fft320:
    { ALLOCFRAME(#16)                       //[ allocate stack                      ]
      R3 = CONST32(#FFTtwiddles)            //[ ptr to twiddle factor               ]
      R1 = #0                               //[ func call: R1 = 0 (mode=0)          ]
      R2 = R1                               //[ func call: R2 = out                 ]
                                            //[ func call: R0 = in                  ]
    }
    { MEMD(R29+#0) = R27:26                 //[ callee-saved registers              ]
      R26 = R3                              //[ R26 = FFTtwiddles                   ]
      R27 = CONST32(#FFTTempBuf360)         //[ ptr to temporay buffer              ]
      NOP
    }
    /* --------------------------------------------------------------------------- */
    /*                re-order input and perform 5-point FFT                       */
    /* --------------------------------------------------------------------------- */
    { MEMD(R29+#8) = R25:24                 //[ callee-saved registers              ]
      R24 = ADD(R27,#(3*64*4))              //[ FFTTempBuf + 3*64*sizeof(CWord2x16) ]
      R25 = R2                              //[ save output ptr in R25              ]
      CALL blkfft5                          //[ blkfft5(in, 0, out)                 ]
    }
    /* --------------------------------------------------------------------------- */
    /*                perform 64-point FFT on 5 rows                               */
    /* --------------------------------------------------------------------------- */

    .falign
.fft320_fft64LOOP:
    { R0 = R25                              //[ func call: R0 = out                 ]
      R1 = #64                              //[ func call: R1 = 64                  ]
      R3:2 = R27:26                         //[ func call: R2=wt64, R3=FFTtemBuf    ]
      CALL sfft16x16                        //[ sfft16x16(out, 64, wt64, FFTTempBuf)]
    }
    { P0 = CMP.GT(R27,R24)                  //[ i*64*4 > 3*64*4                     ]
      R25 = ADD(R25,#256)                   //[ update to next 64 points            ]
      R27 = ADD(R27,#256)                   //[ update to next 64 points            ]
      IF (!P0.new) JUMP:t .fft320_fft64LOOP //[ if !P0 contine loop                 ]
    }
    /* --------------------------------------------------------------------------- */
    /*                Re-order outputs                                             */
    /* --------------------------------------------------------------------------- */
    { R4 = CONST32(#WCTRLM0320)             //[ m0 value                            ]
      R5 = CONST32(#WCTRLM1320)             //[ m1 value                            ]
      R0 = ADD(R27,#-(5*64*4))              //[ set R0 = FFTTempBuf                 ]
      LOOP1(.fft320_ReorderLOOP,#5)         //[ setup loop                          ]
    }
    { M0 = R4                               //[ set m0                              ]
      R1 = ADD(R25,#-(5*64*4))              //[ set R1 = output                     ]
      R27:26 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { M1 = R5                               //[ set m0                              ]
      DEALLOCFRAME                          //[ pop stack                           ]
    }
    .falign
.fft320_ReorderLOOP:
    { R2 = MEMW(R0++#4)                     //[ [p]load xb2[j][i]                   ]
      LOOP0(.fft320_Reorder_inLOOP,#63)     //[ setup i-loop: lc0 = 64-1            ]
    }

    .falign
.fft320_Reorder_inLOOP:
    { R2 = MEMW(R0++#4)                     //[[1]load xb2[j][i]                    ]
      MEMW(R1++I:circ(M0)) = R2             //[[2]output[(65*i+256*j)%320]=xb2[j][i]]
    }:endloop0 

    { MEMW(R1++I:circ(M1)) = R2             //[save last output and update write ptr]
    }:endloop1

    { JUMPR     R31                         //[ return                              ]
    }
    .size    fft320, .-fft320



    /*[*****************************************************************************]*/
    /*[  Function   : void fft360()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 360-point FFT                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input  is aligned by 2048-bytes                                 ]*/
    /*[           - output is aligned by 256-bytes                                  ]*/
    /*[             (due to circular addressing)                                    ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : CWord2x16 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R2, R24-R25                                             ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8+8                               ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 2077                                                            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl fft360
    .type    fft360, @function
fft360:
    { ALLOCFRAME(#8)                        //[ allocate stack                      ]
      R1 = #1                               //[ func call: R1 = 1 (mode= 1)         ]
      R2 = R1                               //[ func call: R2 = output              ]
      NOP
    }
    /* --------------------------------------------------------------------------- */
    /*                   re-order input and perform 5-point FFT                    */
    /* --------------------------------------------------------------------------- */
    { MEMD(R29+#0) = R25:24                 //[ callee-saved registers              ]
      R24 = CONST32(#FFTTempBuf360)         //[ R24 = Ptr of temporary buffer       ]
      R25 = R2                              //[ save "out" in R25                   ]
      CALL blkfft5                          //[ blkfft(in, 1, output)               ]
    }
    /* --------------------------------------------------------------------------- */
    /*                    perform 8-point FFTs                                     */ 
    /* --------------------------------------------------------------------------- */
    .falign
    { R0 = R25                              //[ func call: R0 = output              ]
      R1 = #45                              //[ func call: R1 = 45 (360=45x8)       ]
      R2 = R24                              //[ func call: R2 = temporary buffer    ]
      CALL blkfft8                          //[ blkfft8(output, 45, FFTTempBuf360)  ]
    }
    .falign
    { R1:0 = R25:24                         //[ func call: R0 = FFTTempBuf360       ]
                                            //[ func call: R1 = output              ]
      R25:24 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      DEALLOCFRAME                          //[ pop stack                           ]
      JUMP blkfft9                          //[ blkfft9(FFTTempBuf360, output)      ]
    }
    .size    fft360, .-fft360




    /*[*****************************************************************************]*/
    /*[  Function   : void blkfft5()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform input reodering and 5-point FFT of multi-blocks for   ]*/
    /*[               320-point or 360-point FFT                                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input  is aligned by 2048-bytes due to circular addressing      ]*/
    /*[           - output is aligned by 4-bytes                                    ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : int mode  (mode=0, 320pt,else 360pt)                     ]*/
    /*[               R2 : CWord2x16 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28,R30,R31                                        ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 56                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - Y*(7*X+2) + 20                                                  ]*/
    /*[             o 320-point : Y=1, X=64                                         ]*/
    /*[             o 360-point : Y=9, X=8                                          ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl blkfft5
    .type    blkfft5, @function
blkfft5:
    { R29 = ADD(R29,#-7*8)                  //[ allocate stack                      ]
      R12.H = #HI(W25X2d5)                  //[ R12 =  W2i | W1i                    ]
      R13.H = #HI(NW15X2d5)                 //[ R13 = -W1i | W2i                    ]
      P0 = CMP.EQ(R1,#0)                    //[ Is FFT320?                          ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R12.L = #HI(W15X2d5)                  //[ R12 =  W2i | W1i                    ]
      R13.L = #HI(W25X2d5)                  //[ R13 = -W1i | W2i                    ]
      R16.H = #HI(RCTRLM0320)               //[ to set M0 for 320-point             ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R16.L = #LO(RCTRLM0320)               //[ to set M0 for 320-point             ]
      R17.L = #LO(RCTRLM1320)               //[ to set M1 for 320-point             ]
      R18.L = #LO(RCTRLM0360)               //[ to set M0 for 360-point             ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R17.H = #HI(RCTRLM1320)               //[ to set M1 for 320-point             ]
      R18.H = #HI(RCTRLM0360)               //[ to set M0 for 360-point             ]
      R19.H = #HI(RCTRLM1360)               //[ to set M1 for 360-point             ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R19.L = #LO(RCTRLM1360)               //[ to set M1 for 360-point             ]
      R11:10 = #0                           //[ constant 0                          ]
      R6.H = #LO(W25X2d5)                   //[ R6 = W2r | W1r                      ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R14 = #CONST1d5                       //[ R14 = C15|C15                       ]
      R17:16 = VMUX(P0,R17:16,R19:18)       //[ control word for circular addressing]
      R20 = MUX(P0,#1,#9)                   //[ outer loop count= 320pt: 1, 360pt: 9]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      M0 = R16                              //[ set M0                              ]
      R6.L = #LO(W15X2d5)                   //[ R6 = real(W2*2/5)|real(W1*2/5)      ]
      R11:10 = VSUBH(R11:10,R13:12)         //[ R10 = -W2i | -W1i                   ]
                                            //[ R11 =  W1i | -W2i                   ]
    }
    { MEMD(R29+#48) = R31:30                //[ callee-saved registers              ]
      R11 = R12                             //[ R11 =  W2i | W1i                    ]
      R12 = R11                             //[ R12 =  W1i |-W2i                    ]
      M1 = R17                              //[ set M1                              ]
    }
    { R31 = ADD(R0,#LEN360)                 //[ ptr to end of input array (FFT320)  ]
      R30 = #LEN360                         //[ 360*sizeof(CWord2x16)               ]
      R8 = COMBINE(R6.L,R6.H)               //[ R6  = W1r | W2r                     ] 
      R15 = #CONST4d5                       //[ R15 = C45|C45                       ]
    }
    { R24 = MEMW(R0++I:circ(M0))            //[ update R0 to point at x1            ]
      R14 = COMBINE(R14.L,R14.L)            //[ R14 = C15|C15                       ]
      R15 = COMBINE(R15.L,R15.L)            //[ R15 = C45|C45                       ]
      R18 = MUX(P0,#64,#72)                 //[ 320pt: 64, 360pt: 72                ]
    }
    { R7 = R6                               //[ R7=R6= W2r | W1r                    ]
      R9 = R8                               //[ R9=R8= W1r | W2r                    ] 
      R18 = ASL(R18,#2)                     //[                                     ] 
      LOOP1(.blkfft5_outLOOP,R20)           //[ setup outer loop:lc1 = 1 or 9       ]
    }
    { R5 = R2                               //[ points to line 0 (out)              ]
      R1 = ADD(R2,R18)                      //[ points to line 1 (out)              ]
      R2 = ADDASL(R2,R18,#1)                //[ points to line 2 (out)              ]
      R27 = MUX(P0,#64,#8)                  //[ LC0: 64: (320pt) or 8 (360pt)       ]
    }
    { R3 = ADD(R2,R18)                      //[ points to line 3 (out)              ]
      R4 = ADDASL(R2,R18,#1)                //[ points to line 4 (out)              ]
      P3 = SP1LOOP0(.blkfft5_innerLOOP,R27) //[ [p1]setup inner loop                ]
    }

    .falign
.blkfft5_outLOOP:
    { R16 = MEMW(R0++I:circ(M0))            //[[p1]load x1
      R28 = ADD(R0,#ZINC)                   //[[p1] points to x1 in next dimension
    }

    .falign
.blkfft5_innerLOOP:
    { R17 = MEMW(R0++I:circ(M0))            //[[2]load x2                           ]
      R22 = VDMPY(R19:18,R11:10):<<1:rnd:sat//[[3]G= Cr*W1i+Dr*W2i|-Ci*W1i-Di*W2i   ]
      R23 = VDMPY(R19:18,R13:12):<<1:rnd:sat//[[3]H= Cr*W2i-Dr*W1i|-Ci*W2i+Di*W1i   ]
      R26 = VADDH(R26,R24):sat              //[[3]Y0
    }
    { R19 = MEMW(R0++I:circ(M0))            //[[2]load x3                           ]
      R21:20 = VADDH(R21:20,R23:22):sat     //[[3] F+H : E+G                        ]
      R23:22 = VSUBH(R21:20,R23:22):sat     //[[3] F-H : E-G                        ]
      R25 = R24                             //[[3] R25=R24=x0*1/5                   ]
    }
    { R18 = MEMW(R0++I:circ(M0))            //[[2]load x4                           ]
      R21:20 = VADDH(R25:24,R21:20):sat     //[[3]Y2 : Y1                           ]
      R23:22 = VADDH(R25:24,R23:22):sat     //[[3]Y3 : Y4                           ]
      IF P3 MEMW(R5++#4) = R26              //[[3]save Y0                           ]
    }
    { R17:16=  VAVGH(R17:16,R19:18):crnd    //[[2]B:A= (x2+x3)/2 : (x1+x4)/2        ]
      R19:18= VNAVGH(R17:16,R19:18):crnd:sat//[[2]D:C= (x2-x3)/2 : (x1-x4)/2        ]
      R24 = MEMW(R0++I:circ(M1))            //[[2]load x0                           ]
      IF P3 MEMW(R1++#4) = R20              //[[3]save Y1                           ]
    }
    { R17:16 = PACKHL(R17,R16)              //[[2]Bi|Ai|Br|Ar                       ]
      R24 = VMPYH(R24,R14):<<1:rnd:sat      //[[2]x0*1/5                            ]
      R26 = VAVGH(R16,R17)                  //[[2](x2+x3+x4+x5)/4                   ]
      IF P3 MEMW(R2++#4) = R21              //[[3]save Y2                           ]
    }
    { R18 = COMBINE(R19.H,R18.H)            //[[2]Di|Ci                             ]
      R19 = COMBINE(R19.L,R18.L)            //[[2]Dr|Cr                             ]
      R26 = VMPYH(R26,R15):<<1:rnd:sat      //[[2](x2+x3+x4+x5)/5                   ]
      IF P3 MEMW(R3++#4) = R23              //[[3]save Y3
    }
    { R16 = MEMW(R0++I:circ(M0))            //[[1]load x1                           ]
      R20 = VDMPY(R17:16,R7:6):<<1:rnd:sat  //[[2]E= Ai*W1r+Bi*W2r|Ar*W1r+Br*W2r    ]
      R21 = VDMPY(R17:16,R9:8):<<1:rnd:sat  //[[2]F= Ai*W2r+Bi*W1r|Ar*W2r+Br*W1r    ]
      IF P3 MEMW(R4++#4) = R22              //[[3]save Y4
    }:endloop0

    { P0 = CMP.GTU(R31,R28)                 //[ update next read ptr                ]
      IF  P0.new R0 = R28                   //[ update next read ptr                ]
      IF !P0.new R0 = SUB(R28,R30)          //[ update next read ptr                ]
      LOOP0(.blkfft5_innerLOOP,R27)         //[ setup inner loop                    ]
    }:endloop1

    { R22 = VDMPY(R19:18,R11:10):<<1:rnd:sat//[[e]G= Cr*W1i+Dr*W2i|-Ci*W1i-Di*W2i   ]
      R23 = VDMPY(R19:18,R13:12):<<1:rnd:sat//[[e]H= Cr*W2i-Dr*W1i|-Ci*W2i+Di*W1i   ]
      R26 = VADDH(R26,R24):sat              //[[e]Y0
      R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
    }
    { MEMW(R5) = R26                        //[[e]save Y0                           ]
      R21:20 = VADDH(R21:20,R23:22):sat     //[[e] F+H : E+G                        ]
      R23:22 = VSUBH(R21:20,R23:22):sat     //[[e] F-H : E-G                        ]
      R25 = R24                             //[[e] R25=R24=x0*1/5                   ]
    }
    { R21:20 = VADDH(R25:24,R21:20):sat     //[[e]Y2 : Y1                           ]
      R23:22 = VADDH(R25:24,R23:22):sat     //[[e]Y3 : Y4                           ]
      R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { MEMW(R1) = R20                        //[[e]save Y1                           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }   
    { MEMW(R2) = R21                        //[[e]save Y2                           ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMW(R3) = R23                        //[[e]save Y3                           ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { MEMW(R4) = R22                        //[[e]save Y4                           ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#7*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    blkfft5, .-blkfft5



    /*[*****************************************************************************]*/
    /*[  Function   : void blkfft8()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 8-point FFT of multi-blocks                           ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and output are aligned by 8-bytes                         ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *in  -- Ptr to input array                     ]*/
    /*[               R1 : int n          -- number of 8-point blocks               ]*/
    /*[               R2 : CWord2x16 *out -- Ptr to output array                    ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 11*n + 13                                                       ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .p2align 2
    .p2align 4,,15
    .globl blkfft8
    .type    blkfft8, @function
blkfft8:
    { R29 = ADD(R29, #-5*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-5*8)) = R17:16           //[ callee-saved registers              ]
      R3.H = #HI(FFTtwiddles)               //[ Ptr to twiddle factors              ]
      R0 = ADD(R0,#6*4)                     //[ R0 = &x[6]                          ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R3.L = #LO(FFTtwiddles)               //[ Ptr to twiddle factors              ]
      R28 = R2                              //[ R28 = out                           ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R5:4 = MEMD(R3)                       //[ load Wa1 : Wb1                      ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R7:6 = MEMD(R3+#8)                    //[ load Wb2 : Wc1                      ]
      P3 = SP1LOOP0(.blkfft8_radix4_stageLOOP,R1)
                                            //[ setup loop: lc0 = n                 ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R9:8 = MEMD(R3+#16)                   //[ load Wc2 : Wa2                      ]
      R24 = #0                              //[ R24 = constant 0                    ]
    }

    .falign
.blkfft8_radix4_stageLOOP:
    { R11:10 = MEMD(R0+#(0-6)*4)            //[[1]load A= x1:x0                     ]
      R12 = R19                             //[[2]R12 = A"                          ]
      R19 = CMPY(R12,R5):<<1:rnd:sat        //[[2]B" = B"*Wa                        ]
      R13 = CMPY(R13,R8):<<1:rnd:sat        //[[2]B" = B"*Wa                        ]
    }
    { R13:12 = MEMD(R0+#(4-6)*4)            //[[1]load B= x5:x4                     ]
      R20 = CMPY(R14,R4):<<1:rnd:sat        //[[2]C" = C"*Wb                        ]
      R22 = CMPY(R15,R7):<<1:rnd:sat        //[[2]C" = C"*Wb                        ]
      IF (P3) MEMD(R2+#16) = R13:12         //[[2]save A"B"(2)= Y5:Y4               ]
    }
    { R15:14 = MEMD(R0+#(2-6)*4)            //[[1]load C= x3:x2                     ]
      R11:10=  VAVGH(R11:10,R13:12):crnd    //[[1]A'= (A+B)/2                       ]
      R13:12= VNAVGH(R11:10,R13:12):crnd:sat//[[1]B'= (A-B)/2                       ]
      IF (P3) MEMD(R2) = R19:18             //[[2]save A"B"(1)= Y1:Y0               ]
    }
    { R17:16 = MEMD(R0++#32)                //[[1]load D= x7:6                      ]
      R21 = CMPY(R16,R6):<<1:rnd:sat        //[[2]D" = C"*Wc                        ]
      R23 = CMPY(R17,R9):<<1:rnd:sat        //[[2]D" = D"*Wc                        ]
      R25 = VSUBH(R24,R12):sat              //[[1] R25.H = -B'.imag                 ]
    }
    { R12 = COMBINE(R12.L,R25.H)            //[[1] jB'                              ]
      R25 = VSUBH(R24,R13):sat              //[[1] R25.H = -B'.imag                 ]
      R15:14=  VAVGH(R15:14,R17:16):crnd    //[[1]C'= (C+D)/2                       ]
      R17:16= VNAVGH(R15:14,R17:16):crnd:sat//[[1]D'= (C-D)/2                       ]
    }
    { R13 = COMBINE(R13.L,R25.H)            //[[1] jB'                              ]
      R19:18=  VAVGH(R11:10,R15:14):crnd    //[[1]A"= (A'+C')/2                     ]
      R15:14= VNAVGH(R11:10,R15:14):crnd:sat//[[1]C"= (A'-C')/2                     ]
      IF (P3) MEMD(R2+#8) = R21:20          //[[2]save C"D"(1)= Y3:Y2               ]
    }
    { R13:12=  VAVGH(R13:12,R17:16):crnd    //[[1]B"= (B'+D')/2                     ]
      R17:16= VNAVGH(R13:12,R17:16):crnd:sat//[[1]D"= (B'-D')/2                     ]
      IF (P3) MEMD(R2+#24) = R23:22         //[[2]save C"D"(2)= Y7:6                ]
      IF (P3) R2 = ADD(R2,#32)              //[[2]update write pointer              ]
    }:endloop0

    { R12 = R19                             //[[e]                                  ]
      R19 = CMPY(R12,R5):<<1:rnd:sat        //[[e]B" = B"*Wa                        ]
      R13 = CMPY(R13,R8):<<1:rnd:sat        //[[e]B" = B"*Wa                        ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { MEMD(R2) = R19:18                     //[[e]save A"B"(1)= Y1:0                ]
      R22 = CMPY(R15,R7):<<1:rnd:sat        //[[e]C" = C"*Wb                        ]
      R23 = CMPY(R17,R9):<<1:rnd:sat        //[[e]D" = D"*Wc                        ]
    }
    { MEMD(R2+#24) = R23:22                 //[[e]save C"D"(2)= Y7:6                ]
      R20 = CMPY(R14,R4):<<1:rnd:sat        //[[e]C" = C"*Wb                        ]
      R21 = CMPY(R16,R6):<<1:rnd:sat        //[[e]D" = C"*Wc                        ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { MEMD(R2+#8) = R21:20                  //[[e]save C"D"(1)= Y3:2                ]
      P3= SP1LOOP0(.blkfft8_radix2_stageLOOP,R1)
                                            //[ set loop: lc0 = n                   ]
      R1:0 = COMBINE(R28,R28)               //[ set R0 and R1 to output             ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { MEMD(R2+#16) = R13:12                 //[[e]save A"B"(2)= Y5:4                ]
      R7:6 = MEMD(R0)                       //[[p]load A= X1:0                      ]
    }

    .falign
.blkfft8_radix2_stageLOOP:
    { R9:8 = MEMD(R0+#4*4)                  //[[2]load B = X5:4                     ]
      IF (P3) MEMD(R1+#2*4) = R11:10        //[[3]save A'= Y3:2                     ]
      IF (P3) R1 = ADD(R1,#6*4)             //[[3]update R1 to &Y[6]                ]
    }
    { R11:10 = VAVGH(R7:6,R9:8):crnd        //[[2]A'= (A+B)/2                       ]
      R13:12 = VNAVGH(R7:6,R9:8):crnd:sat   //[[2]B'= (A-B)/2                       ]
      R7:6 = MEMD(R0+#2*4)                  //[[2]load A= X3:2                      ]
      IF (P3) MEMD(R1++#8) = R13:12         //[[3]save B'= Y7:6                     ]
    }
    { MEMD(R1+#4*4) = R13:12                //[[2]save B'= Y5:4                     ]
      R9:8 = MEMD(R0+#6*4)                  //[[2]load X7:6                         ]
      R0 = ADD(R0,#32)                      //[[2]update to next 8-points           ]
    }
    { R7:6 = MEMD(R0)                       //[[1]load A= X1:0                      ]
      R11:10 = VAVGH(R7:6,R9:8):crnd        //[[2]A'= (A+B)/2                       ]
      R13:12 = VNAVGH(R7:6,R9:8):crnd:sat   //[[2]B'= (A-B)/2                       ]
      MEMD(R1+#0*8) = R11:10                //[[2]save A'=Y1:0                      ]
    }:endloop0

    { MEMD(R1+#2*4) = R11:10                //[[e]save A'= Y3:2                     ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMD(R1+#6*4) = R13:12                //[[e]save B'= Y7:6                     ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    blkfft8, .-blkfft8



    /*[*****************************************************************************]*/
    /*[  Function   : void blkfft9()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 9-point FFT of 5 blocks                               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input  is aligned by 4bytes                                     ]*/
    /*[           - output is aligned by 256-bytes (due to circular addr)           ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *in  -- Ptr to input array                     ]*/
    /*[               R1 : CWord2x16 *out -- Ptr to output array                    ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27, R28, R31                                           ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48+8                              ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - Z*(X*25 + 2) + 15, where Z=5, X=8                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .p2align 2
    .p2align 4,,15
    .globl blkfft9
    .type    blkfft9, @function
blkfft9:
    { ALLOCFRAME(#6*8)                      //[ allocate stack                      ]
      R6.H = #HI(WCTRLM0360)                //[ m0 value                            ]
      R7.H = #HI(WCTRLM1360)                //[ m1 value                            ]
      R28 = #0                              //[ cnt = 0                             ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R6.L = #LO(WCTRLM0360)                //[ m0 value                            ]
      R7.L = #LO(WCTRLM1360)                //[ m1 value                            ]
      R17 = #0                              //[ constant 0                          ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R18.H = #HI(W39)                      //[ R18 = W3                            ]
      R19.H = #HI(W39c)                     //[ R19 = W3*                           ]
      M0 = R6                               //[ set m0                              ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R18.L = #LO(W39)                      //[ R18 = W3                            ]
      R19.L = #LO(W39c)                     //[ R19 = conj(W3)                      ]
      M1 = R7                               //[ set m1                              ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R8 =  #CONST4d9                       //[ constant "4/9"                      ]
      R22.H = #HI(W39X1d2)                  //[ R22 = W3*1/2                        ]
      R23.H = #HI(W39X1d2c)                 //[ R23 = conj(W3*1/2)                  ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R22.L = #LO(W39X1d2)                  //[ R22 = W3*1/2                        ]
      R23.L = #LO(W39X1d2c)                 //[ R23 = conj(W3*1/2)                  ]
      R24 = COMBINE(R8.L,R8.L)              //[ R24= "4/9" | "4/9"                  ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R13:12 = COMBINE(R22,R23)             //[ R13:12 = conj(w3*1/2) : (W3*1/2)    ]
      R31 = ADD(R1,#LEN360)                 //[ points to end of output array       ]
      R15 = MEMW(R0+#0*8*4)                 //[ [p][S1_BF1]load in[0]               ]
    }
    { R25.H = #HI(W19X4d9)                  //[ R25 = W1*4/9                        ]
      R26.H = #HI(W29X4d9)                  //[ R26 = W2*4/9                        ]
      R27.H = #HI(W49X4d9)                  //[ R27 = W4*4/9                        ]
      R14 = MEMW(R0+#3*8*4)                 //[ [p][S1_BF1]load in[3]               ]
    }
    { R25.L = #LO(W19X4d9)                  //[ R25 = W1*4/9                        ]
      R26.L = #LO(W29X4d9)                  //[ R26 = W2*4/9                        ]
      R27.L = #LO(W49X4d9)                  //[ R27 = W4*4/9                        ]
      R16 = MEMW(R0+#6*8*4)                 //[ [p][S1_BF1]load in[6]               ]
    }
    { LOOP1(.blkfft9_outLOOP,#5)            //[ setup loop                          ]
    }

    .falign    
.blkfft9_outLOOP:
    { R1 = ADD(R1,#216*4)                   //[ update pointer to next 9x8 block    ]
      R6 = #LEN360                          //[ 360 * sizeof(CWord2x16)             ]
      R2 = R1                               //[ setup write pointer                 ]
    }
    { P0 = CMP.GTU(R31,R1)                  //[ update write pointer with           ]
      IF !P0.new R1 = SUB(R1,R6)            //[ circular addr.                      ]
      LOOP0(.blkfft9_innerLOOP,#8)          //[ setup outer loop                    ]
    }

    .falign    
.blkfft9_innerLOOP:
    /* --------------------------------------------------------------------------- */
    /*                       Stage 1                                               */
    /* --------------------------------------------------------------------------- */
    { R5:4  =  VAVGH(R15:14,R17:16):crnd    //[ [S1_BF1]R5:4= x0/2 : A              ]
      R21:20= VNAVGH(R15:14,R17:16):crnd:sat//[ [S1_BF1]R20=  B                     ]
      R15 = MEMW(R0+#1*8*4)                 //[ [S1_BF2]load in[1]                  ]
      R6 = #LEN360                          //[ 360*sizeof(CWord2x16)               ]
    }
    { R20 = VAVGH(R5,R17)                   //[ [S1_BF1]XS0 = x0/4                  ]
      R3 = VAVGH(R5,R4)                     //[ [S1_BF1]g[0]= Y0                    ]
      R4 = COMBINE(R20.H,R4.L)              //[ [S1_BF1]R4 = Bi|Ar                  ]
      R5 = COMBINE(R20.L,R4.H)              //[ [S1_BF1]R5 = Br|Ai                  ]
    }
    { R14 = MEMW(R0+#4*8*4)                 //[ [S1_BF2]load in[4]                  ]
      R4 = VDMPY(R5:4,R13:12):<<1:rnd:sat   //[ [S1_BF1]Y1                          ]
      R5 = VDMPY(R5:4,R23:22):<<1:rnd:sat   //[ [S1_BF1]Y2                          ]
      R21 = R20                             //[ [S1_BF1]R21:20= XS0:XS0             ]
    }
    { R5:4 = VADDH(R21:20,R5:4):sat         //[ [S1_BF1]g[6:3]= XS0+Y2 : XS0+Y1     ]
      R16 = MEMW(R0+#7*8*4)                 //[ [S1_BF2]load in[7]                  ]
      P0 = CMP.GTU(R31,R2)                  //[ update write pointer                ]
      IF !P0.new R2 = SUB(R2,R6)            //[ with circular addr.                 ]
    }
    { R9:8  =  VAVGH(R15:14,R17:16):crnd    //[ [S1_BF2]R9:8= x0/2 : A              ]
      R21:20= VNAVGH(R15:14,R17:16):crnd:sat//[ [S1_BF2]R20=  B                     ]
      R15 = MEMW(R0+#2*8*4)                 //[ [S1_BF3]load in[2]                  ]
      R14 = MEMW(R0+#5*8*4)                 //[ [S1_BF3]load in[5]                  ]
    }
    { R20 = VAVGH(R9,R17)                   //[ [S1_BF2]XS0 = x0/4                  ]
      R6 = VAVGH(R9,R8)                     //[ [S1_BF2]g[1]= Y0                    ]
      R8 = COMBINE(R20.H,R8.L)              //[ [S1_BF2]R8 = Bi|Ar                  ]
      R9 = COMBINE(R20.L,R8.H)              //[ [S1_BF2]R9 = Br|Ai                  ]
    }
    { R8 = VDMPY(R9:8,R13:12):<<1:rnd:sat   //[ [S1_BF2]Y1                          ]
      R9 = VDMPY(R9:8,R23:22):<<1:rnd:sat   //[ [S1_BF2]Y2                          ]
      R21 = R20                             //[ [S1_BF2]R21:20= XS0:XS0             ]
      R16 = MEMW(R0+#8*8*4)                 //[ [S1_BF3]load in[8]                  ]
    }
    { R8 = VADDH(R20,R8):sat                //[ [S1_BF2]g[7:4]= XS0+Y2 : XS0+Y1     ]
      R9 = VADDH(R21,R9):sat                //[ [S1_BF2]g[7:4]= XS0+Y2 : XS0+Y1     ]
      R11:10=  VAVGH(R15:14,R17:16):crnd    //[ [S1_BF3]R11:10= x0/2 : A            ]
      R21:20= VNAVGH(R15:14,R17:16):crnd:sat//[ [S1_BF3]R20=  B                     ]
    }
    { R20 = VAVGH(R11,R17)                  //[ [S1_BF3]XS0 = x0/4                  ]
      R7 = VAVGH(R11,R10)                   //[ [S1_BF3]g[2]= Y0                    ]
      R10 = COMBINE(R20.H,R10.L)            //[ [S1_BF3]R10 = Bi|Ar                 ]
      R11 = COMBINE(R20.L,R10.H)            //[ [S1_BF3]R11 = Br|Ai                 ]
    }
    { R10 = VDMPY(R11:10,R13:12):<<1:rnd:sat//[ [S1_BF3]Y1                          ]
      R11 = VDMPY(R11:10,R23:22):<<1:rnd:sat//[ [S1_BF3]Y2                          ]
      R21 = R20                             //[ [S1_BF3]R21:20= XS0:XS0             ]
      R13:12 = COMBINE(R18,R19)             //[ [S2]                                ]
    }
    { R10 = VADDH(R20,R10):sat              //[ [S1_BF3]g[8:5]= XS0+Y2 : XS0+Y1     ]
      R11 = VADDH(R21,R11):sat              //[ [S1_BF3]g[8:5]= XS0+Y2 : XS0+Y1     ]
      R6 = VMPYH(R6,R24):<<1:rnd:sat        //[ [S2_BF1]X1=g[1]*w[0]                ]
      R7 = VMPYH(R7,R24):<<1:rnd:sat        //[ [S2_BF1]X2=g[2]*w[1]                ]
    }
    /* --------------------------------------------------------------------------- */
    /*                  R3=g[0], R6=g[1], R7 = g[2]                                */
    /*                  R4=g[3], R8=g[4], R10 = g[5]                               */
    /*                  R5=g[6], R9=g[7], R11 = g[8]                               */
    /* --------------------------------------------------------------------------- */

    /* --------------------------------------------------------------------------- */
    /*                       Stage 2                                               */
    /* --------------------------------------------------------------------------- */
    { R14 = VADDH(R6,R7):sat                //[ [S2_BF1]A=X1+X2                     ]
      R7 = VSUBH(R6,R7):sat                 //[ [S2_BF1]B=X1-X2                     ]
      R8 = CMPY(R8,R25):<<1:rnd:sat         //[ [S2_BF2]X1=g[4]*w[0]                ]
      R10 = CMPY(R10,R26):<<1:rnd:sat       //[ [S2_BF2]X2=g[5]*w[1]                ]
    }
    { R20 = VMPYH(R3,R24):<<1:rnd:sat       //[ [S2_BF1]XS0=g[0]*C49                ]
      R6 = COMBINE(R7.H,R14.L)              //[ [S2_BF1]R6 = Bi|Ar                  ]
      R7 = COMBINE(R7.L,R14.H)              //[ [S2_BF1]R7 = Br|Ai                  ]
      R9 = CMPY(R9,R26):<<1:rnd:sat         //[ [S2_BF3]X1=g[7]*w29                 ]
    }
    { R6 = VDMPY(R7:6,R13:12):<<1:rnd:sat   //[ [S2_BF1]Y1                          ]
      R7 = VDMPY(R7:6,R19:18):<<1:rnd:sat   //[ [S2_BF1]Y2                          ]
      R21 = R20                             //[ [S2_BF1]R21:20= XS0:XS0             ]
      R0 = ADD(R0,#4)                       //[ update R0 to next in the row        ]
    }
    { R14 = VADDH(R20,R14):sat              //[ [S2_BF1]X[0]= XS0+Y0                ]
      R7:6 = VADDH(R21:20,R7:6):sat         //[ [S2_BF1]X[6:3]= XS0+Y2 : XS0+Y1     ]
      R11 = CMPY(R11,R27):<<1:rnd:sat       //[ [S2_BF3]X2=g[8]*w49                 ]
      R28 = ADD(R28,#1)                     //[  for read pointer(R0) update        ]
    }
    { MEMW(R2++I:circ(M0)) = R14            //[ [S2_BF1]save X[0]                   ]
      R9:8 = VADDH(R9:8,R11:10):sat         //[ [S2_BF2/3]A=X1+X2                   ]
      R11:10 = VSUBH(R9:8,R11:10):sat       //[ [S2_BF2/3]B=X1-X2                   ]
    }
    { MEMW(R2++I:circ(M0)) = R6             //[ [S2_BF1]save X[3]                   ]
      R20 = VMPYH(R4,R24):<<1:rnd:sat       //[ [S2_BF2]XS0=g[3]*C49                ]
      R14 = COMBINE(R10.H,R8.L)             //[ [S2_BF2]R14 = Bi|Ar                 ]
      R15 = COMBINE(R10.L,R8.H)             //[ [S2_BF2]R15 = Br|Ai                 ]
    }
    { MEMW(R2++I:circ(M1)) = R7             //[ [S2_BF1]save X[6]                   ]
      R14 = VDMPY(R15:14,R13:12):<<1:rnd:sat//[ [S2_BF2]Y1                          ]
      R15 = VDMPY(R15:14,R19:18):<<1:rnd:sat//[ [S2_BF2]Y2                          ]
      R21 = R20                             //[ [S2_BF2]R21:20= XS0:XS0             ]
    }
    { R8 = VADDH(R20,R8):sat                //[ [S2_BF2]X[1]= XS0+Y0                ]
      R15:14 = VADDH(R21:20,R15:14):sat     //[ [S2_BF2]X[7:4]= XS0+Y2 : XS0+Y1     ]
      R20 = VMPYH(R5,R24):<<1:rnd:sat       //[ [S2_BF3]XS0=g[6]*C49                ]
      R16 = #(72-8)*4
    }
    { MEMW(R2++I:circ(M0)) = R8             //[ [S2_BF2]save X[1]                   ]
      R10 = COMBINE(R11.H,R9.L)             //[ [S2_BF3]R10 = Bi|Ar                 ]
      R11 = COMBINE(R11.L,R9.H)             //[ [S2_BF3]R11 = Br|Ai                 ]
      R9 = VADDH(R20,R9):sat                //[ [S2_BF3]XS0+Y0                      ]
    }
    { MEMW(R2++I:circ(M0)) = R14            //[ [S2_BF2]save X[4]                   ]
      R21 = R20                             //[ [S2_BF3]R21:20= XS0:XS0             ]
      P0 = BITSCLR(R28,#7)                  //[  Is the 9x8 block done?             ]
      P1 = CMP.EQ(R28,#40)                  //[  Is the last iteration?             ]
    }
    { MEMW(R2++I:circ(M1)) = R15            //[ [S2_BF2]save X[7]                   ]
      R10 = VDMPY(R11:10,R13:12):<<1:rnd:sat//[ [S2_BF3]Y1                          ]
      R11 = VDMPY(R11:10,R19:18):<<1:rnd:sat//[ [S2_BF3]Y2                          ]
      IF P0 R0 = ADD(R0,R16)                //[  If true, update R0 to next block   ]
    }
    { MEMW(R2++I:circ(M0)) = R9             //[ [S2_BF3]save X[2]                   ]
      R11:10 = VADDH(R21:20,R11:10):sat     //[ [S2_BF3]XS0+Y2 : XS0+Y1             ]
      IF !P1 R15 = MEMW(R0+#0*8*4)          //[ [2][S1_BF1]load in[0]               ]
    }
    { MEMW(R2++I:circ(M0)) = R10            //[ [S2_BF3]save X[5]                   ]
      IF !P1 R14 = MEMW(R0+#3*8*4)          //[ [2][S1_BF1]load in[3]               ]
    }
    { MEMW(R2) = R11                        //[ [S2_BF3]save X[8]                   ]
      R2 = ADD(R2,#145*4)                   //[  (280+225)%360                      ]
      IF !P1 R16 = MEMW(R0+#6*8*4)          //[ [2][S1_BF1]load in[6]               ]
      R13:12 = COMBINE(R22,R23)             //[ [2]                                 ]
    }:endloop0:endloop1

    { R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { DEALLOCFRAME                          //[ pop stack                           ]
    }
    { JUMPR R31                             //[ return                              ]
    }
    .size    blkfft9, .-blkfft9

    /*-----------------------------------------------------------------------------*/
    /*                 Temporary buffer                                            */
    /*-----------------------------------------------------------------------------*/
    .section    .bss,"aw",@nobits
    .lcomm    FFTTempBuf360,1440,8

