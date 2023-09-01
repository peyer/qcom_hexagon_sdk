/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "kasumi.S"

    /*[*****************************************************************************]*/
    /*[  Function   : UWord64 FOI()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: FO/FI function transform a 32-bit value                       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 in                                               ]*/
    /*[               R1 : UWord16 *KOItable                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R13                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 27                                                              ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .type    FOI, @function
FOI:
    { R7 = MEMUH(R1)                        //[ (*KOIitable)                        ]
      R2 = ADD(R1,#2)                       //[ R2 = KOIitable + 1                  ]
      R1:0 = VZXTHW(R0)                     //[ x = in >> 16; y = (UWord16)in       ]
      R12.H = #HI(S7)                       //[ R12 = S7                            ]
    }
    { R3 = MEMUH(R2++#2)                    //[ subkey = (*KOIitable)               ]
      R1 = XOR(R1,R7)                       //[ x ^= (*KOIitable)                   ]
      R12.L = #LO(S7)                       //[ R12 = S7                            ]
      R13.L = #LO(S9)                       //[ R13 = S9                            ]
    }
    { R6 = AND(R1,#0x7F)                    //[ seven1 = x & 0x7F                   ]
      R7 = EXTRACTU(R1,#9,#7)               //[ nine1 = x>>7                        ]
      R1 = R0                               //[ x = y                               ]
      R13.H = #HI(S9)                       //[ R13 = S9                            ]
    }
    { R8 = ADDASL(R13,R7,#1)                //[ &S9[nine1]                          ]
    }
    { R7 = MEMUH(R8)                        //[ S9[nine1]                           ]
      R9 = ADD(R12,R6)                      //[ S7[seven1]                          ]
      R5 = AND(R3,#0x1FF)                   //[ subkey & 0x1FF                      ]
      R3 = LSR(R3,#9)                       //[ subkey>>9                           ]
    }
    { R4 = MEMUB(R9)                        //[ S7[seven1]                          ]
      R7  = XOR(R7,R6)                      //[ nine1=S9[x>>7]^seven1               ]
      R5 ^= XOR(R7,R6)                      //[ nine2=S9[x>>7]^seven1 ^             ]
                                            //[                  (subkey &0x1FF)    ]
      LOOP0(.FOI_LOOP,#2)                   //[ setup loop0: lc0=2                  ]
    }
    { R7 = ADDASL(R13,R5,#1)                //[ &S9[nine2]                          ]
      R3 ^= XOR(R4,R7)                      //[ R3=S7[seven1] ^ nine1 ^ (subkey >>9)]
    }

    .falign
.FOI_LOOP:
    { R7 = MEMUH(R2++#2)                    //[[1](*KOIitable)                      ]
      R8 = MEMUH(R7)                        //[[2]S9[nine2]                         ]
      R3 = AND(R3,#0x7F)                    //[[2]seven2 = R3                       ]
    }
    { R1 = XOR(R1,R7)                       //[[1]x ^= (*KOIitable)                 ]
      R0 = R1                               //[[2]y = x                             ]
      R11 = XOR(R8,R3)                      //[[2]nine3 = S9[nine2] ^ seven2        ]
      R9 = ADD(R12,R3)                      //[[2]&S7[seven2]                       ]
    }
    { R3 = MEMUH(R2++#2)                    //[[1]subkey = (*KOIitable)             ]
      R7 = EXTRACTU(R1,#9,#7)               //[[1]x>>7                              ]
      R10 = MEMUB(R9)                       //[[2]S7[seven2]                        ]
      R8 = AND(R11,#0x7F)                   //[[2]nine3 & 0x7F                      ]
    }
    { R6 = AND(R1,#0x7F)                    //[[1]seven1 = x & 0x7F                 ]
      R7 = ADDASL(R13,R7,#1)                //[[1]&S9[nine1]                        ]
      R10 = XOR(R10,R8)                     //[[2]seven3 = S7[seven2] ^ (nine3&0x7F)]
    }
    { R7 = MEMUH(R7)                        //[[1]S9[nine1]                         ]
      R9 = ADD(R12,R6)                      //[[1]&S7[seven1]                       ]
      R5 = AND(R3,#0x1FF)                   //[[1]subkey & 0x1FF                    ]
      R3 = LSR(R3,#9)                       //[[1]subkey>>9                         ]
    }
    { R4 = MEMUB(R9)                        //[[1]S7[seven1]                        ]
      R7  = XOR(R7,R6)                      //[[1]nine1 = S9[nine1]^seven1          ]
      R5 ^= XOR(R7,R6)                      //[[1]nine2 = S9[nine1]^seven1 ^        ]
                                            //[                    (subkey &0x1FF)  ]
      R11 |= ASL(R10,#9)                    //[[2]temp = (seven4 << 9) + nine4      ]
    }
    { R7 = ADDASL(R13,R5,#1)                //[[1]&S9[nine2]                        ]
      R3 ^= XOR(R4,R7)                      //[[1]R3=S7[seven1]^nine1^(subkey >>9)  ]
      R1 = XOR(R0,R11)                      //[[2]x = y ^= temp                     ]
    }:endloop0

    { R8 = MEMUH(R7)                        //[ S9[nine2]                           ]
      R3 = AND(R3,#0x7F)                    //[ seven2 = R3                         ]
    }
    { R11 = XOR(R8,R3)                      //[ nine3 = S9[nine2] ^ seven2          ]
      R9 = ADD(R12,R3)                      //[ &S7[seven2]                         ]
    }
    { R10 = MEMUB(R9)                       //[ S7[seven2]                          ]
      R8 = AND(R11,#0x7F)                   //[ nine3 & 0x7F                        ]
      R1 = COMBINE(R1.L,R1.L)               //[ pack x in R1.H                      ]
    }
    { R10 = XOR(R10,R8)                     //[ seven3 = S7[seven3] ^ (nine3&0x7F)  ]
    }
    { R11 |= ASL(R10,#9)                    //[ temp = (seven3 << 9) + nine3        ]
    }
    { R0 = XOR(R1,R11)                      //[ R0.H= x,R0.L= (y ^ temp)            ]
      JUMPR R31                             //[ return                              ]
    }
    .size    FOI, .-FOI


    .data
    .p2align 3
    .type    S7, @object
    .size    S7, 128
S7:
    .byte     54, 50, 62, 56, 22, 34, 94, 96, 38,  6, 63, 93,  2, 18,123,33
    .byte     55,113, 39,114, 21, 67, 65, 12, 47, 73, 46, 27, 25,111,124,81
    .byte     53,  9,121, 79, 52, 60, 58, 48,101,127, 40,120,104, 70, 71,43
    .byte     20,122, 72, 61, 23,109, 13,100, 77,  1, 16,  7, 82, 10,105,98
    .byte    117,116, 76, 11, 89,106,  0,125,118, 99, 86, 69, 30, 57,126,87
    .byte    112, 51, 17,  5, 95, 14, 90, 84, 91,  8, 35,103, 32, 97, 28,66
    .byte    102, 31, 26, 45, 75,  4, 85, 92, 37, 74, 80, 49, 68, 29,115,44
    .byte     64,107,108, 24,110, 83, 36, 78, 42, 19, 15, 41, 88,119, 59, 3



    .p2align 3
    .type    S9, @object
    .size    S9, 1024
S9:
    .hword    167,239,161,379,391,334,  9,338, 38,226, 48,358,452,385, 90,397
    .hword    183,253,147,331,415,340, 51,362,306,500,262, 82,216,159,356,177
    .hword    175,241,489, 37,206, 17,  0,333, 44,254,378, 58,143,220, 81,400
    .hword     95,  3,315,245, 54,235,218,405,472,264,172,494,371,290,399, 76
    .hword    165,197,395,121,257,480,423,212,240, 28,462,176,406,507,288,223
    .hword    501,407,249,265, 89,186,221,428,164, 74,440,196,458,421,350,163
    .hword    232,158,134,354, 13,250,491,142,191, 69,193,425,152,227,366,135
    .hword    344,300,276,242,437,320,113,278, 11,243, 87,317, 36, 93,496, 27
    .hword    487,446,482, 41, 68,156,457,131,326,403,339, 20, 39,115,442,124
    .hword    475,384,508, 53,112,170,479,151,126,169, 73,268,279,321,168,364
    .hword    363,292, 46,499,393,327,324, 24,456,267,157,460,488,426,309,229
    .hword    439,506,208,271,349,401,434,236, 16,209,359, 52, 56,120,199,277
    .hword    465,416,252,287,246,  6, 83,305,420,345,153,502, 65, 61,244,282
    .hword    173,222,418, 67,386,368,261,101,476,291,195,430, 49, 79,166,330
    .hword    280,383,373,128,382,408,155,495,367,388,274,107,459,417, 62,454
    .hword    132,225,203,316,234, 14,301, 91,503,286,424,211,347,307,140,374
    .hword     35,103,125,427, 19,214,453,146,498,314,444,230,256,329,198,285
    .hword     50,116, 78,410, 10,205,510,171,231, 45,139,467, 29, 86,505, 32
    .hword     72, 26,342,150,313,490,431,238,411,325,149,473, 40,119,174,355
    .hword    185,233,389, 71,448,273,372, 55,110,178,322, 12,469,392,369,190
    .hword      1,109,375,137,181, 88, 75,308,260,484, 98,272,370,275,412,111
    .hword    336,318,  4,504,492,259,304, 77,337,435, 21,357,303,332,483, 18
    .hword     47, 85, 25,497,474,289,100,269,296,478,270,106, 31,104,433, 84
    .hword    414,486,394, 96, 99,154,511,148,413,361,409,255,162,215,302,201
    .hword    266,351,343,144,441,365,108,298,251, 34,182,509,138,210,335,133
    .hword    311,352,328,141,396,346,123,319,450,281,429,228,443,481, 92,404
    .hword    485,422,248,297, 23,213,130,466, 22,217,283, 70,294,360,419,127
    .hword    312,377,  7,468,194,  2,117,295,463,258,224,447,247,187, 80,398
    .hword    284,353,105,390,299,471,470,184, 57,200,348, 63,204,188, 33,451
    .hword     97, 30,310,219, 94,160,129,493, 64,179,263,102,189,207,114,402
    .hword    438,477,387,122,192, 42,381,  5,145,118,180,449,293,323,136,380
    .hword     43, 66, 60,455,341,445,202,432, 8,237, 15,376,436,464, 59,461




    /*[*****************************************************************************]*/
    /*[  Function   : void kasumi()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: kasumi block cipher                                           ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input array is aligned by 8bytes                                ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *data                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R6, R24-R27                                             ]*/
    /*[  Hardware Loops affected: Loop1                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 24+8                              ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 283                                                             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl Kasumi
    .type    Kasumi, @function
Kasumi:
    { ALLOCFRAME(#24)                       //[ allocate stack                      ]
      R3:2 = MEMD(R0)                       //[ load *(data+4) : *data              ]
      LOOP1(.kasumi_LOOP,#4)                //[ setup loop0: lc0 = 4                ]
      R6.H = #HI(KLi)                       //[ R6 = KLi                            ]
    }
    { MEMD(R29+#0) = R25:24                 //[ callee-saved registers              ]
      R25 = SWIZ(R3)                        //[ endian swap, R25 = right            ]
      R24 = SWIZ(R2)                        //[ endian swap, R24 = left             ]
      R6.L = #LO(KLi)                       //[ R6 = KLi                            ]
    }
    { MEMD(R29+#8) = R27:26                 //[ callee-saved registers              ]
      R26.H = #HI(KOIi)                     //[ R26 = KOIi                          ]
      R3 = MEMUH(R6)                        //[ (*KLitable)                         ]
      R27 = ADD(R6,#2)                      //[ R27 = KLi+1                         ]
    }
    { MEMW(R29+#16) = R0                    //[ push data to stack                  ]
      R26.L = #LO(KOIi)                     //[ R26 = KOIi                          ]
    }

    .falign
.kasumi_LOOP:
    /* --------------------------------------------------------------------------- */
    /*                    FL(left, KLitable)                                       */
    /* --------------------------------------------------------------------------- */
    { R6 = MEMUH(R27++#2)                   //[ load *KLitable                      ]
      R3 &= LSR(R24,#16)                    //[ a = l & (*KLitable)                 ]
      R1:0 = VZXTHW(R24)                    //[                                     ]
      R4.H = #0x1                           //[                                     ]
    }
    { R2 = ASL(R3,#1)                       //[ ROL16(a,1)                          ]
      R3 = LSR(R3,#15)                      //[ ROL16(a,1)                          ]
      R4.L = #0                             //[                                     ]
    }
    { R0 ^= XOR(R2,R3)                      //[ r ^= ROL16(a,1)                     ]
    }
    { R3 = OR(R6,R0)                        //[ b = r | (*KLitable)                 ]
    }
    { R2  = ASL(R3,#17)                     //[ ROL16(b,1) (in R2.H)                ]
      R4 &= ASL(R3,#1)                      //[ ROL16(b,1) (in R4.H)                ]
      R0 = COMBINE(R1.L,R0.L)               //[ pack R0.H = l, R0.L= r              ]
    }
    { R0 ^= XOR(R2,R4)                      //[ R0.H: l ^= ROL16(b,1);              ]
                                            //[       ((UWord32)l<<16) | r          ]
      R1 = R26                              //[                                     ]
      CALL FOI                              //[ temp = FOI(temp, KOIitable)         ]
      R26 = ADD(R26,#12)                    //[ KOIitable +=6                       ]
    }
    { R25 = XOR(R25,R0)                     //[ right ^= temp                       ]
      R0  = XOR(R25,R0)                     //[ func call:                          ]
      R1 = R26                              //[ func call:                          ]
      CALL FOI                              //[ FOI(right, KOIitable)               ]
    }
    /* --------------------------------------------------------------------------- */
    /*                    FL(temp, KLitable)                                       */
    /* --------------------------------------------------------------------------- */
    { R3 = MEMUH(R27++#2)                   //[ (*KLitable)                         ]
      R1:0 = VZXTHW(R0)                     //[                                     ]
      R26 = ADD(R26,#12)                    //[ KOIitable +=6                       ]
      R4.H = #0x1                           //[                                     ]
    }
    { R6 = MEMUH(R27++#2)                   //[ load *KLitable                      ]
      R3 = AND(R3,R1)                       //[ a = l & (*KLitable)                 ]
      R4.L = #0                             //[                                     ]
    }
    { R2 = ASL(R3,#1)                       //[ ROL16(a,1)                          ]
      R3 = LSR(R3,#15)                      //[ ROL16(a,1)                          ]
    }
    { R0 ^= XOR(R2,R3)                      //[ r ^= ROL16(a,1)                     ]
    }
    { R3 = OR(R6,R0)                        //[ b = r | (*KLitable)                 ]
    }
    { R2  = ASL(R3,#17)                     //[ ROL16(b,1) (in R2.H)                ]
      R4 &= ASL(R3,#1)                      //[ ROL16(b,1) (in R4.H)                ]
      R0 = COMBINE(R1.L,R0.L)               //[ pack R0.H = l, R0.L= r              ]
    }
    { R0 ^= XOR(R2,R4)                      //[ R0.H: l ^= ROL16(b,1);              ]
                                            //[       ((UWord32)l<<16) | r          ]
    }
    { R3 = MEMUH(R27++#2)                   //[ (*KLitable)                         ]
      R24 = XOR(R24,R0)                     //[ left ^= temp                        ]
    }:endloop1

    { R4 = MEMW(R29+#16)                    //[ *data                               ]
      R1 = SWIZ(R25)                        //[ endian swap                         ]
      R0 = SWIZ(R24)                        //[ endian swap                         ]
    }
    { R27:26 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      DEALLOCFRAME                          //[ pop stack                           ]
      R2 = R29                              //[                                     ]
    }
    { R25:24 = MEMD(R2+#0)                  //[ restore callee-saved registers      ]
      MEMD(R4) = R1:0                       //[ save out                            ]
      JUMPR R31                             //[ return                              ]
    }
    .size    Kasumi, .-Kasumi




    /*[*****************************************************************************]*/
    /*[  Function   : void KeySckedule()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: build the key schedule                                        ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input array is aligned by 4bytes                                ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *k                                                ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R17                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 8+32                              ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 103                                                             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl KeySchedule
    .type    KeySchedule, @function
KeySchedule:
    { R29 = ADD(R29,#-(32+8))               //[ 32bytes for key[n], kprime[n]       ]
      MEMD(R29+#(32-(32+8))) = R17:16       //[ callee-saved registers              ]
      R4.H = #HI(C.0)                       //[ R4 = C.0                            ]
      P3 = SP2LOOP0(.BuildKey_LOOP,#4)      //[ setup loop0: lc0= 4                 ]
    }
    { R6 = R29                              //[ R6 = Kprime                         ]
      R5 = ADD(R29,#16)                     //[ R5 = key                            ]
      R4.L = #LO(C.0)                       //[ R4 = C.0                            ]
      R1 = MEMW(R0++#4)                     //[[p]load k[3:2:1:0]                   ]
    }
    { R13.H = #HI(KLi)                      //[ R13 = KLi                           ]
      R12.H = #HI(KOIi)                     //[ R12 = KOIi                          ]
      R3 = MEMW(R4++#4)                     //[[p]load C[n+1:n]                     ]
      R2 = SWIZ(R1)                         //[[p]endian swap k[]                   ]
    }
    { R13.L = #LO(KLi)                      //[ R13 = KLi                           ]
      R12.L = #LO(KOIi)                     //[ R12 = KOIi                          ]
      R1 = MEMW(R0++#4)                     //[[p]load k[7:6:5:4]                   ]
      R2 = COMBINE(R2.L,R2.H)               //[[p] swap                             ]
    }

    .falign
.BuildKey_LOOP:
    { R2 = SWIZ(R1)                         //[[2]endian swap k[]                   ]
      R3 = MEMW(R4++#4)                     //[[2]load C[n+1:n]                     ]
      MEMW(R5++#4) = R2                     //[[3]save key[n+1:n]                   ]
      R7 = XOR(R2,R3)                       //[[3] Kprim[n]= key[n] ^C[n]           ]
    }
    { IF !P3 R1 = MEMW(R0++#4)              //[[1]load k[2*n+3:2*n+2:2*n+1:2*n]     ]
      R2 = COMBINE(R2.L,R2.H)               //[[2]swap key[n+1], key[n]             ]
      MEMW(R6++#4) = R7                     //[[3]save Kprime[n+1:n]                ]
      NOP                                   //[                                     ]
    }:endloop0

    { R0 = R29                              //[ Kprime                              ]
      R1 = ADD(R29,#16)                     //[ key                                 ]
      R14 = #0                              //[ n = 0                               ]
      R17:16 = COMBINE(#15,#15)             //[ R17 = R16 = constant 15             ]
    }
    { R6 = MEMUH(R1)                        //[ key[n]                              ]
      R9:8 = COMBINE(#2,#4)                 //[ 2*[(n+1)&7] : 2*[(n+2)&7] at n=0    ]
      LOOP0(.SubKeys_LOOP,#8)               //[ setup loop0: lc0 = 8                ]
    }

    .falign
.SubKeys_LOOP:
    { R3:2 = VADDW(R9:8,R1:0)               //[ R3=&key[(n+1)&7],R2=&Kprime[(n+2)&7]]
      R7 = LSR(R6,#15)                      //[ KLi[2n] = ROL16(key[n],1)           ]
      R8 = ADD(R14,#8)                      //[ n+4                                 ]
      R9 = ADD(R14,#10)                     //[ n+5                                 ]
    }
    { R6 = MEMUH(R3)                        //[ load key[(n+1)&7]                   ]
      R10 = MEMUH(R2)                       //[ KLi[2n+1] = Kprime[(n+2)&7]         ]
      R9:8 = AND(R9:8,R17:16)               //[ (n+5)&7, (n+4)&7                    ]
      R7 = INSERT(R6,#15,#1)                //[ KLi[2n] = ROL16(key[n],1)           ]
    }
    { MEMH(R13++#2) = R7                    //[ save KLi[2n]                        ]
      R3:2 = VADDW(R9:8,R1:0)               //[ R3=&key[(n+5)&7],R2=&Kprime[(n+4)&7]]
      R7 = LSR(R6,#11)                      //[ for ROL16(key[(n+1)&7],5)           ]
      R9 = ADD(R14,#12)                     //[ n+6                                 ]
    }
    { MEMH(R13++#2) = R10                   //[                                     ]
      R6 = MEMUH(R3)                        //[ key[(n+5)&7]                        ]
      R7 = INSERT(R6,#11,#5)                //[ ROL16(key[(n+1)&7],5)               ]
      R9 = AND(R9,#15)                      //[ (n+6)&7                             ]
    }
    { MEMH(R12++#2) = R7                    //[ save KOi1[n]                        ]
      R10 = MEMUH(R2)                       //[ Kprime[(n+4)&7]                     ]
      R3 = ADD(R9,R1)                       //[ R3 = &key[(n+6)&7]                  ]
      R7 = LSR(R6,#8)                       //[ for ROL16(key[(n+5)&7],8)           ]
    }
    { MEMH(R12++#2) = R10                   //[ save kIi1[n] =Kprime[(n+4)&7]       ]
      R6 = MEMUH(R3)                        //[ key[(n+6)&7]                        ]
      R8 = ADD(R14,#6)                      //[ n+3                                 ]
      R7 = INSERT(R6,#8,#8)                 //[ ROL16(key[(n+5)&7],8)               ]
    }
    { MEMH(R12++#2) = R7                    //[ save KOi2[n]                        ]
      R7 = LSR(R6,#3)                       //[ for ROL16(key[(n+6)&7],13)          ]
      R8 = AND(R8,#15)                      //[ (n+3)&7                             ]
      R9 = ADD(R14,#14)                     //[ (n+7)                               ]
    }
    { R2 = ADD(R0,R8)                       //[ &Kprime[(n+3)&7]                    ]
      R9 = AND(R9,#15)                      //[ (n+7)&7                             ]
      R7 = INSERT(R6,#3,#13)                //[ ROL16(key[(n+6)&7],13)              ]
      R14 = ADD(R14,#2)                     //[ n++                                 ]
    }
    { MEMH(R12+#2) = R7                     //[ save KOi3[n]                        ]
      R10 = MEMUH(R2)                       //[ Kprime[(n+3)&7]                     ]
      R3 = ADD(R9,R0)                       //[ R3 = &Kprime[(n+7)&7]               ]
      R2 = ADD(R1,R14)                      //[ [1]&key[n]                          ]
    }
    { MEMH(R12++#4) = R10                   //[ save KIi2[n]                        ]
      R10 = MEMUH(R3)                       //[ Kprime[(n+7)&7]                     ]
      R8 = ADD(R14,#4)                      //[ [1]n+2                              ]
      R9 = ADD(R14,#2)                      //[ [1]n+1                              ]
    }
    { MEMH(R12++#2) = R10                   //[ save KIi3[n]                        ]
      R6 = MEMUH(R2)                        //[ [1]key[n]                           ]
      R8 = AND(R8,#15)                      //[ [1](n+2)&7                          ]
      R9 = AND(R9,#15)                      //[ [1](n+1)&7                          ]
    }:endloop0

    { R17:16 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#40)                    //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    KeySchedule, .-KeySchedule

    .data
    .p2align 3
    .type    C.0, @object
    .size    C.0, 16
C.0:
    .hword    291
    .hword    17767
    .hword    -30293
    .hword    -12817
    .hword    -292
    .hword    -17768
    .hword    30292
    .hword    12816

    .lcomm    KLi,32,8
    .lcomm    KOIi,96,8

