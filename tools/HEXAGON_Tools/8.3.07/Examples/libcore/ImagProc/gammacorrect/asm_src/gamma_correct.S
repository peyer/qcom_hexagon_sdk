/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file "gamma_correct.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void gamma_correct()                                          ]*/
    /*[*****************************************************************************]*/
    /*[  Description: perform gama correction with lookup table                     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord16 *ch                                              ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : int height                                               ]*/
    /*[               R3 : UWord16 *LUTtable                                        ]*/
    /*[               R4 : int mode                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R13                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - mode1 - 2.0*W*H + 11                                            ]*/
    /*[           - mode0 - 1.5*W*H + 8                                             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl gamma_correct
    .type    gamma_correct, @function
gamma_correct:
    { P0 = CMP.GT(R1,#0)                    //[ width > 0                           ]
      P1 = CMP.GT(R2,#0)                    //[ height > 0                          ]
      P2 = CMP.EQ(R4,#0)                    //[ mode==0?                            ]
      R1 = MPYI(R1,R2)                      //[ width*height                        ]
    }
    {
      P0 = AND(P0,P1)                       //[ P0 = width>0 & height>0             ]
    }
    {
      IF !P0 JUMPR R31                      //[ IF !P0 then return                  ]
      IF  P0 R13 = ADD(R1,#1)               //[ (width*height + 1)                  ]
      R1 = AND(R1,#1)                       //[ (width*height) & 1                  ]
    }
    { P1 = CMP.EQ(R1,#0)                    //[ P0 = (width*height is even)         ]
      R2 = R0                               //[ set write Ptr R2 = ch               ]
      R13 = LSR(R13,#1)                     //[ ceil((width*height)/2)              ]
      IF P2 JUMP .GammaCorrect_Mode0        //[ got mode0                           ]
    }

    /*-----------------------------------------------------------------------------*/
    /*                                MODE 1                                       */
    /*-----------------------------------------------------------------------------*/
