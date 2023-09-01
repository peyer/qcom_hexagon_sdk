/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file    "upsamp.S"
    /*[*****************************************************************************]*/            
    /*[  Function   : void upsamp8()                                                ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: Filter to do upsampling by a factor of 8                      ]*/ 
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - filter_memory is aligned to 8-byte boundary                     ]*/
    /*[           - the most current is the first in filtMem                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : in -> Pointer to input signal.                           ]*/
    /*[               R1 : filter memory                                            ]*/
    /*[               R2 : size of input buffer                                     ]*/
    /*[               R3 : mode (0: mono, otherwise stero)                          ]*/
    /*[               R4 : out -> Pointer to output signal (i)                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R17, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/            


    .text
    .p2align 2
    .p2align 4,,15
    .globl upsamp8
    .type    upsamp8, @function
upsamp8:
    { P1 = CMP.GT(R2,#0)                    //[ P1 = (nsamples>0)                   ]
      P0 = CMP.GTU(R3,#0)                   //[ P0 = (mode!=0)                      ]
      R14 = CONST32(#upsamp_filt_coefs)     //[ set R14 = upsamp_filt_coefs         ]
      P2 = CMP.GT(R0,R0)                    //[ clean P2                            ]
    }
    { IF !P1 JUMP .upsamp8_return           //[ if(nsamples<=0) return              ]
      R29 = ADD(R29,#-1*8)                  //[ reserve stack                       ]
      MEMD(R29+#(0-1*8)) = R17:16           //[ callee-saved registers              ]
      R28 = MUX(P0,#4,#2)                   //[ step = (mode)? 2 : 1                ]
    }

    .falign
.upsamp_mainloop:
    { R15 = MEMH(R0)                        //[ cur= *inputBuffer                   ]
      R0 = ADD(R0,R28)                      //[ inputBuff +=step                    ]
      R3 = ADD(R14,#16)                     //[ start from .upsampe_coefs+8         ]
      LOOP1(.upsamp_polyphase_filt,#8)      //[ setup loop with LC1=8               ]
    }

    .falign
.upsamp_polyphase_filt:
    { R17:16 = ASR(R7:6,#13)                //[ [o2]lTemp >>=13                     ]
      R8 = MEMH(R14++#2)                    //[ load c[j][0]                        ]
      R5 = R1                               //[ R5 = filtMem                        ]
    }
    { R7:6 = MPY(R8.L,R15.L)                //[ cur * c[j][0]                       ]
      LOOP0(.upsamp_innerloop,#5)           //[ setup loop with (NUMTAPS-1)/4-2     ]
      R11:10 = MEMD(R5++#8)                 //[ [p]load filtMem[0-3]                ]
      R9:8   = MEMD(R3++#8)                 //[ [p]load c[j][1-4]                   ]
    }

    .falign
.upsamp_innerloop:
    { R9:8   = MEMD(R3++#8)                 //[[1]load c[j][k]                      ]
      R11:10 = MEMD(R5++#8)                 //[[1]load filtMem[k]                   ]
      R7:6 += VRMPYH(R9:8,R11:10)           //[[2]lTemp += c[j][k] * filtMem[k]     ]
      NOP
    }:endloop0

    { R16 = SATH(R16)                       //[ [o2]saturate(lTemp)                 ]
      R7:6 += VRMPYH(R9:8,R11:10)           //[ lTemp += c[j][k] * filtMem[k]       ]
      R9:8 = MEMD(R3++#8)                   //[ load c[j][k]                        ]
      R13:12 = MEMD(R5++#-16)               //[ load filtMem[k]                     ]
    }
    { IF P2 MEMH(R4) = R16                  //[ [o2]save output                     ]
      IF P2 R4 = ADD(R4,R28)                //[ [o2]outputBuffer+=step              ]
      P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
      R7:6 += VRMPYH(R9:8,R13:12)           //[ lTemp += c[j][k] * filtMem[k]       ]
    }:endloop1

    { R17:16 = ASR(R7:6,#13)                //[ [e2]lTemp >>=13                     ]
      R15 = ASLH(R15)                       //[ R15.H = cur                         ]
      R2 = ADD(R2,#-1)                      //[ update LC for main loop             ]
      P3=SP1LOOP0(.upsamp_update_fileMEM,#5)//[ setup loop with (NUMTAPS-1)/4-2     ]
    }

    .falign
.upsamp_update_fileMEM:
    { R13:12 = R11:10                       //[[1]previous filtMem[]                ]
      R11:10 = MEMD(R5++#-8)                //[[1]load filtMem                      ]
      R9:8 = VALIGNB(R13:12,R11:10,#6)      //[[2]aligne filtMem[]                  ]
      IF P3 MEMD(R5+#24) = R9:8             //[[3]save updated filtMem[]            ]
    }:endloop0

    { R16 = SATH(R16)                       //[ [e2]saturate(lTemp)                 ]
      MEMD(R5+#24) = R9:8                   //[ [e]save updated filtMem[]           ]
      R9:8 = VALIGNB(R13:12,R11:10,#6)      //[ [e]aligne filtMem[]                 ]
    }
    { MEMD(R5+#16) = R9:8                   //[ [e]save updated filtMem[]           ]
      R9:8 = VALIGNB(R11:10,R15:14,#6)      //[ [e]aligne filtMem[]                 ]
      P0 = CMP.GT(R2,#0)                    //[ main loop count > 0 ?               ]
    }
    { MEMD(R5+#8) = R9:8                    //[ [e]save updated filtMem[]           ]
      R14 = ADD(R14,#-16)                   //[ adjust R14 to .upsamp_coefs         ]
      IF P0 JUMP .upsamp_mainloop           //[ if P0 continue main loop            ]
    }
    { MEMH(R4) = R16                        //[ [e2]save last output sample         ]
    }
.upsamp8_return:
    { R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#1*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    upsamp8, .-upsamp8

    /*-----------------------------------------------------------------------------*/
    /*                           coeffients for filter banks                       */
    /*-----------------------------------------------------------------------------*/
    .globl upsamp_filt_coefs
    .section    .rodata
    .p2align 3
    .type    upsamp_filt_coefs, @object
    .size    upsamp_filt_coefs, 464
