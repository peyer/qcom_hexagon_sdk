/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file    "aes2.S"
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
    /*[           - 17*nr + 7, where nr = 14, 12, 10                                ]*/
    /*[                                                                             ]*/
    /*[  Note:                                                                      ]*/
    /*[     This implementation requires less LUT(1024 bytes)                       ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl aes_encrypt2
    .type    aes_encrypt2, @function
aes_encrypt2:
    { R29 = ADD(R29,#-8)                    //[ allocate stack                      ]
      MEMD(R29+#(0-8)) = R17:16             //[ callee-saved registers              ]
      R4 = MEMW(R0++#8)                     //[ key_len                             ]
      R14 = R1                              //[ FT                                  ]
    }
    { R7:6 = MEMD(R2)                       //[ GET_UWORD32_LE(in+0:4)              ]
      R9:8 = MEMD(R2+#8)                    //[ GET_UWORD32_LE(in+8:12)             ]
      P0 = CMP.GT(R4,#16)                   //[ key_len > 16                        ]
      R1 = #9                               //[ nr-1 = 9                            ]
    }
    { R11:10 = MEMD(R0++#16)                //[ E_KEY[1:0]                          ]
      R13:12 = MEMD(R0+#8)                  //[ E_KEY[3:2]                          ]
      IF P0 R1 = #11                        //[ if (key_len > 16), nr-1 = 11        ]
      P0 = CMP.GT(R4,#24)                   //[ if (key_len >24)                    ]
    }
    { R7:6 = XOR(R7:6,R11:10)               //[ b0[1:0]                             ]
      R9:8 = XOR(R9:8,R13:12)               //[ b0[3:2]                             ]
      IF P0 R1 = #13                        //[ nr-1 = 13                           ]
      R17:16 = COMBINE(R3,R0)               //[ R17 = out, R16 = E_KE + 4           ]
    }
    { P3=SP1LOOP0(.aes_encrypt2RoundLoop,R1)//[ setup loop0: lc0 = nr-1             ]
      R14 = TABLEIDXW(R6,#8,#0)             //[[p] &FT[GET_BYTE(Si[0],0)], j=0      ]
    }
    
    .falign
.aes_encrypt2RoundLoop:
    { R0 = MEMW(R14)                        //[ FT[GET_BYTE(Si[0],0)], j=0          ]
      R1 = MEMW(R16++#4)                    //[ *rkey++                             ]
      R14 = TABLEIDXW(R7,#8,#8)             //[ &FT[GET_BYTE(Si[1],1)], j=0         ]
      IF P3 R9 = XOR(R2,R3)                 //[[2] Si[3]= So[3]= ..                 ]
    }
    { R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[1],1)], j=0          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R8,#8,#16)            //[ &FT[GET_BYTE(Si[2],2)], j=0         ]
    }
    { R1:0 ^= ASL(R5:4,#8)                  //[ ROTL32(FT[GET_BYTE(Si[1],1)],8), j=0]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[2],2)], j=0          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#24)            //[ &FT[GET_BYTE(Si[3],3)], j=0         ]
    }
    { R1:0 ^= ASL(R5:4,#16)                 //[ ROTL32(FT[GET_BYTE(Si[2],2)],16),j=0]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[3],3)], j=0          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R7,#8,#0)             //[ &FT[GET_BYTE(Si[1],0)], j=1         ]
    }
    { R1:0 ^= ASL(R5:4,#24)                 //[ ROTL32(FT[GET_BYTE(Si[3],3)],24),j=0]
      R2 = MEMW(R14)                        //[ FT[GET_BYTE(Si[1],0)], j=1          ]
      R3 = MEMW(R16++#4)                    //[ *rkey++                             ]
      R14 = TABLEIDXW(R8,#8,#8)             //[ &FT[GET_BYTE(Si[2],1)], j=1         ]
    }
    { R10 = XOR(R0,R1)                      //[ So[0]                               ]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[2],1)], j=1          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#16)            //[ &FT[GET_BYTE(Si[3],2)], j=1         ]
    }
    { R3:2 ^= ASL(R5:4,#8)                  //[ ROTL32(FT[GET_BYTE(Si[2],1)],8),j=1 ]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[3],2)], j=1          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R6,#8,#24)            //[ &FT[GET_BYTE(Si[0],3)], j=1         ]
    }
    { R3:2 ^= ASL(R5:4,#16)                 //[ ROTL32(FT[GET_BYTE(Si[3],2)],16),j=1]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[0],3)], j=1          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R8,#8,#0)             //[ &FT[GET_BYTE(Si[2],0)], j=2         ]
    }
    { R3:2 ^= ASL(R5:4,#24)                 //[ ROTL32(FT[GET_BYTE(Si[0],3)],24),j=1]
      R0 = MEMW(R14)                        //[ FT[GET_BYTE(Si[2],0)], j=2          ]
      R1 = MEMW(R16++#4)                    //[ *rkey++                             ]
      R14 = TABLEIDXW(R9,#8,#8)             //[ &FT[GET_BYTE(Si[3],1)], j=2         ]
    }
    { R11 = XOR(R2,R3)                      //[ So[1]                               ]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[3],1)], j=2          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R6,#8,#16)            //[ &FT[GET_BYTE(Si[0],2)], j=2         ]
    }
    { R1:0 ^= ASL(R5:4,#8)                  //[ ROTL32(FT[GET_BYTE(Si[3],1)],8),j=2 ]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[0],2)], j=2          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R7,#8,#24)            //[ &FT[GET_BYTE(Si[1],3)], j=2         ]
    }
    { R1:0 ^= ASL(R5:4,#16)                 //[ ROTL32(FT[GET_BYTE(Si[0],2)],16),j=2]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[1],3)], j=2          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#0)             //[ &FT[GET_BYTE(Si[3],0)], j=3         ]
    }
    { R1:0 ^= ASL(R5:4,#24)                 //[ ROTL32(FT[GET_BYTE(Si[1],3)],24),j=2]
      R2 = MEMW(R14)                        //[ FT[GET_BYTE(Si[3],0)], j=3          ]
      R3 = MEMW(R16++#4)                    //[ *rkey++                             ]
      R14 = TABLEIDXW(R6,#8,#8)             //[ &FT[GET_BYTE(Si[0],1)], j=3         ]
    }
    { R12 = XOR(R0,R1)                      //[ So[2]                               ]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[0],1)], j=3          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R7,#8,#16)            //[ &FT[GET_BYTE(Si[1],2)], j=3         ]
    }
    { R3:2 ^= ASL(R5:4,#8)                  //[ ROTL32(FT[GET_BYTE(Si[0],1)],8), j=3]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[1],2)], j=3          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R8,#8,#24)            //[ &FT[GET_BYTE(Si[2],3)], j=3         ]
    }
    { R3:2 ^= ASL(R5:4,#16)                 //[ ROTL32(FT[GET_BYTE(Si[1],2)],16),j=3]
      R4 = MEMW(R14)                        //[ FT[GET_BYTE(Si[2],3)], j=3          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R10,#8,#0)            //[ &FT[GET_BYTE(Si[0],0)], j=0         ]
    }
    { R3:2 ^= ASL(R5:4,#24)                 //[ ROTL32(FT[GET_BYTE(Si[2],3)],24),j=3]
      R7:6 = R11:10                         //[ Si[1:0] = So[1:0]                   ]
      R8 = R12                              //[                                     ]
    }:endloop0

.aes_encrypt2_LastRound:
    { R9 = XOR(R2,R3)                       //[[e] Si[3] = So[3] =                  ]
                                            //[      rkey^FT0[]^FT1[]^FT2[]^FT3[]   ]
      R10 = MEMUB(R14+#1)                   //[ read byte #0                        ]
      R14 = TABLEIDXW(R7,#8,#8)             //[ &FT0[GET_BYTE(Si[1],1)], j=0        ]
    }
    { R15 = R14                             //[ R15 = &FT0[GET_BYTE(Si[1],1)], j=0  ]
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
    { R4 = MEMW(R16++#4)                    //[ rkey[0]                             ]
      R10 = COMBINE(R1.L,R10.L)             //[ PUT_BYTES(byte0, byte1, byte2,byte3)]
      R1 = MEMUB(R15+#1)                    //[ read byte #1                        ]
      R15 = TABLEIDXW(R9,#8,#16)            //[ &FT0[GET_BYTE(Si[3],2)], j=1        ]
    }
    { R10 = XOR(R10,R4)                     //[ So[0]= PUT_BYTES()^rkey[0]          ]
      R11 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0, byte1, ....), j=0  ]
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
    { R4 = MEMW(R16++#4)                    //[ rkey[1]
      R11 = COMBINE(R1.L,R11.L)             //[ PUT_BYTES(byte0,byte1,..,byte3),j=1 ]
      R1 = MEMUB(R14+#1)                    //[ read byte #1                        ]
      R14 = TABLEIDXW(R6,#8,#16)            //[ &FT0[GET_BYTE(Si[0],2)], j=2        ]
    }
    { R11 = XOR(R11,R4)                     //[ So[1]= PUT_BYTES()^rkey[1]          ]
      R12 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,....)         ]
      R1 = MEMUB(R14+#1)                    //[ read byte #2                        ]
      R14 = TABLEIDXW(R7,#8,#24)            //[ &FT0[GET_BYTE(Si[1],3)], j=2        ]
    }
    { R2 = MEMUB(R14+#1)                    //[ read byte #3                        ]
      R15 = TABLEIDXW(R9,#8,#0)             //[ &FT0[GET_BYTE(Si[3],0)], j=3        ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R13 = MEMUB(R15+#1)                   //[ read byte #0                        ]
      R15 = TABLEIDXW(R6,#8,#8)             //[ &FT0[GET_BYTE(Si[0],1)], j=3        ]
    }
    { R4 = MEMW(R16++#4)                    //[ rkey[2]                             ]
      R12 = COMBINE(R1.L,R12.L)             //[ PUT_BYTES(byte0,byte1,..,byte3), j=2]
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
    { R4 = MEMW(R16++#4)                    //[ rkey[3]                             ]
      R13 |= ASL(R2,#24)                    //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
    }
    { MEMD(R17) = R11:10                    //[ OUT_UWORD32_LE(So[0 : 1])           ]
      R13 = XOR(R13,R4)                     //[ So[3]= PUT_BYTES()^rkey[3]          ]
    }
    { MEMD(R17+#8) = R13:12                 //[ OUT_UWORD32_LE(So[2 : 3])           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#8 )                    //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    aes_encrypt2, .-aes_encrypt2




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
    /*[  Register Usage: R0-R17                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 17*nr + 9, where nr = 14, 12, 10                                ]*/
    /*[                                                                             ]*/
    /*[  Note:                                                                      ]*/
    /*[     This implementation requires less LUT(1024 bytes)                       ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl aes_decrypt2
    .type    aes_decrypt2, @function
aes_decrypt2:
    { R29 = ADD(R29,#-8)                    //[ allocate stack                      ]
      R4 = MEMW(R0)                         //[ key_len                             ]
      R14 = R1                              //[ IT                                  ]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R16 = ADD(R0,#328)                    //[ D_KEY+20                            ]
      R17 = R3                              //[ save out in R17                     ]
    }
    { R7:6 = MEMD(R2)                       //[ GET_UWORD32_LE(in+ 0:4)             ]
      R9:8 = MEMD(R2+#8)                    //[ GET_UWORD32_LE(in+ 8:12)            ]
      P0 = CMP.GT(R4,#16)                   //[ (key_len>16)?                       ]
      R0 += ASL(R4,#2)                      //[ &ctx+ key_len                       ]
    }
    { R11:10 = MEMD(R0+#104)                //[ E_KEY[key_len+24:25]                ]
      R13:12 = MEMD(R0+#112)                //[ E_KEY[key_len+26:27]                ]
      R16 += ASL(R4,#2)                     //[ rkey = D_KEY+key_len+20             ]
      R1 = MUX(P0,#11,#9)                   //[ if(key_len>16) nr-1=11, else nr-1=9 ]
    }
    { R7:6 = XOR(R7:6,R11:10)               //[ b0[1]:b[0]                          ]
      R9:8 = XOR(R9:8,R13:12)               //[ b0[3]:b[2]                          ]
      P0 = CMP.GT(R4,#24)                   //[ (key_len>24)?                       ]
      IF P0.new R1 = #13                    //[ if(key_len>24) nr-1 = 13            ]
    }
    { R15 = ADD(R14,#1024)                  //[ ISBT = DecryptTAB+ 256              ]
      R14 = TABLEIDXW(R6,#8,#0)             //[ &IT[GET_BYTE(Si[0],0)], j=0         ]
      P3=SP1LOOP0(.aes_decrypt2RoundLoop,R1)//[ setup loop0: lc0 = nr-1             ]
    }
    
    .falign
.aes_decrypt2RoundLoop:
    { R0 = MEMW(R14)                        //[ IT[GET_BYTE(Si[0],0)], j=0          ]
      R1 = MEMW(R16++#4)                    //[ rkey[0]                             ]
      R14 = TABLEIDXW(R8,#8,#16)            //[ &IT[GET_BYTE(Si[2],2)], j=1         ]
      IF P3 R9 = XOR(R2,R3)                 //[[2] Si[3]= So[3]= ..                 ]
    }
    { R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[2],2)], j=0          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#8)             //[ &IT[GET_BYTE(Si[3],1)], j=0         ]
    }
    { R1:0 ^= ASL(R5:4,#16)                 //[ ROTL32(IT[GET_BYTE(Si[2],2)],16),j=0]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[3],1)], j=0          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R7,#8,#24)            //[ &IT[GET_BYTE(Si[1],3)], j=0         ]
    }
    { R1:0 ^= ASL(R5:4,#8)                  //[ ROTL32(IT[GET_BYTE(Si[3],1)],8),j=0 ]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[1],3)], j=0          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R7,#8,#0)             //[ &IT[GET_BYTE(Si[1],0)], j=1         ]
    }
    { R1:0 ^= ASL(R5:4,#24)                 //[ ROTL32(IT[GET_BYTE(Si[1],3)],24),j=0]
      R2 = MEMW(R14)                        //[ IT[GET_BYTE(Si[1],0)], j=1          ]
      R3 = MEMW(R16++#4)                    //[ rkey[1]                             ]
      R14 = TABLEIDXW(R6,#8,#8)             //[ &IT[GET_BYTE(Si[0],1)], j=1         ]
    }
    { R10 = XOR(R0,R1)                      //[ So[0]                               ]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[0],1)], j=1          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#16)            //[ &IT[GET_BYTE(Si[3],2)], j=1         ]
    }
    { R3:2 ^= ASL(R5:4,#8)                  //[ ROTL32(IT[GET_BYTE(Si[0],1)],8), j=1]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[3],2)], j=1          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R8,#8,#24)            //[ &IT[GET_BYTE(Si[2],3)], j=1         ]
    }
    { R3:2 ^= ASL(R5:4,#16)                 //[ ROTL32(IT[GET_BYTE(Si[3],2)],16),j=1]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[2],3)], j=1          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R8,#8,#0)             //[ &IT[GET_BYTE(Si[2],0)], j=2         ]
    }
    { R3:2 ^= ASL(R5:4,#24)                 //[ ROTL32(IT[GET_BYTE(Si[2],3)],24),j=1]
      R0 = MEMW(R14)                        //[ IT[GET_BYTE(Si[2],0)], j=2          ]
      R1 = MEMW(R16++#4)                    //[ rkey[2]                             ]
      R14 = TABLEIDXW(R7,#8,#8)             //[ &IT[GET_BYTE(Si[1],1)], j=2         ]
    }
    { R11 = XOR(R2,R3)                      //[ So[1]                               ]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[1],1)], j=2          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R6,#8,#16)            //[ &IT[GET_BYTE(Si[0],2)], j=2         ]
    }
    { R1:0 ^= ASL(R5:4,#8)                  //[ ROTL32(IT[GET_BYTE(Si[1],1)],8), j=2]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[0],2)], j=2          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#24)            //[ &IT[GET_BYTE(Si[3],3)], j=2         ]
    }
    { R1:0 ^= ASL(R5:4,#16)                 //[ ROTL32(IT[GET_BYTE(Si[0],2)],16),j=2]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[3],3)], j=2          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R9,#8,#0)             //[ &IT[GET_BYTE(Si[3],0)], j=3         ]
    }
    { R1:0 ^= ASL(R5:4,#24)                 //[ ROTL32(IT[GET_BYTE(Si[3],3)],24),j=2]
      R2 = MEMW(R14)                        //[ IT[GET_BYTE(Si[3],0)], j=3          ]
      R3 = MEMW(R16++#-(12+16))             //[ rkey[3]; rkey-=4                    ]
      R14 = TABLEIDXW(R8,#8,#8)             //[ &IT[GET_BYTE(Si[2],1)], j=3         ]
    }
    { R8 = XOR(R0,R1)                       //[ So[2]                               ]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[2],1)], j=3          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R7,#8,#16)            //[ &IT[GET_BYTE(Si[1],2)], j=3         ]
    }
    { R3:2 ^= ASL(R5:4,#8)                  //[ ROTL32(IT[GET_BYTE(Si[2],1)],8), j=3]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[1],2)], j=3          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R6,#8,#24)            //[ &IT[GET_BYTE(Si[0],3)], j=3         ]
    }
    { R3:2 ^= ASL(R5:4,#16)                 //[ ROTL32(IT[GET_BYTE(Si[1],2)],16),j=3]
      R4 = MEMW(R14)                        //[ IT[GET_BYTE(Si[0],3)], j=3          ]
      R5 = #0                               //[                                     ]
      R14 = TABLEIDXW(R10,#8,#0)            //[ &IT[GET_BYTE(Si[0],0)], j=0         ]
    }
    { R3:2 ^= ASL(R5:4,#24)                 //[ ROTL32(IT[GET_BYTE(Si[0],3)],24),j=3]
      R7:6 = R11:10                         //[ Si[1:0] = So[1:0]                   ]
    }:endloop0

.aes_decrypt2_LastRound:
    { R9 = XOR(R2,R3)                       //[[e] Si[3]= So[3]= ..                 ]
      R14 = R15                             //[ R14 = ISBT                          ]
      R15 = TABLEIDXB(R6,#8,#0)             //[ &ISBT[GET_BYTE(Si[0],0)], j=0       ]
    }
    { R10 = MEMUB(R15)                      //[ read byte #0                        ]
      R15 = TABLEIDXB(R9,#8,#8)             //[ &ISBT[GET_BYTE(Si[3],1)], j=0       ]
    }
    { R1 = MEMUB(R15)                       //[ read byte #1                        ]
      R15 = TABLEIDXB(R8,#8,#16)            //[ &ISBT[GET_BYTE(Si[2],2)], j=0       ]
    }
    { R10 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R15)                       //[ read byte #2                        ]
      R15 = TABLEIDXB(R7,#8,#24)            //[ &ISBT[GET_BYTE(Si[1],3)], j=0       ]
    }
    { R2 = MEMUB(R15)                       //[ read byte #3                        ]
      R14 = TABLEIDXB(R7,#8,#0)             //[ &ISBT[GET_BYTE(Si[1],0)], j=1       ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R11 = MEMUB(R14)                      //[ read byte #0                        ]
      R14 = TABLEIDXB(R6,#8,#8)             //[ &ISBT[GET_BYTE(Si[0],1)], j=1       ]
    }
    { R4 = MEMW(R16++#4)                    //[ rkey[0]                             ]
      R10 = COMBINE(R1.L,R10.L)             //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
      R1 = MEMUB(R14)                       //[ read byte #1                        ]
      R14 = TABLEIDXB(R9,#8,#16)            //[ &ISBT[GET_BYTE(Si[3],2)], j=1       ]
    }
    { R10 = XOR(R10,R4)                     //[ So[0]= PUT_BYTES()^rkey[0]          ]
      R11 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14)                       //[ read byte #2                        ]
      R14 = TABLEIDXB(R8,#8,#24)            //[ &ISBT[GET_BYTE(Si[2],3)], j=1       ]
    }
    { R2 = MEMUB(R14)                       //[ read byte #3                        ]
      R15 = TABLEIDXB(R8,#8,#0)             //[ &ISBT[GET_BYTE(Si[2],0)], j=2       ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R12 = MEMUB(R15)                      //[ read byte #0                        ]
      R15 = TABLEIDXB(R7,#8,#8)             //[ &ISBT[GET_BYTE(Si[1],1)], j=2       ]
    }
    { R4 = MEMW(R16++#4)                    //[ rkey[1]                             ]
      R11 = COMBINE(R1.L,R11.L)             //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
      R1 = MEMUB(R15)                       //[ read byte #1                        ]
      R15 = TABLEIDXB(R6,#8,#16)            //[ &ISBT[GET_BYTE(Si[0],2)], j=2       ]
    }
    { R11 = XOR(R11,R4)                     //[ So[1]=PUT_BYTES()^rkey[1]           ]
      R12 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R15)                       //[ read byte #2                        ]
      R15 = TABLEIDXB(R9,#8,#24)            //[ &ISBT[GET_BYTE(Si[3],3)], j=2       ]
    }
    { R2 = MEMUB(R15)                       //[ read byte #3                        ]
      R14 = TABLEIDXB(R9,#8,#0)             //[ &ISBT[GET_BYTE(Si[3],0)], j=3       ]
    }
    { R1 |= ASL(R2,#8)                      //[ PUT_BYTES(...,byte2,byte3)          ]
      R13 = MEMUB(R14)                      //[ read byte #0                        ]
      R14 = TABLEIDXB(R8,#8,#8)             //[ &ISBT[GET_BYTE(Si[2],1)], j=3       ]
    }
    { R4 = MEMW(R16++#4)                    //[ rkey[2]                             ]
      R12 = COMBINE(R1.L,R12.L)             //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
      R1 = MEMUB(R14)                       //[ read byte #1                        ]
      R14 = TABLEIDXB(R7,#8,#16)            //[ &ISBT[GET_BYTE(Si[1],2)], j=3       ]
    }
    { R12 = XOR(R12,R4)                     //[ So[2]=PUT_BYTES()^rkey[2]           ]
      R13 |= ASL(R1,#8)                     //[ PUT_BYTES(byte0,byte1,...)          ]
      R1 = MEMUB(R14)                       //[ read byte #2                        ]
      R14 = TABLEIDXB(R6,#8,#24)            //[ &ISBT[GET_BYTE(Si[0],3)], j=3       ]
    }
    { R2 = MEMUB(R14)                       //[ read byte #3                        ]
      R13 |= ASL(R1,#16)                    //[ PUT_BYTES(byte0,byte1,byte2,..)     ]
    }
    { R4 = MEMW(R16++#4)                    //[ rkey[3]                             ]
      R13 |= ASL(R2,#24)                    //[ PUT_BYTES(byte0,byte1,byte2,byte3)  ]
    }
    { MEMD(R17) = R11:10                    //[ OUT_UWORD32_LE(So[0 : 1])           ]
      R13 = XOR(R13,R4)                     //[ So[3]=PUT_BYTES()^rkey[3]           ]
    }
    { MEMD(R17+#8) = R13:12                 //[ OUT_UWORD32_LE(So[2 : 3])           ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#8)                     //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    aes_decrypt2, .-aes_decrypt2

