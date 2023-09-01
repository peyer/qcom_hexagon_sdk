/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file "idct.S"

#define  COS1_16               32138
#define  COS2_16               30274
#define  COS3_16               27246
#define  COS4_16               23170
#define  COS5_16               18205
#define  COS6_16               12540
#define  COS7_16                6393


    /*[*****************************************************************************]*/
    /*[  Function   : void iDCT()                                                   ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform inverse DCT transform on 8x8 blocks                   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - input and output are aligned by 8bytes                          ]*/
    /*[           - can be used for inplaced-implementation, i.e., output = input   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *input                                            ]*/
    /*[               R1 : int nblocks                                              ]*/
    /*[               R2 : Word16 *output                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 112*n + 22                                                      ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl iDCT
    .type    iDCT, @function
iDCT:
    { P0 = CMP.GT(R1,#0)                    //[ if !(nblocks>0)                     ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
      IF  P0.new R3 = R2                    //[ setup R3 = output (write ptr)       ]
      R1 = ASL(R1,#3)                       //[ R1 = 8*nBlocks                      ]
    }
    { R29 = ADD(R29, #-6*8)                 //[ allocate stack                      ]
      MEMD(R29+#(40-6*8)) = R27:26          //[ callee-saved registers              ]
      R26.H = #COS1_16                      //[ c1                                  ]
      R27.H = #COS5_16                      //[ c5                                  ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R26.L = #COS7_16                      //[ R26 = C1||C7                        ]
      R27.L = #COS3_16                      //[ R27 = C5||C3                        ]
      R5:4 = MEMD(R0++#8)                   //[[p]load x0-x3                        ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R24.H = #COS4_16                      //[ c4                                  ]
      R25.H = #COS2_16                      //[ c2                                  ]
      R7:6 = MEMD(R0++#8)                   //[ [p]load x4-x7                       ]
    }
    { R24.L = #COS4_16                      //[ R24 = C4||C4                        ]
      R25.L = #COS6_16                      //[ R25 = C2||C6                        ]
      R8  = COMBINE(R6.L,R4.L)              //[[p]x4|x0                             ]
      R10 = COMBINE(R7.L,R5.L)              //[[p]x6|x2                             ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R23.H = #(65536-COS4_16)              //[ -C4                                 ]
      R8  = CMPY(R8, R24):<<1:rnd:sat       //[[p]s0|s1                             ]
      R10 = CMPY(R10,R25):<<1:rnd:sat       //[[p]s2|s3                             ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R9  = COMBINE(R7.H,R4.H)              //[[p]x7|x1                             ]
      R11 = COMBINE(R5.H,R6.H)              //[[p]x3|x5                             ]
      P3=SP1LOOP0(.iDCT_horizontal_loop,R1) //[ setup horizontal loop: lc0 = 8nblock]
    }
    { MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
      R23.L = #COS4_16                      //[ R23 = -C4||C4                       ]
      R9  = CMPY(R9,R26):<<1:rnd:sat        //[ [p]s4|s5                            ]
      R11 = CMPY(R11,R27):<<1:rnd:sat       //[ [p]s6|s7                            ]
    }
    
    /*-----------------------------------------------------------------------------*/
    /*                           Horizontal Pass                                   */
    /*-----------------------------------------------------------------------------*/
    .falign
