/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 


    .file "median3x3.S"

#define     ENABLE_PREFETCH         // enable inlined dcfetch and dczero

    /*[*****************************************************************************]*/
    /*[  Function   : void median()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform 3x3 median filtering on image                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and (output+width) are aligned by 8bytes                  ]*/
    /*[           - width*(height-2) > 8                                            ]*/
    /*[           - width*(height-2) is a multiple of 8, otherwise overwrite bottom ]*/
    /*[           - boundary pixels are undefined                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *input                                            ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : int height                                               ]*/
    /*[               R3 : UWord8 *output                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23 (R24-R27), R28                                      ]*/
    /*[  Hardware Loops affected: Loop0, (Loop1)                                    ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 11*(W/8)*(H-2) + 14                                             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl median3x3
    .type    median3x3, @function
median3x3:
    { P0 = CMP.GT(R2,#2)                    //[ P0 = height > 2 ?                   ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 return                       ]
      R2 = ADD(R2,#-2)                      //[ height -2                           ]
    }
    { P0 = CMP.GT(R1,#2)                    //[ P0 = width > 2                      ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 return                       ]
      R6 = MPYI(R1,R2)                      //[ width *(height-2)                   ]
    }
    { P0 = CMP.GT(R6,#8)                    //[ P0 = ((width *(height-2))/8) >1     ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 return (due to loop )        ]
      R28 = ADD(R6,#7)                      //[ width *(height-2) + 7               ]
    }
    { R29 = ADD(R29, #-6*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-6*8)) = R17:16           //[ callee-saved registers              ]
      R28 = ASR(R28,#3)                     //[ NP8= ceil( (width *(height-2))/8 )  ]
#ifndef ENABLE_PREFETCH
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R20 = ADD(R0,R1)                      //[ R20 points to line #1               ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R22 = ADD(R20,R1)                     //[ R22 points to line #2               ]
      R1 = ADD(R3,R1)                       //[ out += width                        ]
      P3 = SP2LOOP0(.median3x3_Loop,R28)    //[ setup loop:lc0 = width*(heigh-2)/8  ]
    }
#else
      R8 = #4                               //[ constant                            ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R7 = ADD(R28,#3)                      //[ NP8 + 3                             ]
      R8 = MIN(R8,R28)                      //[ min(4, NP8)                         ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R20 = ADD(R0,R1)                      //[ R20 points to line #1               ]
      R7 = ASR(R7,#2)                       //[ ceil(NP8/4)                         ]
      P3 = SP2LOOP0(.median3x3_Loop,R8)     //[ setup inner loop:lc0 = min(4,LC)    ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R22 = ADD(R20,R1)                     //[ R22 points to line #2               ]
      R1 = ADD(R3,R1)                       //[ out += width                        ]
      LOOP1(.median3x3_PREFETCHLoop,R7)     //[ setup outer loop: lc1 = ceil(NP8/4) ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R25 = ADD(R1,R6)                      //[ out + width*(height-2)              ]
      R24 = ADD(R1,#31)                     //[ out + 31                            ]
      R28 = ADD(R28,#-4)                    //[ LC = NP8-4                          ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R25 = ADD(R25,#-32)                   //[ out + 8*(width/8)*(height-2)-32     ]
      R24 = AND(R24,#-32)                   //[ $-algined output                    ]
      R26 = R8                              //[ min(4,LC)                           ]
    }
    .falign
.median3x3_PREFETCHLoop:
    { P0 = CMP.GTU(R25,R24)                 //[ P0 = within dczero boundary?        ]
      IF !P0.new JUMP:nt .median3x3_Loop    //[ if !P0 skip dczero                  ] 
      R26 = MIN(R26,R28)                    //[ min(4,LC)                           ]
    }
    { DCZEROA(R24)                          //[ dczero output by 1$line             ]
      R24 = ADD(R24,#32)                    //[ update address                      ]
    }
#endif

    .falign
.median3x3_Loop:
    { R5:4 = MEMD(R0++#8)                   //[[1]load p[0][2] from line #0         ]
      R13:12 = VMINUB(R13:12,R9:8)          //[[2]find pmid_m of p[1]               ]
      R17:16 = VMAXUB(R15:14,R11:10)        //[[2]find middleof pmax_l,pmid_m,pmin_h]
    }
    { R7:6 = MEMD(R20++#8)                  //[[2]load p[1][2] from line #1         ]
      R13:12 = VMAXUB(R13:12,R7:6)          //[[3]pmid_m of p[1]                    ]
      R11:10 = VMINUB(R15:14,R11:10)        //[[3]find middleof pmax_l,pmid_m,pmin_h]
    }
    { R9:8 = MEMD(R22++#8)                  //[[2]load p[2][2] from line #2         ]
      R7:6 = VMAXUB(R7:6,R5:4)              //[[2]sort p[][2]                       ]
      R5:4 = VMINUB(R7:6,R5:4)              //[[2]sort p[][2]                       ]
    }
    { R15:14 = VMAXUB(R9:8,R7:6)            //[[2]sorted p[0][2] (max)              ]
      R17:16 = VMINUB(R13:12,R17:16)        //[[3]find middleof pmax_l,pmid_m,pmin_h]
#ifdef ENABLE_PREFETCH
      DCFETCH(R0+#32)                       //[ prefetch 1 $line on row #0          ]
#endif
    }
    { R11:10 = VMINUB(R9:8,R5:4)            //[[2]sorted p[2][2] (min)              ]
      R17:16 = VMAXUB(R17:16,R11:10)        //[[3]middle of pmax_l,pmid_m,pmin_h    ]
#ifdef ENABLE_PREFETCH
      DCFETCH(R20+#32)                      //[ prefetch 1 $line on row #1          ]
#endif
    }
    { R13:12 = VMINUB(R9:8,R7:6)            //[[2]sort p[][2]                       ]
      R7:6 = VALIGNB(R15:14,R19:18,#7)      //[[2]R7:6= p[0][1]                     ]
      R9 = COMBINE(R15.L,R14.H)             //[[2]R9:8= p[0][0]                     ]
      R8 = COMBINE(R14.L,R19.H)             //[[2]R9:8= p[0][0]                     ]
    }
    { R15:14 = VMINUB(R15:14,R7:6)          //[[2]find pmax_l of p[0]               ]
      R19 = R15                             //[[2]save part of sorted p[0] for next ]
      R3:2 = VALIGNB(R17:16,R3:2,#1)        //[[3]pack output                       ]
#ifdef ENABLE_PREFETCH
      DCFETCH(R22+#32)                      //[ prefetch 1 $line on row #2          ]
#endif
    }
    { R15:14 = VMINUB(R15:14,R9:8)          //[[2]pmax_l of p[0]                    ]
      R7:6 = VALIGNB(R11:10,R23:22,#7)      //[[2]R7:6= p[2][1]                     ]
      R9 = COMBINE(R11.L,R10.H)             //[[2]R9:8= p[2][0]                     ]
      R8 = COMBINE(R10.L,R23.H)             //[[2]                                  ]
    }
    { R11:10 = VMAXUB(R11:10,R7:6)          //[[2]find pmin_h of p[2]               ]
      R23 = R11                             //[[2]save part of sorted p[2] for next ]
      R13:12 = VMAXUB(R13:12,R5:4)          //[[2]sorted p[1][2]                    ]
      IF P3 MEMD(R1++#8) = R3:2             //[[3]save                              ]
    }
    { R11:10 = VMAXUB(R11:10,R9:8)          //[[2]pmin_h of p[2]                    ]
      R7:6 = VALIGNB(R13:12,R21:20,#7)      //[[2]R7:6= p[1][1]                     ]
      R9 = COMBINE(R13.L,R12.H)             //[[2]R9:8= p[1][0]                     ]
      R8 = COMBINE(R12.L,R21.H)             //[[2]R9:8= p[1][0]                     ]
    }
    { R13:12 = VMAXUB(R13:12,R7:6)          //[[2]find pmid_m of p[1]               ]
      R7:6 = VMINUB(R13:12,R7:6)            //[[2]find pmid_m of p[1]               ]
      R21 = R13                             //[[2]save part of sorted p[1] for next ]
      R3:2 = R17:16                         //[[3]save R17:16 in R3:2               ]
    }:endloop0

#ifdef ENABLE_PREFETCH
    { LOOP0(.median3x3_Loop,R26)            //[ setup inner loop: lc0=min(4,LC)     ]
      R28 = ADD(R28,#-4)                    //[ LC -= 4                             ]
    }:endloop1

    { R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
#else
    {
#endif
      R13:12 = VMINUB(R13:12,R9:8)          //[e]find pmid_m of p[1]
      R17:16 = VMAXUB(R15:14,R11:10)        //[e]find middle of pmax_l,pmid_m,pmin_h
    }
    { R13:12 = VMAXUB(R13:12,R7:6)          //[e]pmid_m of p[1]
      R11:10 = VMINUB(R15:14,R11:10)        //[e]find middle of pmax_l,pmid_m,pmin_h
    }
    { R17:16 = VMINUB(R13:12,R17:16)        //[e]find middle of pmax_l,pmid_m,pmin_h
    }
    { R17:16 = VMAXUB(R17:16,R11:10)        //[e]middle of pmax_l,pmid_m,pmin_h
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R3:2 = VALIGNB(R17:16,R3:2,#1)        //[e]pack output
      R7:6 = ASR(R17:16,#8)
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { MEMD(R1++#8) = R3:2                   //[e] save
    }
    { MEMD(R1) = R7:6                       //[e] save
      R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    median3x3, .-median3x3
