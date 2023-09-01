/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "cxfir.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void cxfir()                                                  ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform FIR filtering on a complex array                      ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - N, T are multiples of 2                                         ]*/
    /*[           - input, coefficients and output arrays are aligned by 8bytes     ]*/
    /*[           - Coefficients arranged in reverse order                          ]*/
    /*[           - 32-bit accumulation for real/imaginary part                     ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : CWord2x16 *coefs                                         ]*/
    /*[               R2 : int T             -- number of taps                      ]*/
    /*[               R3 : int N             -- number of samples                   ]*/
    /*[               R4 : int Qshift        -- shift factor                        ]*/
    /*[               R5 : CWord2x16 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R17, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (T+3)* N/2  + 4                                                 ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl cxfir
    .type    cxfir, @function
cxfir:
    { P0 = CMP.GT(R3,#1)                    //[ P0 = (N >= 2)                       ]
      IF !P0.new JUMPR:nt R31               //[ If !P0 return                       ]
      R3:2  = VASRW(R3:2, #1)               //[ R3:2 = N/2: T/2                     ]
    }
    { P0 = CMP.GT(R2,#3)                    //[ P1 = (T>=4)                         ]
      IF !P0.new JUMPR:nt R31               //[ If !P0 return                       ]
      IF  P0.new R28 = R1                   //[ R28 = coefs                         ]
      IF  P0.new R2 = ADD(R2, #-1)          //[ inner loop counter = T/2 -1         ]
    }
    { R29 = ADD(R29, #-1*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-1*8)) = R17:16           //[ callee-saved registers              ]
      P3 = CMP.GT(R2,R2)                    //[ clear P3                            ]
      LOOP1(.cxfir_sampLoop,R3)             //[ setup outer loop                    ]
    }
    
    .falign
.cxfir_sampLoop:
    { R7:6   = MEMD(R0++#8)                 //[ R7:6 = X(n+1)||X(n)                 ]
      R11:10 = MEMD(R1++#8)                 //[ load coeficients                    ]
      LOOP0(.cxfir_filtLoop,R2)             //[ setup inner loop: lc0=T/2-1         ]
    }
    { R9:8  = MEMD(R0)                      //[ load X(n+3)|X(n+2)                  ]
      R13:12 = CMPY(R10,R6):<<1:sat         //[ sum0 = coefs[0] * x[n  ]            ]
      R15:14 = CMPY(R10,R7):<<1:sat         //[ sum1 = coefs[0] * x[n+1]            ]
      R3 = ADD(R0,#8)                       //[ set pointer for load                ]
    }

    .falign
.cxfir_filtLoop:
    { R11:10 = MEMD(R1++#8)                 //[ load coefficients                   ]
      R13:12 += CMPY(R11,R7):<<1:sat        //[ sum0 += coefs[] * x[n+1]            ]
      R15:14 += CMPY(R11,R8):<<1:sat        //[ sum1 += coefs[] * x[n+2]            ]
      R7:6 = R9:8                           //[ R7:6= X[n+3]:X[n+2]                 ]
    }
    { R9:8  = MEMD(R3++#8)                  //[ load X(n+5)|X(n+4)                  ]
      R13:12 += CMPY(R10,R6):<<1:sat        //[ sum0 += coefs[] * x[n+2]            ]
      R15:14 += CMPY(R10,R7):<<1:sat        //[ sum1 += coefs[] * x[n+3]            ]
    }:endloop0

    { R13:12 += CMPY(R11,R7):<<1:sat        //[ sum0 += coefs[] * x[]               ]
      R15:14 += CMPY(R11,R8):<<1:sat        //[ sum1 += coefs[] * x[]               ]
      R1 = R28                              //[ set R1 = coeff                      ]
    }
    { IF P3 MEMD(R5++#8) = R17:16           //[[O2] Save output                     ]
      R13:12 = VASRW(R13:12,R4)             //[ Scaling output                      ]
      R15:14 = VASRW(R15:14,R4)             //[ Scaling output                      ]
    }
    { R16 = VSATWH(R13:12)                  //[ saturate to 16-bit                  ]
      R17 = VSATWH(R15:14)                  //[ saturate to 16-bit                  ]
      P3 = CMP.EQ(R2,R2)                    //[ set P3                              ]
    }:endloop1

    { MEMD(R5+#0) = R17:16                  //[[e1]save last two outputs            ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#1*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    cxfir, .-cxfir
