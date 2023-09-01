/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "sfht.S"

#define    CXADD(x, y)      VAVGH(x,y):crnd            // if non-scaled, VADDH(x,y):sat
#define    CXSUB(x, y)     VNAVGH(x,y):crnd:sat        // if non-scaled, VSUBH(x,y):sat
    /*[*****************************************************************************]*/
    /*[  Function   : void cxsFHT_a16()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform fast Hadmard transform on complex array with          ]*/
    /*[               a scaling factor of 1/N                                       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Complex number: interleave real/image with real part at         ]*/
    /*[             low memory address                                              ]*/
    /*[           - Input and output are aligned by 8bytes                          ]*/
    /*[           - N >= 4                                                          ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : CWord2x16 *input                                         ]*/
    /*[               R1 : int N                                                    ]*/
    /*[               R2 : CWord2x16 *output                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32                                ]*/
    /*[=============================================================================]*/
    /*[  Implementation:                                                            ]*/
    /*[           - Radix-4 inplace. (So, it is possilbe for output = input)        ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (N/2+3)*K + N/2 + 7 ;  N =    4^K                               ]*/
    /*[           - (N/2+3)*K + N   + 10;  N = 2*(4^K)                              ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl cxsFHT_a16
    .type    cxsFHT_a16, @function
cxsFHT_a16:
    { P0 = CMP.GT(R1,#3)                    //[ P0 = N > 3                          ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
    }
    { R29 = ADD(R29, #-4*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-4*8)) = R17:16           //[ callee-saved registers              ]
      R1 = ASR(R1,#2)                       //[ LC0 = N/4                           ]
      R28 = R2                              //[ save output Ptr in R28              ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      P3=SP1LOOP0(.cxsFHT_a16_Stage1Loop,R1)//[ setup loop: lc0 = N/4               ]
      R5:4 = MEMD(R0++#8)                   //[[p]load A,B                          ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R7:6 = MEMD(R0++#8)                   //[[p]load C,D                          ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R5 = R6                               //[[p]R5:4 = C : A                      ]
      R6 = R5                               //[[p]R7:6 = D : B                      ]
    }

    .falign
.cxsFHT_a16_Stage1Loop:
    { R5:4 = MEMD(R0++#8)                   //[[1]load A,B                          ]
      R9:8   = CXADD(R5:4,R7:6)             //[[2]C': A' = C + D : A + B            ]
      R11:10 = CXSUB(R5:4,R7:6)             //[[2]D': B' = C - D : A - B            ]
      IF P3 MEMD(R2++#8) = R11:10           //[[3]save A", B"                       ]
    }
    { R7:6 = MEMD(R0++#8)                   //[[1]load C,D                          ]
      R9  = R10                             //[[2]R9:8   = B' : A'                  ]
      R10 = R9                              //[[2]R11:10 = D' : C'                  ]
      IF P3 MEMD(R2++#8) = R13:12           //[[3]save C", D"                       ]
    }
    { R5 = R6                               //[[1]R5:4 = C : A                      ]
      R6 = R5                               //[[1]R7:6 = D : B                      ]
      R11:10 = CXADD(R9:8,R11:10)           //[[2]B" : A" = B' + D' : A' + C'       ]
      R13:12 = CXSUB(R9:8,R11:10)           //[[2]D" : C" = B' - D' : A' - C'       ]
    }:endloop0
    
    { MEMD(R2++#8) = R11:10                 //[[e]save A", B"                       ]
      R21:20 = COMBINE(#-48,#16)            //[ -3M0 : M0 =(-3*nS*4 : nS*4), nS=4   ]
      P0 = CMP.GT(R1,#1)                    //[ P0 = (N>4)                          ]
      P1 = CMP.EQ(R1,#2)                    //[ P1 = (N=8)                          ]
    }
    { MEMD(R2)     = R13:12                 //[[e]save C", D"                       ]
      R22 = R1                              //[ R22 = N/4                           ]
      R23 = ASR(R1,#2)                      //[ nG  = N/16                          ]
      IF !P0 JUMP .cxsFHT_a16_a32_DONE      //[ if N=4 return                       ]
    }
    { IF P1 JUMP .cxsFHT_a16_Radix2Stage    //[ if N=8 goto radix2 stage            ]
      R3 = #-1                              //[ constant                            ]
    }

    .falign
.cxsFHT_a16_StagesLOOP:
    { R1 = ADD(R28,R20)                     //[ &output + nS                        ]
      M0 = R20                              //[ set m0 = ns*4                       ]
      R15 = ADD(R21,#8)                     //[ -3*ns*4 + 8                         ]
      R5:4 = MEMD(R28)                      //[[p]load A                            ]
    }
    { M1 = R15                              //[ set m1 = -3*ns*4 + 8                ]
      P1 = CMP.EQ(R23,#2)                   //[ P1 = (nG==2), i.e.,  2*4^k-point FHT]
      R3 += ASR(R20,#3)                     //[ R3 = nS/2 -1                        ]
      R7:6 = MEMD(R1++M0)                   //[p]load B                             ]
    }
    { R2 = R23                              //[ cnt = nG                            ]
      R23 = ASR(R23,#2)                     //[ update nG >>= 2                     ]
      R9:8 = MEMD(R1++M0)                   //[[p]load C                            ]
      LOOP0(.cxsFHT_a16_innerLoop,R3)       //[[ setup loop: lc0 = ns-1             ]
    }

    .falign
.cxsFHT_a16_mainLoop:
    { R11:10 = MEMD(R1++M1)                 //[[p] load D                           ]
      R13:12 = CXADD(R5:4,R7:6)             //[[p] A' = A + B                       ]
      R15:14 = CXSUB(R5:4,R7:6)             //[[p] B' = A - B                       ]
      R2 = ADD(R2,#-1)                      //[ cnt--                               ]
    }
    { R9:8   = CXADD(R9:8,R11:10)           //[[p] C' = C + D                       ]
      R11:10 = CXSUB(R9:8,R11:10)           //[[p] D' = C - D                       ]
      R5:4 = MEMD(R1++M0)                   //[[p] load A                           ]
    }
    { R17:16 = CXADD(R13:12,R9:8)           //[[p] A" = A' + C'                     ]
      R9:8   = CXSUB(R13:12,R9:8)           //[[p] C" = A' - C'                     ]
      R7:6 = MEMD(R1++M0)                   //[[p] load B                           ]
      P0 = CMP.EQ(R2,#0)                    //[ P0=(cnt==0) i.e., last iteration    ]
    }

    .falign
.cxsFHT_a16_innerLoop:
    { R9:8 = MEMD(R1++M0)                   //[[2] load C                           ]
      MEMD(R1+#-8) = R9:8                   //[[3] save C"                          ]
      R19:18 = CXADD(R15:14,R11:10)         //[[3] B" = B' + D'                     ]
      R11:10 = CXSUB(R15:14,R11:10)         //[[3] D" = B' - D'                     ]
    }
    { R11:10 = MEMD(R1++M1)                 //[[2] load D                           ]
      R13:12 = CXADD(R5:4,R7:6)             //[[2] A' = A + B                       ]
      R15:14 = CXSUB(R5:4,R7:6)             //[[2] B' = A - B                       ]
      MEMD(R1+#-8) = R11:10                 //[[3] save D"                          ]
    }
    { R5:4 = MEMD(R1++M0)                   //[[1] load A                           ]
      R9:8   = CXADD(R9:8,R11:10)           //[[2] C' = C + D                       ]
      R11:10 = CXSUB(R9:8,R11:10)           //[[2] D' = C - D                       ]
      MEMD(R1+#-16) = R17:16                //[[3] save A"                          ]
    }
    { R7:6 = MEMD(R1++M0)                   //[[1] load B                           ]
      R17:16 = CXADD(R13:12,R9:8)           //[[2] A" = A' + C'                     ]
      R9:8   = CXSUB(R13:12,R9:8)           //[[2] C" = A' - C'                     ]
      MEMD(R1+#-16) = R19:18                //[[3] save B"                          ]
    }:endloop0

    { MEMD(R1+#-8) = R9:8                   //[[e] save C"                          ]
      R13:12 = CXSUB(R15:14,R11:10)         //[[e] D" = B' - D'                     ]
      R1 = ADD(R1,R20)                      //[ point to A[0] (next group)          ]
      LOOP0(.cxsFHT_a16_innerLoop,R3)       //[[ setup loop: lc0 = ns-1             ]
    }
    { MEMD(R1+#-8) = R13:12                 //[[e] save D"                          ]
      R0 = ADD(R1,R21)                      //[[e] next A[0] - 3M0, i.e, B[0]       ]
      R5:4 = MEMD(R1++M0)                   //[[p] load A                           ]
      IF P0 JUMP .cxsFHT_a16_mainLP_Handle  //[ if last iteration then jump         ]
    }
    { MEMD(R0+#-8) = R17:16                 //[[e] save A"                          ]
      R0 = ADD(R0,R20)                      //[[e] point to C[0]                    ]
      R19:18 = CXADD(R15:14,R11:10)         //[[e] B" = B' + D'                     ]
      R7:6 = MEMD(R1++M0)                   //[[p] load B                           ]
    }
    { MEMD(R0+#-8) = R19:18                 //[[e] save B"                          ]
      R9:8 = MEMD(R1++M0)                   //[[p] load C                           ]
      JUMP .cxsFHT_a16_mainLoop             //[ continue main loop                  ]
    }

    .falign
.cxsFHT_a16_mainLP_Handle: 
    { MEMD(R0+#-8) = R17:16                 //[[e] save A"                          ]
      R0 = ADD(R0,R20)                      //[[e] point to C[0]                    ]
      R19:18 = CXADD(R15:14,R11:10)         //[[e] B" = B' + D'                     ]
      P0 = CMP.GT(R23,#0)                   //[ P0 = nG > 0                         ]
    }
    { MEMD(R0+#-8) = R19:18                 //[[e] save B"                          ]
      R21:20 = VASLW(R21:20,#2)             //[ update R21:20 = (-3*nS*8 : ns*8)    ]
      R3 = #-1                              //[ constant                            ]
      IF P0 JUMP .cxsFHT_a16_StagesLOOP     //[ if P0 go to next stage              ]
    }

    { IF !P1 JUMP .cxsFHT_a16_a32_DONE      //[ if N = 4^k then done                ]
    }

.cxsFHT_a16_Radix2Stage:
    { R0 = R28                              //[ R0 = output                         ]
      R1 = ADDASL(R28,R22,#3)               //[ R1 = output + N/2                   ] 
    }
    { R3:2 = R1:0                           //[ write Ptrs                          ]
      P3 = SP1LOOP0(.cxsFHT_a16_Radix2StageLoop,R22)
                                            //[ setup loop: lc0 = N/4               ]
      R5:4 = MEMD(R0++#8)                   //[[p] load A                           ]
    }

    .falign
.cxsFHT_a16_Radix2StageLoop:
    { R7:6 = MEMD(R1++#8)                   //[[2] load B                           ]
      IF P3 MEMD(R3++#8) = R11:10           //[[3] save B1                          ]
    }
    { R5:4 = MEMD(R0++#8)                   //[[1] load A                           ]
      R9:8   = CXADD(R5:4,R7:6)             //[[2] A1 = A + B                       ]
      R11:10 = CXSUB(R5:4,R7:6)             //[[2] B1 = A - B                       ]
      IF P3 MEMD(R2++#8) = R9:8             //[[3] save A1                          ]
    }:endloop0
    
    { MEMD(R3) = R11:10                     //[[e] save B1                          ]
    }
    { MEMD(R2) = R9:8                       //[[e] save A1                          ]
    }
.cxsFHT_a16_a32_DONE:
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#4*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    cxsFHT_a16, .-cxsFHT_a16

