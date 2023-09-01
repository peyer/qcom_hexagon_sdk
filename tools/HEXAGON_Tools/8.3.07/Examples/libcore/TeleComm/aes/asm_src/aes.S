/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 


    .file    "aes.S"
    /*[*****************************************************************************]*/
    /*[  Function   : int aes_setkey()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  Description: key schedule from the user supplied key                       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8       *in_key    - pointer to secret key          ]*/
    /*[               R1 : unsigned int ken_len    - key length (in bytes)          ]*/
    /*[               R2 : UWord32      *EnctryTAB - pointer to lookup table        ]*/
    /*[               R3 : AES_CTX      *ctx       - pointer to AES context         ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : 1 (failure) or 0 (success)                               ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 308  (128-bit)                                                  ]*/
    /*[           - 357  (192-bit)                                                  ]*/
    /*[           - 438  (256-bit)                                                  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl aes_setkey
    .type    aes_setkey, @function
aes_setkey:
    { R29 = ADD(R29, #-4*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-4*8)) = R17:16           //[ callee-saved registers              ]
      P0 = CMP.EQ(R1,#16)                   //[ (key_len == 16)                     ]
      P1 = CMP.EQ(R1,#24)                   //[ (key_len == 24)                     ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      P2 = CMP.EQ(R1,#32)                   //[ (key_len == 32)                     ]
      P3 = OR(P0,P1)                        //[ key_len==16 || key_len==24          ]
      R5 = LSR(R1,#3)                       //[ key_len>>3                          ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      P3 = OR(P3,P2)                        //[key_len==16||key_len==24||key_len==32]
      R5 = ADD(R5,#-1)                      //[ LC0 = (ken_len>>3) - 1              ]
      R4 = ADD(R3,#8)                       //[ E_KEY                               ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      IF !P3 R0 = #1                        //[ if !P3 then return 1                ]
      IF !P3 JUMP .aes_setkey_END           //[ if !P3 then return 1                ]
    }
    { MEMW(R3) = R1                         //[ ctx->key_length = key_len           ]
      R15:14 = COMBINE(R2,R2)               //[ R15=R14=EncryptTAB                  ]
      R18 = #0x7F7F                         //[To set R19=R18=0x7F7F7F7F for setDkey]
    }
    { R7:6 = MEMD(R0++#8)                   //[ GET_UWORD32_LE(in_key+ 0: 4)        ]
      R17 = ADD(R1,#20)                     //[ (key_len+24)-4 for setDkey_LOOP     ]
      R19:18 = VSPLATH(R18)                 //[ set R19=R18=0x7F7F7F7F for setDkey  ]
    }
    { MEMD(R4++#8) = R7:6                   //[ save E_KEY[0:1]                     ]
      R9:8 = MEMD(R0++#8)                   //[ GET_UWORD32_LE(in_key+ 8:12)        ]
      R16.H = #HI(RCOtab)                   //[ R16 = RCOtab                        ]
      R17 = LSR(R17,#1)                     //[ ((key_len+24)-4)/2 for setDkey_LOOP ]
    }
    { MEMD(R4++#8) = R9:8                   //[ save E_KEY[2:3]                     ]
      R16.L = #LO(RCOtab)                   //[ R16 = RCOtab                        ] 
      IF !P0 R11:10 = MEMD(R0++#8)          //[ GET_UWORD32_LE(in_key+ 16:20)       ]
      IF  P0 JUMP .aes_setkey_case16        //[ if(key_len==16) jump                ]
    }
    { MEMD(R4++#8) = R11:10                 //[ save E_KEY[4:5]                     ]
      IF !P1 R13:12 = MEMD(R0++#8)          //[ GET_UWORD32_LE(in_key+ 24:28)       ]
      IF  P1 JUMP .aes_setkey_case24        //[ if(key_len==24) jump                ]
    }
    { MEMD(R4++#8) = R13:12                 //[ save E_KEY[6:7]                     ]
      JUMP .aes_setkey_case32               //[ jump to key_len=32 case             ]
    }

    .falign
.aes_setkey_case16:
    /* --------------------------------------------------------------------------- */
    /*                 Cipher Key expansion for AES-128                            */
    /* --------------------------------------------------------------------------- */

    { P3= SP1LOOP0(.setEkey_case16_LOOP,#10)//[ setup loop0: lc0 = 10               ]
    }

    .falign
.setEkey_case16_LOOP:
    { R14 = TABLEIDXW(R9,#8,#8)             //[ &FT[(UWord8)(E_KEY[4*i+3]>>8 )]     ]
      R15 = TABLEIDXW(R9,#8,#16)            //[ &FT[(UWord8)(E_KEY[4*i+3]>>16)]     ]
      R5 = MEMUB(R16++#1)                   //[ RCOtab[i]                           ]
    }
    { R0 = MEMUB(R14+#1)                    //[ read byte #0                        ]
      R1 = MEMUB(R15+#2)                    //[ read byte #1                        ]
      R14 = TABLEIDXW(R9,#8,#24)            //[ &FT[(UWord8)(E_KEY[4*i+3]>>24)]     ]
      R15 = TABLEIDXW(R9,#8,#0)             //[ &FT[(UWord8)(E_KEY[4*i+3]    )]     ]
    }
    { R0 |= ASL(R1,#8)                      //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2                        ]
      R2 = MEMUB(R15+#2)                    //[ read byte #3                        ]
    }
    { IF P3 MEMD(R4++#8) = R7:6             //[                                     ]
      R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R0 = XOR(R0,R5)                       //[ ls_box(E_KEY[4*i+3])^RCOtab[i]      ]
    }
    { IF P3 MEMD(R4++#8) = R9:8             //[                                     ]
      R0 ^= ASL(R1,#16)                     //[ ls_box(E_KEY[4*i+3])^RCOtab[i]      ]
      R2 = XOR(R6,R7)                       //[ E_KEY[4*i+0]^E_KEY[4*i+1]           ]
      R9 = XOR(R9,R8)                       //[ E_KEY[4*i+2]^E_KEY[4*i+3]           ]
    }
    { R6 = XOR(R0,R6)                       //[ E_KEY[4*i+4]                        ]
      R7 = XOR(R0,R2)                       //[ E_KEY[4*i+5]                        ]
      R8 ^= XOR(R0,R2)                      //[ E_KEY[4*i+6]                        ]
      R9 ^= XOR(R0,R2)                      //[ E_KEY[4*i+7]                        ]
    }:endloop0

    { MEMD(R4++#8) = R7:6                   //[[e]                                  ]
      R13:12 = R9:8                         //[                                     ]
      JUMP .aes_setDkey                     //[ jump to set Dkey                    ]
    }

    .falign
.aes_setkey_case24:
    /* -------------------------------------------- */
    /*  Cipher Key expansion for AES-192            */
    /* -------------------------------------------- */

    { P3 = SP1LOOP0(.setEkey_case24_LOOP,#8)//[ setup loop0                         ]
    }

    .falign
.setEkey_case24_LOOP:
    { R14 = TABLEIDXW(R11,#8,#8)            //[ &FT[(UWord8)(E_KEY[6*i+5]>>8 )]     ]
      R15 = TABLEIDXW(R11,#8,#16)           //[ &FT[(UWord8)(E_KEY[6*i+5]>>16)]     ]
      R5 = MEMUB(R16++#1)                   //[ RCOtab[i]                           ]
    }
    { R0 = MEMUB(R14+#1)                    //[ read byte #0                        ]
      R1 = MEMUB(R15+#2)                    //[ read byte #1                        ]
      R14 = TABLEIDXW(R11,#8,#24)           //[ &FT[(UWord8)(E_KEY[6*i+5]>>24)]     ]
      R15 = TABLEIDXW(R11,#8,#0)            //[ &FT[(UWord8)(E_KEY[6*i+5]    )]     ]
    }
    { R0 |= ASL(R1,#8)                      //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2                        ]
      R2 = MEMUB(R15+#2)                    //[ read byte #3                        ]
    }
    { IF P3 MEMD(R4++#8) = R9:8
      R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R0 = XOR(R0,R5)                       //[ ls_box(E_KEY[6*i+5])^RCOtab[i]      ]
    }
    { IF P3 MEMD(R4++#8) = R11:10           //[                                     ]
      R0 ^= ASL(R1,#16)                     //[ ls_box(E_KEY[6*i+5])^RCOtab[i]      ]
      R2 = XOR(R6,R7)                       //[ E_KEY[6*i+0]^E_KEY[6*i+1]           ]
      R9 = XOR(R9,R8)                       //[ E_KEY[6*i+2]^E_KEY[6*i+3]           ]
    }
    { R6 = XOR(R0,R6)                       //[ E_KEY[6*i+6]                        ]
      R7 = XOR(R0,R2)                       //[ E_KEY[6*i+7]                        ]
      R8 ^= XOR(R0,R2)                      //[ E_KEY[6*i+8]                        ]
      R9 ^= XOR(R0,R2)                      //[ E_KEY[6*i+9]                        ]
    }
    { MEMD(R4++#8) = R7:6                   //[                                     ]
      R10 = XOR(R9,R10)                     //[ E_KEY[6*i+10]                       ]
      R11 ^= XOR(R9,R10)                    //[ E_KEY[6*i+11]                       ]
    }:endloop0

    { MEMD(R4++#8) = R9:8                   //[[e]                                  ]
      R13:12 = R11:10                       //[                                     ]
      JUMP .aes_setDkey                     //[ jump to set Dkey                    ]
    }

    .falign
.aes_setkey_case32:
    /* -------------------------------------------- */
    /*  Cipher Key expansion for AES-256            */
    /* -------------------------------------------- */

    { P3= SP1LOOP0(.setEkey_case32_LOOP,#7) //[ setup loop0                         ]
    }

    .falign
.setEkey_case32_LOOP:
    { R14 = TABLEIDXW(R13,#8,#8)            //[ &FT[(UWord8)(E_KEY[8*i+7]>>8 )]     ]
      R15 = TABLEIDXW(R13,#8,#16)           //[ &FT[(UWord8)(E_KEY[8*i+7]>>16)]     ]
      R5 = MEMUB(R16++#1)                   //[ RCOtab[i]                           ]
    }
    { R0 = MEMUB(R14+#1)                    //[ read byte #0
      R1 = MEMUB(R15+#2)                    //[ read byte #1
      R14 = TABLEIDXW(R13,#8,#24)           //[ &FT[(UWord8)(E_KEY[8*i+7]>>24)]     ]
      R15 = TABLEIDXW(R13,#8,#0)            //[ &FT[(UWord8)(E_KEY[8*i+7]    )]     ]
    }
    { R0 |= ASL(R1,#8)                      //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2                        ]
      R2 = MEMUB(R15+#2)                    //[ read byte #3                        ]
    }
    { IF P3 MEMD(R4++#8) = R11:10           //[                                     ]
      R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R0 = XOR(R0,R5)                       //[ ls_box(E_KEY[8*i+7])^RCOtab[i]      ]
    }
    { IF P3 MEMD(R4++#8) = R13:12           //[                                     ]
      R0 ^= ASL(R1,#16)                     //[ ls_box(E_KEY[8*i+7])^RCOtab[i]      ]
      R2 = XOR(R6,R7)                       //[ E_KEY[8*i+0]^E_KEY[8*i+1]           ]
      R9 = XOR(R9,R8)                       //[ E_KEY[8*i+2]^E_KEY[8*i+3]           ]
    }
    { R6 = XOR(R0,R6)                       //[ E_KEY[8*i+8 ]                       ]
      R7 = XOR(R0,R2)                       //[ E_KEY[8*i+9 ]                       ]
      R8 ^= XOR(R0,R2)                      //[ E_KEY[8*i+10]                       ]
      R9 ^= XOR(R0,R2)                      //[ E_KEY[8*i+11]                       ]
    }
    { MEMD(R4++#8) = R7:6                   //[                                     ]
      R14 = TABLEIDXW(R9,#8,#0)             //[ &FT[(UWord8)(E_KEY[8*i+11]   )]     ]
      R15 = TABLEIDXW(R9,#8,#8)             //[ &FT[(UWord8)(E_KEY[8*i+11]>>8)]     ]
    }
    { R0 = MEMUB(R14+#1)                    //[ read byte #0                        ]
      R1 = MEMUB(R15+#2)                    //[ read byte #1                        ]
      R14 = TABLEIDXW(R9,#8,#16)            //[ &FT[(UWord8)(E_KEY[8*i+11]>>16)]    ]
      R15 = TABLEIDXW(R9,#8,#24)            //[ &FT[(UWord8)(E_KEY[8*i+11]>>24)]    ]
    }
    { R0 |= ASL(R1,#8)                      //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2                        ]
      R2 = MEMUB(R15+#2)                    //[ read byte #3                        ]
    }
    { MEMD(R4++#8) = R9:8                   //[                                     ]
      R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R10 = XOR(R0,R10)                     //[ ls_box(ROTL32(E_KEY[8*i+11]))       ]
                                            //[  ^E_KEY[8*i+4]                      ]
      R13 = XOR(R12,R13)                    //[ E_KEY[8*i+6]^E_KEY[8*i+7]           ]
    }
    { R10 ^= ASL(R1,#16)                    //[ E_KEY[8*i+12] = ls_box(             ]
                                            //[  ROTL32(E_KEY[8*i+11]))^E_KEY[8*i+4]]
    }
    { R11 = XOR(R10,R11)                    //[ E_KEY[8*i+13]                       ]
      R12 ^= XOR(R10,R11)                   //[ E_KEY[8*i+14]                       ]
      R13 ^= XOR(R10,R11)                   //[ E_KEY[8*i+15]                       ]
    }:endloop0

    { MEMD(R4++#8) = R11:10                 //[[e]                                  ]
    }

    .falign
.aes_setDkey:
    { MEMD(R4) = R13:12                     //[[e]                                  ]
      R1:0 = MEMD(R3+#8)                    //[ E_KEY[0:1]                          ]
      R20 = ADD(R3,#248)                    //[ D_KEY                               ]
      R21 = ADD(R3,#24)                     //[ &E_KEY[4]                           ]
    }
    { MEMD(R20++#8) = R1:0                  //[ save D_KEY[0:1]                     ]
      R1:0 = MEMD(R3+#16)                   //[ E_KEY[2:3]                          ]
      R16 = #0x1B1B                         //[ To set R17=R16=0x1B1B1B1B           ]
      P3 = SP1LOOP0(.setDkey_LOOP,R17)      //[ setup loop0                         ]
    }
    { MEMD(R20++#8) = R1:0                  //[ save D_KEY[2:3]                     ]
      R17:16 = VSPLATH(R16)                 //[ R17=R16=0x1B1B1B1B                  ]
      R23:22 = COMBINE(#0,#0)               //[ Set R23:22=0                        ]
      R1:0 = MEMD(R21++#8)                  //[ e = E_KEY[i]                        ]
    }

    .falign
.setDkey_LOOP:
    { P0 = VCMPB.GTU(R1:0,R19:18)           //[ XTIME4(e)                           ]
      R3:2 = VADDUB(R1:0,R1:0)              //[ XTIME4(e)                           ]
    }
    { R9:8 = VMUX(P0,R17:16,R23:22)         //[ XTIME4(e)                           ]
      P0 = VCMPB.GTU(R3:2,R19:18)           //[ XTIME4(ex)                          ]
    }
    { R3:2 = XOR(R3:2,R9:8)                 //[ ex = XTIME4(e)                      ]
      R9:8 = VMUX(P0,R17:16,R23:22)         //[ XTIME4(ex)                          ]
    }
    { R5:4 = VADDUB(R3:2,R3:2)              //[ XTIME4(ex)                          ]
      R13:12 = XOR(R7:6,R13:12)             //[[2] D_KEY[i] ^= ROTL32(e^ex2^ex3,16) ]
    }
    { R5:4 = XOR(R5:4,R9:8)                 //[ ex2 = XTIME4(ex)                    ]
      P0 = VCMPB.GTU(R5:4,R19:18)           //[ XTIME4(ex)                          ]
    }
    { R9:8 = VMUX(P0,R17:16,R23:22)         //[ XTIME4(ex)                          ]
      R7:6 = VADDUB(R5:4,R5:4)              //[ XTIME4(ex)                          ]
    }
    { R7:6 = XOR(R7:6,R9:8)                 //[ ex3 = XTIME4(ex2)                   ]
      R15:14 = XOR(R13:12,R15:14)           //[[2] D_KEY[i] ^= ROTL32(e^ex^ex3,24)  ]
    }
    { R11:10 = XOR(R1:0,R7:6)               //[ e^ex3                               ]
      R7:6 = XOR(R3:2,R7:6)                 //[ ex^ex3                              ]
      IF P3 MEMD(R20++#8) = R15:14          //[[2] save D_KEY[i]                    ]
    }
    { R13:12 = XOR(R11:10,R5:4)             //[ e^ex2^ex3                           ]
      R15:14 = XOR(R11:10,R3:2)             //[ e^ex^ex3                            ]
    }
    { R9:8 = VASLW(R11:10,#8)               //[ ROTL32(e^ex3,8)                     ]
      R11:10 = VLSRW(R11:10,#24)            //[ ROTL32(e^ex3,8)                     ]
      R13 = COMBINE(R13.L,R13.H)            //[ ROTL32(e^ex2^ex3,16)                ]
      R12 = COMBINE(R12.L,R12.H)            //[ ROTL32(e^ex2^ex3,16)                ]
    }
    { R11:10 = XOR(R9:8,R11:10)             //[ ROTL32(e^ex3,8)                     ]
      R7:6 = XOR(R5:4,R7:6)                 //[ ex^ex2^ex3                          ]
    }
    { R9:8 = VASLW(R15:14,#24)              //[ ROTL32(e^ex^e3,24)                  ]
      R15:14 = VLSRW(R15:14,#8)             //[ ROTL32(e^ex^e3,24)                  ]
    }
    { R15:14 = XOR(R9:8,R15:14)             //[ ROTL32(e^ex^e3,24)                  ]
      R7:6 = XOR(R7:6,R11:10)               //[ D_KEY[i] ^= ROTL32(e^ex3,8)         ]
      R1:0 = MEMD(R21++#8)                  //[ e = E_KEY[i]                        ]
    }:endloop0

    { R15 ^= XOR(R7,R13)                    //[ [e]D_KEY[i] ^= ROTL32(e^ex^ex3,24)^ ]
                                            //[                ROTL32(e^ex2^ex3,16) ]
      R14 ^= XOR(R6,R12)                    //[ [e]D_KEY[i] ^= ROTL32(e^ex^ex3,24)^ ]
                                            //[                ROTL32(e^ex2^ex3,16) ]
    }
    { MEMD(R20++#8) = R15:14                //[ save                                ]
      R0 = #0                               //[ return value                        ]
    }
.aes_setkey_END:
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#4*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    aes_setkey, .-aes_setkey

    .data
    .p2align 3
    .type    RCOtab, @object
    .size    RCOtab, 10
RCOtab:
    .byte    1,2,4,8,16,32,64,128,27,54



    /*[*****************************************************************************]*/
    /*[  Function   : void aes_encrpt()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: AES encryption of a block of text                             ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : AES_CTX  *ctx                                            ]*/
    /*[               R1 : UWord32  *EnctryTAB                                      ]*/
    /*[               R2 : UWord8   *in                                             ]*/
    /*[               R1 : UWord8   *out                                            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R17                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 12*nr + 13, where nr = 14, 12, 10                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl aes_encrypt
    .type    aes_encrypt, @function
aes_encrypt:
    { R29 = ADD(R29,#-8)                    //[ allocate stack                      ]
      MEMD(R29+#(0-8)) = R17:16             //[ callee-saved registers              ]
    }
    { R4 = MEMW(R0)                         //[ key_len                             ]
      R5 = ADD(R0,#8)                       //[ E_KEY                               ]
      R14 = R1                              //[ FT0                                 ]
      R1 = #9                               //[ nr -1 = 9                           ]
    }
    { R7:6 = MEMD(R2)                       //[ GET_UWORD32_LE(in+0:4)              ]
      R9:8 = MEMD(R2+#8)                    //[ GET_UWORD32_LE(in+8:12)             ]
      P0 = CMP.GT(R4,#16)                   //[ key_len > 16                        ]
      R15 = ADD(R14,#1024)                  //[ FT1                                 ]
    }
    { R11:10 = MEMD(R5++#16)                //[ E_KEY[1:0]                          ]
      R13:12 = MEMD(R5+#8)                  //[ E_KEY[3:2]                          ]
      IF P0 R1 = #11                        //[ if (key_len > 16), nr-1 = 11        ]
      R16 = ADD(R14,#2048)                  //[ FT2                                 ]
    }
    { R11:10 = XOR(R7:6,R11:10)             //[ b0[1:0]                             ]
      R9:8 = XOR(R9:8,R13:12)               //[ b0[3:2]                             ]
      P0 = CMP.GT(R4,#24)                   //[ if (key_len >24)                    ]
      IF P0.new R1 = #13                    //[ nr-1 = 13                           ]
    }
    { R17 = ADD(R14,#3072)                  //[ FT3                                 ]
      R1:0 = COMBINE(#0,#0)                 //[ reset R0,R1                         ]
      R14 = TABLEIDXW(R10,#8,#0)            //[ &FT0[GET_BYTE(Si[0],0)], j=0        ]
      LOOP0(.aes_encrypt_RoundLoop,R1)      //[ setup loop0: lc0 = nr-1             ]
    }
    
    .falign
.aes_encrypt_RoundLoop:
    { R9 ^= XOR(R0,R1)                      //[[2] Si[3] = So[3]=                   ]
                                            //[      rkey^FT0[]^FT1[]^FT2[]^FT3[]   ]
      R7:6 = COMBINE(R11,R10)               //[[2] R7:6 = Si[1:0] = So[1:0]         ]
      R16 = TABLEIDXW(R8,#8,#16)            //[ &FT2[GET_BYTE(Si[2],2)], j=0        ]
      R11:10 = MEMD(R5++#8)                 //[ *rkey++                             ]
    }
    { R0 = MEMW(R14)                        //[ FT0[GET_BYTE(Si[0],0)], j=0         ]
      R1 = MEMW(R16)                        //[ FT2[GET_BYTE(Si[2],2)], j=0         ]
      R15 = TABLEIDXW(R7,#8,#8)             //[ &FT1[GET_BYTE(Si[1],1)], j=0        ]
      R17 = TABLEIDXW(R9,#8,#24)            //[ &FT3[GET_BYTE(Si[3],3)], j=0        ]
    }
    { R10 ^= XOR(R0,R1)                     //[ *rkey^FT0[]^FT2[]                   ]
      R0 = MEMW(R15)                        //[ FT1[GET_BYTE(Si[1],1)], j=0         ]
      R1 = MEMW(R17)                        //[ FT3[GET_BYTE(Si[3],3)], j=0         ]
      R14 = TABLEIDXW(R7,#8,#0)             //[ &FT0[GET_BYTE(S[1],0)], j=1         ]
    }
    { R10 ^= XOR(R0,R1)                     //[ So[0]= *rkey^FT0[]^FT1[]^FT2[]^FT3[]]
      R15 = TABLEIDXW(R8,#8,#8)             //[ &FT1[GET_BYTE(Si[2],1)], j=1        ]
      R0 = MEMW(R14)                        //[ FT0(GET_BYTE(Si[1],0)], j=1         ]
    }
    { R1 = MEMW(R15)                        //[ FT1(GET_BYTE(Si[2],1)], j=1         ]
      R16 = TABLEIDXW(R9,#8,#16)            //[ &FT2[GET_BYTE(Si[3],2)], j=1        ]
      R17 = TABLEIDXW(R6,#8,#24)            //[ &FT3[GET_BYTE(Si[0],3)], j=1        ]
    }
    { R11 ^= XOR(R0,R1)                     //[ *reky^FT0[]^FT1[]
      R0 = MEMW(R16)                        //[ FT2[GET_BYTE(Si[3],2)], j=1         ]
      R1 = MEMW(R17)                        //[ FT3[GET_BYTE(Si[0],3)], j=1         ]
      R14 = TABLEIDXW(R8,#8,#0)             //[ &FT0[GET_BYTE(Si[2],0)], j=2        ]
    }
    { R11 ^= XOR(R0,R1)                     //[ So[1]=*rkey^FT0[]^FT1[]^FT2[]^FT3[] ]
      R15 = TABLEIDXW(R9,#8,#8)             //[ &FT1[GET_BYTE[Si[3],1)], j=2        ]
      R13:12 = MEMD(R5++#8)                 //[ *rkey++                             ]
      R0 = MEMW(R14)                        //[ FT0[GET_BYTE(Si[2],0)], j=2         ]
    }
    { R1 = MEMW(R15)                        //[ FT1[GET_BYTE(Si[3],1)], j=2         ]
      R16 = TABLEIDXW(R6,#8,#16)            //[ &FT2[GET_BYTE(Si[0],2)], j=2        ]
      R17 = TABLEIDXW(R7,#8,#24)            //[ &FT3[GET_BYTE(Si[1],3)], j=2        ]
    }
    { R12 ^= XOR(R0,R1)                     //[ rkey^FT0^FT1
      R0 = MEMW(R16)                        //[ FT3[GET_BYTE(Si[0],2)], j=2         ]
      R1 = MEMW(R17)                        //[ FT3[GET_BYTE(Si[1],3)], j=2         ]
      R14 = TABLEIDXW(R9,#8,#0)             //[ &FT0[GET_BYTE(Si[3],0)], j=3        ]
    }
    { R12 ^= XOR(R0,R1)                     //[ S0[2]=*rkey^FT0[]^FT1[]^FT2[]^FT3[] ]
      R15 = TABLEIDXW(R6,#8,#8)             //[ &FT1[GET_BYTE(Si[0],1)], j=3        ]
      R0 = MEMW(R14)                        //[ FT0[GET_BYTE(Si[3],0)], j=3         ]
      R9 = R13                              //[ R9 = rkey                           ]
    }
    { R1 = MEMW(R15)                        //[ FT1[GET_BYTE(Si[0],1)], j=3         ]
      R16 = TABLEIDXW(R7,#8,#16)            //[ &FT2[GET_BYTE(Si[1],2)], j=3        ]
      R17 = TABLEIDXW(R8,#8,#24)            //[ &FT3[GET_BYTE(Si[2],3)], j=3        ]
      R8 = R12                              //[ R8= Si[2] = So[2]                   ]
    }
    { R9 ^= XOR(R0,R1)                      //[ rkey^FT0[]^FT1[]                    ]
      R0 = MEMW(R16)                        //[ FT2[GET_BYTE(Si[1],2)], j=3         ]
      R1 = MEMW(R17)                        //[ FT2[GET_BYTE(Si[2],3)], j=3         ]
      R14 = TABLEIDXW(R10,#8,#0)            //[ &FT0[GET_BYTE(Si[0],0)], j=0        ]
    }:endloop0

.aes_encrypt_LastRound:
    { R9 ^= XOR(R0,R1)                      //[[e] Si[3] = So[3]=                   ]
                                            //[      rkey^FT0[]^FT1[]^FT2[]^FT3[]   ]
      R7:6 = COMBINE(R11,R10)               //[[e] R7:6 = Si[1:0] = So[1:0]         ]
      R10 = MEMUB(R14+#1)                   //[ read byte #0                        ]
      R14 = TABLEIDXW(R11,#8,#8)            //[ &FT0[GET_BYTE(Si[1],1)], j=0        ]
    }
    { R15 = R14                             //[ R15 =  &FT0[GET_BYTE(Si[1],1)], j=0 ]
      R1 = MEMUB(R14+#1)                    //[ read byte #1                        ]
      R14 = TABLEIDXW(R8,#8,#16)            //[ &FT0[GET_BYTE(Si[2],2)], j=0        ]
    }
    { R10 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2                        ]
      R14 = TABLEIDXW(R9,#8,#24)            //[ &FT0[GET_BYTE[Si[3],3)], j=0        ]
    }
    { R2 = MEMUB(R14+#1)                    //[ read byte #3                        ]
      R15 = TABLEIDXW(R7,#8,#0)             //[ &FT0[GET_BYTE[Si[1],0)], j=1        ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R11 = MEMUB(R15+#1)                   //[ read byte #0                        ]
      R15 = TABLEIDXW(R8,#8,#8)             //[ &FT0[GET_BYTE(Si[2],1)], j=1        ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[0]                             ]
      R10 = COMBINE(R1.L,R10.L)             //[ PUT_BYTES(byte0, byte1, .., byte3)  ]
      R1 = MEMUB(R15+#1)                    //[ read byte #1                        ]
      R15 = TABLEIDXW(R9,#8,#16)            //[ &FT0[GET_BYTE(Si[3],2)], j=1        ]
    }
    { R10 = XOR(R10,R4)                     //[ So[0]= PUT_BYTES()^rkey[0]          ]
      R11 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,..,byte3), j=0]
      R1 = MEMUB(R15+#1)                    //[ read byte #2                        ]
      R15 = TABLEIDXW(R6,#8,#24)            //[ &FT0[GET_BYTE(Si[0],3)], j=1        ]
    }
    { R2 = MEMUB(R15+#1)                    //[ read byte #3                        ]
      R14 = TABLEIDXW(R8,#8,#0)             //[ &FT[GET_BYTE(Si[2],0)], j=2         ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R12 = MEMUB(R14+#1)                   //[ read byte #0                        ]
      R14 = TABLEIDXW(R9,#8,#8)             //[ &FT0[GET_BYTE(Si[3],1)], j=2        ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[1]                             ]
      R11 = COMBINE(R1.L,R11.L)             //[PUT_BYTES(byte0,byte1,..,byte3), j=1 ]
      R1 = MEMUB(R14+#1)                    //[ read byte #1                        ]
      R14 = TABLEIDXW(R6,#8,#16)            //[ &FT0[GET_BYTE(Si[0],2)], j=2        ]
    }
    { R11 = XOR(R11,R4)                     //[ So[1]= PUT_BYTES()^rkey[1]          ]
      R12 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,....)         ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2
      R14 = TABLEIDXW(R7,#8,#24)            //[ &FT0[GET_BYTE(Si[1],3)], j=2        ]
    }
    { R2 = MEMUB(R14+#1)                    //[ read byte #3
      R15 = TABLEIDXW(R9,#8,#0)             //[ &FT0[GET_BYTE(Si[3],0)], j=3        ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R13 = MEMUB(R15+#1)                   //[ read byte #0                        ]
      R15 = TABLEIDXW(R6,#8,#8)             //[ &FT0[GET_BYTE(Si[0],1)], j=3        ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[2]                             ]
      R12 = COMBINE(R1.L,R12.L)             //[PUT_BYTES(byte0,byte1,..,byte3), j=2 ]
      R1 = MEMUB(R15+#1)                    //[ read byte #1                        ]
      R15 = TABLEIDXW(R7,#8,#16)            //[ &FT0[GET_BYTE(Si[1],2)], j=3        ]
    }
    { R12 = XOR(R12,R4)                     //[ So[2]= PUT_BYTES()^rkey[2]          ]
      R13 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R15+#1)                    //[ read byte #2                        ]
      R15 = TABLEIDXW(R8,#8,#24)            //[ &FT0[GET_BYTE[Si[2],3)], j=3        ]
    }
    { R2 = MEMUB(R15+#1)                    //[ read byte #3                        ]
      R13 |= ASL(R1,#16)                    //[ PUT_BYTES(byte0,byte1,byte2,..)     ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[3]                             ]
      R13 |= ASL(R2,#24)                    //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
    }
    { MEMD(R3) = R11:10                     //[ OUT_UWORD32_LE(So[0 : 1])           ]
      R13 = XOR(R13,R4)                     //[ So[3]= PUT_BYTES()^rkey[3]          ]
    }
    { MEMD(R3+#8) = R13:12                  //[ OUT_UWORD32_LE(So[2 : 3])           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#8)                     //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    aes_encrypt, .-aes_encrypt



    /*[*****************************************************************************]*/
    /*[  Function   : void aes_decrypt()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: AES decryption of a block of text                             ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : AES_CTX  *ctx                                            ]*/
    /*[               R1 : UWord32  *EnctryTAB                                      ]*/
    /*[               R2 : UWord8   *in                                             ]*/
    /*[               R1 : UWord8   *out                                            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R19                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 16                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 12*nr + 15, where nr = 14, 12, 10                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl aes_decrypt
    .type    aes_decrypt, @function
aes_decrypt:
    { R29 = ADD(R29, #-2*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-2*8)) = R17:16           //[ callee-saved registers              ]
      R4 = MEMW(R0)                         //[ key_len                             ]
      R5 = ADD(R0,#328)                     //[ D_KEY+20                            ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R14 = R1                              //[ IT0                                 ]
      R1 = #9                               //[ nr-1 = 9                            ]
    }
    { R7:6 = MEMD(R2)                       //[ GET_UWORD32_LE(in+ 0:4)             ]
      R9:8 = MEMD(R2+#8)                    //[ GET_UWORD32_LE(in+ 8:12)            ]
      P0 = CMP.GT(R4,#16)                   //[ (key_len>16)?                       ]
      R0 += ASL(R4,#2)                      //[ &ctx+ key_len                       ]
    }
    { R11:10 = MEMD(R0+#104)                //[ E_KEY[key_len+24:25]                ]
      R13:12 = MEMD(R0+#112)                //[ E_KEY[key_len+26:27]                ]
      R5 += ASL(R4,#2)                      //[ rkey = D_KEY+key_len+20             ]
      IF P0 R1 = #11                        //[ if(key_len>16) nr-1 = 11            ]
    }
    { R11:10 = XOR(R7:6,R11:10)             //[ b0[1]:b[0]                          ]
      R9:8 = XOR(R9:8,R13:12)               //[ b0[3]:b[2]                          ]
      P0 = CMP.GT(R4,#24)                   //[ (key_len>24)?                       ]
      R15 = ADD(R14,#1024)                  //[ IT1 = DecryptTAB+ 1*256             ]
    }
    { IF P0 R1 = #13                        //[ if(key_len>24) nr-1 = 13            ]
      R16 = ADD(R14,#2048)                  //[ IT2 = DecryptTAB+ 2*256             ]
      R17 = ADD(R14,#3072)                  //[ IT3 = DecryptTAB+ 3*256             ]
      R18 = ADD(R14,#4096)                  //[ ISBT = DecryptTAB+ 4*256            ]
    }
    { R1:0 = #0                             //[ reset R0,R1                         ]
      R14 = TABLEIDXW(R10,#8,#0)            //[ &IT0[GET_BYTE(Si[0],0)], j=0        ]
      LOOP0(.aes_decrypt_RoundLoop,R1)      //[ setup loop0: lc0 = nr-1             ]
    }

    .falign
.aes_decrypt_RoundLoop:
    { R9 ^= XOR(R0,R1)                      //[[2] Si[3]= So[3]=                    ]
                                            //       rkey[3]^IT0[]^IT1[]^IT2[]^IT3[]]
      R7:6 = COMBINE(R11,R10)               //[[2] R7:6= Si[1:0] = So[1:0]          ]
      R16 = TABLEIDXW(R8,#8,#16)            //[ &IT2[GET_BYTE(Si[2],2)], j=1        ]
      R11:10 = MEMD(R5++#8)                 //[ rkey[j]
    }
    { R0 = MEMW(R14)                        //[ IT0[GET_BYTE(Si[0],0)], j=0         ]
      R1 = MEMW(R16)                        //[ IT2[GET_BYTE(Si[2],2)], j=0         ]
      R15 = TABLEIDXW(R9,#8,#8)             //[ &IT1[GET_BYTE(Si[3],1)], j=0        ]
      R17 = TABLEIDXW(R7,#8,#24)            //[ &IT3[GET_BYTE(Si[1],3)], j=0        ]
    }
    { R10 ^= XOR(R0,R1)                     //[ rkey[j]^IT0[]^IT2[]                 ]
      R0 = MEMW(R15)                        //[ IT1[GET_BYTE(Si[3],1)], j=0         ]
      R1 = MEMW(R17)                        //[ IT3[GET_BYTE(Si[1],3)], j=0         ]
      R14 = TABLEIDXW(R7,#8,#0)             //[ &IT0[GET_BYTE(Si[1],0)], j=1        ]
    }
    { R10 ^= XOR(R0,R1)                     //[So[0]=rkey[0]^IT0[]^IT1[]^IT2[]^IT3[]]
      R15 = TABLEIDXW(R6,#8,#8)             //[ &IT1[GET_BYTE(Si[0],1)], j=1        ]
      R0 = MEMW(R14)                        //[ IT0[GET_BYTE(Si[1],0)], j=1         ]
    }
    { R1 = MEMW(R15)                        //[ IT1[GET_BYTE(Si[0],1)], j=1         ]
      R16 = TABLEIDXW(R9,#8,#16)            //[ &IT2[GET_BYTE(Si[3],2)], j=1        ]
      R17 = TABLEIDXW(R8,#8,#24)            //[ &IT3[GET_BYTE(Si[2],3)], j=1        ]
    }
    { R11 ^= XOR(R0,R1)                     //[ rkey[j]^IT0[]^IT1[]                 ]
      R0 = MEMW(R16)                        //[ IT2[GET_BYTE(Si[3],2)], j=1         ]
      R1 = MEMW(R17)                        //[ IT3[GET_BYTE(Si[2],3)], j=1         ]
      R14 = TABLEIDXW(R8,#8,#0)             //[ &IT0[GET_BYTE(Si[2],0)], j=2        ]
    }
    { R11 ^= XOR(R0,R1)                     //[So[1]=rkey[1]^IT0[]^IT1[]^IT2[]^IT3[]]
      R15 = TABLEIDXW(R7,#8,#8)             //[ &IT1[GET_BYTE(Si[1],1)], j=2        ]
      R13:12 = MEMD(R5++#-(8+16))           //[ rkey[j]; rkey-=4                    ]
      R0 = MEMW(R14)                        //[ IT0[GET_BYTE(Si[2],0)], j=2         ]
    }
    { R1 = MEMW(R15)                        //[ &IT1[GET_BYTE(Si[1],1)], j=2        ]
      R16 = TABLEIDXW(R6,#8,#16)            //[ &IT2[GET_BYTE(Si[0],2)], j=2        ]
      R17 = TABLEIDXW(R9,#8,#24)            //[ &IT3[GET_BYTE(Si[3],3)], j=2        ]
    }
    { R12 ^= XOR(R0,R1)                     //[ rkey[j]^IT0[]^IT1[]                 ]
      R0 = MEMW(R16)                        //[ IT2[GET_BYTE(Si[0],2)], j=2         ]
      R1 = MEMW(R17)                        //[ IT3[GET_BYTE(Si[3],3)], j=2         ]
      R14 = TABLEIDXW(R9,#8,#0)             //[ &IT0[GET_BYTE(Si[3],0)], j=3        ]
    }
    { R12 ^= XOR(R0,R1)                     //[So[2]=rkey[2]^IT0[]^IT1[]^IT2[]^IT3[]]
      R15 = TABLEIDXW(R8,#8,#8)             //[ &IT1[GET_BYTE(Si[2],1)], j=3        ]
      R0 = MEMW(R14)                        //[ IT0[GET_BYTE(Si[3],0)], j=3         ]
      R9 = R13                              //[ R9 = rkey[3]                        ]
    }
    { R1 = MEMW(R15)                        //[ IT1[GET_BYTE(Si[2],1)], j=3         ]
      R16 = TABLEIDXW(R7,#8,#16)            //[ &IT2[GET_BYTE(Si[1],2)], j=3        ]
      R17 = TABLEIDXW(R6,#8,#24)            //[ &IT3[GET_BYTE(Si[0],3)], j=3        ]
      R8 = R12                              //[ Si[2] = So[2]                       ]
    }
    { R9 ^= XOR(R0,R1)                      //[ rkey[3]^IT0[]^IT1[]                 ]
      R0 = MEMW(R16)                        //[ IT2[GET_BYTE(Si[1],2)], j=3         ]
      R1 = MEMW(R17)                        //[ IT3[GET_BYTE(Si[0],3)], j=3         ]
      R14 = TABLEIDXW(R10,#8,#0)            //[ &IT0[GET_BYTE(Si[0],0)], j=0        ]
    }:endloop0

.aes_decrypt_LastRound:
    { R9 ^= XOR(R0,R1)                      //[[e] Si[3]= So[3]=                    ]
                                            //[      rkey[3]^IT0[]^IT1[]^IT2[]^IT3[]]
      R7:6 = COMBINE(R11,R10)               //[[e] R7:6= Si[1:0] = So[1:0]          ]
      R19 = R18                             //[ R19 = ISBT                          ]
      R18 = TABLEIDXB(R10,#8,#0)            //[ &ISBT[GET_BYTE(Si[0],0)], j=0       ]
    }
    { R10 = MEMUB(R18)                      //[ read byte #0                        ]
      R18 = TABLEIDXB(R9,#8,#8)             //[ &ISBT[GET_BYTE(Si[3],1)], j=0       ]
    }
    { R1 = MEMUB(R18)                       //[ read byte #1                        ]
      R18 = TABLEIDXB(R8,#8,#16)            //[ &ISBT[GET_BYTE(Si[2],2)], j=0       ]
    }
    { R10 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R18)                       //[ read byte #2                        ]
      R18 = TABLEIDXB(R7,#8,#24)            //[ &ISBT[GET_BYTE(Si[1],3)], j=0       ]
    }
    { R2 = MEMUB(R18)                       //[ read byte #3                        ]
      R19 = TABLEIDXB(R7,#8,#0)             //[ &ISBT[GET_BYTE(Si[1],0)], j=1       ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R11 = MEMUB(R19)                      //[ read byte #0                        ]
      R19 = TABLEIDXB(R6,#8,#8)             //[ &ISBT[GET_BYTE(Si[0],1)], j=1       ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[0]                             ]
      R10 = COMBINE(R1.L,R10.L)             //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
      R1 = MEMUB(R19)                       //[ read byte #1                        ]
      R19 = TABLEIDXB(R9,#8,#16)            //[ &ISBT[GET_BYTE(Si[3],2)], j=1       ]
    }
    { R10 = XOR(R10,R4)                     //[ So[0]= PUT_BYTES()^rkey[0]          ]
      R11 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R19)                       //[ read byte #2                        ]
      R19 = TABLEIDXB(R8,#8,#24)            //[ &ISBT[GET_BYTE(Si[2],3)], j=1       ]
    }
    { R2 = MEMUB(R19)                       //[ read byte #3                        ]
      R18 = TABLEIDXB(R8,#8,#0)             //[ &ISBT[GET_BYTE(Si[2],0)], j=2       ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R12 = MEMUB(R18)                      //[ read byte #0                        ]
      R18 = TABLEIDXB(R7,#8,#8)             //[ &ISBT[GET_BYTE(Si[1],1)], j=2       ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[1]                             ]
      R11 = COMBINE(R1.L,R11.L)             //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
      R1 = MEMUB(R18)                       //[ read byte #1                        ]
      R18 = TABLEIDXB(R6,#8,#16)            //[ &ISBT[GET_BYTE(Si[0],2)], j=2       ]
    }
    { R11 = XOR(R11,R4)                     //[ So[1]=PUT_BYTES()^rkey[1]           ]
      R12 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R18)                       //[ read byte #2                        ]
      R18 = TABLEIDXB(R9,#8,#24)            //[ &ISBT[GET_BYTE(Si[3],3)], j=2       ]
    }
    { R2 = MEMUB(R18)                       //[ read byte #3                        ]
      R19 = TABLEIDXB(R9,#8,#0)             //[ &ISBT[GET_BYTE(Si[3],0)], j=3       ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R13 = MEMUB(R19)                      //[ read byte #0                        ]
      R19 = TABLEIDXB(R8,#8,#8)             //[ &ISBT[GET_BYTE(Si[2],1)], j=3       ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[2]                             ]
      R12 = COMBINE(R1.L,R12.L)             //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
      R1 = MEMUB(R19)                       //[ read byte #1                        ]
      R19 = TABLEIDXB(R7,#8,#16)            //[ &ISBT[GET_BYTE(Si[1],2)], j=3       ]
    }
    { R12 = XOR(R12,R4)                     //[ So[2]=PUT_BYTES()^rkey[2]           ]
      R13 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R19)                       //[ read byte #2                        ]
      R19 = TABLEIDXB(R6,#8,#24)            //[ &ISBT[GET_BYTE(Si[0],3)], j=3       ]
    }
    { R2 = MEMUB(R19)                       //[ read byte #3                        ]
      R13 |= ASL(R1,#16)                    //[ PUT_BYTES(byte0,byte1,byte2,..)     ]
    }
    { R4 = MEMW(R5++#4)                     //[ rkey[3]                             ]
      R13 |= ASL(R2,#24)                    //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
    }
    { MEMD(R3) = R11:10                     //[ OUT_UWORD32_LE(So[0 : 1])           ]
      R13 = XOR(R13,R4)                     //[ So[3]=PUT_BYTES()^rkey[3]           ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMD(R3+#8) = R13:12                  //[ OUT_UWORD32_LE(So[2 : 3])           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#2*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    aes_decrypt, .-aes_decrypt

