/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "sobel.S"

#define     ENABLE_PREFETCH                 // enable inlined dcfetch and dczero
#define     PREFETCH_AHEAD                  32

    /*[*****************************************************************************]*/
    /*[  Function   : void sobel()                                                  ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 3x3 sobel filtering on image                          ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and (output+width) are aligned by 4bytes                  ]*/
    /*[           - width*(height-2) > 4                                            ]*/
    /*[           - width*(height-2) is a multiple of 4, otherwise overwrite bottom ]*/
    /*[           - boundary pixels are undefined                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *input                                            ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : int height                                               ]*/
    /*[               R3 : UWord8 *output                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R21 (R22-R23), R28                                      ]*/
    /*[  Hardware Loops affected: Loop0, (Loop1)                                    ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6*(W/4)*(H-2) + 15  ( + ~4*(W*(H-2)/32 prefetch overhead)       ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl sobel
    .type    sobel, @function
sobel:
    { P0 = CMP.GT(R2,#2)                    //[ P0= (height > 2)                    ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 then return                  ]
      R2 = ADD(R2,#-2)                      //[ height-2                            ]
    }
    { P0 = CMP.GT(R1,#2)                    //[ P0 = width > 2                      ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 then return                  ]
      R6 = MPYI(R1,R2)                      //[ width *(height-2)                   ]
    }
    { P0 = CMP.GT(R6,#4)                    //[ P0 = width*(height-2)/4 > 1         ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 then return (due to loop)    ]
      R28 = ADD(R6,#(3+4))                  //[ width*(height-2) + 3 + 4            ]
    }
    { R29 = ADD(R29, #-4*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-4*8)) = R17:16           //[ callee-saved registers              ]
      R28 = ASR(R28,#2)                     //[ NP4= ceil( (width*(height-2))/4 )+1 ]
      R7 = #0                               //[ initial A = 0                       ]
    }
#ifndef ENABLE_PREFETCH
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R20 = ADD(R0,R1)                      //[ R20 points to line #1               ]
      R21 = ADDASL(R0,R1,#1)                //[ R21 points to line #2               ]
      R1 = ADD(R3,R1)                       //[ write ptr R1 = output + width       ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R3 = #0                               //[ initial B = 0                       ]
      P3 = SP3LOOP0(.sobel_Loop,R28)        //[ setup loop: lc0= NP4                ]
      R5:4 = MEMUBH(R0++#4)                 //[p]load from line #0                  ]
    }
#else
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R9 = ADD(R28,#7)                      //[ to calculate ceil(NP4/8)            ]
      R6 = ADD(R6,#-32)                     //[ width*(height-2) -32                ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R9 = ASR(R9,#3)                       //[ ceil(NP4/8)                         ]
      R20 = ADD(R0,R1)                      //[ R20 points to line #1               ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R21 = ADD(R20,R1)                     //[ R21 points to line #2               ]
      R1 = ADD(R3,R1)                       //[ write ptr R1 = output + width       ]
      R22 = #8                              //[ constant 8                          ]
    }
    { R3 = #0                               //[ initial B = 0                       ]
      R23 = ADD(R1,R6)                      //[ output+width*(height-2)-32 (dczero) ]
      R22 = MIN(R22,R28)                    //[ min(8, NP4)                         ]
      LOOP1(.sobel_PREFETCHLoop,R9)         //[ setup outer loop: lc1 = ceil(NP4/8) ]
    }
    { P3 = SP3LOOP0(.sobel_Loop,R22)        //[ setup loop: lc0= min(8,NP4)         ]
      R28 = ADD(R28,#-8)                    //[ LC = NP4 -8                         ]
      R12 = ADD(R1,#31)                     //[ to get $aligned output              ]
      R5:4 = MEMUBH(R0++#4)                 //[[p]load from line #0                 ]
    }

    .falign
.sobel_PREFETCHLoop:
    { DCFETCH(R20+#PREFETCH_AHEAD)          //[ fetch 1$line from row #1            ]
      R22 = MIN(R22,R28)                    //[ min(8, LC)                          ]
      R12 = AND(R12,#-32)                   //[ $aligned output                     ]
    }
    { DCFETCH(R21+#PREFETCH_AHEAD)          //[ fetch 1$line from row #2            ]
      R28 = ADD(R28,#-8)                    //[ LC = NP4 -8                         ]
      P0 = CMP.GTU(R23,R12)                 //[ P0 = dczero is with boundary ?      ]
      IF !P0.new JUMP:nt .sobel_Loop        //[ IF !P0 skip dczero                  ]
    }
    { DCZEROA(R12)                          //[ dczero 1$line of output             ]
    }
#endif

    .falign
.sobel_Loop:
    { R7:6 = MEMUBH(R20++#4)                //[2]load from line #1                  ]
      R11:10 = VADDH(R11:10,R9:8)           //[3]H= B[-2] + 2*B[-1] + B[0]          ]
      R13:12 = VSUBH(R15:14,R7:6)           //[3]V= A[-2] - A[0]                    ]
      R15:14 = COMBINE(R7,R3)               //[4]save previous A,B for next         ]
    }
    { R9:8 = MEMUBH(R21++#4)                //[2]load from line #2                  ]
      R7:6 = VADDH(R7:6,R7:6)               //[2]A[0]= 2*L1                         ]
      R11:10 = VADDH(R11:10,R3:2)           //[3]H= 2*B[-1]+B[0]                    ]
      R16 = R17                             //[4]update output buffer               ]
    }
    { R6 = VADDH(R6,R4)                     //[2]A[0]= L0 + 2*L1                    ]
      R11:10 = VABSH(R11:10)                //[3]abs(H)                             ]
      R13:12 = VABSH(R13:12)                //[3]abs(V)                             ]
      IF P3 MEMW(R1++#4) = R18              //[4]save output                        ]
    }
    { R5:4 = MEMUBH(R0++#4)                 //[1]load from line #0                  ]
      R3:2 = VSUBH(R5:4,R9:8)               //[2]B[0]= L0 -L2                       ]
      R7 = VADDH(R7,R5)                     //[2]A[0]= L0 + 2*L1                    ]
      R11:10 = VADDH(R11:10,R13:12)         //[3]SUM= abs(H) + abs(V)               ]
    }
    { R7:6 = VADDH(R7:6,R9:8)               //[2]A[0]= L0 + 2*L1 + L2               ]
      R9 = COMBINE(R3.L,R2.H)               //[2]R9:8= B[-1]                        ]
      R8 = COMBINE(R2.L,R14.H)              //[2]R9:8= B[-1]                        ]
      R17 = VSATHUB(R11:10)                 //[3]sature                             ]
    }
    { R11:10 = VADDH(R9:8,R9:8)             //[2]H= 2*B[-1]                         ]
      R9:8 = COMBINE(R2,R14)                //[2]R9:8= B[-2]                        ]
      R15:14 = COMBINE(R6,R15)              //[2]R15:14 = A[-2]                     ]
      R19:18 = LSR(R17:16,#8)               //[3]align output                       ]
    }:endloop0

#ifdef ENABLE_PREFETCH
    { LOOP0(.sobel_Loop,R22)                //[ setup loop: lc0= min(8,NP4)         ]
      DCFETCH(R0+#PREFETCH_AHEAD)           //[ fetch 1$line from row #0            ]
      R12 = ADD(R1,#31)                     //[ to get $aligned output              ]
    }:endloop1

    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
#else
    {
#endif
      MEMW(R1++#4) = R18                    //[[e]save output                       ]
    }
    { MEMW(R1) = R19                        //[[e]save output                       ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#4*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    sobel, .-sobel