.GammaCorrect_Mode1:
    { R1 = CONST32(#OfstShftTab)            //[ setup table                         ]
      R10 = R3                              //[ validate first LUT1                 ]
      R9 = #0                               //[ validate first LUT2                 ]
      R4 = MEMUH(R0++#2)                    //[[p]load ch[i]                        ]
    }
    { P3 = SP1LOOP0(.GammaCorrect_loop1,R13)//[ setup loop, lc0 = width*height/2+1  ]
      R1 = TABLEIDXW(R4,#4,#8)              //[[p]&OfstShftTab[ (ch[i]&0x0FFF) >> 8]]
      R5 = MEMUH(R0++#2)                    //[[p]load ch[i+1]                      ]
    }
    { R6 = MEMW(R1)                         //[[p]get offset|nshift (ch[i])         ]
      R1 = TABLEIDXW(R5,#4,#8)              //[[p]&OfstShftTab[(ch[i+1]&0x0FFF) >>8]]
    }

    .falign
.GammaCorrect_loop1:
    { R8 = ASRH(R6)                         //[[2] offset (ch[i])                   ]
      R7 = MEMW(R1)                         //[[2]get offset|nshift (ch[i+1])       ]
      R11 = MEMUH(R10)                      //[[3]LUT1                              ]
      R10 = ADDASL(R3,R9,#1)                //[[3]LUT addr2                         ]
    }
    { R4 = MEMUH(R0++#2)                    //[[1]load ch[i]                        ]
      R8 += LSR(R4,R6)                      //[[2](x >>nshift) + offset (ch[i])     ]
      R9 = ASRH(R7)                         //[[2] offset (ch[i+1])                 ]
      R12 = MEMUH(R10)                      //[[3]LUT2                              ]
    }
    { R1 = TABLEIDXW(R4,#4,#8)              //[[1]&OfstShftTab[ (ch[i]&0x0FFF) >> 8]]
      R5 = MEMUH(R0++#2)                    //[[1]load ch[i+1]                      ]
      R9 += LSR(R5,R7)                      //[[2](x >>nshift) + offset (ch[i+1])   ]
      IF P3 MEMH(R2++#2) = R11              //[[3] save corrected chi[i+1]          ]
    }
    { R6 = MEMW(R1)                         //[[1]get offset|nshift (ch[i])         ]
      R1 = TABLEIDXW(R5,#4,#8)              //[[1]&OfstShftTab[(ch[i+1]&0x0FFF)>>8] ]
      R10 = ADDASL(R3,R8,#1)                //[[2]LUT addr1                         ]
      IF P3 MEMH(R2++#2) = R12              //[[3] save corrected chi[i]            ]
    }:endloop0

      
    { R11 = MEMUH(R10)                      //[[e]LUT1                              ]
      R10 = ADDASL(R3,R9,#1)                //[[e]LUT addr2                         ]
    }
    { MEMH(R2++#2) = R11                    //[[e] save corrected chi[i+1]          ]
      IF !P1 JUMPR R31                      //[ if width*height is odd, then done   ]
    }
    { R12 = MEMUH(R10)                      //[[e]LUT2                              ]
    }
    { MEMH(R2) = R12                        //[[e] save corrected chi[i]            ]
      JUMPR R31                             //[ return                              ]
    }

    /*-----------------------------------------------------------------------------*/
    /*                                MODE 0                                       */
    /*-----------------------------------------------------------------------------*/
    .falign
.GammaCorrect_Mode0:
    { R9 = R3                               //[ make first LUT valid                ]
      P3 = SP1LOOP0(.GammaCorrect_loop2,R13)//[ setup loop, lc0 = width*height/2+1  ]
      R4 = MEMUH(R0++#2)                    //[[p]load ch[i]                        ]
      R6.L = #0xFFFE                        //[[p]used to mask out the most LSB     ]
    }

    .falign
.GammaCorrect_loop2:
    { R5 = MEMUH(R0++#2)                    //[[2]load ch[i+1]                      ]
      R6 &=LSR(R4,#1)                       //[[2]( ch[i]>>2 ) << 1                 ]
      R7.L = #0XFFFE                        //[[2]used to mask out the most LSB     ]
      IF P3 MEMH(R2++#2) = R10              //[[3]save corrected ch[i]              ]
    }
    { R4 = MEMUH(R0++#2)                    //[[1]load ch[i]                        ]
      R8 = ADD(R3,R6)                       //[[2]address in LUT for ch[i]          ]
      R7 &=LSR(R5,#1)                       //[[2](ch[i+1]>>2) << 1                 ]
      R11 = MEMUH(R9)                       //[[3] LUT 2                            ]
    }
    { R6.L = #0xFFFE                        //[[1]used to mask out the most LSB     ]
      R9 = ADD(R3,R7)                       //[[2]address in LUT for ch[i+1]        ]
      R10 = MEMUH(R8)                       //[[2]LUT 1                             ]
      IF P3 MEMH(R2++#2) = R11              //[[3]save corrected ch[i+1]            ]
    }:endloop0
      
    { MEMH(R2++#2) = R10                    //[[e]save corrected ch[i]              ]
      IF !P1 JUMPR R31                      //[ if width*height is odd, then done   ]
    }
    { R11 = MEMUH(R9)                       //[[e] LUT 2                            ]
    }
    { MEMH(R2) = R11                        //[[e]save corrected ch[i+1]            ]
      JUMPR R31                             //[ return                              ]
    }
    .size    gamma_correct, .-gamma_correct




    /*-----------------------------------------------------------------------------*/
    /*                                Table                                        */
    /*-----------------------------------------------------------------------------*/
    .section    .rodata
    .p2align 6                              // aligned for tableidx
    .type    OfstShftTab, @object
    .size    OfstShftTab, 64
OfstShftTab:
    .hword    0,0                           // x < 256
    .hword    1,128
    .hword    1,128                         // 256 <= x < 768
    .hword    2,320
    .hword    2,320
    .hword    2,320                         // 768 <= x < 1536
    .hword    3,512
    .hword    3,512
    .hword    3,512
    .hword    3,512
    .hword    3,512                         // 1536 <= x < 2816
    .hword    4,688
    .hword    4,688
    .hword    4,688
    .hword    4,688
    .hword    4,688                         // x >= 2816

