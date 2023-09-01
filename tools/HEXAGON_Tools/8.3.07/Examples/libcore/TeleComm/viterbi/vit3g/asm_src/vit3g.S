/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "vit3g.S"

#define NUM_STATES     256

    /*[*****************************************************************************]*/
    /*[  Function   : void vit3g_1r3k9()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: viterbi decoding of covolutional code in WCDMA (rate=1/3)     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Working buffer must be aligned by 8                             ]*/
    /*[           - A total of 2x2x256+ N*256/8 bytes working buffer is required    ]*/
    /*[           - Decoded bits are packed as                                      ]*/
    /*[             Byte 0 = I0I1...I7                                              ]*/
    /*[             Byte 1 = I8I1...I15                                             ]*/
    /*[             ....                                                            ]*/
    /*[             Byte 33 = I264.. I267,xx..x                                     ]*/
    /*[           - Overflow is not a concern                                       ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word8  *INsoft - Ptr to soft-decision input bytes        ]*/
    /*[               R1 : UWord8 *output - Ptr to packed decoded bits              ]*/
    /*[               R2 : Word16 *buffer - Ptr to working buffer                   ]*/
    /*[               R3 : int    numbits - number of decoded bits                  ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R28                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48+72                             ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 171N + 2*[(N+7)/8] +78.  example: 45,974 for N=268              ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl vit3g_1r3k9
    .type  vit3g_1r3k9, @function
vit3g_1r3k9:
    { R29 = ADD(R29,#-(64+8+48))            //[ allocate stacks                     ]
                                            //[ 64bytes for BMs, 48bytes for R16-R27]
                                            //[ and 8bytes for others               ]
      MEMD(R29+#-48) = R17:16               //[ callee-saved registers              ]
      R28 = R0                              //[ R28 = *input                        ]
      R8 = MEMB(R0+#2)                      //[ load input z                        ]
    }
    { MEMD(R29+#(72+8)) = R19:18            //[ callee-saved registers              ]
      R7 = MEMB(R0+#1)                      //[ load input y                        ]
      R9 = NEG(R8)                          //[ -z                                  ]
    }
    { MEMD(R29+#(72+16)) = R21:20           //[ callee-saved registers              ]
      R6 = MEMB(R28++#3)                    //[ load input x                        ]
    }
    { MEMD(R29+#(72+24)) = R23:22           //[ callee-saved registers              ]
      R6 = ADD(R6,R7)                       //[ x+y                                 ]
      R7 = SUB(R6,R7)                       //[ x-y                                 ]
    }
    { MEMD(R29+#(72+32)) = R25:24           //[ callee-saved registers              ]
      R22 = R1                              //[ R22 = output                        ]
      R23.H = #HI(BMIndex+1)                //[ R23 = BMIndex                       ]
      R25 = R3                              //[ number of decoded bits in R25       ]
    }
    { MEMD(R29+#(72+40)) = R27:26           //[ callee-saved registers              ]
      R1:0 = COMBINE(R2,R2)                 //[ PM read buffer                      ]
      R3 = ADD(R2,#1024)                    //[ tace bit buffer                     ]
      R23.L = #LO(BMIndex+1)
    }
    { MEMD(R29+#64) = R23:22                //[ push *output, BMIndex stack         ]
      R11:10 = #0                           //[ 0                                   ]
      LOOP0(.vit3g_1r3k9_Initialization_LOOP,#(NUM_STATES/4-2))
                                            //[ setup loop0: lc0 = NUM_STATES/4-2   ]
    }

    .falign
.vit3g_1r3k9_Initialization_LOOP:
    { MEMD(R1++#8) = R11:10                 //[ initialize s[4k]--s[4k+3]           ]
      NOP; NOP; NOP                         //[                                     ]
    }:endloop0

    { MEMD(R1++#8) = R11:10                 //[ initialize s[248-251] = 0           ]
      R7:6 = VADDW(R7:6,R9:8)               //[ R7:6 = m3 | m0                      ]
      R9:8 = VSUBW(R9:8,R7:6)               //[ R9:8 = m5 | m6                      ]
      R2 = ADD(R0,#256)                     //[ R0 = &s[0], R2 = &s[128]            ]
    }
    { MEMD(R1++#8) = R11:10                 //[ initialize s[252-255] = 0           ]
      R8 = COMBINE(R7.L,R6.L)               //[ R9:8 = m6|m5|m3|m0                  ]
      R9 = COMBINE(R8.L,R9.L)               //[ R9:8 = m6|m5|m3|m0                  ]
      LOOP1(.vit3g_1r3k9_Forward_LOOP,R25)  //[ setup loop1: lc1= numbits           ]
    }

    .falign
.vit3g_1r3k9_Forward_LOOP:
    { MEMD(R29+#0) = R9:8                   //[ save m6|m5|m3|m0 in BM buffer       ]
      R15:14 = COMBINE(R8,R9)               //[ m3|m0|m6|m5                         ]
      R18 = COMBINE(R8.L,R8.H)              //[ R19:18 = m5|m6|m0|m3                ]
      R27:26 = R1:0                         //[ save R/W pointers                   ]
    }
    { MEMD(R29+#32) = R15:14                //[ save m3|m0|m6|m5 in BM buffer       ]
      R5:4 = MEMD(R0++#8)                   //[ load state s[0-3]                   ]
      R19 = COMBINE(R9.L,R9.H)              //[ R19:18 = m5|m6|m0|m3                ]
      LOOP0(.vit3g_1r3k9_ACS_LOOP,#(NUM_STATES/8-1))
                                            //[ setup loop0: lc0 = NUM_STATES/8-1   ]
    }
    { MEMD(R29+#40) = R19:18                //[ save m5|m6|m0|m3 in BM buffer       ]
      R7:6 = MEMD(R2++#8)                   //[ load state s[128] - s[131]          ]
      R17:16 = #0                           //[ 0                                   ]
      R19:18 = COMBINE(R18,R19)             //[ m0|m3|m5|m6                         ]
    }
    { MEMD(R29+#8) = R19:18                 //[ save m0|m3|m5|m6 in BM buffer       ]
      R23 = MEMW(R29+#(64+4))               //[ rest R23 = BMIndex                  ]
      R19:18 = VSUBH(R17:16,R19:18)         //[ m7|m4|m2|m1                         ]
      R11:10 = VADDH(R5:4,R9:8)             //[ ACS-1                               ]
    }
    { MEMD(R29+#56) = R19:18                //[ save m7|m4|m2|m1 in BM buffer       ]
      R18 = COMBINE(R19.L,R19.H)            //[ m1|m2|m4|m7                         ]
      R19 = COMBINE(R18.L,R18.H)            //[ m1|m2|m4|m7                         ]
      R13:12 = VSUBH(R7:6,R9:8)             //[ ACS-1                               ]
    }
    { MEMD(R29+#48) = R19:18                //[ save m1|m2|m4|m7 in BM buffer       ]
      R17:16 = COMBINE(R18,R19)             //[ R17:16 = m4|m7|m1|m2                ]
      R18 = COMBINE(R18.L,R18.H)            //[ R19:18 = m2|m1|m7|m4                ]
      R19 = COMBINE(R19.L,R19.H)            //[ R19:18 = m2|m1|m7|m4                ]
    }
    { MEMD(R29+#16) = R17:16                //[ save m4|m7|m1|m2 in BM buffer       ]
      R24 = MEMUB(R23++#1)                  //[ load index[1]                       ]
      R15:14 = VMAXH(R11:10,R13:12)         //[ R15:14 = s'[6]|s'[4]|s'[2]|s'[0]    ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[ ACS-1                               ]
    }
    { MEMD(R29+#24) =  R19:18               //[ save m2|m1|m7|m4 in BM buffer       ]
      R5:4 = MEMD(R0++#8)                   //[ load state s[4-7] 
      R11:10 = VSUBH(R5:4,R9:8)             //[ ACS-2
      R13:12 = VADDH(R7:6,R9:8)             //[ ACS-2
    }

    .falign
.vit3g_1r3k9_ACS_LOOP:
    { R7:6 = MEMD(R2++#8)                   //[ load state s[128+4(i+1) + 3|2|1|0 ] ]
      R24 = ADD(R29,R24)                    //[ &BM[index[(i+1)]]                   ]
      R17:16 = VMAXH(R11:10,R13:12)         //[ R17:16 = s'[8i+ 7|5|3|1]            ]
      P1 = VCMPH.GT(R13:12,R11:10)          //[ ACS-2                               ]
    }
    { R9:8 = MEMD(R24)                      //[ load BM                             ]
      R20 = COMBINE(R17.L,R15.L)            //[ R20 = s'[8i+5]|s'[8i+4]             ]
      R19:18 = PACKHL(R16,R14)              //[ R19:18 = s'[8i + 3|2|1|0]           ]
      R22 = VITPACK(P0,P1)                  //[ pack trace bits                     ]
    }
    { MEMD(R1++#8) = R19:18                 //[ save s'[8i] - s'[8i+3]              ]
      R11:10 = VADDH(R5:4,R9:8)             //[ ACS-1                               ]
      R13:12 = VSUBH(R7:6,R9:8)             //[ ACS-1                               ]
      R21 = COMBINE(R17.H,R15.H)            //[ R21:20 = s'[8i + 7|6|5|4]           ]
    }
    { MEMD(R1++#8) = R21:20                 //[ save s'[8i+4] - s'[8i+7]            ]
      R24 = MEMUB(R23++#1)                  //[ load index[i+2]                     ]
      R15:14 = VMAXH(R11:10,R13:12)         //[ R15:14 = s'[8(i+1) + 6|4|2|0]       ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[ ACS-1                               ]
    }
    { MEMB(R3++#1) = R22                    //[ save trace bits                     ]
      R5:4 = MEMD(R0++#8)                   //[ load state s[4(i+2)] - s[4(i+2)+3]  ]
      R11:10 = VSUBH(R5:4,R9:8)             //[ ACS-2                               ]
      R13:12 = VADDH(R7:6,R9:8)             //[ ACS-2                               ]
    }:endloop0

    { R7 = MEMB(R28+#1)                     //[ load input y                        ]
      R8 = MEMB(R28+#2)                     //[ load input z                        ]
      R17:16 = VMAXH(R11:10,R13:12)         //[ R17:16 =  s'[255|253|251|249]       ]
      P1 = VCMPH.GT(R13:12,R11:10)          //[ ACS-2                               ]
    }
    { R6 = MEMB(R28++#3)                    //[ load input x                        ]
      R19:18 = PACKHL(R16,R14)              //[ R19:18 = s'[251|250|249|248]        ]
      R22 = VITPACK(P0,P1)                  //[ pack trace bits                     ]
      R9 = NEG(R8)                          //[ -z                                  ]
    }
    { MEMD(R1++#8) = R19:18                 //[ save s'[248] - s'[251]              ]
      R21:20 = PACKHL(R17,R15)              //[ R21:20 = s'[255|254|253|252]        ]
      R6 = ADD(R6,R7)                       //[ x+y                                 ]
      R7 = SUB(R6,R7)                       //[ x-y                                 ]
    }
    { MEMD(R1) = R21:20                     //[ save s'[252] - s'[255]              ]
      R1:0 = COMBINE(R26,R27)               //[ switch R/W pointer                  ]
      R7:6 = VADDW(R7:6,R9:8)               //[ R7:6 = m3 | m0                      ]
      R9:8 = VSUBW(R9:8,R7:6)               //[ R9:8 = m5 | m6                      ]
    }
    { MEMB(R3++#1) = R22                    //[ save trace bits                     ]
      R2 = ADD(R0,#256)                     //[ R0 = &s[0], R2 = &s[128]            ]
      R8 = COMBINE(R7.L,R6.L)               //[ R9:8 = m6|m5|m3|m0                  ]
      R9 = COMBINE(R8.L,R9.L)               //[ R9:8 = m6|m5|m3|m0                  ]
    }:endloop1

.vit3g_1r3k9_ACS_LOOPEND:
    { R2 = ADD(R3,#-32)                     //[ pts to the last stage of trace bits ]
      R25 = ADD(R25,#7)                     //[ numbits+7                           ]
      R7 = AND(R25,#7)                      //[ numbits%8                           ]
      R9 = #0x10
    }
    { R0 = MEMW(R29+#64)                    //[ R0 = *output                        ]
      R25 = ASR(R25,#3)                     //[ # of dec bytes = (numbits+7)/8      ]
      P0 = CMP.EQ(R7,#0)                    //[ if (numbits%8 ==0)                  ]
      IF P0.new R7 = #8                     //[ then set to 8                       ]
    }
    { LOOP1(.vit3g_1r3k9_TracebackLOOPs,R25)//[ setup loop1: lc1 = ceil(numbits/8)  ]
      R0 += ADD(R25,#-1)                    //[ &output[num_dec_bytes-1]            ]
      R5:4 = #0                             //[ initial state = 0  R5.[4-0] R4[31-29]
                                            //[ clear output bits buffer R4.[28-21] ]
      R6 = #0                               //[ bitpos = 0                          ]
    }
    { R12 = R2                              //[ &trace[i*(NUM_STATES/8)]            ]
      LOOP0(.vit3g_1r3k9_TracebackLOOPs,R7) //[ setup loop0: lc0 = numbits%8        ]
    }

    .falign
.vit3g_1r3k9_TracebackLOOPs:
    { R7 = MEMUB(R12)                       //[ trace[i*(NUM_STATES/8)+wordpos]     ]
      R5:4 = LSR(R5:4,#1)                   //[                                     ]
      R2 = ADD(R2,#-32)                     //[ R2 = &trace[i*(NUM_STATES/8)     ]  ]
      R3 = ADD(R2,#-16)                     //[ R3 = &trace[i*(NUM_STATES/8) + 16]  ]
    }
    { P0 = TSTBIT(R7,R6)                    //[ bit=(trace[i*(NUM_STATES/8) +       ]
                                            //[            wordpos]>>bitpos)&1      ]
      R8 = OR(R5,R9)                        //[                                     ]
      R10 = ADD(R2,R5)                      //[                                     ]
      R11 = ADD(R3,R5)                      //[                                     ]
    }
    { R6 = LSR(R4,#29)                      //[ bitpos                              ]
      R12 = MUX(P0,R11,R10)                 //[ &trace[i*(NUM_STATES/8)+wordpos]    ]
      R5 = MUX(P0,R8,R5)                    //[ state = bit << (K-2) | (state>>1)   ]
    }:endloop0

    { R8 = LSR(R4,#21)                      //[ shift down output byte              ]
      LOOP0(.vit3g_1r3k9_TracebackLOOPs,#8) //[ setup loop0:lc0=8 after 1st iteratn ]
    }
    { MEMB(R0++#-1) = R8                    //[ save output byte                    ]
    }:endloop1

.vit3g_1r3k9_END:
    { R17:16 = MEMD(R29+#(72+0))            //[ restore callee-saved registers      ]
      R19:18 = MEMD(R29+#(72+8))            //[ restore callee-saved registers      ]
    }
    { R21:20 = MEMD(R29+#(72+16))           //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#(72+24))           //[ restore callee-saved registers      ]
    }
    { R25:24 = MEMD(R29+#(72+32))           //[ restore callee-saved registers      ]
      R27:26 = MEMD(R29+#(72+40))           //[ restore callee-saved registers      ]
      R29 = ADD(R29,#(64+8+48))             //[ pop stacks
      JUMPR R31                             //[ return                              ]
    }
    .size vit3g_1r3k9, .-vit3g_1r3k9



    .section	.rodata
    .size	BMIndex, 32	
BMIndex:
    .byte	0,8,16,24,32,40,48,56 
    .byte	24,16,8,0,56,48,40,32
    .byte	8,0,24,16,40,32,56,48
    .byte	16,24,0,8,48,56,32,40



    /*[*****************************************************************************]*/
    /*[  Function   : void vit3g_1r2k9()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: viterbi decoding of covolutional code in WCDMA (rate=1/2)     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Working buffer must be aligned by 8                             ]*/
    /*[           - A total of 2x2x256+ N*256/8 bytes working buffer is required    ]*/
    /*[           - Decoded bits are packed as                                      ]*/
    /*[             Byte 0 = I0I1...I7                                              ]*/
    /*[             Byte 1 = I8I1...I15                                             ]*/
    /*[             ....                                                            ]*/
    /*[             Byte 33 = I264.. I267,xx..x                                     ]*/
    /*[           - Overflow is not a concern                                       ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word8  *INsoft - Ptr to soft-decision input bytes        ]*/
    /*[               R1 : UWord8 *output - Ptr to packed decoded bits              ]*/
    /*[               R2 : Word16 *buffer - Ptr to working buffer                   ]*/
    /*[               R3 : int    numbits - number of decoded bits                  ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R28                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48+40                             ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 168N + 2*[(N+7)/8] +82.                                         ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl vit3g_1r2k9
    .type  vit3g_1r2k9, @function
vit3g_1r2k9:
    { R29 = ADD(R29,#-(32+8+48))            //[ Allocate stacks                     ]
                                            //[ 32bytes for BMs, 48bytes for R16-R27]
                                            //[ and 8bytes for others               ]
      MEMW(R29+#(-88+32)) = R1              //[ push *output to stack               ]
    }
    { MEMD(R29+#(40+0)) = R17:16            //[ callee-saved registers              ]
      R7 = MEMB(R0+#1)                      //[ load input y                        ]
    }
    { MEMD(R29+#(40+8))  = R19:18           //[ callee-saved registers              ]
      R6 = MEMB(R0++#2)                     //[ load input x                        ]
    }
    { MEMD(R29+#(40+16)) = R21:20           //[ callee-saved registers              ]
      R6 = ADD(R6,R7)                       //[ x+y                                 ]
      R7 = SUB(R6,R7)                       //[ x-y                                 ]
      LOOP0(.vit3g_1r2k9_Initialization_LOOP,#(NUM_STATES/4-2))
                                            //[ setup loop0: lc0 = (NUM_STATES/4-2) ]
    }
    { MEMD(R29+#(40+24)) = R23:22           //[ callee-saved registers              ]
      R23.H = #HI(BMIndexR2+1)              //[ R23 = BMIndexR2+1                   ]
    }
    { MEMD(R29+#(40+32)) = R25:24           //[ callee-saved registers              ]
      R23.L = #LO(BMIndexR2+1)              //[ R23 = BMIndexR2 + 1                 ]
      R28 = R0                              //[ save input ptr in R28               ]
      R25 = R3                              //[ save numbits in R25                 ]
    }
    { MEMD(R29+#(40+40)) = R27:26           //[ callee-saved registers              ]
      R1:0 = COMBINE(R2,R2)                 //[ PM read buffer                      ]
      R3 = ADD(R2,#1024)                    //[ tace bit buffer                     ]
      R11:10 = #0                           //[ 0                                   ]
    }

    .falign
.vit3g_1r2k9_Initialization_LOOP:
    { MEMD(R1++#8) = R11:10                 //[ initialize s[4k]--s[4k+3]           ]
    }:endloop0

    { MEMD(R1++#8) = R11:10                 //[ initialize s[248-251] = 0           ]
      R2 = ADD(R0,#256)                     //[ R0 = &s[0], R2 = &s[128]            ]
      R9:8 = VSUBW(R11:10,R7:6)             //[  y-x : -x-y                         ]
      LOOP1(.vit3g_1r2k9_Forward_LOOP,R25)  //[ setup loop1: lc1 = numbits          ]
    }
    { MEMD(R1++#8) = R11:10                 //[ initialize s[252-255] = 0           ]
      R9:8   = VTRUNEWH(R9:8,R7:6)          //[ R9:8   = m2|m3|m1|m0                ]
      R15:14 = VTRUNEWH(R7:6,R9:8)          //[ R15:14 = m1|m0|m2|m3                ]
    }

    .falign
.vit3g_1r2k9_Forward_LOOP:
    { MEMD(R29+#0) = R9:8                   //[ push m2|m3|m1|m0 to BM buffer       ]
      R27:26 = R1:0                         //[ save R/W pointers                   ]
      P3 =SP1LOOP0(.vit3g_1r2k9_ACS_LOOP,#(NUM_STATES/8-1))
                                            //[ setup loop0: lc0 = NUM_STATES/8-1   ]
      R5:4 = MEMD(R0++#8)                   //[[p]load state s[0-3]                 ]
    }
    { MEMD(R29+#8) = R15:14                 //[ push m1|m0|m2|m3 to BM buffer       ]
      R18 = COMBINE(R8.L,R8.H)              //[ R19:18 = m3|m2|m0|m1                ]
      R19 = COMBINE(R9.L,R9.H)              //[ R19:18 = m3|m2|m0|m1                ]
      R7:6 = MEMD(R2++#8)                   //[[p]load state s[128] - s[131]        ]
    }
    { MEMD(R29+#24) = R19:18                //[ push m3|m2|m0|m1 to BM buffer       ]
      R19:18 = COMBINE(R18,R19)             //[ m0|m1|m3|m2                         ]
      R11:10 = VADDH(R5:4,R9:8)             //[[p]ACS-1                             ]
      R13:12 = VSUBH(R7:6,R9:8)             //[[p]ACS-1                             ]
    }
    { MEMD(R29+#16) = R19:18                //[ push m0|m1|m3|m2 to BM buffer       ]
      R24 = MEMUB(R23++#1)                  //[[p]load index[1]                     ]
      R15:14 = VMAXH(R11:10,R13:12)         //[[p]R15:14 = s'[6]|s'[4]|s'[2]|s'[0]  ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[[p]ACS-1                             ]
    }

    .falign
.vit3g_1r2k9_ACS_LOOP:
    { R5:4 = MEMD(R0++#8)                   //[[1]load state s[4(i+1) + 3|2|1|0]    ]
      R11:10 = VSUBH(R5:4,R9:8)             //[[2]ACS-2                             ]
      R13:12 = VADDH(R7:6,R9:8)             //[[2]ACS-2                             ]
      IF P3 MEMB(R3++#1) = R22              //[[3]save trace bits                   ]
    }
    { R7:6 = MEMD(R2++#8)                   //[[1]load state s[128+4(i+1)+ 3|2|1|0] ]
      R24 = ADD(R29,R24)                    //[[1]&BM[index[(i+1)]]                 ]
      R17:16 = VMAXH(R11:10,R13:12)         //[[2]R17:16 =  s'[8i+ 7|5|3|1]         ]
      P1 = VCMPH.GT(R13:12,R11:10)          //[[2]ACS-2                             ]
    }
    { R9:8 = MEMD(R24)                      //[[1]load BM                           ]
      R20 = COMBINE(R17.L,R15.L)            //[[2]R20 = s'[8i+5]|s'[8i+4]           ]
      R19:18 = PACKHL(R16,R14)              //[[2]R19:18 = s'[8i+ 3|2|1|0]          ]
      R22 = VITPACK(P0,P1)                  //[[2]pack trace bits                   ]
    }
    { R11:10 = VADDH(R5:4,R9:8)             //[[1]ACS-1                             ]
      R13:12 = VSUBH(R7:6,R9:8)             //[[1]ACS-1                             ]
      R21 = COMBINE(R17.H,R15.H)            //[[2]R21:20 = s'[8i+ 7|6|5|4]          ]
      MEMD(R1++#8) = R19:18                 //[[2]save s'[8i+0] - s'[8i+3]          ]
    }
    { R24 = MEMUB(R23++#1)                  //[[0]load index[i+2]                   ]
      R15:14 = VMAXH(R11:10,R13:12)         //[[1]R15:14 = s'[8(i+1)+6|4|2|0]       ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[[1]ACS-1                             ]
      MEMD(R1++#8) = R21:20                 //[[2]save s'[8i+4] - s'[8i+7]          ]
    }:endloop0

    { MEMB(R3++#1) = R22                    //[ save trace bits                     ]
      R11:10 = VSUBH(R5:4,R9:8)             //[ ACS-2                               ]
      R13:12 = VADDH(R7:6,R9:8)             //[ ACS-2                               ]
      R23.H = #HI(BMIndexR2+1)              //[ reset R23 = BMIndexR2 +1            ]
    }
    { R6 = MEMB(R28++#2)                    //[ load input x                        ]
      R7 = MEMB(R28+#1)                     //[ load input z                        ]
      R17:16 = VMAXH(R11:10,R13:12)         //[ R17:16 =  s'[255|253|251|249]       ]
      P1 = VCMPH.GT(R13:12,R11:10)          //[ ACS-2                               ]
    }
    { R19:18 = PACKHL(R16,R14)              //[ R19:18 = s'[251|250|249|248]        ]
      R22 = VITPACK(P0,P1)                  //[ pack trace bits                     ]
      R6 = ADD(R6,R7)                       //[ x+y                                 ]
      R7 = SUB(R6,R7)                       //[ x-y                                 ]
    }
    { MEMD(R1++#8) = R19:18                 //[ save s'[248] - s'[251]              ]
      R21:20 = PACKHL(R17,R15)              //[ R21:20 = s'[255|254|253|252]        ]
      R8 = NEG(R6)                          //[ -x-y                                ]
      R9 = NEG(R7)                          //[ -x+y                                ]
    }
    { MEMD(R1) = R21:20                     //[ save s'[252] - s'[255]              ]
      R1:0 = COMBINE(R26,R27)               //[ switch R/W pointer                  ]
      R23.L = #LO(BMIndexR2+1)              //[ R23 = BMIndexR2 + 1                 ]
      R9:8 = VTRUNEWH(R9:8,R7:6)            //[ R9:8 = m2|m3|m1|m0                  ]
    }
    { MEMB(R3++#1) = R22                    //[ save trace bits                     ]
      R2 = ADD(R0,#256)                     //[ R2 = &s[128]                        ]
      R15:14 = COMBINE(R8,R9)               //[ m1|m0|m2|m3                         ]
      NOP
    }:endloop1

vit3g_1r2k9_ACS_LOOPEND:
    { R2 = ADD(R3,#-32)                     //[ pts to the last stage of trace bits ]
      R25 = ADD(R25,#7)                     //[ numbits +7                          ]
      R7  = AND(R25,#7)                     //[ numbits%8                           ]
      R9 = #0x10                            //[ constant                            ]
    }
    { R0 = MEMW(R29+#32)                    //[ R0 = *output                        ]
      R25 = ASR(R25,#3)                     //[ # of dec bytes = (numbits+7)/8      ]
      P0 = CMP.EQ(R7,#0)                    //[ if (numbits%8 ==0)                  ]
      IF P0.new R7 = #8                     //[ then set to 8                       ]
    }
    { LOOP1(.vit3g_1r2k9_TracebackLOOPs,R25)//[ setup loop1: lc1 = ceil(numbits/8)  ]
      R0 += ADD(R25,#-1)                    //[ &output[Num_dec_bytes-1]            ]
      R5:4 = #0                             //[ initial state = 0  R5.[4-0] R4[31-29]
                                            //[ clear output bits buffer R4.[28-21] ]
      R6 = #0                               //[ bitpos = 0                          ]
    }
    { R12 = R2                              //[ &trace[i*(NUM_STATES/8)]            ]
      LOOP0(.vit3g_1r2k9_TracebackLOOPs,R7) //[ setup loop0: lc0 = numbits%8        ]
    }

    .falign
.vit3g_1r2k9_TracebackLOOPs:
    { R7 = MEMUB(R12)                       //[ trace[i*(NUM_STATES/8)+wordpos]     ]
      R5:4 = LSR(R5:4,#1)                   //[                                     ]
      R2 = ADD(R2,#-32)                     //[ R2 = &trace[i*(NUM_STATES/8)     ]  ]
      R3 = ADD(R2,#-16)                     //[ R3 = &trace[i*(NUM_STATES/8) + 16]  ]
    }
    { P0 = TSTBIT(R7,R6)                    //[ bit = (trace[i*(NUM_STATES/8)+      ]
                                            //[              wordpos]>>bitpos)&1    ]
      R8 = OR(R5,R9)                        //[                                     ]
      R10 = ADD(R2,R5)                      //[                                     ]
      R11 = ADD(R3,R5)                      //[                                     ]
    }
    { R6 = LSR(R4,#29)                      //[ bitpos                              ]
      R12 = MUX(P0,R11,R10)                 //[ &trace[i*(NUM_STATES/8)+wordpos]    ]
      R5 = MUX(P0,R8,R5)                    //[ state = bit << (K-2) | (state>>1)   ]
    }:endloop0
    
    { R8 = LSR(R4,#21)                      //[ shift down output byte              ]
      LOOP0(.vit3g_1r2k9_TracebackLOOPs,#8) //[ setup loop0:lc0=8 after 1st iteratn ]
    }
    { MEMB(R0++#-1) = R8                    //[ save output byte                    ]
    }:endloop1

vit3g_1r2k9_END:
    { R17:16 = MEMD(R29+#(40+0))            //[ restore callee-saved registers      ]
      R19:18 = MEMD(R29+#(40+8))            //[ restore callee-saved registers      ]
    }
    { R21:20 = MEMD(R29+#(40+16))           //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#(40+24))           //[ restore callee-saved registers      ]
    }
    { R25:24 = MEMD(R29+#(40+32))           //[ restore callee-saved registers      ]
      R27:26 = MEMD(R29+#(40+40))           //[ restore callee-saved registers      ]
      R29 = ADD(R29,#(32+8+48))             //[ pop stacks                          ]
      JUMPR R31                             //[ return                              ]
    }
    .size vit3g_1r2k9, .-vit3g_1r2k9

    .section	.rodata
    .size	BMIndexR2, 32	
BMIndexR2:
    .byte	0,8,16,24,24,16,8,0
    .byte	0,8,16,24,24,16,8,0
    .byte	24,16,8,0,0,8,16,24
    .byte	24,16,8,0,0,8,16,24

