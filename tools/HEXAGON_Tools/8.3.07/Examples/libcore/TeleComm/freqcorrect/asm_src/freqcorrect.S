/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "freqcorrect.S"

#define LUT_SIZE     512
#define IDX_BITS     9                    // log2(LUT_SIZE) 
    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 freqcorrect()                                         ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform phase rotation on complex input vector to             ]*/
    /*[               compensate frequency offset                                   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - phacc and ft are in Q24                                         ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : Word32    ft     - normalized frequency defined as f*Ts  ]*/
    /*[               R2 : UWord32   phacc  - accumulation of absolute ft           ]*/
    /*[               R3 : int       nsamp                                          ]*/
    /*[               R2 : CWord2x16 *output                                        ]*/
    /*[                                                                             ]*/
    /*[  Outputs    : R0 : accumulation of absolute ft, i.e., phacc                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -  5*N + 8                                                        ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl freqcorrect
    .type    freqcorrect, @function
freqcorrect:
    { P0 = CMP.GT(R3,#0)                    //[ P0 = nsampe > 0?                    ]
      IF !P0.new R0 = R2                    //[ IF !P0 return value = phacc         ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
    }
    { P1 = CMP.GT(R1,#0)                    //[ ft > 0 ?                            ]
      R15:14 = #0                           //[ R15:14 = 0                          ]
      R5.L = #LO(COSSINTAB)                 //[ R5 = COSSINTAB                      ]
      R28 = #0x7FFF                         //[ set R28 = 0x7FFF                    ]
    }
    { R1 = ABS(R1)                          //[ ftabs                               ]
      R3 = ADD(R3,#1)                       //[ LC = nsamp+1                        ]
      R5.H = #HI(COSSINTAB)                 //[ R5 = COSSINTAB                      ]
      IF !P1 JUMP .freqcorrect_NegativeFT   //[ if ft<0 jump                        ]
    }
    /* --------------------------------------------------------------------------- */
    /*                   for case ft >=0                                           */
    /* --------------------------------------------------------------------------- */
    { P3 = SP2LOOP0(.freqcorrect_PostiveFtLOOP,R3)
    }                                       //[ setup loop0: lc0= nsamp+1           ]

    .falign
.freqcorrect_PostiveFtLOOP:
    { R5 = TABLEIDXW(R2,#IDX_BITS-1,#15)    //[[1]COSSINTAB[index]                  ]
      R13 = AND(R2,R28)                     //[[1]frac = phacc & 0x7FFF             ]
      R11:10 = VSUBW(R15:14,R9:8):sat       //[[2]-xs : -xc                         ]
      IF P3 MEMW(R4++#4) = R3               //[[3]save output[i]                    ]
    }
    { R6 = MEMW(R5)                         //[[1]COSSINTAB[index+1 : index  ]      ]
      R7 = MEMW(R5+#4)                      //[[1]COSSINTAB[index+3 : index+2]      ]
      P0 = TSTBIT(R2,#(IDX_BITS-1+15))      //[[1](phacc>>15)& (LUT_SIZE/2)         ]
      IF !P0 R11:10 = COMBINE(R9,R8)        //[[2] xs : xc                          ]
    }                                       //[[1]COSSINTAB[index+3 : index+2]      ]
    { R6 = VSUBH(R7,R6)                     //[[1]- COSSINTAB[index+1 : index]      ]
      R9:8 = PACKHL(R6,R14)                 //[[1]COSSINTAB[index+1: index] << 16   ]
      R13 = COMBINE(R13.L,R13.L)            //[[1]R13 = frac|frac                   ]
      R10 = COMBINE(R11.H,R10.H)            //[[2]x                                 ]
    }
    { R9:8 += VMPYH(R13,R6):<<1:sat         //[[1]xs: xc                            ]
      R12 = MEMW(R0++#4)                    //[[1]load input[i]                     ]
      R2 = ADD(R2,R1)                       //[[1]phacc += ftabs                    ]
      R3 = CMPY(R12,R10):<<1:rnd:sat        //[[2]output[i]                         ]
    }:endloop0

    { MEMW(R4) = R3                         //[ [e]save output[i]                   ]
      R0 = SUB(R2,R1)                       //[ modify phacc                        ]
      JUMPR R31                             //[ return                              ]
    }

    .falign
    /* --------------------------------------------------------------------------- */
    /*                   for case ft <0                                            */
    /* --------------------------------------------------------------------------- */
.freqcorrect_NegativeFT:
    { P3 = SP2LOOP0(.freqcorrect_NegativeFtLOOP,R3)
    }                                       //[ setup loop0: lc0= nsamp+1           ]

    .falign
.freqcorrect_NegativeFtLOOP:
    { R5 = TABLEIDXW(R2,#IDX_BITS-1,#15)    //[[1]COSSINTAB[index]                  ]
      R13 = AND(R2,R28)                     //[[1]frac = phacc & 0x7FFF             ]
      R11:10 = VSUBW(R15:14,R9:8):sat       //[[2]-xs : -xc                         ]
      IF P3 MEMW(R4++#4) = R3               //[[3]save output[i]                    ]
    }
    { R6 = MEMW(R5)                         //[[1]COSSINTAB[index+1 : index  ]      ]
      R7 = MEMW(R5+#4)                      //[[1]COSSINTAB[index+3 : index+2]      ]
      P0 = TSTBIT(R2,#(IDX_BITS-1+15))      //[[1](phacc>>15)& (LUT_SIZE/2)         ]
      IF !P0 R11:10 = COMBINE(R9,R8)        //[[2] xs : xc                          ]
    }                                       //[[1]COSSINTAB[index+3 : index+2]      ]
    { R6 = VSUBH(R7,R6)                     //[[1]- COSSINTAB[index+1 : index]      ]
      R9:8 = PACKHL(R6,R14)                 //[[1]COSSINTAB[index+1: index] << 16   ]
      R13 = COMBINE(R13.L,R13.L)            //[[1]R13 = frac|frac                   ]
      R10 = COMBINE(R11.H,R10.H)            //[[2]x                                 ]
    }
    { R9:8 += VMPYH(R13,R6):<<1:sat         //[[1]xs: xc                            ]
      R12 = MEMW(R0++#4)                    //[[1]load input[i]                     ]
      R2 = ADD(R2,R1)                       //[[1]phacc += ftabs                    ]
      R3 = CMPY(R12,R10*):<<1:rnd:sat       //[[2]output[i]                         ]
    }:endloop0

    { MEMW(R4) = R3                         //[[e]save output[i]                    ]
      R0 = SUB(R2,R1)                       //[ modify phacc                        ]
      JUMPR R31                             //[ return                              ]
    }
    .size    freqcorrect, .-freqcorrect
