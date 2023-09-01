/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file    "ycbcr2rgb.S"

#define   CR2R      0x59ba 
#define   CR2G      0xd24c
#define   CB2G      0xe9fa
#define   CB2B      0x7168

    /*[*****************************************************************************]*/            
    /*[  Function   : void H1V1Convert()                                            ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: convert YCbCr H1V1 to 24-bit RGB                              ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - src and dst aligned by 2 bytes                                  ]*/
    /*[           - width is a multiple of 2                                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *src                                              ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : UWord8 *dst                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R19, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 16                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6*(width/2) + 10                                                ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl H1V1Convert
    .type    H1V1Convert, @function
H1V1Convert:
    { R6 = ASR(R1,#1)                       //[ LC = width/2                        ]
      R7 = #255                             //[ 255                                 ]
      R3.H = #CB2B                          //[ R3 = CB2B|CR2R                      ]
      R4.H = #CB2G                          //[ R4 = CB2G|CR2G                      ]
    }
    { P0 = CMP.GT(R6,#0)                    //[ P0 = (LC > 0 )                      ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 then return                  ]
      R3.L = #CR2R                          //[ R3 = CB2B|CR2R                      ]
      R4.L = #CR2G                          //[ R4 = CB2G|CR2G                      ]
    }
    { R29 = ADD(R29,#-2*8)                  //[ reserve stack                       ]
      MEMD(R29+#(0-2*8)) = R17:16           //[ callee-saved registers              ]
      R17:16 = VSPLATH(R7)                  //[ R17:16 = 255                        ]
      R28 = #(64*3-8)                       //[ offset for every 8iter              ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R5 = R4                               //[ R5=R4 = CB2G|CR2G                   ]
      R1 = #1                               //[ set cnt=1                           ]
      P3 = SP1LOOP0(.H1V1Convert_LOOP,R6)   //[ setup loop with LC = width/2        ]
    }
    { R6 = MEMUBH(R0+#64)                   //[ [p]load cb                          ]
    }

    .falign
.H1V1Convert_LOOP:
    { R7 = MEMUBH(R0+#128)                  //[[2]load cr                           ]
      P0 = BITSCLR(R1,#3)                   //[[2]P0 = (cnt%4 == 0)                 ]
      R1 = ADD(R1,#1)                       //[[2]cnt++                             ]
      R9 = VADDH(R8,R9)                     //[[3]G= y+CB2G*cb'+CR2G*cr'            ]
    }
    { R8 = MEMUBH(R0++#2)                   //[[2]load y                            ]
      R7:6 = PACKHL(R6,R7)                  //[[2]cb|cr                             ]
      R14 = COMBINE(R9.L,R12.L)             //[[3]R14 = G0|R0                       ]
      R15 = COMBINE(R13.L,R12.H)            //[[3]R15 = R1|B0                       ]
    }
    { R7:6 = VADDH(R7:6,R7:6)               //[[2]2*cb|2*cr                         ]
      R12 = COMBINE(R8.L,R8.L)              //[[2]y|y                 (1)           ]
      R14 = VSATHUB(R15:14)                 //[[3]clamp                             ]
      R15 = COMBINE(R13.H,R9.H)             //[[3]R15 = B2|G2                       ]
    }
    { R7:6 = VSUBH(R7:6,R17:16)             //[[2]cb'|cr'= 2*cb-255|2*cr-255        ]
      R13 = COMBINE(R8.H,R8.H)              //[[2]y|y                  (2)          ]
      IF P3 MEMH(R2++#2) = R14              //[[3]save                              ]
      R15 = VSATHUB(R15)                    //[[3]clamp                             ]
    }
    { R18 = VMPYH(R6,R3):<<1:rnd:sat        //[[2]CB2B*cb' | CR2R*cr'  (1)          ]
      R19 = VMPYH(R7,R3):<<1:rnd:sat        //[[2]CB2B*cb' | CR2R*cr'  (2)          ]
      IF P0 R0 = ADD(R0,R28)                //[[2]if(cnt%4==0) update src           ]
      IF P3 MEMH(R2++#2) = R14.H            //[[3]save                              ]
    }
    { R6 = MEMUBH(R0+#64)                   //[[1]load cb                           ]
      R13:12 = VADDH(R13:12,R19:18)         //[[2]B=y+CB2B*cb'|R=y+CR2R*cr'         ]
      R9 = VDMPY(R7:6,R5:4):<<1:rnd:sat     //[[2]CB2G*cb' + CR2G*cr'               ]
      IF P3 MEMH(R2++#2) = R15              //[[3]save                              ]
    }:endloop0

    { R6 = COMBINE(R13.L,R12.H)             //[ [e]R6 = R1|B0                       ]
      R9 = VADDH(R8,R9)                     //[ [e]G= y+CB2G*cb'+CR2G*cr'           ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { R6 = VSATHUB(R6)                      //[ [e]clamp to 8-bit R1|B0             ]
      R14 = COMBINE(R9.L,R12.L)             //[ [e]R14 = G0|R0                      ]
      R15 = COMBINE(R13.H,R9.H)             //[ [e]R15 = B2|G2                      ]
    }
    { MEMH(R2+#2) = R6                      //[ [e]save                             ]
      R14 = VSATHUB(R15:14)                 //[ [e]clamp to 8 bit B2|G2|G0|R0       ]
    }
    { MEMH(R2) = R14                        //[ [e]save                             ]
    }
    { MEMH(R2+#4) = R14.H                   //[ [e]save                             ]
      R29 = ADD(R29,#2*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    H1V1Convert, .-H1V1Convert


    /*[*****************************************************************************]*/            
    /*[  Function   : void H2V2Convert()                                            ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: convert YCbCr H2V2 to 24-bit RGB                              ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - src and dst aligned by 4 bytes                                  ]*/
    /*[           - width is a multiple of 2                                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *src                                              ]*/
    /*[               R1 : UWord8 *csrc                                             ]*/
    /*[               R2 : int width                                                ]*/
    /*[               R3 : UWord8 *dst                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 48                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 14*(width/4) + 13                                               ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl H2V2Convert
    .type	H2V2Convert, @function
H2V2Convert:
    { P0 = CMP.GT(R2,#3)                    //[ if !(width/4>0)                     ]
      IF !P0.new JUMPR:nt R31               //[ then return                         ]
    }
    { R29 = ADD(R29,#-6*8)                  //[ allocate stack                      ]
      MEMD(R29+#(0-6*8)) = R17:16           //[ callee-saved registers              ]
      R8 = ASR(R2,#2)                       //[ LC = width/4                        ]
      R2 += ADD(R2,R2)                      //[ 3*width                             ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R4 = ADD(R3,R2)                       //[ dst + 3*width                       ]
      R5.H = #CB2B                          //[ R5 = CB2B|CR2R                      ]
      R6.H = #CB2G                          //[ R6 = CB2G|CR2G                      ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R5.L = #CR2R                          //[ R5 = CB2B|CR2R                      ]
      R6.L = #CR2G                          //[ R6 = CB2G|CR2G                      ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R7 = #255                             //[ 255                                 ]
      R23:22 = COMBINE(R6,R6)               //[ R23=R22 = CB2G|CR2G                 ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R2 = #0                               //[ set cnt=0                           ]
      R25:24 = VSPLATH(R7)                  //[ R25:24 = 255|255|255|255            ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R26 = #(64*6-64-8)                    //[ offset for csrc                     ]
      R27 = #(64*6-8)                       //[ offset for ysrc                     ]
      P3 = SP1LOOP0(.H2V2Convert_LOOP,R8)   //[ setup loop with LC = width/4        ]
    }

    .falign
.H2V2Convert_LOOP:
    { R7 = MEMUBH(R1+#64)                   //[[1]load cr                           ]
      R19 = COMBINE(R12.L,R10.H)            //[[2]R11|B10   (1)                     ]
      R20 = COMBINE(R12.H,R15.L)            //[[2]B11|G11   (1)                     ]
      R21 = COMBINE(R14.H,R11.L)            //[[2]G10|R10   (2)                     ]
    }
    { R6 = MEMUBH(R1++#2)                   //[[1]load cb                           ]
      R20 = VSATHUB(R19:18)                 //[[2]clamp to 8-bit                    ]
      R21 = VSATHUB(R21:20)                 //[[2]clamp to 8-bit                    ]
      R2 = AND(R2,#3)                       //[ cnt%4                               ]
    }
    { R9:8 = MEMUBH(R0++#8)                 //[[1]load y01(2)|y00(2)|y01(1)|y00(1)  ]
      R7:6 = PACKHL(R6,R7)                  //[[1] cb(2)|cr(2)|cb(1)|cr(1)          ]
      R18 = COMBINE(R13.L,R11.H)            //[[2]R11|B10   (2)                     ]
      R19 = COMBINE(R13.H,R15.H)            //[[2]B11|G11   (2)                     ]
    }
    { R7:6 = VADDH(R7:6,R7:6)               //[[1]2*cb|2*cr                         ]
      R11:10 = SHUFFEH(R9:8,R9:8)           //[[1]y00|y00                           ]
      P0 = CMP.EQ(R2,#1)                    //[[1]P0 = (cnt%4 == 1)                 ]
      IF P3 MEMW(R4++#4) = R20              //[[2]save1                             ]
    }
    { R7:6 = VSUBH(R7:6,R25:24)             //[[1]cb'|cr'= 2*cb-255|2*cr-255        ]
      R15:14 = PACKHL(R9,R8)                //[[1]y01(2)|y01(1)|y00(2)|y00(1)       ]
      P1 = CMP.EQ(R2,#3)                    //[[1]P1 = (cnt%4/ ==3)                 ]
      IF P3 MEMW(R4++#4) = R21              //[[2]save2                             ]
    }
    { R16 = VMPYH(R6,R5):<<1:rnd:sat        //[[1]bc | rc = CB2B*cb' | CR2R*cr'  (1)]
      R17 = VMPYH(R7,R5):<<1:rnd:sat        //[[1]bc | rc = CB2B*cb' | CR2R*cr'  (2)]
      R12 = COMBINE(R8.H,R8.H)              //[[1]y01|y01   (1)                     ]
      R13 = COMBINE(R9.H,R9.H)              //[[1]y01|y01   (2)                     ]
    }
    { R21 = VDMPY(R7:6,R23:22):<<1:rnd:sat  //[[1]gc= CB2G*cb'+CR2G*cr'             ]
      R11:10 = VADDH(R11:10,R17:16)         //[[1]B00= y00+bc|R00= y00+rc           ]
      R2 = ADD(R2,#1)                       //[[1]cnt++                             ]
      R9:8 = MEMUBH(R0++#-4)                //[[1]load y11(2)|y10(2)|y11(1)|y10(1)  ]
    }
    { R13:12 = VADDH(R13:12,R17:16)         //[[1]B01= y01+bc|R01=y01+rc            ]
      R14 = VADDH(R14,R21)                  //[[1]G00= y00+gc                       ]
      R15 = VADDH(R15,R21)                  //[[1]G01= y01+gc                       ]
      R20 = VSATHUB(R19:18)                 //[[2]clamp to 8-bit                    ]
    }
    { R18 = COMBINE(R14.L,R10.L)            //[[1]G00|R00   (1)                     ]
      R19 = COMBINE(R12.L,R10.H)            //[[1]R01|B00   (1)                     ]
      IF P0 R0 = ADD(R0,#(64-8))            //[[1]if(cnt%4==1) ysrc+=64-8           ]
      IF P3 MEMW(R4++#4) = R20              //[[2]save3                             ]
    }
    { R20 = VSATHUB(R19:18)                 //[[1]clamp to 8 bit                    ]
      R18 = COMBINE(R12.H,R15.L)            //[[1]B01|G01   (1)                     ]
      R19 = COMBINE(R14.H,R11.L)            //[[1]G00|R00   (2)                     ]
      R7:6 = SHUFFEH(R9:8,R9:8)             //[[1]y10|y10                           ]
    }
    { MEMW(R3++#4) = R20                    //[[1]save1                             ]
      R20 = VSATHUB(R19:18)                 //[[1]clamp to 8-bit                    ]
      R18 = COMBINE(R13.L,R11.H)            //[[1]R01|B00   (2)                     ]
      R19 = COMBINE(R13.H,R15.H)            //[[1]B01|G01   (2)                     ]
    }
    { MEMW(R3++#4) = R20                    //[[1]save2                             ]
      R20 = VSATHUB(R19:18)                 //[[1]clamp to 8-bit                    ]
      R15:14 = PACKHL(R9,R8)                //[[1]y11(2)|y11(1)|y10(2)|y10(1)       ]
      IF P1 R0 = ADD(R0,R26)                //[[1]ysrc += 6*64-(64+8)               ]
    }
    { R11:10 = VADDH(R7:6,R17:16)           //[[1]B10= y10+bc|R10= y10+rc           ]
      R13:12 = SHUFFOH(R9:8,R9:8)           //[[1]y11|y11                           ]
      R14 = VADDH(R14,R21)                  //[[1]G10= y10+gc                       ]
      R15 = VADDH(R15,R21)                  //[[1]G11= y11+gc                       ]
    }
    { MEMW(R3++#4) = R20                    //[[1]save3                             ]
      R13:12 = VADDH(R13:12,R17:16)         //[[1]B11= y11+bc|R11=y11+rc            ]
      R18 = COMBINE(R14.L,R10.L)            //[[1]G10|R10   (1)                     ]
      IF P1 R1 = ADD(R1,R27)                //[[1]csrc += 6*64-8                    ]
    }:endloop0

    { R19 = COMBINE(R12.L,R10.H)            //[[e]R11|B10   (1)                     ]
      R20 = COMBINE(R12.H,R15.L)            //[[e]B11|G11   (1)                     ]
      R21 = COMBINE(R14.H,R11.L)            //[[e]G10|R10   (2)                     ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { R20 = VSATHUB(R19:18)                 //[[e]clamp to 8-bit                    ]
      R21 = VSATHUB(R21:20)                 //[[e]clamp to 8-bit                    ]
      R18 = COMBINE(R13.L,R11.H)            //[[e]R11|B10   (2)                     ]
      R19 = COMBINE(R13.H,R15.H)            //[[e]B11|G11   (2)                     ]
    }
    { MEMW(R4++#4) = R20                    //[[e]save1                             ]
      R20 = VSATHUB(R19:18)                 //[[e]clamp to 8bits                    ]
      R19:18 = MEMD(R29+#8)                 //[ callee-saved registers              ]
    }
    { MEMW(R4+#0) = R21                     //[[e]save2                             ]
      R23:22 = MEMD(R29+#24)                //[ callee-saved registers              ]
    }
    { MEMW(R4+#4) = R20                     //[[e]save3                             ]
      R21:20 = MEMD(R29+#16)                //[ callee-saved registers              ]
    }
    { R25:24 = MEMD(R29+#32)                //[ callee-saved registers              ]
      R27:26 = MEMD(R29+#40)                //[ callee-saved registers              ]
      R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	H2V2Convert, .-H2V2Convert



    /*[*****************************************************************************]*/            
    /*[  Function   : void H2V1Convert()                                            ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: convert YCbCr H2V1 to 24-bit RGB                              ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - src and dst aligned by 4 bytes                                  ]*/
    /*[           - width is a multiple of 4                                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *src                                              ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : UWord8 *dst                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25,R28                                                 ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 9*(width/4) + 11                                                ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl H2V1Convert
    .type	H2V1Convert, @function
H2V1Convert:
    { P0 = CMP.GT(R1,#3)                    //[ if !(width/4>0)                     ]
      IF !P0.new JUMPR:nt R31               //[ return                              ]
    }
    { R29 = ADD(R29,#-5*8)                  //[ allocate stack                      ]
      MEMD(R29+#(0-5*8)) = R17:16           //[ callee-saved registers              ]
      R3.H = #CB2B                          //[ R3 = CB2B|CR2R                      ]
      R4.H = #CB2G                          //[ R4 = CB2G|CR2G                      ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R3.L = #CR2R                          //[ R3 = CB2B|CR2R                      ]
      R4.L = #CR2G                          //[ R4 = CB2G|CR2G                      ]
      R6 = ASR(R1,#2)                       //[ LC0 = width/4                       ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R1 = ADD(R0,#128)                     //[ csrc = ysrc + 2*64                  ]
      R5 = R4                               //[ R5=R4 = CB2G|CR2G                   ]
      R7 = #255                             //[ 255                                 ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R22 = #(64*4-64-8)                    //[ offset for csrc                     ]
      R23 = #(64*4-8)                       //[ offset for ysrc                     ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R25:24 = VSPLATH(R7)                  //[ R25:24 = 255|255|255|255            ]
      R28 = #0                              //[ set cnt=0                           ]
      P3 = SP1LOOP0(.H2V1Convert_LOOP,R6)   //[ setup loop with LC0 = width/4       ]
    }

    .falign
.H2V1Convert_LOOP:
    { R7 = MEMUBH(R1+#64)                   //[[1]load cr                           ]
      R20 = VSATHUB(R19:18)                 //[[2]clamp to 8-bit                    ]
      R18 = COMBINE(R12.H,R15.L)            //[[2]B1|G1   (1)                       ]
      R19 = COMBINE(R14.H,R11.L)            //[[2]G0|R0   (2)                       ]
    }
    { R6 = MEMUBH(R1++#2)                   //[[1]load cb                           ]
      R21 = VSATHUB(R19:18)                 //[[2]clamp to 8-bit                    ]
      R18 = COMBINE(R13.L,R11.H)            //[[2]R1|B0   (2)                       ]
      R19 = COMBINE(R13.H,R15.H)            //[[2]B1|G1   (2)                       ]
    }
    { R9:8 = MEMUBH(R0++#4)                 //[[1]load y1(2)|y0(2)|y1(1)|y0(1)      ]
      R7:6 = PACKHL(R6,R7)                  //[[1]cb(2)|cr(2)|cb(1)|cr(1)           ]
      P0 = CMP.EQ(R28,#1)                   //[[1]P0 = (cnt%4 == 1)                 ]
      P1 = CMP.EQ(R28,#3)                   //[[1]P1 = (cnt%4/ ==3)                 ]
    }
    { R7:6 = VADDH(R7:6,R7:6)               //[[1]2*cb|2*cr                         ]
      R11:10 = SHUFFEH(R9:8,R9:8)           //[[1]y0|y0                             ]
      IF P0 R0 = ADD(R0,#(64-8))            //[[1]if(cnt%4==1) ysrc+=64-8           ]
      IF P3 MEMW(R2++#4) = R20              //[[2]save1                             ]
    }
    { R7:6 = VSUBH(R7:6,R25:24)             //[[1]cb'|cr'= 2*cb-255|2*cr-255        ]
      R15:14 = PACKHL(R9,R8)                //[[1]y1(2)|y1(1)|y0(2)|y0(1)           ]
      R28 = ADD(R28,#1)                     //[[1]cnt++                             ]
      IF P1 R0 = ADD(R0,R22)                //[[1]ysrc += 6*64-(64+8)               ]
    }
    { R16 = VMPYH(R6,R3):<<1:rnd:sat        //[[1]bc | rc = CB2B*cb' | CR2R*cr'  (1)]
      R17 = VMPYH(R7,R3):<<1:rnd:sat        //[[1]bc | rc = CB2B*cb' | CR2R*cr'  (2)]
      R12 = COMBINE(R8.H,R8.H)              //[[1]y1|y1   (1)                       ]
      R13 = COMBINE(R9.H,R9.H)              //[[1]y1|y1   (2)                       ]
    }
    { R21 = VDMPY(R7:6,R5:4):<<1:rnd:sat    //[[1]gc= CB2G*cb'+CR2G*cr'             ]
      R11:10 = VADDH(R11:10,R17:16)         //[[1]B0= y0+bc|R0= y0+rc               ]
      IF P1 R1 = ADD(R1,R23)                //[[1]csr += 6*64-8                     ]
      IF P3 MEMW(R2++#4) = R21              //[[2]save2                             ]
    }
    { R13:12 = VADDH(R13:12,R17:16)         //[[1]B1= y1+bc|R1=y1+rc                ]
      R14 = VADDH(R14,R21)                  //[[1]G0= y0+gc                         ]
      R15 = VADDH(R15,R21)                  //[[1]G1= y1+gc                         ]
      R20 = VSATHUB(R19:18)                 //[[2]clamp to 8-bit                    ]
    }
    { R18 = COMBINE(R14.L,R10.L)            //[[1]G0|R0   (1)                       ]
      R19 = COMBINE(R12.L,R10.H)            //[[1]R1|B0   (1)                       ]
      R28 = AND(R28,#3)                     //[[1]cnt%4                             ]
      IF P3 MEMW(R2++#4) = R20              //[[2]save3                             ]
    }:endloop0

    { R20 = VSATHUB(R19:18)                 //[[e]clamp to 8bit                     ]
      R18 = COMBINE(R12.H,R15.L)            //[[e]B1|G1   (1)                       ]
      R19 = COMBINE(R14.H,R11.L)            //[[e]G0|R0   (2)                       ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { R21 = VSATHUB(R19:18)                 //[[e]clamp to 8-bit                    ]
      R18 = COMBINE(R13.L,R11.H)            //[[e]R1|B0   (2)                       ]
      R19 = COMBINE(R13.H,R15.H)            //[[e]B1|G1   (2)                       ]
      MEMW(R2++#4) = R20                    //[[e]save1                             ]
    }
    { MEMW(R2++#4) = R21                    //[[e]save2                             ]
      R20 = VSATHUB(R19:18)                 //[[e]clamp to 8bit                     ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMW(R2) = R20                        //[[e]save3                             ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	H2V1Convert, .-H2V1Convert





    /*[*****************************************************************************]*/            
    /*[  Function   : void H1V2Convert()                                            ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: convert YCbCr H1V2 to 24-bit RGB                              ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - src and dst aligned by 2 bytes                                  ]*/
    /*[           - width is a multiple of 2                                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *src                                              ]*/
    /*[               R1 : UWord8 *csrc                                             ]*/
    /*[               R2 : int width                                                ]*/
    /*[               R3 : UWord8 *dst                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23,R28                                                 ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 8*(width/2) + 2*ceil(width/8) + 14                              ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl H1V2Convert
    .type	H1V2Convert, @function
H1V2Convert:
    { P0 = CMP.GT(R2,#1)                    //[ P0= (width/2 >0)?                   ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 return                       ]
      R4.H = #CB2G                          //[ R4 = CB2G|CR2G                      ]
      R8 = #255                             //[ 255                                 ]
    }
    { R29 = ADD(R29,#-4*8)                  //[ allocate stack                      ]
      MEMD(R29+#(0-4*8)) = R17:16           //[ callee-saved registers              ]
      R28 = ASR(R2,#1)                      //[ R28 = cnt = width/2                 ]
      R4.L = #CR2G                          //[ R4 = CB2G|CR2G                      ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R2 += ADD(R2,R2)                      //[ OUTPUT_PER_PIXEL*width              ]
      R6 = ADD(R28,#3)                      //[ cnt + 3                             ]
      R7 = #4                               //[ R7 = 4                              ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R2 = ADD(R3,R2)                       //[ dst + OUTPUT_PER_PIXEL*width        ]
      R6 = ASR(R6,#2)                       //[ LC1 = ceil( cnt/4 )                 ]
      R21.H = #CB2B                         //[ R21 = CB2B|CR2R                     ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R21.L = #CR2R                         //[ R21 = CB2B|CR2R                     ]
      R7 = MIN(R7,R28)                      //[ min(#4, cnt)                        ]
      LOOP1(.H1V2Convert_LOOPS,R6)          //[ setup outer loop                    ]
    }
    { R5 = R4                               //[ R5 = R4 = CB2G|CR2G                 ]
      R23:22 = VSPLATH(R8)                  //[ R23:22 = 255|255|255|255            ]
      R28 = ADD(R28,#-4)                    //[ cnt -= 4                            ]
      P3 = SP1LOOP0(.H1V2Convert_LOOPS,R7)  //[ [p] setup inner loop                ]
    }

    .falign
.H1V2Convert_LOOPS:
    { R7 = MEMUBH(R1+#64)                   //[[1]load cr                           ]
      R16 = COMBINE(R8.L,R12.L)             //[[2] G0(1)|R0(1)                      ]
      R17 = COMBINE(R13.L,R12.H)            //[[2] R0(2)|B0(1)                      ]
      R18 = COMBINE(R13.H,R8.H)             //[[2] B0(2)|G0(2)                      ]
    }
    { R6 = MEMUBH(R1++#2)                   //[[1]load cb                           ]
      R19 = COMBINE(R9.L,R14.L)             //[[2]G1(1)|R1(1)                       ]
      R16 = COMBINE(R15.L,R14.H)            //[[2]R1(2)|B1(1)                       ]
      R20 = VSATHUB(R17:16)                 //[[2] clamp                            ]
    }
    { R8 = MEMUBH(R0++#8)                   //[[1]load y0                           ]
      R7:6 = PACKHL(R6,R7)                  //[[1]cb|cr                             ]
      IF P3 MEMH(R3++#2) = R20              //[[2] save01                           ]
      R18 = VSATHUB(R19:18)                 //[[2] clamp to 8bit                    ]
    }
    { R9 = MEMUBH(R0++#(2-8))               //[[1]load y1                           ]
      R7:6 = VADDH(R7:6,R7:6)               //[[1]2*cb|2*cr                         ]
      IF P3 MEMH(R3++#2) = R20.H            //[[2]save02                            ]
      R17 = COMBINE(R15.H,R9.H)             //[[2]B1(2)|G1(2)                       ]
    }
    { R7:6 = VSUBH(R7:6,R23:22)             //[[1]cb'|cr'= 2*cb-255|2*cr-255        ]
      R13:12 = PACKHL(R8,R8)                //[[1]y0|y0                             ]
      R14 = COMBINE(R9.L,R9.L)              //[[1]y1|y1 (1)                         ]
      IF P3 MEMH(R3++#2) = R18              //[[2]save03                            ]
    }
    { R16 = VDMPY(R7:6,R5:4):<<1:rnd:sat    //[[1]CB2G*cb' + CR2G*cr'               ]
      R15 = COMBINE(R9.H,R9.H)              //[[1]y1|y1 (2)                         ]
      R20 = VSATHUB(R17:16)                 //[[2]clamp                             ]
      IF P3 MEMH(R2++#2) = R18.H            //[[2]save11                            ]
    }
    { R6 = VMPYH(R6,R21):<<1:rnd:sat        //[[1]CB2B*cb' | CR2R*cr'  (1)          ]
      R7 = VMPYH(R7,R21):<<1:rnd:sat        //[[1]CB2B*cb' | CR2R*cr'  (2)          ]
      R8 = VADDH(R8,R16)                    //[[1]G0= y0+CB2G*cb'+CR2G*cr'          ]
      IF P3 MEMH(R2++#2) = R20              //[[2]save12                            ]
    }
    { R13:12 = VADDH(R13:12,R7:6)           //[[1]B0=y0+CB2B*cb'|R0=y0+CR2R*cr'     ]
      R15:14 = VADDH(R15:14,R7:6)           //[[1]B1=y1+CB2B*cb'|R1=y1+CR2R*cr' (2) ]
      R9 = VADDH(R9,R16)                    //[[1]G1= y0+CB2G*cb'+CR2G*cr'          ]
      IF P3 MEMH(R2++#2) = R20.H            //[[2]save13                            ]
    }:endloop0

    { P0 = CMP.GE(R28,#4)                   //[ R7 = min(4, cnt)                    ]
      IF  P0.new R7 = #4                    //[                                     ]
      IF !P0.new R7 = R28                   //[                                     ]
    }
    { R0 = ADD(R0,#(64*4-8))                //[ ysrc += 64*4 -8                     ]
      R1 = ADD(R1,#(64*4-8))                //[ csrc += 64*4 -8                     ]
      LOOP0(.H1V2Convert_LOOPS,R7)          //[ setup loop                          ]
      R28 = ADD(R28,#-4)                    //[ cnt -= 4                            ]
    }:endloop1

    { R16 = COMBINE(R8.L,R12.L)             //[[e] G0(1)|R0(1)                      ]
      R17 = COMBINE(R13.L,R12.H)            //[[e] R0(2)|B0(1)                      ]
      R18 = COMBINE(R13.H,R8.H)             //[[e] B0(2)|G0(2)                      ]
    }
    { R19 = COMBINE(R9.L,R14.L)             //[[e]G1(1)|R1(1)                       ]
      R16 = COMBINE(R15.L,R14.H)            //[[e]R1(2)|B1(1)                       ]
      R17 = COMBINE(R15.H,R9.H)             //[[e]B1(2)|G1(2)                       ]
      R20 = VSATHUB(R17:16)                 //[[e] clamp to 8bit                    ]
    }
    { MEMH(R3++#2) = R20                    //[[e] save01                           ]
      R8 = VSATHUB(R19:18)                  //[[e] clamp                            ]
      R9 = VSATHUB(R17:16)                  //[[e]clamp                             ]
    }
    { MEMH(R3++#2) = R20.H                  //[[e]save02                            ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { MEMH(R3++#2) = R8                     //[[e]save03                            ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { MEMH(R2+#0) = R8.H                    //[[e]save11                            ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { MEMH(R2+#2) = R9                      //[[e]save12                            ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    }
    { MEMH(R2+#4) = R9.H                    //[[e]save13                            ]
      R29 = ADD(R29,#4*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size	H1V2Convert, .-H1V2Convert

