/***************************************************************************
* Copyright (c) Date: Tue Mar 06 16:26:05 CST 2018 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Tue Mar 06 16:26:05 CST 2018
****************************************************************************/

    .file   "flt_iir_cas_bq.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void flt_IIR_casBiquad()                                      ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Implements a cascade of biquads IIR filter                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input/output arrays are aligned by 8bytes                       ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : float *input                                             ]*/
    /*[               R1 : float *coef                                              ]*/
    /*[               R2 : float *states                                            ]*/
    /*[               R3 : int    nsec     -- number of biquad sections             ]*/
    /*[               R4 : int    nsamples -- number of samples                     ]*/
    /*[               R2 : float *output                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R19                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 16                                ]*/
    /*[=============================================================================]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl flt_IIR_casBiquad
    .type    flt_IIR_casBiquad, @function
flt_IIR_casBiquad:
    { P0 = CMP.GT(R4,#0)                    //[ P0 = (nsamp > 0 ) &&                ]
      P0 = CMP.GT(R3,#0)                    //[      (nSec  > 0)                    ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
      R28 = LSR(R4,#1)                      //[ nsampe>>1                           ]
    }
    { P2 = TSTBIT(R4,#0)                    //[ nsampe is odd number?               ]
      R4 = R2                               //[ r4 = pState                         ]
      P0 = CMP.GT(R28,#0)                   //[ if nsamp==1                         ]
      IF !P0.new JUMP:nt .IIR_filt_1sample  //[ then jump                           ]
    }
    { R29 = ADD(R29,#-2*8)                  //[ reserve stack                       ]
      MEMD(R29+#(-2*8 + 0)) = R17:16        //[ callee-saved registers              ]
      MEMD(R29+#(-2*8 + 8)) = R19:18        //[ callee-saved registers              ]
      LOOP1(.IIR_sampLoop,R28)              //[ setup loop1: lR10 = nsamp/2         ]
    }
    { R15:14 = COMBINE(R2,R1)               //[ coef = pCoef; state = pState        ]
      P1 = CMP.EQ(R3,#1)                    //[ nsec==1                             ]
      R3 = ADD(R3,#-1)                      //[ to unroll inner loop once           ]
      P3 = XOR(P3,P3)                       //[ clean P3                            ]
    }

    .falign
.IIR_sampLoop:
    { R7:6 = MEMD(R15++#8)                  //[[1]load state[1]:state[0]            ]
      R11:10 = MEMD(R14+#8)                 //[[1]load c1, c3                       ]
      R18 = SFADD(R12,R16)                  //[[e]x0 = t0 + c2*s0 + c3*s1           ]
      LOOP0(.IIR_filtLoop,R3)               //[ setup loop0 : lR8 = nsec            ]
    }
    { R9:8 = MEMD(R14++#16)                 //[[1]load c0,c2                        ]
      R13:12 = MEMD(R0++#8)                 //[ load xin[i+1:i]                     ]
      R19 = SFADD(R13,R17)                  //[[e]x1 = t1 + c2*t0 + c3*s0           ]
      IF P1 JUMP .IIR_filtLpend             //[ if nsec=1 skip the loop             ]
    }

    .falign
.IIR_filtLoop:
    { R12 += SFMPY(R10,R7)                  //[[2] x0 + c1*s1                       ]
      R16  = SFMPY(R11,R7)                  //[[2]      c3*s1                       ]
    }
    { R11:10 = MEMD(R14+#8)                 //[[1]load c1, c3 for next iter         ]
      R13 += SFMPY(R10,R6)                  //[[2] x1 + c1*s0                       ]
      R17  = SFMPY(R11,R6)                  //[[2]      c3*s0                       ]
    }
    { R7:6 = MEMD(R15++#8)                  //[[1]load state[1], state[0] for next  ]
      R12 += SFMPY(R8,R6)                   //[[2]t0 = x0 + c0*s0 + c1*s1           ]
      R16 += SFMPY(R9,R6)                   //[[2]          c2*s0 + c3*s1           ]
    }
    { R9:8 = MEMD(R14++#16)                 //[[1]load c0, c2 for next iter         ]
      R13 += SFMPY(R8,R12)                  //[[2]t1 = x1 + c0*t0 + c1*s0           ]
      R17 += SFMPY(R9,R12)                  //[[2]          c2*t0 + c3*s0           ]
    }
    { MEMW(R4+#4) = R12                     //[[2]state[1] = t0                     ]
      R12 = SFADD(R12,R16)                  //[[2]x0 = t0 + c2*s0 + c3*s1           ]
    }
    { MEMW(R4++#8) = R13                    //[[2]state[0] = t1                     ]
      R13 = SFADD(R13,R17)                  //[[2]x1 = t1 + c2*t0 + c3*s0           ]
    }:endloop0

.IIR_filtLpend:
    { R12 += SFMPY(R10,R7)                  //[[2] x0 + c1*s1                       ]
      R16  = SFMPY(R11,R7)                  //[[2]      c3*s1                       ]
      IF P3 MEMD(R5++#8) = R19:18           //[[e]save out[i+1:i]                   ]
    }
    { R13 += SFMPY(R10,R6)                  //[[2] x1 + c1*s0                       ]
      R17  = SFMPY(R11,R6)                  //[[2]      c3*s0                       ]
      R15:14 = COMBINE(R2,R1)               //[ coef = pCoef; state = pState        ]
      P3 = CMP.GT(R3,#-1)                   //[ set P3                              ]
    }
    { R12 += SFMPY(R8,R6)                   //[[2]t0 = x0 + c0*s0 + c1*s1           ]
      R16 += SFMPY(R9,R6)                   //[[2]          c2*s0 + c3*s1           ]
      MEMW(R4+#4) = R12.new                 //[[2]state[1] = t0                     ]
    }
    { R13 += SFMPY(R8,R12)                  //[[2]t1 = x1 + c0*t0 + c1*s0           ]
      R17 += SFMPY(R9,R12)                  //[[2]          c2*t0 + c3*s0           ]
      MEMW(R4+#0) = R13.new                 //[[2]state[0] = t1                     ]
      R4 = R2                               //[ r4 = pState                         ]
    }:endloop1

    { R18 = SFADD(R12,R16)                  //[[2]x0 = t0 + c2*s0 + c3*s1           ]
      R19 = SFADD(R13,R17)                  //[[2]x1 = t1 + c2*t0 + c3*s0           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      IF P2 R3 = ADD(R3,#1)                 //[ restore R3 = nsec                   ]
    }
    { MEMD(R5++#8) = R19:18                 //[ save out[i+1:i]                     ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#2*8)                   //[ pop stack                           ]
      IF !P2 JUMPR R31                      //[ if nsamp is even then return        ]
    }
    /* -------------------------------------*/
    /* Process one sample only              */
    /* -------------------------------------*/
