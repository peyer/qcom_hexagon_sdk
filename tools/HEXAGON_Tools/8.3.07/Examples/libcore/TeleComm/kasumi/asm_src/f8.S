/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "f8.S"

#define        OFST_A                    0
#define        OFST_TEMP                 8
#define        OFST_MODKEY               16
#define        OFST_R27R26               32
#define        OFST_R25R24               40
#define        BYTES_ON_STACK            (OFST_R25R24+8)
    /*[*****************************************************************************]*/
    /*[  Function   : void f8()                                                     ]*/
    /*[*****************************************************************************]*/
    /*[  Description: F8 confidential algorithm                                     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *key                                              ]*/
    /*[               R1 : int    count                                             ]*/
    /*[               R2 : int    bearer                                            ]*/
    /*[               R3 : int    dir                                               ]*/
    /*[               R4 : UWord8 *data                                             ]*/
    /*[               R5 : int    length                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15, R24-R27                                            ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 16+8                              ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 289*ceil(N/64) + ceil(N/8) + 514                                ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl f8
    .type    f8, @function
f8:
    { ALLOCFRAME(#BYTES_ON_STACK)           //[ allocate stack                      ]
      R2 = ASL(R2,#3)                       //[ bearer << 3                         ]
      R8 = SWIZ(R1)                         //[ endian swap, R8 = A.b8[0-3]         ]
      R7 = #0x55                            //[ R7 = 0x55                           ]
    }
    { MEMD(R29+#OFST_R25R24) = R25:24       //[ callee-saved registers              ]
      R25:24 = COMBINE(R5,R4)               //[ R24 = data, R25 = length            ]
      R2 |= ASL(R3,#2)                      //[ A.b8[4] = (bearer<<3) | (dir<<2)    ]
      LOOP0(.ModKey_LOOP,#15)               //[ setup loop0: lc0 = 15               ]
    }
    { MEMD(R29+#OFST_R27R26) = R27:26       //[ callee-saved registers              ]
      R27 = R0                              //[ R27 = key                           ]
      R9 = ZXTB(R2)                         //[ A.b8[5-7] = 0                       ]
      R3 = MEMB(R0++#1)                     //[p]load key[0]                        ]
    }
    { MEMD(R29+#OFST_A) = R9:8              //[ save A.b32[0-1]                     ]
      R4 = ADD(R29,#OFST_MODKEY)            //[ R4 = ModKey                         ]
      R3 = MEMB(R0++#1)                     //[[p]load key[1]                       ]
      R2 = XOR(R7,R3)                       //[[p] ModKey[0] = key[0] ^ 0x55        ]
    }

    .falign
.ModKey_LOOP:
    { R3 = MEMB(R0++#1)                     //[[1] load key[n]                      ]
      R2 = XOR(R7,R3)                       //[[2] ModKey[n] = key[n] ^ 0x55        ]
      MEMB(R4++#1) = R2                     //[[3] save ModKey[n]                   ]
    }:endloop0

    { MEMB(R4) = R2                         //[[e]save Modkey[n]                    ]
      R0 = ADD(R29,#OFST_MODKEY)            //[ func call: R0 = ModKey              ]
      CALL KeySchedule                      //[ KeySchedule(ModKey)                 ]
      R26 = ADD(R25,#7)                     //[ length + 7                          ]
    }
    .falign
    { R0 = ADD(R29,#OFST_A)                 //[ func call: R0 = A.b8                ]
      CALL Kasumi                           //[ Kasumi(A.b8)                        ]
      R26 = ASR(R26,#3)                     //[ (length+7)/8                        ]
    }
    .falign
    { R0 = R27                              //[ func call: R0 = key                 ]
      CALL KeySchedule                      //[ KeySchedule(key)                    ]
      R27 = #0                              //[ blkcnt = 0                          ]
    }
    .falign
    { P0 = CMP.GT(R25,#0)                   //[ P0 = (lenght >0)                    ]
      IF !P0.new JUMP:nt .F8_END            //[ if (length <=0) return              ]
      IF  P0.new R3:2 = MEMD(R29+#OFST_A)   //[ A                                   ]
      R7:6 = #0                             //[ temp = 0                            ]
    }

    .falign
.BlockCipher_LOOP:
    { R3:2 = XOR(R3:2,R7:6)                 //[ temp ^= A                           ]
      R4 = ASL(R27,#24)                     //[ r4.hh = blkcnt                      ]
      R25 = #8                              //[ constant                            ]
    }
    { R25 = MIN(R25,R26)                    //[ if(length>64) n=8, else (length+7)/8]
      R26 = ADD(R26,#-8)                    //[ length -= 64                        ]
      R3 = XOR(R3,R4)                       //[ temp.b8[7] ^= blkcnt                ]
    }
    { MEMD(R29+#OFST_TEMP) = R3:2           //[ save temp[]                         ]
      R0 = ADD(R29,#OFST_TEMP)              //[ paramter for func call              ]
      CALL Kasumi                           //[ Kasumi(temp.b8)                     ]
    }
    .falign
    { R7:6 = MEMD(R29+#OFST_TEMP)           //[ R7:6 = temp.b8[0-7]                 ]
      LOOP0(.f8_XORInput_LOOP,R25)          //[ setup loop0: lc0 = n                ]
      R0 = MEMB(R24)                        //[[p]load data[]                       ]
    }
    { R0 = MEMB(R24+#1)                     //[[p]load data[]                       ]
      R1 = XOR(R0,R6)                       //[[p]*data++ ^= temp.b8[i]             ]
      R5:4 = ASR(R7:6,#8)                   //[[p] R4.ll = temp.b8[i]               ]
      P0 = CMP.GT(R26,#0)                   //[ P0 = length > 0                     ]
    }

    .falign
.f8_XORInput_LOOP:
    { R0 = MEMB(R24+#2)                     //[[1] load data[]                      ]
      R1 = XOR(R0,R4)                       //[[2] *data++ ^= temp.b8[i]            ]
      R5:4 = ASR(R5:4,#8)                   //[[2] R4.ll = temp.b8[i]               ]
      MEMB(R24++#1) = R1                    //[[3] save data[]                      ]
    }:endloop0

    { R3:2 = MEMD(R29+#OFST_A)              //[ Load A                              ]
      R27 = ADD(R27,#1)                     //[ blkcnt++                            ]
      IF P0 JUMP .BlockCipher_LOOP          //[ if P0 continue loop                 ]
    }

.F8_END:
    { R27:26 = MEMD(R29+#OFST_R27R26)       //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#OFST_R25R24)       //[ restore callee-saved registers      ]
    }
    { DEALLOCFRAME                          //[ pop stack                           ]
    }
    { JUMPR R31                             //[ return                              ]
    }
    .size    f8, .-f8