.iDCT_horizontal_loop:
    { R5:4 = MEMD(R0++#16)                  //[[1] load x0-x3                       ]
      R7:6 = MEMD(R0+#8)                    //[[1] load x4-x7                       ]
      R13:12 = VADDH(R9:8,R11:10)           //[[2] t4|t5|t0|t1                      ]
      R15:14 = VSUBH(R9:8,R11:10)           //[[2] t6|t7|t2|t3                      ]
    }
    { R8  = COMBINE(R6.L,R4.L)              //[[1] R8  = x4||x0                     ]
      R10 = COMBINE(R7.L,R5.L)              //[[1] R10 = x6||x2                     ]
      R15 = CMPY(R15,R23):<<1:rnd:sat       //[[2] u7|u5                            ]
      IF P3 MEMD(R3+#8) = R19:18            //[[3] save y4-y7                       ]
    }
    { R8 = CMPY(R8,R24):<<1:rnd:sat         //[[1] s1|s0                            ]
      R13 = R14                             //[[2] u2|u3 = t2|t3; R13:12=u2|u3|u0|u1]
      R14 = COMBINE(R13.H,R15.L)            //[[2] u4|u5 = t4|u5                    ]
      R15 = COMBINE(R13.L,R15.H)            //[[2] u6|u7 = t5|u7; R15:14=u6|u7|u4|u5]
    }
    { R9  = COMBINE(R7.H,R4.H)              //[[1] R9  = x7||x1                     ]
      R11 = COMBINE(R5.H,R6.H)              //[[1] R11 = x3||x5                     ]
      R10 = CMPY(R10,R25):<<1:rnd:sat       //[[1] s3|s2                            ]
      R16 = COMBINE(R16.L,R16.H)            //[[3] reorder y0,y1 in R16             ]
    }
    { R9 = CMPY(R9,R26):<<1:rnd:sat         //[[1] s5|s4                            ]
      R19:18 = VSUBH(R13:12,R15:14)         //[[2] y4|y5|y7|y6                      ]
      IF P3 MEMD(R3++#16) = R17:16          //[[3] save y0-y3                       ]
    }
    { R11 = CMPY(R11,R27):<<1:rnd:sat       //[[1] s7|s6                            ]
      R19 = R18                             //[[2] reorder y7, y6 in R19            ]
      R18 = COMBINE(R19.L,R19.H)            //[[2] reorder y5, y4 in R18            ]
      R17:16 = VADDH(R13:12,R15:14)         //[[2] y3|y2|y0|y1                      ]
    }:endloop0

    { MEMD(R3+#8) = R19:18                  //[[e] save last row y4-y7              ]
      R16 = COMBINE(R16.L,R16.H)            //[[e] reorder y0,y1 in R16             ]
      R0 = R2                               //[ R0 = output                         ]
      R20.H = #31                           //[ set rounding factor                 ]
    }
    { MEMD(R3+#0) = R17:16                  //[[e] save last row y0-y3              ]
      R20.L = #32                           //[ set rounding factor                 ]
      R1 = ASR(R1,#1)                       //[ 4*nBlocks                           ]
      R3 = #4                               //[ constant                            ]
    }
    /*-----------------------------------------------------------------------------*/
    /*                           Vertical Pass                                     */
    /*-----------------------------------------------------------------------------*/
    { R21 = R20                             //[ R20=R21 = 31|32                     ]
      R8 = MEMW(R0+#16)                     //[[p] load x1                          ]
      R9 = MEMW(R0+#112)                    //[[p] load x7                          ]
      R2.H = #0x0404                        //[                                     ]
    }
    { R2.L = #0x7400                        //[ R3:2=(4|4|4|116)<<8 (for ptr update)]
      R10 = MEMW(R0+#80)                    //[[p] load x5                          ]
      R11 = MEMW(R0+#48)                    //[[p] load x3                          ]
      R9:8 = PACKHL(R9,R8)                  //[[p] R9:8 = x7||x1                    ]
    }
    { R10 = COMBINE(R11.L,R10.L)            //[[p] R10,11 = x3||x5                  ]
      R11 = COMBINE(R11.H,R10.H)            //[[p] R10,11 = x3||x5                  ]
      R8 = CMPY(R8,R26):<<1:rnd:sat         //[[p] R8 = s4 ||s5                     ]
      R9 = CMPY(R9,R26):<<1:rnd:sat         //[[p] R9 = s4'||s5'                    ]
    }
    { R4 = MEMW(R0+#0)                      //[[p] load x0                          ]
      R5 = MEMW(R0+#64)                     //[[p] load x4                          ]
      R10 = CMPY(R10,R27):<<1:rnd:sat       //[[p] R10 = s6 ||s7                    ]
      R11 = CMPY(R11,R27):<<1:rnd:sat       //[[p] R11 = s6'||s7'                   ]
    }
    { LOOP0(.iDCT_vertical_loop,R1)         //[ setup vertical loop:lc0 = 4*nblocks ]
      R6 = MEMW(R0+#32)                     //[[p] load x2                          ]
      R5:4 = PACKHL(R5,R4)                  //[[p] R4,R5 = x4||x0                   ]
      R12 = VADDH(R8,R10)                   //[[p] R12= t4||t5,R13= t4'||t5'        ]
    }
    { R7 = MEMW(R0+#96)                     //[[p] load x6                          ]
      R13 = VADDH(R9,R11)                   //[[p] R12= t4||t5,R13= t4'||t5'        ]
      R4 = CMPY(R4,R24):<<1:rnd:sat         //[[p] R4 = s0 ||s1                     ]
      R5 = CMPY(R5,R24):<<1:rnd:sat         //[[p] R5 = s0'||s1'                    ]
    }
    { R6 = COMBINE(R7.L,R6.L)               //[[p] R6 = x6 ||x2                     ]
      R7 = COMBINE(R7.H,R6.H)               //[[p] R7 = x6'||x2'                    ]
      R11:10 = VSUBH(R9:8,R11:10)           //[[p] R10= t6||t7,R11= t6'||t7'        ]
      R5:4 = VADDH(R5:4,R21:20):sat         //[[p] add rounding factor to s0,s1     ]
    }
    { R0 = ADD(R0,R3)                       //[ update read pointer                 ]
      R1 = R0                               //[ set write pointer                   ]
      R10 = CMPY(R10,R23):<<1:rnd:sat       //[[p] R10 = u7 ||u5                    ]
      R11 = CMPY(R11,R23):<<1:rnd:sat       //[[p] R11 = u7'||u5'                   ]
    }
    { R6 = CMPY(R6,R25):<<1:rnd:sat         //[[p] R6 = s2 ||s3                     ]
      R7 = CMPY(R7,R25):<<1:rnd:sat         //[[p] R7 = s2'||s3'                    ]
    }

    .falign
.iDCT_vertical_loop:
    { R8 = COMBINE(R12.H,R10.L)             //[[2] R8 = u4 ||u5                     ]
      R9 = COMBINE(R13.H,R11.L)             //[[2] R9 = u4'||u5'                    ]
      R7:6 = VSUBH(R5:4,R7:6)               //[[2] R6= t2||t3, R7= t2'||t3'         ]
      R5:4 = VADDH(R5:4,R7:6)               //[[2] R4= t0||t1, R5= t0'||t1'         ]
    }
    { R10 = COMBINE(R12.L,R10.H)            //[[2] R10 = u6 ||u7                    ]
      R11 = COMBINE(R13.L,R11.H)            //[[2] R11 = u6'||u7'                   ]
      R13:12 = VADDH(R5:4,R9:8)             //[[2] R12= w0||w1 R13= w0'||w1'        ]
      R17:16 = VSUBH(R5:4,R9:8)             //[[2] R16= w7||w6 R17= w7'||w6'        ]
    }
    { R4 = MEMW(R0+#0)                      //[[1] load x0                          ]
      R8 = MEMW(R0+#16)                     //[[1] load x1                          ]
      R15:14 = VADDH(R7:6,R11:10)           //[[2] R14= w3||w2, R15= w3'||w2'       ]
      R19:18 = VSUBH(R7:6,R11:10)           //[[2] R18= w4||w5, R19= w4'||w5'       ]
    }
    { R5  = MEMW(R0+#64)                    //[[1] load x4                          ]
      R10 = MEMW(R0+#80)                    //[[1] load x5                          ]
      R13:12 = VADDH(R13:12,R13:12):sat     //[[2]to saturate w0,w1 (0x4000,0x3FFFF)]
      R15:14 = VADDH(R15:14,R15:14):sat     //[[2]to saturate w2,w3                 ]
    }
    { R6  = MEMW(R0+#32)                    //[[1] load x2                          ]
      R11 = MEMW(R0+#48)                    //[[1] load x3                          ]
      R5:4 = PACKHL(R5,R4)                  //[[1] R4,R5 = x4||x0                   ]
      R19:18 = VADDH(R19:18,R19:18):sat     //[[2] to sature w4,w5                  ]
    }
    { R7 = MEMW(R0+#96)                     //[[1] load x6                          ]
      R9 = MEMW(R0+#112)                    //[[1] load x7                          ]
      R4 = CMPY(R4,R24):<<1:rnd:sat         //[[1] R4 = s0||s1                      ]
      R13:12 = VASRH(R13:12,#7)             //[[2] truncate to get 9-bit y0,y1      ]
    }
    { R7:6 = PACKHL(R7,R6)                  //[[1] R7,R6 = x6||x2                   ]
      R5 = CMPY(R5,R24):<<1:rnd:sat         //[[1] R5 = s0'||s1'                    ]
      R13 = COMBINE(R13.H,R12.H)            //[[2] pack into y0'||y0                ]
      R12 = COMBINE(R13.L,R12.L)            //[[2] pack into y1'||y1                ]
    }
    { R6 = CMPY(R6,R25):<<1:rnd:sat         //[[1]R6 = s2||s3                       ]
      R2 = ADD(R2,R3)                       //[[1]with shift to rotate R2 by 1byte  ]
      MEMW(R1+#0) = R13                     //[[2]save y0,y0'                       ]
      R15:14 = VASRH(R15:14,#7)             //[[2]truncate to 9-bit y3,y2           ]
    }
    { R7 = CMPY(R7,R25):<<1:rnd:sat         //[[1] R7 = s2'||s3'                    ]
      R9:8 = PACKHL(R9,R8)                  //[[1] R8,9 = x7||x1                    ]
      MEMW(R1+#16) = R12                    //[[2] save y1,y1'                      ]
      R13 = COMBINE(R15.L,R14.L)            //[[2] pack into y2'||y2                ]
    }
    { R8 = CMPY(R8,R26):<<1:rnd:sat         //[[1] R8 = s4||s5                      ]
      MEMW(R1+#32) = R13                    //[[2] save y2,y2'                      ]
      R19:18 = VASRH(R19:18,#7)             //[[2] truncate to 9-bit y4,y5          ]
      R12 = COMBINE(R15.H,R14.H)            //[[2] pack into y3'||y3                ]
    }
    { R11:10 = PACKHL(R11,R10)              //[[1] R10,11 = x3||x5                  ]
      R9 = CMPY(R9,R26):<<1:rnd:sat         //[[1] R9 = s4'||s5'                    ]
      MEMW(R1+#48) = R12                    //[[2] save y3,y3'                      ]
      R13 = COMBINE(R19.H,R18.H)            //[[2] pack into y4'||y4                ]
    }
    { R10 = CMPY(R10,R27):<<1:rnd:sat       //[[1] R10 = s6||s7                     ]
      MEMW(R1+#64) = R13                    //[[2] save y4,y4'                      ]
      R12 = COMBINE(R19.L,R18.L)            //[[2] pack into y5'||y5                ]
      R17:16 = VADDH(R17:16,R17:16):sat     //[[2] to saturate w6,w7                ]
    }
    { R11 = CMPY(R11,R27):<<1:rnd:sat       //[[1] R11 = s6'||s7'                   ]
      R3 = #0                               //[[1] clear R3                         ]
      MEMW(R1+#80) = R12                    //[[2] save y5,y5'                      ]
      R17:16 = VASRH(R17:16,#7)             //[[2] truncate to 9-bit y7,y6          ]
    }
    { R15:14 = VSUBH(R9:8,R11:10)           //[[1] R10= t6||t7, R11=t6'||t7'        ]
      R17 = COMBINE(R17.L,R16.L)            //[[2] pack into y6'||y6                ]
      R16 = COMBINE(R17.H,R16.H)            //[[2] pack into y7'||y7                ]
      R3:2 = ASL(R3:2,#8)                   //[[2] get index for pointer update     ]
    }
    { R0 = ADD(R0,R3)                       //[[1] update read pointer              ]
      R10 = CMPY(R14,R23):<<1:rnd:sat       //[[1] R10 = u7||u5                     ]
      R13:12 = VADDH(R9:8,R11:10)           //[[1] R12= t4||t5, R13= t4'||t5'       ]
      MEMW(R1+#96) = R17                    //[[2] save y6,y6'                      ]
    }
    { R1 = SUB(R0,R3)                       //[[1] set write pointer                ]
      R11 = CMPY(R15,R23):<<1:rnd:sat       //[[1] R11= u7'||u5'                    ]
      R5:4 = VADDH(R5:4,R21:20):sat         //[[1] add rounding factor to s0,s1     ]
      MEMW(R1+#112) = R16                   //[[2] save y7,y7'                      ]
    }:endloop0

    { R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    iDCT, .-iDCT