upsamp_filt_coefs:
    .hword  1, 1, 1, 0, -2, -6, -10, -15    // coefs for curr input
    .hword    -20, 9, 33, -42               // filter #0
    .hword    96, -142, 208, -261, 313, -349, 344, -200
    .hword    -304, 7386, 1667, -1078, 798, -573, 405, -277
    .hword    178, -92, 42, 4, -4, 35, -26, 1
    .hword    -24, 22, 16, -22              // filter #1
    .hword    75, -127, 207, -287, 382, -487, 597, -651
    .hword    599, 7552, 599, -651, 597, -487, 382, -287
    .hword    207, -127, 75, -22, 16, 22, -24, 1
    .hword    -26, 35, -4, 4                // filter #2
    .hword    42, -92, 178, -277, 405, -573, 798, -1078
    .hword    1667, 7386, -304, -200, 344, -349, 313, -261
    .hword    208, -142, 96, -42, 33, 9, -20, 1
    .hword    -27, 46, -24, 33              // filter #3
    .hword    1, -42, 124, -228, 376, -588, 913, -1424
    .hword    2837, 6904, -993, 220, 74, -179, 209, -204
    .hword    184, -139, 105, -54, 47, -4, -15, 0
    .hword    -26, 54, -41, 61              // filter #4
    .hword    -43, 18, 50, -145, 293, -526, 920, -1631
    .hword    4029, 6141, -1442, 566, -178, -1, 88, -126
    .hword    139, -119, 100, -58, 55, -14, -10, 0
    .hword    -21, 57, -53, 85              // filter #5
    .hword    -85, 81, -37, -35, 163, -385, 804, -1649
    .hword    5158, 5158, -1649, 804, -385, 163, -35, -37
    .hword    81, -85, 85, -53, 57, -21, -6, 0
    .hword    -14, 55, -58, 100             // filter #6
    .hword    -119, 139, -126, 88, -1, -178, 566, -1442
    .hword    6141, 4029, -1631, 920, -526, 293, -145, 50
    .hword    18, -43, 61, -41, 54, -26, -2, 0
    .hword    -4, 47, -54, 105              // filter #7
    .hword    -139, 184, -204, 209, -179, 74, 220, -993
    .hword    6904, 2837, -1424, 913, -588, 376, -228, 124
    .hword    -42, 1, 33, -24, 46, -27, 0, 0


