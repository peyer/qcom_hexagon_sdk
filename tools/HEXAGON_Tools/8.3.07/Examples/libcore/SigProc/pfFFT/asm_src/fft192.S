/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file	"fft192.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void fft192()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 192-point FFT                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and output are aligned by 1024-bytes                      ]*/
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
    /*[           - 1010                                                            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl fft192
    .type    fft192, @function
fft192:
    { ALLOCFRAME(#16)                       //[ allocate stack                      ]
      R3 = CONST32(#FFTtwiddles)            //[ ptr to twiddle factor               ]
      NOP; NOP
    }
    { MEMD(R29+#0) = R27:26                 //[ callee-saved registers              ]
      R26 = R3                              //[ R26 = FFTtwiddles                   ]
      R27 = CONST32(#FFTTempBuf192)         //[ ptr to temporay buffer              ]
      NOP
    }
    /* --------------------------------------------------------------------------- */
    /*                re-order input and perform 5-point FFT                       */
    /* --------------------------------------------------------------------------- */
    { MEMD(R29+#8) = R25:24                 //[ callee-saved registers              ]
      R24 = ADD(R27,#(1*64*4))              //[ FFTTempBuf + 1*64*sizeof(CWord2x16) ]
      R25 = R1                              //[ save output ptr in R25              ]
      CALL blkfft3                          //[ blkfft3(in,out)                     ]
    }
    /* --------------------------------------------------------------------------- */
    /*                perform 64-point FFT on 5 rows                               */
    /* --------------------------------------------------------------------------- */

    .falign
.fft192_fft64LOOP:
    { R0 = R25                              //[ func call: R0 = out                 ]
      R1 = #64                              //[ func call: R1 = 64                  ]
      R3:2 = R27:26                         //[ func call: R2=wt64, R3=FFTtemBuf    ]
      CALL sfft16x16                        //[ sfft16x16(out, 64, wt64, FFTTempBuf)]
    }
    { P0 = CMP.GT(R27,R24)                  //[ i*64*4 > 1*64*4                     ]
      R25 = ADD(R25,#256)                   //[ update to next 64 points            ]
      R27 = ADD(R27,#256)                   //[ update to next 64 points            ]
      IF (!P0.new) JUMP:t .fft192_fft64LOOP //[ if !P0 contine loop                 ]
    }
    /* --------------------------------------------------------------------------- */
    /*                Re-order outputs                                             */
    /* --------------------------------------------------------------------------- */
    { R4 = CONST32(#0x18020300)             //[ m0 value: I = 129           (:circ) ]
      R5 = CONST32(#0x08020300)             //[ m1 value: I = (129+64)%192=1(:circ) ]
      R0 = ADD(R27,#-(3*64*4))              //[ set R0 = FFTTempBuf                 ]
      LOOP1(.fft192_ReorderLOOP,#3)         //[ setup loop                          ]
    }
    { M0 = R4                               //[ set m0                              ]
      R1 = ADD(R25,#-(3*64*4))              //[ set R1 = output                     ]
      R27:26 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { M1 = R5                               //[ set m0                              ]
      DEALLOCFRAME                          //[ pop stack                           ]
    }
    .falign
.fft192_ReorderLOOP:
    { R2 = MEMW(R0++#4)                     //[ [p]load xb2[j][i]                   ]
      LOOP0(.fft192_Reorder_inLOOP,#63)     //[ setup i-loop: lc0 = 64-1            ]
    }

    .falign
.fft192_Reorder_inLOOP:
    { R2 = MEMW(R0++#4)                     //[[1]load xb2[j][i]                    ]
      MEMW(R1++I:circ(M0)) = R2             //[[2]output[(65*i+256*j)%192]=xb2[j][i]]
    }:endloop0 

    { MEMW(R1++I:circ(M1)) = R2             //[save last output and update write ptr]
    }:endloop1

    { JUMPR     R31                         //[ return                              ]
    }
    .size    fft192, .-fft192



    /*[*****************************************************************************]*/
    /*[  Function   : void blkfft3()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 3-point FFT on 64 blocks                              ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input  is aligned by 1024-bytes (due to circular addressing)    ]*/
    /*[           - output is aligned by 8-bytes                                    ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : CWord2x16 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R21                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 24                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 7*(64/2) + 12                                                   ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl blkfft3
    .type	blkfft3, @function
blkfft3:
    { R29 = ADD(R29,#-3*8)                  //[ reserve stack                       ]
      MEMD(R29+#(-3*8+0)) = R17:16          //[ callee-saved regs                   ]
      R16.H = #0xB619                       //[ R16 =  conj(WN3x23)                 ]
      R17.H = #0x49E7                       //[ R17 =  WN3x23                       ]
    }
    { MEMD(R29+#8 ) = R19:18                //[ callee-save regs                    ]
      R16.L = #0x2AAB                       //[ R16 =  conj(WN3x23)                 ]
      R17.L = #0x2AAB                       //[ R17 = WN3x23                        ]
      R3 = R1                               //[ &output[0]                          ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-save regs                    ]
      R20 = COMBINE(R16.L,R16.L)            //[ R20 = 0x2AAB2AAB (1/3)              ]
      R21.L = #0x5555                       //[ R21 = 0x55555555 (2/3)              ]
      R6.L = #192*4                         //[ M0 value for circular addressing    ]
    }
    { R4 = ADD(R1,#64*4)                    //[ &output[64]                         ]
      R5 = ADD(R1,#128*4)                   //[ &output[128]                        ]
      R6.H = #0x0800                        //[ M0 value for circular addressing    ]
    }
    { M0 = R6                               //[ set M0 for circular addressing      ]
      R1 = ADD(R0,#64*4)                    //[ &buffer_ptr[64]                     ]
      R2 = ADD(R0,#128*4)                   //[ &buffer_ptr[128]                    ]
      R21.H = #0x5555                       //[ R21 = ##0x55555555                  ]
    }
    { R19:18 = COMBINE(R16,R17)             //[ R19:18 = conj(WN3x23) : WN3x23      ]
      P3 = SP1LOOP0(.blkfft3_LOOP,#64/2)    //[ setup loop                          ]
    }

    .falign
.blkfft3_LOOP:
    { R8 = MEMW(R1++#12:circ(M0))           //[[1]load x[0]                         ]
      R13:12 = VADDH(R13:12,R7:6):sat       //[[2]x[0]= cadd(XS0, Y0)               ]
      R10 = COMBINE(R11.H,R9.L)             //[[2]Bi|Ar                             ]
      R11 = COMBINE(R11.L,R9.H)             //[[2]Br|Ai                             ]
    }
    { R9 = MEMW(R1++#12:circ(M0))           //[[1]load x[0]                         ]
      IF P3 MEMD(R3++#8) = R13:12           //[[2]save x[0]                         ]
      R12 = VDMPY(R15:14,R17:16):<<1:rnd:sat//[[2]Y1                                ]
      R13 = VDMPY(R11:10,R17:16):<<1:rnd:sat//[[2]Y1                                ]
    }
    { R10 = MEMW(R2++#12:circ(M0))          //[[1]load x[2]                         ]
      R14 = VDMPY(R15:14,R19:18):<<1:rnd:sat//[[2]Y2                                ]
      R15 = VDMPY(R11:10,R19:18):<<1:rnd:sat//[[2]Y2                                ]
    }
    { R11 = MEMW(R2++#12:circ(M0))          //[[1]load x[2]                         ]
      R13:12 = VSUBH(R7:6,R13:12):sat       //[[2]x[1] = csub(XS0,Y1)               ]
      R15:14 = VSUBH(R7:6,R15:14):sat       //[[2]x[2] = csub(XS0,Y2)               ]
    }
    { R6 = MEMW(R0++#12:circ(M0))           //[[1]load x[0]                         ]
      R9:8   = VAVGH(R9:8,R11:10):crnd      //[[1]A = x[1] + x[2]                   ]
      R11:10 =VNAVGH(R9:8,R11:10):crnd:sat  //[[1]B = x[1] - x[2]                   ]
      IF P3 MEMD(R4++#8) = R13:12           //[[2]save x[1]                         ]
    }
    { R7 = MEMW(R0++#12:circ(M0))           //[[1]load x[0]                         ]
      R6  = VMPYH(R6,R20):<<1:rnd:sat       //[[1]XS0 = x[0] * 1/3                  ]
      R12 = VMPYH(R8,R21):<<1:rnd:sat       //[[1]Y0 = A* 1/3                       ]
      IF P3 MEMD(R5++#8) = R15:14           //[[2]save x[2]                         ]
    }
    { R7  = VMPYH(R7,R20):<<1:rnd:sat       //[[1]XS0 = x[0] * 1/3                  ]
      R13 = VMPYH(R9,R21):<<1:rnd:sat       //[[1]Y0 = A* 1/3                       ]
      R14 = COMBINE(R10.H,R8.L)             //[[1]Bi|Ar                             ]
      R15 = COMBINE(R10.L,R8.H)             //[[1]Br|Ai                             ]
    }:endloop0

    { R13:12 = VADDH(R13:12,R7:6):sat       //[[e]x[0]= cadd(XS0, Y0)               ]
      R10 = COMBINE(R11.H,R9.L)             //[[e]Bi|Ar                             ]
      R11 = COMBINE(R11.L,R9.H)             //[[e]Br|Ai                             ]
    }
    { MEMD(R3++#8) = R13:12                 //[[e]save x[0]                         ]
      R12 = VDMPY(R15:14,R17:16):<<1:rnd:sat//[[e]Y1                                ]
      R13 = VDMPY(R11:10,R17:16):<<1:rnd:sat//[[e]Y1                                ]
    }
    { R14 = VDMPY(R15:14,R19:18):<<1:rnd:sat//[[e]Y2                                ]
      R15 = VDMPY(R11:10,R19:18):<<1:rnd:sat//[[e]Y2                                ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved regs           ]
    }
    { R13:12 = VSUBH(R7:6,R13:12):sat       //[[e]x[1] = csub(XS0,Y1)               ]
      R15:14 = VSUBH(R7:6,R15:14):sat       //[[e]x[2] = csub(XS0,Y2)               ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved regs           ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved regs           ]
    }
    { MEMD(R4) = R13:12                     //[[e]save x[1]                         ]
    }
    { MEMD(R5) = R15:14                     //[[e]save x[2]                         ]
      R29 = ADD(R29,#3*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	blkfft3, .-blkfft3


    /*-----------------------------------------------------------------------------*/
    /*                 Temporary buffer                                            */
    /*-----------------------------------------------------------------------------*/
    .section    .bss,"aw",@nobits
    .lcomm    FFTTempBuf192,768,8