.IIR_filt_1sample:
    { R12 = MEMW(R0+#0)                     //[ load xin[i]                         ]
      R13 = #0                              //[                                     ]
      R6  = #0                              //[                                     ]
      P3 =SP1LOOP0(.IIR_filt_1sampleLoop,R3)//[ setup loop0 : lc0 = nsec            ]
    }

    .falign
.IIR_filt_1sampleLoop:
    { R9:8   = MEMD(R1++#16)                //[[1]load c0, c2                       ]
      R11:10 = MEMD(R1+#8)                  //[[1]load c1, c3                       ]
      R12 += SFMPY(R8,R6)                   //[[2]t = x + + c0*s[0] + c1*s[1]       ]
      R13 += SFMPY(R9,R6)                   //[[2]    c2*s[0] + c3*s[1]             ]
    }
    { R7:6 = MEMD(R2++#8)                   //[[1]load state[0-1]                   ]
      R12 = SFADD(R12,R13)                  //[[2]x = t + c2*s[0] + c3*s[1]         ]
      R15:14 = COMBINE(R6,R12)              //[[2]s[1 = s[0]; s[0] = t              ]
    }
    { R12 += SFMPY(R10,R7)                  //[[1]x + c1*s[1]                       ]
      R13  = SFMPY(R11,R7)                  //[[1]c3*s[1]                           ]
      IF P3 MEMD(R4++#8) = R15:14           //[[2]save state[0-1]                   ]
    }:endloop0

    { R12 += SFMPY(R8,R6)                   //[[2]t = x + + c0*s[0] + c1*s[1]       ]
      R13 += SFMPY(R9,R6)                   //[[2]    c2*s[0] + c3*s[1]             ]
    }
    { R12 = SFADD(R12,R13)                  //[[2]x = t + c2*s[0] + c3*s[1]         ]
      R15:14 = COMBINE(R6,R12)              //[[2]s[1 = s[0]; s[0] = t              ]
      MEMW(R5++#4) = R12.new                //[ save out[i]                         ]
    }
    { MEMD(R4+#0) = R15:14                  //[[2]save state[0-1]                   ]
      JUMPR R31                             //[ return                              ]
    }
    .size    flt_IIR_casBiquad, .-flt_IIR_casBiquad

