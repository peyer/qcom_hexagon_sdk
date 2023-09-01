/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "bkfir.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void bkfir()                                                  ]*/
    /*[*****************************************************************************]*/
    /*[  Description: FIR filtering on a bulk of data                               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - length and Taps are multiple of 4                               ]*/
    /*[           - length > 0 * Taps >= 8 (callee-checked)                         ]*/
    /*[           - Input/Output array is aligned by 8                              ]*/
    /*[           - Coefficient array is aligned by 8                               ]*/ 
    /*[           - Coefficients arranged in reverse order                          ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *xin   -- pointer to input buffer                 ]*/
    /*[               R1 : Word16 *coefs -- pointer to Coefficients                 ]*/
    /*[               R2 : int taps      -- Number of Taps                          ]*/
    /*[               R3 : int length    -- Number of Samples                       ]*/
    /*[               R4 : int Qshift    -- shift factor                            ]*/
    /*[               R5 : Word16 *yout  -- pointer to output buffer                ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32                                ]*/
    /*[=============================================================================]*/
    /*[  Implementation:                                                            ]*/
    /*[           - process 4 samples per iteration in inner loop                   ]*/
    /*[           - generate 4 outputs per iteration in outer loop                  ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (N/4)*[3*(T/4)+4] + 9                                           ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl bkfir
    .type    bkfir, @function
bkfir:
    { P0 = CMP.GT(R3,#0)                    //[ P0 = length>0                       ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
      IF  P0.new R28 = R1                   //[ save coefs in R28                   ]
      R3:2  = VASRW(R3:2, #2)               //[ N/4 : T/4
    }
    { P0 = CMP.GT(R2,#1)                    //[ P0 = T/4 > 1                        ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
      R2 = ADD(R2,#-2)                      //[ LC0 = T/4-2                         ]
      P3 = CMP.GT(R2,R2)                    //[ clean P3                            ]
    }
    { R29 = ADD(R29, #-4*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-4*8)) = R17:16           //[ callee-saved registers              ]
      R7:6 = MEMD(R0++#8)                   //[ R7:6 = x3|x2|x1|x0                  ]
      R4 = ADD(R4,#16)                      //[ Qshift+16                           ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R10  = MEMW(R0+#0)                    //[ R10 = x5|x4                         ]
      LOOP1(.bkfir_mainLOOP,R3)             //[ setup outer loop:lc1 = N/4          ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R9:8 = COMBINE(R10,R7)                //[ R9:8 = x5|x4|x3|x2,                 ]
      R15:14 = MEMD(R1++#8)                 //[ load coefficients                   ]
      R3 = ADD(R0,#8)                       //[ set read pointer                    ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R17:16 = VRMPYH(R7:6,R15:14)          //[ [p] sum0 += coefs[] * x[]           ]
      R19:18 = VRMPYH(R9:8,R15:14)          //[ [p] sum2 += coefs[] * x[]           ]
      R12 = MEMW(R0+#4)                     //[ [p] R12 = x7|x6                     ]
    }

    .falign
.bkfir_mainLOOP:
    { IF P3 MEMD(R5++#8) = R21:20           //[ [e1]save yout[i] -- yout[i+4]       ]
      R7:6 = COMBINE(R12,R10)               //[ R7:6 = x7|x6|x5|x4                  ]
      R11:10 = VALIGNB(R11:10,R7:6,#2)      //[ R11:10 = x4|x3|x2|x1                ]
      LOOP0(.bkfir_innerLOOP,R2)            //[ setup inner loop: lc0 = T/4-2       ]
    }
    { R13:12 = VALIGNB(R13:12,R9:8,#2)      //[ R13:12 = x6|x5|x4|x3                ]
      R21:20 = VRMPYH(R11:10,R15:14)        //[ sum1 += coefs[] * x[]               ]
      R9 = MEMW(R3++#4)                     //[ R8 = x9|x8                          ]
      P0 = CMP.GT(R2,#0)                    //[ P0 = (LC0 > 0)                      ]
    }
    { R15:14 = MEMD(R1++#8)                 //[ load coefficients                   ]
      R23:22 = VRMPYH(R13:12,R15:14)        //[ sum3 += coefs[] * x[]               ]
      R8 = R7                               //[ R9:8 = x9|x8|x7|x6                  ]
      IF !P0 JUMP .bkfir_innerLPEND         //[ if !(LC0>0) skip loop               ]
    }

    .falign
.bkfir_innerLOOP:
    { R10 = R9                              //[[1]R10 = x5|x4                       ]
      R12 = MEMW(R3++#4)                    //[[1]R12 = x7|x6                       ]
      R17:16 += VRMPYH(R7:6,R15:14)         //[[2]sum0 += coefs[] * x[]             ]
      R19:18 += VRMPYH(R9:8,R15:14)         //[[2]sum2 += coefs[] * x[]             ]
    }
    { R11:10 = VALIGNB(R11:10,R7:6,#2)      //[[1]R11:10 = x4|x3|x2|x1              ]
      R13:12 = VALIGNB(R13:12,R9:8,#2)      //[[1]R13:12 = x6|x5|x4|x3              ]
      R7:6 = COMBINE(R12,R10)               //[[1]R7:6 = x7|x6|x5|x4                ]
      R9 = MEMW(R3++#4)                     //[[1]R8 = x9|x8                        ]
    }
    { R21:20 += VRMPYH(R11:10,R15:14)       //[[1]sum1 += coefs[] * x[]             ]
      R23:22 += VRMPYH(R13:12,R15:14)       //[[1]sum3 += coefs[] * x[]             ]
      R8 = R7                               //[[1]R9:8 = x9|x8|x7|x6                ]
      R15:14 = MEMD(R1++#8)                 //[[1]load coefficients                 ]
    }:endloop0

    .falign
.bkfir_innerLPEND:
    { R17:16 += VRMPYH(R7:6,R15:14)         //[ sum0 += coefs[] * x[]               ]
      R19:18 += VRMPYH(R9:8,R15:14)         //[ sum2 += coefs[] * x[]               ]
      R10 = R9                              //[ R10 = x5|x4                         ]
      R12 = MEMW(R3++#4)                    //[ R12 = x7|x6                         ]
    }
    { R11:10 = VALIGNB(R11:10,R7:6,#2)      //[ R11:10 = x4|x3|x2|x1                ]
      R13:12 = VALIGNB(R13:12,R9:8,#2)      //[ R13:12 = x6|x5|x4|x3                ]
      R7:6 = MEMD(R0++#8)                   //[ [O1] R7:6 = x3|x2|x1|x0             ]
      R9 = MEMW(R0+#8)                      //[ [O1] R9 = x5|x4                     ]
    }
    { R21:20 += VRMPYH(R11:10,R15:14)       //[ sum1 += coefs[] * x[]               ]
      R23:22 += VRMPYH(R13:12,R15:14)       //[ sum3 += coefs[] * x[]               ]
      R3 = ADD(R0,#4)                       //[ [O1]                                ]
      R1 = R28                              //[ [O1]set R1 = coeff                  ]
    }
    { R17:16 = ASR(R17:16,R4)               //[ sum0 >>= Qshift                     ]
      R21:20 = ASR(R21:20,R4)               //[ sum1 >>= Qshift                     ]
      R8 = R7                               //[ [O1] R9:8 = x5|x4|x3|x2             ]
      R10 = R9                              //[ [O1] R10 = x5|4                     ]
    }
    { R19:18 = ASR(R19:18,R4)               //[ sum2 >>= Qshift                     ]
      R23:22 = ASR(R23:22,R4)               //[ sum3 >>= Qshift                     ]
      R15:14 = MEMD(R1++#8)                 //[ [O1] load coefficients              ]
    }
    { R21:20 = COMBINE(R20,R16)             //[ pack to sum1:sum0                   ]
      R23:22 = COMBINE(R22,R18)             //[ pack to sum3:sum2                   ]
      R17:16 = VRMPYH(R7:6,R15:14)          //[ [O1] sum0 += coefs[] * x[]          ]
      R19:18 = VRMPYH(R9:8,R15:14)          //[ [O1] sum2 += coefs[] * x[]          ]
    }
    { R20 = VSATWH(R21:20)                  //[ saturate to 16-bit                  ]
      R21 = VSATWH(R23:22)                  //[ saturate to 16-bit                  ]
      P3 = CMP.EQ(R2,R2)                    //[ set P3 (for conditional write)      ]
      R12 = MEMW(R3++#4)                    //[ [O1] R12 = x7|x6                    ]
    }:endloop1

    { MEMD(R5) = R21:20                     //[[e1]save output                      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#4*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    bkfir, .-bkfir
