/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "rsdec.S"
    /*[*****************************************************************************]*/
    /*[  Function   : int syndrome()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calculate syndrome                                            ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - if RS_T is not a multiple of 2, the extended output syndromes   ]*/
    /*[             array is required                                               ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *received_symbol                                  ]*/
    /*[               R1 : UWord8 *syndrome                                         ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : = 0 if all syndrome are zero , else 1                    ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -  (N*6 -4)*[T/2] +11, where N = K + 2T                           ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[   MZ              05/20/10           Created for V3                         ]*/
    /*[*****************************************************************************]*/


    /*----------------------------------------------------------------------------*/
    /*  Constants                                                                 */
    /*----------------------------------------------------------------------------*/
#define    RS_K         188
#define    RS_T         8
#define    RS_2T        (2*RS_T)
#define    RS_T_M2      ((RS_T+1)&(~1))
#define    RS_T_M4      ((RS_T+3)&(~3))
#define    RS_T_M8      ((RS_T+7)&(~7))
#define    RS_2T_M8     ((RS_2T+7)&(~7))


    .text
    .p2align 2
    .p2align 4,,15
    .globl calculate_syndrome
    .type	calculate_syndrome, @function
calculate_syndrome:
    { R29 = ADD(R29,#-40)                   //[ reserve stack                       ]
      MEMD(R29+#(32-40)) = R17:16           //[ callee-saved registers              ]
      R2.H = #HI(GF_LOGTAB)                 //[ R2 = GF_LOGTAB                      ]
      R3.H = #HI(GF_EXPTAB)                 //[ R3 = GF_EXPTAB                      ]
    }
    { MEMD(R29+#24) = R19:18                //[ callee-saved registers              ]
      R0 = ADD(R0,#(RS_K+RS_2T-3))          //[ &rv_symbol[RS_K+RS_2T-3]            ]
      R2.L = #LO(GF_LOGTAB)                 //[ R2 = GF_LOGTAB                      ]
      R3.L = #LO(GF_EXPTAB)                 //[ R3 = GF_EXPTAB                      ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R7:6 = COMBINE(#2,#1)                 //[ R7:6 = 2 : 1                        ]
      R9:8 = COMBINE(#4,#3)                 //[ R9:8 = 4 : 3                        ]
      R20 = #255                            //[ used for modwrap                    ]
    }
    { MEMD(R29+#8) = R23:22                 //[ callee-saved registers              ]
      R19:18 = VTRUNEWH(R9:8,R7:6)          //[ R19:18 = i = 4|3|2|1                ]
      R22 = #511                            //[ 511 for addNN()                     ]
      R10 = MEMUB(R0+#1)                    //[ rv_symbol[RS_K+RS_2T-2]             ]
    }
    { MEMD(R29+#0) = R25:24                 //[ callee-saved registers              ]
      R10 = ADDASL(R2,R10,#1)               //[&GF_LOGTAB[rv_symbol[RS_K+RS_2T-2]]  ]
      R24 = MEMUB(R0+#2)                    //[ rv_symbol[RS_K+RS_2T-1]             ]
      R21:20 = VSPLATH(R20)                 //[ 255|255|255|255                     ]
    }
    { R25 = MEMUH(R10)                      //[GF_LOGTAB[rv_symbol[RS_K+RS_2T-2]]   ]
      R23:22 = VSPLATH(R22)                 //[ 511|511|511|511                     ]
      R28 = #0                              //[ acc = 0                             ]
      P2 = CMP.GT(R0,R0)                    //[ clear P2                            ]
    }
    { R4 = R0                               //[ &rv_symbol[RS_K+RS_2T-3]            ]
      R9:8 = R19:18                         //[ ak = i                              ]
      LOOP1(.syndrome_outLOOP,#(RS_T_M2/2)) //[ setup outer loop:lc1= (T+1)/2       ]
    }

    .falign
.syndrome_outLOOP:
    { R17:16 = VTRUNEWH(R17:16,R15:14)      //[[O2]acc4|acc3|acc2|acc1              ]
      IF P2 R28 = OR(R28,R10)               //[[O2]OR(acc[i])                       ]
      R10 = OR(R17,R16)                     //[[O2]or(acc3,acc4)                    ]
      R7:6 = VSPLATH(R25)                   //[[1]duplicate                         ]
    }
    { R16 = VTRUNEHB(R17:16)                //[[O2]truncate to 8-bit                ]
      IF P2 R28 = OR(R28,R10)               //[[O2]OR(acc[i]                        ]
      R15:14 = COMBINE(R24,R24)             //[ acc = rv_symbol[RS_K+RS_2T-1] (1,2) ]
      R7:6 = VADDH(R7:6,R9:8)               //[[1]rk + ak                           ]
    }
    { IF P2 MEMW(R1++#4) = R16              //[[O2] save syndrome[i]                ]
      R17:16 = COMBINE(R24,R24)             //[ acc = rv_symbol[RS_K+RS_2T-1] (3,4) ]
      P3 = SP1LOOP0(.syndrome_innerLOOP,#(RS_K+RS_2T-2))
                                            //[ setup loop0: lc0 = 2T+ K -2         ]
      R7:6 = VMINH(R7:6,R23:22)             //[[1]addNN(rk+ak)                      ]
    }

    .falign
.syndrome_innerLOOP:
    { R5 = MEMUB(R4++#-1)                   //[[1]load rv_sample[j],                ]
      R11:10 = VZXTHW(R6)                   //[[2]extend to 32-bit                  ]
      R13:12 = VZXTHW(R7)                   //[[2]extend to 32-bit                  ]
      IF P3 R17 = XOR(R17,R13)              //[[3]                                  ]
    }
    { R5 = ADDASL(R2,R5,#1)                 //[[1]&GF_LOGTAB[rv_symbol[j]]          ]
      R9:8 = VADDH(R9:8,R19:18)             //[[1] ak + i                           ]
      R10 = ADD(R3,R10)                     //[[2]&GF_EXPTAB[addNN(rk, ak)]         ]
      R11 = ADD(R3,R11)                     //[[2]&GF_EXPTAB[addNN(rk, ak)]         ]
    }
    { R5 = MEMUH(R5)                        //[[1]rk = GF_LOGTAB[rv_symbol[j]]      ]
      P0 = VCMPH.GT(R21:20,R9:8)            //[[1]modwrap(ak+i)                     ]
      R7:6 = VSUBH(R9:8,R21:20)             //[[1]modwrap(ak+i)                     ]
      R12 = ADD(R3,R12)                     //[[2]&GF_EXPTAB[addNN(rk, ak)]         ]
    }
    { R9:8 = VMUX(P0,R9:8,R7:6)             //[[1]modwrap(ak+i)                     ]
      R7:6 = VSPLATH(R5)                    //[[1]duplicate                         ]
      R10 = MEMUB(R10)                      //[[2]GF_EXPTAB[addNN(rk, ak)]          ]
      R11 = MEMUB(R11)                      //[[2]GF_EXPTAB[addNN(rk, ak)]          ]
    }
    { R7:6 = VADDH(R7:6,R9:8)               //[[1]rk + ak                           ]
      R12 = MEMUB(R12)                      //[[2]GF_EXPTAB[addNN(rk, ak)]          ]
      R13 = ADD(R3,R13)                     //[[2]&GF_EXPTAB[addNN(rk, ak)]         ]
      R15:14 = XOR(R15:14,R11:10)           //[[2]acc ^=                            ]
    }
    { R7:6 = VMINH(R7:6,R23:22)             //[[1]addNN(rk+ak)                      ]
      R13 = MEMUB(R13)                      //[[2]                                  ]
      R16 = XOR(R16,R12)                    //[[2]acc ^=                            ]
    }:endloop0

    { R11:10 = VZXTHW(R6)                   //[[e]extend to 32-bit                  ]
      R13:12 = VZXTHW(R7)                   //[[e]extend to 32-bit                  ]
      R17 = XOR(R17,R13)                    //[[e]                                  ]
      R8 = #4
    }
    { R10 = ADD(R3,R10)                     //[[e]&GF_EXPTAB[addNN(rk, ak)]         ]
      R11 = ADD(R3,R11)                     //[[e]&GF_EXPTAB[addNN(rk, ak)]         ]
      R12 = ADD(R3,R12)                     //[[e]&GF_EXPTAB[addNN(rk, ak)]         ]
      R13 = ADD(R3,R13)                     //[[e]&GF_EXPTAB[addNN(rk, ak)]         ]
    }
    { R10 = MEMUB(R10)                      //[[e]GF_EXPTAB[addNN(rk, ak)]          ]
      R11 = MEMUB(R11)                      //[[e]                                  ]
      R9:8 = VSPLATH(R8)                    //[ 4|4|4|4                             ]
      P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
    }
    { R12 = MEMUB(R12)                      //[[e]GF_EXPTAB[addNN(rk, ak)]          ]
      R13 = MEMUB(R13)                      //[[e]                                  ]
      R15:14 = XOR(R15:14,R11:10)           //[[e]acc ^=                            ]
      R19:18 = VADDH(R19:18,R9:8)           //[ i                                   ]
    }
    { R17:16 = XOR(R17:16,R13:12)           //[[e]acc ^=                            ]
      R10 = OR(R14,R15)                     //[ or(acc1,acc2)                       ]
      R4 = R0                               //[ &rv_symbol[RS_K+RS_2T-3]            ]
      R9:8 = R19:18                         //[ ak = i                              ]
    }:endloop1

    { R17:16 = VTRUNEWH(R17:16,R15:14)      //[[e]acc4|acc3|acc2|acc1               ]
      R28 = OR(R28,R10)                     //[ OR(acc[i])                          ]
      R10 = OR(R17,R16)                     //[ or(acc3,acc4)                       ]
      R25:24 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { R16 = VTRUNEHB(R17:16)                //[[e]truncate to 8-bit                 ]
      R28 = OR(R28,R10)                     //[OR(acc[i])                           ]
      R23:22 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { MEMW(R1++#4) = R16                    //[[e]save syndrome[i]                  ]
      R19:18 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#40)                    //[ pop stack                           ]
      R0 = ZXTB(R28)                        //[ UWord8                              ]
    }
    { R17:16 = MEMD(R29+#(32-40))           //[ restore callee-saved registers      ]
      P0 = CMP.EQ(R28,#0)                   //[ if (acc[1]|..|acc[2T]) ==0          ]
      IF !P0.new R0 = #1                    //[ if syndrom[i]!=0, return 1          ]
      JUMPR R31                             //[ return                              ]
    }
    .size	calculate_syndrome, .-calculate_syndrome
     


    /*[*****************************************************************************]*/
    /*[  Function   : int belerkamp_massey()                                        ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calculates coefficients of the error polynomial by            ]*/
    /*[               Berlekamp-massey algorithm                                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - 16-bits are used for logLambda[i] due to "0" element            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *syndrome                                        ]*/
    /*[               R1 : UWord16 *logLambda                                       ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : L: number of errors                                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): ((T+7)&(~7)) + ((T+3)&(~3))*4     ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - Worst case: T*(11*T/2 + 6*[T/2] + 3*[T/4]) + 67*T/2 + [T/4] + 1 ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     LAMBDA_ARRAY    0
#define     LOG_B_ARRAY     (RS_T_M8)
#define     LOG_T_ARRAY     (RS_T_M8 + 2*RS_T_M4)

    .text
    .p2align 2
    .p2align 4,,15
    .globl belerkamp_massey
    .type	belerkamp_massey, @function
belerkamp_massey:
    { R29 = ADD(R29,#-(RS_T_M8+4*RS_T_M4))  //[ reserve bytes for lambda,logB,logT  ]
      R12 = #511                            //[ "log(0)"                            ]
      R14.L = #LO(GF_LOGTAB)                //[ R14 = GF_LOGTAB                     ]
      R15.L = #LO(GF_EXPTAB)                //[ R15 = GF_EXPTAB                     ]
    }
    { R2 = ADD(R29,#LOG_B_ARRAY)            //[ logB[]                              ]
      R14.H = #HI(GF_LOGTAB)                //[ R14 = GF_LOGTAB                     ]
      R15.H = #HI(GF_EXPTAB)                //[ R15 = GF_EXPTAB                     ]
      R11:10 = COMBINE(#1,#0)               //[ i=1 : Li =0                         ]
    }
    { R3 = ADD(R29,#LAMBDA_ARRAY)           //[ lambda[]                            ]
      R13:12 = VSPLATH(R12)                 //[ 511|511|511|511                     ]
      R6 = #0                               //[ 2*Li, where Li = 0                  ]
      LOOP0(.bk_initialize_LOOP, #RS_T_M4/4)//[ setup loop0: lc0= ceil(M/4)         ]
    }

    .falign
.bk_initialize_LOOP:
    { MEMW(R3++#4) = R6                     //[ lambda[i] = 0                       ]
      NOP;
    }
    { MEMD(R2++#8) = R13:12                 //[ logB[i] = 511                       ]
      NOP
    }:endloop0 

    { R4 = MEMUB(R0++#1)                    //[ delta = syndrome[0]                 ]
      MEMH(R29+#(LOG_B_ARRAY+0)) = R6       //[ logB[0] = 0                         ]
      R13 = COMBINE(R11.L,R11.L)            //[ 1 | 1                               ]
    }

    .falign
.bk_mainLOOP:
    { P2 = CMP.EQ(R4,#0)                    //[ P2 = (delta==0)                     ]
      P0 = CMP.GT(R11,R6)                   //[ P0 = (i > 2*L)                      ]
      R4 = ADDASL(R14,R4,#1)                //[ &GF_LOGTAB[delta]                   ]
      NOP
    }
    { R8 = MEMUH(R4)                        //[ log(delta)                          ]
      P0 = AND(P0,!P2)                      //[ if (i> 2*L) && (delta!=0)           ]
      IF (P0.new) R10 = SUB(R11,R10)        //[ then L = i-L                        ]
      IF (P0.new) JUMP:t .bk_delta_CASE1    //[ then jump                           ]
    }

    .falign
.bk_delta_CASE0:
    { R2 = ADD(R29,#LOG_T_ARRAY)            //[ logT[]                              ]
      R3 = ADD(R29,#LOG_B_ARRAY)            //[ logB[]                              ]
      P3 = SP1LOOP0(.bk_CopyBtoT_LOOP,#RS_T_M4/4)
      NOP                                   //[ setup loo0: lc0= ceil(M/4)          ]
    }

    .falign
.bk_CopyBtoT_LOOP:
    { R5:4 = MEMD(R3++#8)                   //[ load logT[j]                        ]
      IF P3 MEMD(R2++#8) = R5:4             //[ logT[j] = logB[j]                   ]
    }:endloop0 

    { MEMD(R2++#8) = R5:4                   //[ logT[j] = logB[j]                   ]
      R9 = #511                             //[ log (0)                             ]
      IF !P2 JUMP .bk_updateLambda          //[ if delta!=0 jump                    ]
    }
    { R3 = ADD(R29,#LAMBDA_ARRAY)           //[lambda[]                             ]
      R2 = ADD(R29,#LOG_T_ARRAY)            //[ logT[]                              ]
      JUMP .bk_updateB                      //[ go to update B[]                    ]
    }

.bk_delta_CASE1:
    { R9 = SUB(#255,R8)                     //[ log( 1/delta)                       ]
      R2 = ADD(R29,#LOG_T_ARRAY)            //[ logT[]                              ]
      R3 = ADD(R29,#(LAMBDA_ARRAY+1))       //[ &lambda[1]                          ]
      R4 = MEMUB(R29+#LAMBDA_ARRAY)         //[[1]load lambda[0]                    ]
    }
    { R4 = ADDASL(R14,R4,#1)                //[[1]&GF_LOGTAB[lambda[j]]             ]
      P0 = CMP.EQ(R4,#0)                    //[[1]if (lambda[j] ==0)                ]
      R5 = #511                             //[[1]                                  ]
      LOOP0(.bk_ConstructT_LOOP, #RS_T)     //[ setup loop0: lc0=T                  ]
    }

    .falign
.bk_ConstructT_LOOP:
    { IF  P0 JUMP .bk_ConstructT_CONT       //[[2]then set logT[j] = 511            ]
      IF !P0 R5 = MEMUH(R4)                 //[[2]log( lambda[j] )                  ]
      R4 = MEMUB(R3++#1)                    //[[1]load lambda[j]                    ]
      NOP
    }
    { P0 = CMP.GT(R8,R5)                    //[[2]log (lambda[j] GF/ delta )        ]
      IF  P0.new R5 = ADD(R5,R9)            //[[2]                                  ]
      IF !P0.new R5 = SUB(R5,R8)            //[[2]                                  ]
      NOP
    }
.bk_ConstructT_CONT:
    { MEMH(R2++#2) = R5                     //[[2] save logT[j]                     ]
      R4 = ADDASL(R14,R4,#1)                //[[1]&GF_LOGTAB[lambda[j]]             ]
      P0 = CMP.EQ(R4,#0)                    //[[1]if (lambda[j] ==0)                ]
      R5 = #511                             //[[1]                                  ]
    }:endloop0 

    .falign
.bk_updateLambda:
    { R8 = COMBINE(R8.L,R8.L)               //[ delta|delta                         ]
      R2 = ADD(R29,#(LOG_B_ARRAY+4))        //[&logB[2]                             ]
      R6 = MEMW(R29+#(LOG_B_ARRAY+0))       //[[1]load logB[0], logB[1]             ]
    }
    { R6 = VADDH(R6,R8)                     //[[1]addNN( log(delta), logB[j] )      ]
    }
    { R3 = ADD(R29,#LAMBDA_ARRAY)           //[ lambda[]                            ]
      P3 =SP1LOOP0(.bk_updateLambda_LOOP,#RS_T_M2/2)
                                            //[ setup loop0: lc0 = ceil(T/2)        ]
      R7:6 = VMINH(R7:6,R13:12)             //[[1]addNN( log(delta), logB[j] )      ]
      R5:4 = COMBINE(R15,R15)               //[[1]GF_EXPTAB                         ]
    }

    .falign
.bk_updateLambda_LOOP:
    { R6 = MEMW(R2++#4)                     //[[1]load logB[j], logB[j+1]           ]
      R5:4 += VMPYH(R6,R13)                 //[[2]&GF_EXPTAB[ log( delta * B[j] ]   ]
      IF P3 R7 = MEMH(R3)                   //[[3]lambda[j]                         ]
      NOP
    }
    { R6 = VADDH(R6,R8)                     //[[1]addNN( log(delta), logB[j] )      ]
      R28 = MEMUB(R4)                       //[[2]delta*B[j]                        ]
      R5 = MEMUB(R5)                        //[[2]delta*B[j+1]                      ]
      IF P3 R7 = XOR(R7,R28)                //[[3]lambda[j]+(delta*B[j])            ]
    }
    { R7:6 = VMINH(R7:6,R13:12)             //[[1]R6:addNN( log(delta), logB[j] )   ]
                                            //[   R7 not used                       ]
      R5:4 = COMBINE(R15,R15)               //[[1]GF_EXPTAB                         ]
      R28 |= ASL(R5,#8)                     //[[2]pack                              ]
      IF P3 MEMH(R3++#2) = R7               //[[3]save                              ]
    }:endloop0

    { R7 = MEMH(R3)                         //[[3]lambda[j]                         ]
      R2 = ADD(R29,#LOG_T_ARRAY)            //[ logT[]                              ]
    }

    .falign
.bk_updateB:
    { IF !P2 R7 = XOR(R7,R28)               //[[3]lambda[j]+(delta*B[j])            ]
      R9 = ASLH(R9)                         //[                                     ]
      P1 = CMP.EQ(R11,#RS_2T)               //[ P1 = (i == RS_2T) ?                 ]
      P3 = SP1LOOP0(.bk_updateB_LOOP,#RS_T_M4/4)
                                            //[ setup loop0: lc0 = ceil(M/4)        ]
    }
    { IF !P2 MEMH(R3) = R7                  //[[3]save lambda[j]                    ]
      IF P1 JUMP .bk_mainLPEND              //[ IF P1 jump                          ]
      R3 = ADD(R29,#LOG_B_ARRAY)            //[ logB[]                              ]
      R5:4 = MEMD(R2++#8)                   //[[1]load logT[i+0-4]                  ]
    }

    .falign
.bk_updateB_LOOP:
    { R5:4 = MEMD(R2++#8)                   //[[1]load logT[i+0-4]                  ]
      R9:8 = R5:4                           //[[2]                                  ]
      R7:6 = VALIGNB(R5:4,R9:8,#6)          //[[2]align logT[] for logB[]           ]
      IF P3 MEMD(R3++#8) = R7:6             //[[3]save logB[j]                      ]
    }:endloop0

    { MEMD(R3++#8) = R7:6                   //[[3]save logB[j]                      ]
      R4 = MEMUB(R0++#1)                    //[ delta = syndrome[i]                 ]
      R9 = #0                               //[                                     ]
      IF (R10<=#0) JUMP:nt .bk_calculate_deltaEND
                                            //[ if Li <=0 skip the loop             ]
    }
    { R2 = ADD(R29,#LAMBDA_ARRAY)           //[ lambda[]                            ]
      R3 = ADD(R0,#-2)                      //[ &syndrome[i-j]                      ]
      R9 = ADD(R10,#-1)                     //[ Li-1                                ]
      NOP
    }
    { R28 = ADD(R15,#511)                   //[ &GF_EXPTAB[511]                     ]
      P3=SP1LOOP0(.bk_calculate_deltaLOOP,R9)
                                            //[ setup loop0: lc0 = Li -1            ]
      R5 = MEMUB(R2++#1)                    //[[1]loda lambda[0]                    ]
      R6 = MEMUB(R3++#-1)                   //[[2]load syndrome[i-1]                ]
    }
    { R5 = MEMUB(R2++#1)                    //[[1]loda lambda[1]                    ]
      R6 = ADDASL(R14,R6,#1)                //[[2]&GF_LOGTAB[syndrome[i-1]]         ]
      R7 = ADDASL(R14,R5,#1)                //[[2]&GF_LOGTAB[lambda[0]]             ]
      P0 = CMP.GT(R9,#0)                    //[ P0 = Li-1 > 0                       ]
    }
    { R6 = MEMUH(R6)                        //[[2]log( syndrome[i-j] )              ]
      R8 = MEMUH(R7)                        //[[2]log( lambda[0] )                  ]
      IF !P0 JUMP .bk_calculate_delta_SKIP  //[ if Li-1 <=0 skip loop               ]
    }

    .falign
.bk_calculate_deltaLOOP:
    { R6 = MEMUB(R3++#-1)                   //[[2]load syndrome[i-j]                ]
      R7 = ADDASL(R14,R5,#1)                //[[2]&GF_LOGTAB[lambda[j-1]]           ]
      R8 += ADD(R15,R6)                     //[[3] GF_EXPTAB + log(lambda[j-1])+    ]
                                            //[                log(syndrome[i-j])   ]
      IF P3 R4 = XOR(R4,R9)                 //[[4] delta +=                         ]
                                            //[       lambda[j-1] GF* syndrome[i-j] ]
    }
    { R5 = MEMUB(R2++#1)                    //[[1]loda lambda[j-1]                  ]
      R6 = ADDASL(R14,R6,#1)                //[[2]&GF_LOGTAB[syndrome[i-j]]         ]
      R8 = MEMUH(R7)                        //[[2]log( lambda[j-1] )                ]
      R9 = MINU(R8,R28)                     //[[3]                                  ]
    }
    { R6 = MEMUH(R6)                        //[[2]log( syndrome[i-j] )              ]
      R9 = MEMUB(R9)                        //[[3] lambda[j-1] GF* syndrome[i-j]    ]
      NOP; NOP
    }:endloop0

.bk_calculate_delta_SKIP:
    { R8 += ADD(R15,R6)                     //[[3] GF_EXPTAB + log(lambda[j-1]) +   ]
                                            //[                log(syndrome[i-j])   ]
      IF P3 R4 = XOR(R4,R9)                 //[[4] delta +=                         ]
                                            //[       lambda[j-1] GF* syndrome[i-j] ]
    }
    { R9 = MINU(R8,R28)                     //[[3]                                  ]
    }
    { R9 = MEMUB(R9)                        //[[3] lambda[j-1] GF* syndrome[i-j]    ]
    }
.bk_calculate_deltaEND:
    { R4 = XOR(R4,R9)                       //[[4] delta +=                         ]
                                            //[       lambda[j-1] GF* syndrome[i-j] ]
      R11 = add(R11,#1)                     //[ i++                                 ]
      R6 = ADD(R10,R10)                     //[ 2*Li                                ]
      JUMP .bk_mainLOOP                     //[ continue main loop                  ]
    }

    .falign
.bk_mainLPEND:
    { R2 = ADD(R29,#(LAMBDA_ARRAY+1))       //[ R2 = &lambda[1]                     ]
      R3 = MEMUB(R29+#(LAMBDA_ARRAY+0))     //[[1]load lambda[0]                    ]
    }
    { R3 = ADDASL(R14,R3,#1)                //[[1]&GF_LOGTAB[lambda[i]]             ]
      LOOP0(.bk_logLambda_LOOP,#RS_T)       //[ setup loop0: lc0 = T                ]
    }
    .falign
.bk_logLambda_LOOP:
    { R3 = MEMUB(R2++#1)                    //[[1]load lambda[i]                    ]
      R4 = MEMH(R3)                         //[[2]log(lambda[i])                    ]
    }
    { R3 = ADDASL(R14,R3,#1)                //[[1]&GF_LOGTAB[lambda[i]]             ]
      MEMH(R1++#2) = R4                     //[[2]save logLambda[i]                 ]
    }:endloop0 

    { R0 = R10                              //[ return value = L                    ]
      R29 = ADD(R29,#(RS_T_M8+4*RS_T_M4))   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	belerkamp_massey, .-belerkamp_massey


    /*[*****************************************************************************]*/
    /*[  Function   : int chien_search()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Finds roots of the error polynomial by Chine search algorithm ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord16 *logLambda                                       ]*/
    /*[               R1 : UWord8  *roots                                           ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : number of roots found                                    ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): (2*T+7)&(~7)                      ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -  2*T + (3*Nc + 5)*[(Nr+1)/2] + 7                                ]*/
    /*[              where Nc: # of non-zero coefficients                           ]*/
    /*[                    Nr: maximum (log(root[i]))                               ]*/
    /*[              Therefore, the worse case is Nc = T, Nr = 254                  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl chien_search
    .type	chien_search, @function
chien_search:
    { R29 = ADD(R29,#-RS_2T_M8)             //[ reserve stack                       ]
      P1 = CMP.EQ(R0,R0)                    //[ set P1                              ]
      R3 = #0                               //[ i                                   ]
      R13.H = #HI(GF_EXPTAB)                //[ R13 = GF_EXPTAB                     ]
    }
    { R2 = R29                              //[ R2: jk[] and logGama[]              ]
      R13.L = #LO(GF_EXPTAB)                //[ R13 = GF_EXPTAB                     ]
      R12 = #-2                             //[                                     ]
      LOOP0(.chiensrch_find_nzcoefsLOOP,#RS_T)
                                            //[ setup loop0: lc0 = T                ]
    }

    .falign
.chiensrch_find_nzcoefsLOOP:
    { R4 = MEMUH(R0++#2)                    //[[1]load logLambda[i-1]               ]
      IF !P1 MEMB(R2+#1) = R4               //[[2]logGmama[num_coefs]=logLambda[i-1]]
    }
    { P1 = CMP.EQ(R4,#511)                  //[[1]lambda[i-1]== 0?                  ]
      R3 = ADD(R3,#1)                       //[[1]i++                               ]
      IF !P1 MEMB(R2++#2) = R3              //[[2]jk[num_coefs] = i                 ]
    }:endloop0

    { IF !P1 MEMB(R2+#1) = R4               //[[2]logGmama[num_coefs]=logLambda[i-1]]
      R12 += SUB(R2,R29)                    //[                                     ]
      R14 = #255                            //[                                     ]
    }
    { IF !P1 MEMB(R2++#2) = R3              //[[2]jk[num_coefs] = i                 ]
      IF !P1 R12 = ADD(R12,#2)              //[ (2*num_coefs -2)                    ]
      R28 = #0                              //[ i = 0                               ]
      R0 = #0                               //[ num_roots = 0                       ]
    }
    { R15 = MEMB(R2+#-2)                    //[ R3 = jk[num_coefs-1], i.e.,         ]
                                            //[                degree of polynomial ]
      P2 = CMP.GT(R12,#0)                   //[ P2 = 2*(num_coefs-1) >0             ]
      R12 = LSR(R12,#1)                     //[ num_coefs-1                         ]
      LOOP1(.chien_search_outLOOP,#256/2)   //[ setup loop0:lc0 = 256/2 (2elem/loop)]
    }
    { R11:10 = COMBINE(#1, #1)              //[[p]acc= 1                            ]
      R4 = MEMUB(R29+#0)                    //[[p]jk[0]                             ]
      R5 = MEMUB(R29+#1)                    //[[p]logGama[0]                        ]
    }

    .falign
.chien_search_outLOOP:
    { R2 = ADD(R29,#2)                      //[ R2: read ptr for logGama[], jk[]    ]
      LOOP0(.chien_search_innerLOOP,R12)    //[ setup inner loop:lc0= num_coefs     ]
      R7 = ADD(R13,R5)                      //[[1]&GF_EXPTAB[logGama[k]]            ]
      R5 = ADD(R5,R4)                       //[[1]modwrap(logGama[k]+jk[k],255)     ]
    }
    { R3 = ADD(R29,#1)                      //[ R3: write ptr for logGama[]         ]
      R6 = ADD(R5,R4)                       //[[1]logGama[k]+jk[k]                  ]
      R8 = MEMUB(R7)                        //[[1]GF_EXPTAB[logGama[k]]             ]
      IF !P2 JUMP .chien_srch_innerLPEND    //[ if !P2 skip loop                    ]
    }

    .falign
.chien_search_innerLOOP:
    { R4 = MEMUB(R2++#2)                    //[[1]jk[k]                             ]
      R5 = MEMUB(R2+#1)                     //[[1]logGama[k]                        ]
      R7 = ADD(R13,R5)                      //[[2]&GF_EXPTAB[logGama[k]]            ]
      P0 = CMP.GT(R14,R6)                   //[[2]modwrap(*, 255)                   ]
    }
    { R7 = ADD(R13,R5)                      //[[1]&GF_EXPTAB[logGama[k]]            ]
      R5 = ADD(R5,R4)                       //[[1]modwrap(logGama[k]+jk[k],255)     ]
      R9 = MEMUB(R7)                        //[[2]GF_EXPTAB[logGama[k]]             ]
      IF !P0 R6 = SUB(R6,R14)               //[[1]modwrap(*,255)                    ]
    }
    { R6 = ADD(R5,R4)                       //[[1]logGama[k]+jk[k]                  ]
      R8 = MEMUB(R7)                        //[[1]GF_EXPTAB[logGama[k]]             ]
      R11:10 = XOR(R11:10, R9:8)            //[[2]                                  ]
      MEMB(R3++#2) = R6                     //[[2]save logGama[k]                   ]
    }:endloop0

.chien_srch_innerLPEND:
    { R7 = ADD(R13,R5)                      //[[2]&GF_EXPTAB[logGama[k]]            ]
      P0 = CMP.GT(R14,R6)                   //[[2]modwrap(*, 255)                   ]
      IF (!P0.new) R6 = SUB(R6,R14)         //[[2]modwrap(*,255)                    ]
      P3 = CMP.EQ(R28,#254)                 //[ i == 254                            ]
    }
    { MEMB(R3++#2) = R6                     //[[2]save logGama[k]                   ]
      R9 = MEMUB(R7)                        //[[2]GF_EXPTAB[logGama[k]]             ]
      P0 = CMP.EQ(R10,R8)                   //[ P0 = (acc1 == 0)                    ]
      IF (P0.new) R0 = ADD(R0,#1)           //[ if P0 num_roots++                   ]
    }
    { IF P0 MEMB(R1++#1) = R28              //[ if P0 roots[k] = i                  ]
      P1 = CMP.EQ(R0,R15)                   //[ if (num_roots == degree)            ]
      IF P1.new JUMP:nt  .chien_search_DONE //[ then search done                    ]
    }
    { P0 = CMP.EQ(R11,R9)                   //[ P0 = (acc2 == 0)                    ]
      IF (P0.new) R0 = ADD(R0,#1)           //[ if P0 num_roots++                   ]
      R28 = ADD(R28,#1)                     //[ i++                                 ]
      IF P3 JUMP .chien_search_DONE         //[ if (i==254) done                    ]
    }
    { IF P0 MEMB(R1++#1) = R28              //[ if P0 roots[k] = i                  ]
      P1 = CMP.EQ(R0,R15)                   //[ if num_roots == degree              ]
      IF P1.new JUMP:nt  .chien_search_DONE //[ then search done                    ]
      R28 = ADD(R28,#1)                     //[ i++                                 ]
    }
    { R11:10 = COMBINE(#1, #1)              //[[O1]acc= 1                           ]
      R4 = MEMUB(R29+#0)                    //[[O1]jk[0]                            ]
      R5 = MEMUB(R29+#1)                    //[[O1]logGama[0]                       ]
    }:endloop1

.chien_search_DONE:
    { R29 = ADD(R29,#RS_2T_M8)              //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	chien_search, .-chien_search



    /*[*****************************************************************************]*/
    /*[  Function   : int forney()                                                  ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calculates error magnitudes by forney's algorithm             ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord16 *logLambda                                       ]*/
    /*[               R1 : UWord8  *roots                                           ]*/
    /*[               R2 : Word32  nerros        - number of errors                 ]*/
    /*[               R3 : UWord8  *error_values                                    ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : 1 if any denominator = 0, else 0 (sucess)                ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32 + 4*((T+1)&(~1))               ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 3*[T/2]^2 + 11*[T/2] +                                          ]*/
    /*[             (4*deg_omega + 4*[nerrors/2] + 15 )*[nerrors/2] + 8             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl forney
    .type	forney, @function
forney:
    { MEMD(R29+#-8) = R23:22                //[ callee-saved registers              ]
      R29 = ADD(R29,#-(4*RS_T_M2 + 4*8))    //[ reserve stack                       ]
      R15.H = #HI(GF_EXPTAB)                //[ R15 = GF_EXPTAB                     ]
      R14.H = #HI(GF_LOGTAB)                //[ R14 = GF_LOGTAB                     ]
    }
    { MEMD(R29+#(4*RS_T_M2+16)) = R21:20    //[ callee-saved registers              ]
      R15.L = #LO(GF_EXPTAB)                //[ R15 = GF_EXPTAB                     ]
      R14.L = #LO(GF_LOGTAB)                //[ R14 = GF_LOGTAB                     ]
      R6 = R0                               //[ s[]                                 ]
    }
    { MEMD(R29+#(4*RS_T_M2+8)) = R19:18     //[ callee-saved registers              ]
      R7 = ADD(R29,#0)                      //[ logS[]                              ]
      R8 = MEMUB(R6++#1)                    //[[1]load s[i]                         ]
      R20 = #511                            //[                                     ]
    }
    { MEMD(R29+#(4*RS_T_M2+0)) = R17:16     //[ callee-saved registers              ]
      R21 = #511                            //[ R21:20 = 511 : 511                  ]
      R8 = ADDASL(R14,R8,#1)                //[[1]&GF_LOGTAB[s[i]]                  ]
      LOOP0(.forney_logSyndrome_LOOP,#RS_T_M2-1)
                                            //[ setup loop0: lc0 = (T+(T&1))-1      ]
    }

    .falign
.forney_logSyndrome_LOOP:
    { R8 = MEMUB(R6++#1)                    //[[1]load s[i]                         ]
      R9 = MEMH(R8)                         //[[2]GF_LOGTAB[s[i]]                   ]
    }
    { R8 = ADDASL(R14,R8,#1)                //[[1]&GF_LOGTAB[s[i]]                  ]
      MEMH(R7++#2) = R9                     //[[2]logS[i] = GF_LOGTAB[s[i]]         ]
    }:endloop0

    { R9 = MEMH(R8)                         //[[2]GF_LOGTAB[s[i]]                   ]
      R6 = ADD(R29,#0)                      //[ logS[]                              ]
      P3 = CMP.GT(R0,R0)                    //[ clear P3                            ]
      NOP
    }
    { MEMH(R7) = R9                         //[[2]logS[i] = GF_LOGTAB[s[i]]         ]
      R7 = ADD(R29,#2*RS_T_M2)              //[ logOmega[]                          ]
      R23:22 = COMBINE(#0,#1)               //[ deg_omega=0 : i=1                   ]
      LOOP1(.forney_OmegaLOOP,#RS_T_M2/2)   //[ setup loop1: lc1 = ceil(T/2)        ]
    }

    .falign
.forney_OmegaLOOP:
    { R10 = MEMUB(R0++#2)                   //[ acc1 = s[i+0]                       ]
      R11 = MEMUB(R0+#1)                    //[ acc2 = s[i+1]                       ]
      R9:8 = COMBINE(R1,R6)                 //[ logLambda[]: &logS[i]               ]
      R13 = ADD(R22,#-2)                    //[                                     ]
    }
    { R18 = MEMUH(R8++#-2)                  //[                                     ]
      R28 = MEMUH(R9++#2)                   //[                                     ]
      R17:16 = #0                           //[                                     ]
      IF !P3 JUMP .forney_Omega_innerLPEND  //[                                     ]
    }
    { R16 = ADDASL(R14,R12,#1)              //[[O2]                                 ]
      R19 = R18                             //[[1]                                  ]
      R18 = MEMUH(R8++#-2)                  //[[1]                                  ]
      LOOP0(.forney_Omega_innerLOOP,R13)
    }
    { R16 = MEMUH(R16)                      //[[O2]                                 ]
      R12 = ADD(R18,R28)                    //[[1]                                  ]
      R13 = ADD(R19,R28)                    //[[1]                                  ]
      NOP
    }
    { MEMH(R7++#2) = R16                    //[[O2]                                 ]
      R13:12 = VMINW(R13:12,R21:20)         //[[1]                                  ]
      R17:16 = COMBINE(R15,R15)             //[[1]                                  ]
      R28 = MEMUH(R9++#2)                   //[[1]                                  ]
    }

    .falign
.forney_Omega_innerLOOP:
    { R19 = R18                             //[[1]                                  ]
      R18 = MEMUH(R8++#-2)                  //[[1]                                  ]
      R17:16 = VADDW(R17:16,R13:12)         //[[2]                                  ]
      NOP
    }
    { R12 = ADD(R18, R28)                   //[[1]                                  ]
      R13 = ADD(R19, R28)                   //[[1]                                  ]
      R16 = MEMUB(R16)                      //[[2]                                  ]
      R17 = MEMUB(R17)                      //[[2]                                  ]
    }
    { R13:12 = VMINW(R13:12,R21:20)         //[[1]                                  ]
      R17:16 = COMBINE(R15,R15)             //[[1]                                  ]
      R28 = MEMUH(R9++#2)                   //[[1]                                  ]
      R11:10 = XOR(R11:10,R17:16)           //[[2]                                  ]
    }:endloop0

    { R17:16 = VADDW(R17:16,R13:12)         //[[2]                                  ]
    }
    { R16 = MEMUB(R16)                      //[[2]                                  ]
      R17 = MEMUB(R17)                      //[[2]                                  ]
    }
    .falign
.forney_Omega_innerLPEND:
    { R11:10 = XOR(R11:10,R17:16)           //[[2]                                  ]
      R13 = ADD(R18, R28)                   //[                                     ]
      R16 = ADD(R15,R28)                    //[&GF_EXPTAB[logLambda[i-1]]           ]
      R28 = MEMUH(R9++#2)                   //[                                     ]
    }
    { R13 = MIN(R13,R21)                    //[                                     ]
      R16 = MEMUB(R16)                      //[                                     ]
      R12 = ADD(R15,R28)                    //[                                     ]
      R6 = ADD(R6,#4)                       //[ update logS[i-2]                    ]
    }
    { R17 = ADD(R15,R13)                    //[                                     ]
      R10 = XOR(R10,R16)                    //[acc1                                 ]
      R12 = MEMUB(R12)                      //[                                     ]
      P3 = CMP.EQ(R0,R0)                    //[set P3                               ]
    }
    { R17 = MEMUB(R17)                      //[                                     ]
      R11 = XOR(R11,R12)                    //[                                     ]
      P0 = CMP.EQ(R10,#0)                   //[ acc1 == 0 ?                         ]
      R10 = ADDASL(R14,R10,#1)              //[ GF_LOGTAB[acc1]                     ]
    }
    { R12 = XOR(R11,R17)                    //[acc2                                 ]
      R10 = MEMUH(R10)                      //[ GF_LOGTAB[acc1]                     ]
      IF !P0 R23 = R22                      //[ deg_omega = i                       ]
      R22 = ADD(R22,#1)                     //[ i++                                 ]
    }
    { P0 = CMP.EQ(R12,#0)                   //[ if(acc2==0)                         ]
      IF !P0.new R23 = R22                  //[ then deg_omega = i                  ]
      MEMH(R7++#2) = R10                    //[ save logOmega[i]                    ]
      R22 = ADD(R22,#1)                     //[ i++                                 ]
    }:endloop1

    { R16 = ADDASL(R14,R12,#1)              //[[e]                                  ]
      R6 = ADD(R29,#(2*RS_T_M2+2*RS_T-2))   //[ logOmega[RS_T-1]                    ]
      R28 = #255                            //[                                     ]
      R23 = ADD(R23,#-1)                    //[ deg_omega -1                        ]
    }
    /*-----------------------------------------------------------------------------*/
    /*      In case RS_T is odd, fill logOmega[RS_T] with 511, i.e omega=0         */
    /*-----------------------------------------------------------------------------*/
    { P0 = CMP.EQ(R6,R7)                    //[ if (RS_T != RS_T_M2)                ]
      IF (!P0.new) R16 = R20                //[ then set omega[RS_T_M2-1] =0        ]
      IF ( P0.new) R16 = MEMUH(R16)         //[                                     ]
      R22 = ASR(R3,#0):rnd                  //[ (ndegree+1)/2                       ]
    }
    { MEMH(R7) = R16                        //[[e]                                  ]
      P2 = CMP.GT(R23,#0)                   //[ set P2 = (deg_omega-1 > 0)          ]
      R22 = ADD(R22,#-1)                    //[ R22= (ndegree+1)/2 -1               ]
      NOP
    }

    .falign
.forney_ComputeErrors_LOOP:
    { R6 = MEMUB(R2++#2)                    //[ root[i  ]                           ]
      R7 = MEMUB(R2+#1)                     //[ root[i+1]                           ]
      R11:10 = COMBINE(#1,#1)               //[ nominator = 1                       ]
      R0 = ADD(R29,#16)                     //[ logOmega[]                          ]
    }
    { R18 = MEMUH(R0++#2)                   //[[1]load logOmega[j]                  ]
      R17:16 = #0                           //[                                     ]
      P0 = CMP.GT(R23,#-1)                  //[ if (deg_omega-1 < 0)                ]
      IF !P0.new JUMP:nt .forney_nominator_Done 
                                            //[ then nominator computation done     ]
    }
    { R18 = ADD(R18,R6)                     //[[1]addNN(logOmega[j], xk)            ]
      R19 = ADD(R18,R7)                     //[[1]                                  ]
      R9:8 = VADDW(R7:6, R7:6)              //[[1]xk + x                            ]
      NOP
    }
    { LOOP0(.forney_nominator_LOOP,R23)     //[ setup loop0: lc0 = deg_omega-1      ]
      R19:18 = VMINW(R19:18,R21:20)         //[[1]addNN(logOmega[j], xk)            ]
      P0 = CMP.GT(R28,R8)                   //[[1]modwrap(*, 255)                   ]
      IF !P0.new R8 = SUB(R8,R28)           //[[1]                                  ]
    }
    { R16 = ADD(R15,R18)                    //[[1]&GF_EXPTAB[addNN()]               ]
      R17 = ADD(R15,R19)                    //[[1]                                  ]
      IF !P2 JUMP .forney_nominator_LPEND   //[ if !(deg_omega-1 >0) skip the loop  ]
      NOP
    }

    .falign
.forney_nominator_LOOP:
    { R18 = MEMUH(R0++#2)                   //[[1]load logOmega[j]                  ]
      P0 = CMP.GT(R28,R9)                   //[[2]modwrap(*, 255)                   ]
      IF !P0.new R9 = SUB(R9,R28)           //[[2]                                  ]
      NOP
    }
    { R18 = ADD(R18,R8)                     //[[1]addNN(logOmega[j], xk)            ]
      R19 = ADD(R18,R9)                     //[[1]                                  ]
      R16 = MEMUB(R16)                      //[[2]Omega[i] GF* (alph^xk)            ]
      R17 = MEMUB(R17)                      //[[2]                                  ]
    }
    { R19:18 = VMINW(R19:18,R21:20)         //[[1]addNN(logOmega[j], xk)            ]
      R9:8 = VADDW(R9:8, R7:6)              //[[1]xk + x                            ]
      R11 = XOR(R11,R17)                    //[[2]nominator ^=                      ]
      R10 = XOR(R10,R16)                    //[[2]                                  ]
    }
    { R16 = ADD(R15,R18)                    //[[1]&GF_EXPTAB[addNN()]               ]
      R17 = ADD(R15,R19)                    //[[1]                                  ]
      P0 = CMP.GT(R28,R8)                   //[[1]modwrap(*, 255)                   ]
      IF !P0.new R8 = SUB(R8,R28)           //[[1]                                  ]
    }:endloop0

.forney_nominator_LPEND:
    { R16 = MEMUB(R16)                      //[[e]Omega[i] GF* (alph^xk)            ]
      R17 = MEMUB(R17)                      //[[e]                                  ]
      NOP; NOP
    }
.forney_nominator_Done:
    { R11:10 = XOR(R11:10,R17:16)           //[[e]nominator ^=                      ]
      R9:8 = R7:6                           //[ xk = x                              ]
      R7:6 = VADDW(R7:6, R7:6)              //[ x + x                               ]
      R18 = MEMUH(R1)                       //[[1]load logLambda[0]                 ]
    }
    { P0 = CMP.GT(R28,R6)                   //[ x2 = modwrap(x+x, 255)              ]
      P1 = CMP.GT(R28,R7)                   //[                                     ]
      IF !P0.new R6 = SUB(R6,R28)           //[                                     ]
      IF !P1.new R7 = SUB(R7,R28)           //[                                     ]
    }
    { R0 = ADD(R1,#4)                       //[ logLambda[2]                        ]
      R18 = ADD(R18,R8)                     //[[1]addNN(logLambda[j], xk)           ]
      R19 = ADD(R18,R9)                     //[[1]                                  ]
      R9:8 = VADDW(R9:8, R7:6)              //[[1]xk + x2                           ]
    }
    { R19:18 = VMINW(R19:18,R21:20)         //[[1]addNN(logLambda[j], xk)           ]
      LOOP0(.forney_denominator_LOOP,R22)   //[ setup loop0: lc0=(ndegree+1)/2 -1   ] 
      P0 = CMP.GT(R28,R8)                   //[[1]modwrap(*, 255)                   ]
      IF !P0.new R8 = SUB(R8,R28)           //[[1]                                  ]
    }
    { R13:12 = #0                           //[ denominator = 0                     ]
      IF (R22<=#0) JUMP:nt .forney_denominator_LPEND 
                                            //[ IF (ndegree+1)/2-1 < =0 skip loop   ]
      R16 = ADD(R15,R18)                    //[[1]&GF_EXPTAB[addNN()]               ]
      R17 = ADD(R15,R19)                    //[[1]
    }

    .falign
.forney_denominator_LOOP:
    { R18 = MEMUH(R0++#4)                   //[[1]load logLambda[j]                 ]
      P0 = CMP.GT(R28,R9)                   //[[2]modwrap(*, 255)                   ]
      IF !P0.new R9 = SUB(R9,R28)           //[[2]                                  ]
      NOP
    }
    { R18 = ADD(R18,R8)                     //[[1]addNN(logLambda[j], xk)           ]
      R19 = ADD(R18,R9)                     //[[1]                                  ]
      R16 = MEMUB(R16)                      //[[2]Omega[i] GF* (alph^xk)            ]
      R17 = MEMUB(R17)                      //[[2]                                  ]
    }
    { R19:18 = VMINW(R19:18,R21:20)         //[[1]addNN(logLambda[j], xk)           ]
      R9:8 = VADDW(R9:8, R7:6)              //[[1]xk + x2                           ]
      R12 = XOR(R12,R16)                    //[[2]                                  ]
      R13 = XOR(R13,R17)                    //[[2]denominator ^=                    ]
    }
    { R16 = ADD(R15,R18)                    //[[1]&GF_EXPTAB[addNN()]               ]
      R17 = ADD(R15,R19)                    //[[1]                                  ]
      P0 = CMP.GT(R28,R8)                   //[[1]modwrap(*, 255)                   ]
      IF !P0.new R8 = SUB(R8,R28)           //[[1]                                  ]
    }:endloop0

.forney_denominator_LPEND:
    { R16 = MEMUB(R16)                      //[[e]Omega[i] GF* (alph^xk)            ]
      R17 = MEMUB(R17)                      //[[e]                                  ]
    }
.forney_denominator_Done:
    { R12 = XOR(R12,R16)                    //[[e]denominator ^=                    ]
      R13 = XOR(R13,R17)                    //[[e]denominator ^=                    ]
      R10 = ADDASL(R14,R10,#1)              //[ &GF_LOGTAB[nominator]               ]
      R11 = ADDASL(R14,R11,#1)              //[                                     ]
    }
    { R10 = MEMUH(R10)                      //[ log(nominator)                      ]
      R11 = MEMUH(R11)                      //[ log(nominator)                      ]
      R16 = ADDASL(R14,R12,#1)              //[ &GF_LOGTAB[denominator]             ]
      R17 = ADDASL(R14,R13,#1)              //[                                     ]
    }
    { R16 = MEMUH(R16)                      //[ log(nominator)                      ]
      R17 = MEMUH(R17)                      //[                                     ]
      P1 = CMP.EQ(R3,#1)                    //[ if ndegree is odd number, then      ]
                                            //[ last the output should be ignored   ]
    }
    { R10 +=SUB(R28,R16)                    //[ log( nominator/denominator)         ]
      R11 +=SUB(R28,R17)                    //[                                     ]
      IF !P1 R12 = OR(R12,R13)              //[                                     ]
    }
    { R11:10 = VMINW(R11:10,R21:20)         //[ addNN()                             ]
      P0 = CMP.EQ(R12,#0)                   //[ if (denominator ==0)                ]
      IF P0.new R0 = #1                     //[ return 1                            ]
      IF P0.new JUMP:nt .forney_DONE        //[                                     ]
    }
    { R10 = ADD(R15,R10)                    //[&GF_EXPTA[*]                         ]
      R11 = ADD(R15,R11)                    //[                                     ]
    }
    { R10 = MEMUB(R10)                      //[                                     ]
      IF !P1 R11 = MEMUB(R11)               //[                                     ]
      R3 = ADD(R3,#-2)                      //[                                     ]
    } 
    { MEMB(R4++#1) = R10                    //[                                     ]
      P0 = CMP.GT(R3,#0)                    //[                                     ]
    }
    { IF !P1 MEMB(R4++#1) = R11             //[                                     ]
      IF P0 JUMP .forney_ComputeErrors_LOOP //[                                     ]
    }
    { R0 = #0                               //[ return 0                            ]
    }
.forney_DONE:
    { R17:16 = MEMD(R29+#(4*RS_T_M2+0))     //[ restore callee-saved registers      ]
      R19:18 = MEMD(R29+#(4*RS_T_M2+8))     //[ restore callee-saved registers      ]
    }
    { R21:20 = MEMD(R29+#(4*RS_T_M2+16))    //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#(4*RS_T_M2+24))    //[ restore callee-saved registers      ]
      R29 = ADD(R29,#(4*RS_T_M2 + 4*8))     //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	forney, .-forney

