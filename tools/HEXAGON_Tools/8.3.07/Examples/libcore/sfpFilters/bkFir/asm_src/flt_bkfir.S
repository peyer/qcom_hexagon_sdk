/***************************************************************************
* Copyright (c) Date: Tue Mar 06 16:26:05 CST 2018 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Tue Mar 06 16:26:05 CST 2018 
****************************************************************************/ 

    .file   "flt_bkfir.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void flt_bkfir()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  Description: FIR filtering on a bulk of data                               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - length and taps are multiple of 4                               ]*/
    /*[           - length > 0 * Taps >= 4 (callee-checked)                         ]*/
    /*[           - Input/Output array is aligned by 8                              ]*/
    /*[           - Coefficient array is aligned by 8                               ]*/ 
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : float *xin   -- pointer to input buffer                  ]*/
    /*[               R1 : float *coefs -- pointer to Coefficients                  ]*/
    /*[               R2 : int taps      -- Number of Taps                          ]*/
    /*[               R3 : int length    -- Number of Samples                       ]*/
    /*[               R4 : float *yout  -- pointer to output buffer                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Implementation:                                                            ]*/
    /*[           - process 4 samples per iteration in inner loop                   ]*/
    /*[           - generate 4 outputs per iteration in outer loop                  ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl flt_bkfir
    .type    flt_bkfir, @function
flt_bkfir:
    { P0 = CMP.GT(R3,#3)                    //[ P0 = length >= 4 &&                 ]
      P0 = CMP.GT(R2,#3)                    //[      Taps   >= 4                    ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
    }
    { R29 = ADD(R29,#-2*8)                  //[ reserve stack                       ]
      R3  = LSR(R3, #2)                     //[ N/4                                 ]
      R1 += ASL(R2,#2)                      //[ R1 = &c[T]                          ]
      R5 = ADD(R0,#16)                      //[ &x[4]                               ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved regs                   ]
      R2 = LSR(R2,#2)                       //[ T/4                                 ]
      P1 = CMP.GT(R2,#4)                    //[ P1 = (LC0 > 0)                      ]
      LOOP1(.flt_bkfir_mainLOOP,R3)         //[ setup outer loop:lc1 = N/4          ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved regs                   ]
      R1 = ADD(R1,#-8)                      //[ R1 = &c[T-2]                        ]
      R2 = ADD(R2,#-1)                      //[ LC0 = T/4-1                         ]
      P3 = XOR(P3,P3)                       //[ clean P3                            ]
    }

    .falign
.flt_bkfir_mainLOOP:
    { R18 += SFMPY(R6,R11)                  //[ [e]sum2 += c[T-k-4]*x[k+5]          ]
      R19 += SFMPY(R6,R14)                  //[ [e]sum3 += c[T-k-4]*x[k+6]          ]
      R11:10 = MEMD(R0++#8)                 //[ load x[k+1|k+0]                     ]
      IF P3 MEMD(R4++#8) = R17:16           //[ [e]save output                      ]
    }
    { R13:12 = MEMD(R0++#8)                 //[ load x[k+3|k+2]                     ]
      R9:8 = MEMD(R1+#0)                    //[ load c[T-1]|c[T-2]                  ]
      R3 = ADD(R1,#-8)                      //[ R3= &c[T-4]                         ]
      LOOP0(.flt_bkfir_innerLOOP,R2)        //[ setup inner loop: lc0 = T/4-1       ]
    }
    { IF P3 MEMD(R4++#8) = R19:18           //[ [e]save output                      ]
      R17:16 = #0                           //[ sum0 = sum1 = 0                     ]
      R19:18 = #0                           //[ sum2 = sum3 = 0                     ]
      IF !P1 JUMP .flt_bkfir_innerLPEND     //[ if !(LC0>0) skip loop               ]
    }

    .falign
.flt_bkfir_innerLOOP:
    { R16 += SFMPY(R9,R10)                  //[ sum0 += c[T-k-1]*x[k  ]             ]
      R17 += SFMPY(R9,R11)                  //[ sum1 += c[T-k-1]*x[k+1]             ]
      R15:14 = MEMD(R5++#8)                 //[ load x[k+5]:x[k+4]                  ]
    }
    { R18 += SFMPY(R9,R12)                  //[ sum2 += c[T-k-1]*x[k+2]             ]
      R19 += SFMPY(R9,R13)                  //[ sum3 += c[T-k-1]*x[k+3]             ]
      R7:6 = MEMD(R3++#-8)                  //[ load c[T-k-3]:c[T-k-4]              ]
    }
    { R16 += SFMPY(R8,R11)                  //[ sum0 += c[T-k-2]*x[k+1]             ]
      R17 += SFMPY(R8,R12)                  //[ sum1 += c[T-k-2]*x[k+2]             ]
      R11:10 = R15:14                       //[                                     ]
    }
    { R18 += SFMPY(R8,R13)                  //[ sum2 += c[T-k-2]*x[k+3]             ]
      R19 += SFMPY(R8,R14)                  //[ sum3 += c[T-k-2]*x[k+4]             ]
    }
    { R16 += SFMPY(R7,R12)                  //[ sum0 += c[T-k-3]*x[k+2]             ]
      R17 += SFMPY(R7,R13)                  //[ sum1 += c[T-k-3]*x[k+3]             ]
      R15:14 = MEMD(R5++#8)                 //[ load x[k+7]:x[k+6]                  ]
    }
    { R18 += SFMPY(R7,R10)                  //[ sum2 += c[T-k-3]*x[k+4]             ]
      R19 += SFMPY(R7,R11)                  //[ sum3 += c[T-k-3]*x[k+5]             ]
      R9:8 = MEMD(R3++#-8)                  //[ load c[T-k-5]:c[T-k-6]              ]
    }
    { R16 += SFMPY(R6,R13)                  //[ sum0 += c[T-k-4]*x[k+3]             ]
      R17 += SFMPY(R6,R10)                  //[ sum1 += c[T-k-4]*x[k+4]             ]
      R13:12 = R15:14                       //[
    }
    { R18 += SFMPY(R6,R11)                  //[ sum2 += c[T-k-4]*x[k+5]             ]
      R19 += SFMPY(R6,R14)                  //[ sum3 += c[T-k-4]*x[k+6]             ]
    }:endloop0

.flt_bkfir_innerLPEND:
    { R16 += SFMPY(R9,R10)                  //[ sum0 += c[T-k-1]*x[k  ]             ]
      R17 += SFMPY(R9,R11)                  //[ sum1 += c[T-k-1]*x[k+1]             ]
      R15:14 = MEMD(R5++#8)                 //[ load x[k+5]:x[k+4]                  ]
    }
    { R18 += SFMPY(R9,R12)                  //[ sum2 += c[T-k-1]*x[k+2]             ]
      R19 += SFMPY(R9,R13)                  //[ sum3 += c[T-k-1]*x[k+3]             ]
      R7:6 = MEMD(R3++#-8)                  //[ load c[T-k-3]:c[T-k-4]              ]
    }
    { R16 += SFMPY(R8,R11)                  //[ sum0 += c[T-k-2]*x[k+1]             ]
      R17 += SFMPY(R8,R12)                  //[ sum1 += c[T-k-2]*x[k+2]             ]
      R11:10 = R15:14                       //[                                     ]
    }
    { R18 += SFMPY(R8,R13)                  //[ sum2 += c[T-k-2]*x[k+3]             ]
      R19 += SFMPY(R8,R14)                  //[ sum3 += c[T-k-2]*x[k+4]             ]
    }
    { R16 += SFMPY(R7,R12)                  //[ sum0 += c[T-k-3]*x[k+2]             ]
      R17 += SFMPY(R7,R13)                  //[ sum1 += c[T-k-3]*x[k+3]             ]
      R14 = MEMW(R5+#0)                     //[ load x[k+7]:x[k+6]                  ]
    }
    { R18 += SFMPY(R7,R10)                  //[ sum2 += c[T-k-3]*x[k+4]             ]
      R19 += SFMPY(R7,R11)                  //[ sum3 += c[T-k-3]*x[k+5]             ]
      R5 = ADD(R0,#16)                      //[ &x[k+4]                             ]
    }
    { R16 += SFMPY(R6,R13)                  //[ sum0 += c[T-k-4]*x[k+3]             ]
      R17 += SFMPY(R6,R10)                  //[ sum1 += c[T-k-4]*x[k+4]             ]
      P3 = CMP.GT(R2,#-1)                   //[ set P3                              ]
    }:endloop1

    { R18 += SFMPY(R6,R11)                  //[ sum2 += c[T-k-4]*x[k+5]             ]
      R19 += SFMPY(R6,R14)                  //[ sum3 += c[T-k-4]*x[k+6]             ]
    }
    { MEMD(R4+#0) = R17:16                  //[ save output                         ]
      MEMD(R4+#8) = R19:18                  //[ save output                         ]
    }
    { R17:16 = MEMD(R29+#0)                 //[ restore callee-saved regs           ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved regs           ]
      R29 = ADD(R29,#2*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    flt_bkfir, .-flt_bkfir

