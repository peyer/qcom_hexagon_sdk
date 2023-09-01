/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file       "scale.S"

#define  ENABLE_PREFETCH

#ifdef  ENABLE_PREFETCH
#define DH_PREFETCH_AHEAD     64
#define DQ_PREFETCH_AHEAD     16
#endif
    /*[*****************************************************************************]*/
    /*[  Function   : int scale_DownSampleToHalf()                                  ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Down sampling the image by half                               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - imgSrc is aligned by 2bytes                                     ]*/
    /*[           - width and height are multiples of 2                             ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *imgSrc                                           ]*/
    /*[               R1 : int    width                                             ]*/
    /*[               R2 : int    height                                            ]*/
    /*[               R3 : UWord8 *imgDst                                           ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : 0: error, 1: sucess                                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R21, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 24                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (height/2)*(4 + 2*ceil(width/4)) + 8 (ideal)                    ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl scale_DownSampleToHalf
    .type       scale_DownSampleToHalf, @function
scale_DownSampleToHalf:
    { R4 = OR(R2,R1)                        //[ width || height                     ]
      P0 = CMP.GT(R1,#1)                    //[ (dstWidth >0)                       ]
      P1 = CMP.GT(R2,#1)                    //[ (dstHeight>0)                       ]
      R5 = MPYI(R1,R2)                      //[ width*height                        ]
    }
    {
      P0 = AND(P0,P1)                       //[ if !((dstHeight>0)&&(dstWidth>0))   ]
    }
    {
      IF !P0 R0 = #1                        //[ then return 1                       ]
      IF !P0 JUMPR R31                      //[                                     ]
      IF  P0 R28 = R1                       //[ R28 = width                         ]
    }
    { P0 = TSTBIT(R4,#0)                    //[ if((width&1)!=0||(height&1)!=0)     ]
      IF  P0.new R0 = #0                    //[ then return 0                       ]
      IF  P0.new JUMPR:nt R31               //[
      IF !P0.new R1 = ADD(R0,R1)            //[ srcPixels1 = srcPixels+width        ]
    }
    { R29 = ADD(R29,#-3*8)                  //[ reserve stack                       ]
      MEMD(R29+#(-3*8+0)) = R17:16          //[ callee-saved regs.                  ]
      P1 = TSTBIT(R28,#1)                   //[ if (width%4==2)                     ]
      IF  P1.new R1 = ADD(R1,#-2)           //[ then offset srcPixels1              ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved regs.                  ]
      R4 = MUX(P1,#4,#0)                    //[ set  alignment offset = 4 or 0      ]
      R16 = ASRRND(R28,#2)                  //[ ceil( (width/2)/2)                  ]
      R17 = ADD(R28,#2*31)                  //[ 2*(width/2 + 31)                    ]
    }
#ifndef ENABLE_PREFETCH
    { MEMD(R29+#16) = R21:20                //[ callee-saved regs.                  ]
      P2 = R4                               //[ CR = alignment offset               ]
      R28 = ADD(R28,R28)                    //[ R28 = 2*width                       ]
    }

    .falign
.DownSampleToHalf_outerLOOP:
    { R4 = R0                               //[ srcPixels0                          ]
      R5 = ADD(R1,#8)                       //[ srcPixels1                          ]
      R9:8   = MEMUBH(R1+#0)                //[[p]load from odd row                 ]
      R11:10 = MEMUBH(R1+#4)                //[[p]load from odd row                 ]
    }
    { P3 = SP1LOOP0(.DownSampleToHalf_innerLOOP,R16)
                                            //[ setup loop0: lc0= ceil((width/2)/2) ]
      R7:6 = MEMUBH(R4++#4)                 //[[p]load from even row                ]
      R13:12 = VALIGNB(R11:10,R9:8,P2)      //[[p]align data                        ]
    }
#else
    { MEMD(R29+#16) = R21:20                //[ callee-saved regs.                  ]
      P2 = R4                               //[ CR = alignment offset               ]
      R17 = ASR(R17,#6)                     //[ ceil((width/2)/32)                  ]
      R21 = ADD(R3,#-32)                    //[ imgDst-32                           ]
    }
    { R28 = ADD(R28,R28)                    //[ R28 = 2*width                       ]
      R21 += LSR(R5,#2)                     //[ imgDst+(width/2)*(height/2)-32(DZ)  ]
      R18 = #16                             //[ constant                            ]
    }

    .falign
.DownSampleToHalf_outerLOOP:
    { R5:4 = R1:0                           //[ srcPixels1 : srcPixels0             ]
      R19 = R16                             //[ LC = ceil(dstWidth/2)               ]
      R18 = MIN(R18,R16)                    //[ min(16, ceil((width/2)/2))          ]
      R20 = ADD(R3,#31)                     //[ (dczero)round to next $line         ]
    }
    { LOOP1(.DownSampleToHalf_PREFETCHLOOP,R17)
                                            //[ setup loop1:lc1=ceil((width/2)/32)  ]
      R20 = AND(R20,#-32)                   //[ (dczero)algined to $line            ]
      R9:8   = MEMUBH(R5++#8)               //[[p]load from odd row                 ]
      R11:10 = MEMUBH(R5+#4)                //[[p]load from odd row                 ]
    }
    { P3 = SP1LOOP0(.DownSampleToHalf_innerLOOP,R18)
                                            //[ setup loop0: lc0=min(16,LC)         ]
      R7:6 = MEMUBH(R4++#4)                 //[[p]load from even row                ]
      R13:12 = VALIGNB(R11:10,R9:8,P2)      //[[p]align data                        ]
      DCFETCH(R5+#(DH_PREFETCH_AHEAD+0))    //[ prefetch 1$line                     ]
    }

    .falign
.DownSampleToHalf_PREFETCHLOOP:
    { DCFETCH(R5+#(DH_PREFETCH_AHEAD+32))   //[ prefetch 1$line                     ]
      R19 = ADD(R19,#-16)                   //[ LC -= 16                            ]
    }
    { DCFETCH(R4+#(DH_PREFETCH_AHEAD+0))    //[ prefetch 1$line                     ]
      R18 = MIN(R18,R19)                    //[ min(16,LC)                          ]
      P0 = CMP.GTU(R20,R21)                 //[ dczero out of boundary?             ]
    }
    { DCFETCH(R4+#(DH_PREFETCH_AHEAD+32))   //[ prefetch 1$line                     ]
      IF P0 JUMP .DownSampleToHalf_innerLOOP//[ if out of boundary then skip dczero ]
    }
    { DCZEROA(R20)                          //[ dczero 1 $line of output array      ]
      R20 = ADD(R20,#32)                    //[ update to next $line                ]
    }
#endif

    .falign
.DownSampleToHalf_innerLOOP:
    { R11:10 = MEMUBH(R5++#4)               //[[1]load from odd row                 ]
      R9:8 = R11:10                         //[[1]store previous pixels             ]
      R13:12 = VRADDUB(R7:6,R13:12)         //[[2]sum                               ]
      IF P3 MEMB(R3++#1) = R14              //[[3]save output                       ]
    }
    { R7:6 = MEMUBH(R4++#4)                 //[[1]load from even row                ]
      R13:12 = VALIGNB(R11:10,R9:8,P2)      //[[1]align data                        ]
      R15:14 = VASRW(R13:12,#2)             //[[2]sum>>2                            ]
      IF P3 MEMB(R3++#1) = R15              //[[3]save output                       ]
    }:endloop0

#ifdef ENABLE_PREFETCH
    { DCFETCH(R5+#(DH_PREFETCH_AHEAD+0))    //[ prefetch 1$line                     ]
      LOOP0(.DownSampleToHalf_innerLOOP,R18)
                                            //[ setup loop0: lc0=min(16,LC)         ]
    }:endloop1
#endif

    { MEMB(R3++#1) = R14                    //[[e]save output                       ]
      R0 = ADD(R0,R28)                      //[ update srcPixels0                   ]
      R1 = ADD(R1,R28)                      //[ update srcPixels1                   ]
      R2 = ADD(R2,#-2)                      //[ i -=2                               ]
    }
    { IF !P1 MEMB(R3++#1) = R15             //[[e]save output                       ]
      P0 = CMP.GT(R2,#0)                    //[ P0 = (i > 0)                        ]
      IF P0.new R18 = #16                   //[ Constant                            ]
      IF P0.new JUMP:t .DownSampleToHalf_outerLOOP
                                            //[ IF P0 contine next line             ]
    }
.scale_DownSampleToHalf_END:
    { R21:20 = MEMD(R29+#16)                //[ restore callee-saved regs.          ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved regs.          ]
    }
    { R17:16 = MEMD(R29+#0)                 //[ restore callee-saved regs.          ]
      R29 = ADD(R29,#3*8)                   //[ pop stack                           ]
      R0 = #1                               //[ return value =1                     ]
      JUMPR R31                             //[ return                              ]
    }
    .size       scale_DownSampleToHalf, .-scale_DownSampleToHalf


    /*[*****************************************************************************]*/
    /*[  Function   : int scale_DownSampleToQuarter()                               ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Down sampling the image by Quarter                            ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - imgSrc is aligned by 8bytes                                     ]*/
    /*[           - imgDst is aligned by 2bytes                                     ]*/
    /*[           - width and height are mulitples of 4                             ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *imgSrc                                           ]*/
    /*[               R1 : int    width                                             ]*/
    /*[               R2 : int    height                                            ]*/
    /*[               R3 : UWord8 *imgDst                                           ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : 0: error, 1: sucess                                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (height/4)*(5 + 5*ceil(width/16)) + 11 (ideal)                  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl scale_DownSampleToQuarter
    .type       scale_DownSampleToQuarter, @function
scale_DownSampleToQuarter:
    { R4 = OR(R1,R2)                        //[ width | height                      ]
      P0 = CMP.GT(R1,#3)                    //[ (width /4>0)                        ]
      P1 = CMP.GT(R2,#3)                    //[ (height/4>0)                        ]
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
    }
    {
      P0 = AND(P0,P1)                       //[ if !((height/4>0)&&(Width/4>0))     ]
    }
    {
      IF !P0 R0 = #1                        //[ then return 1                       ]
      IF !P0 JUMPR R31                      //[                                     ]
      IF  P0 R28 = R1                       //[ R28 = width                         ]
    }
    { P0 = BITSCLR(R4,#3)                   //[ if (width&3)!=0||(height&3)!=0      ]
      IF !P0.new R0 = #0                    //[                                     ]
      IF !P0.new JUMPR:nt R31               //[ then return 0                       ]
      R6 = ADD(R1,#15)                      //[ width +15                           ]
    }
    { R29 = ADD(R29,#-5*8)                  //[ allocate stack                      ]
      MEMD(R29+#-8) = R25:24                //[ callee-saved regs                   ]
      P2 = TSTBIT(R1,#3)                    //[ P2 =  (width&8)                     ]
      M0 = R1                               //[ M0 = width                          ]
    }
#ifdef ENABLE_PREFETCH
    { MEMD(R29+#24) = R23:22                //[ callee-saved regs                   ]
      R7 = MPYI(R1,R2)                      //[ width*height                        ]
      R24 = LSR(R6,#4)                      //[ n16= ceil(width/16)                 ]
      R22 = ADD(R3,#-64)                    //[                                     ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved regs                   ]
      R5 = ADDASL(R1,R1,#1)                 //[ 3*width                             ]
      R25 = ADD(R24,#15)                    //[ n16 + 15                            ]
      R20 = #16                             //[ constant                            ]
    }
    { MEMD(R29+#8)  = R19:18                //[ callee-saved regs                   ]
      R25 = LSR(R25,#4)                     //[ ceil(n16/16)
      R22 += LSR(R7,#4)                     //[ dst+(width/4)*(height/4)-64:boundary]
      R5 = SUB(#16,R5)                      //[ 16 - 3*width                        ]
    }
    { MEMD(R29+#0)  = R17:16                //[ callee-saved regs                   ]
      M1 = R5                               //[ 16 - 3*width                        ]
      R1 = ADDASL(R0,R28,#2)                //[ img + 4*width                       ]
      R21 = ADD(R3,#31)                     //[ imgDst + 31                         ] 
    }

    .falign
.DownSampleToQuarter_outerLOOP:             //[ P3 must be clean @here              ]
    { R19 = R24                             //[ cnt = n16                           ]
      R20 = MIN(R24,R20)                    //[ LC = min(cnt,16)                    ]
      R21 = AND(R21,#-32)                   //[ $line-aligned address of output     ]
      LOOP1(.DownSampleToQuarter_PREFETCHLOOP,R25)
                                            //[ setup loop1:lc1=ceil(n16/16)        ]
    }

    .falign
.DownSampleToQuarter_PREFETCHLOOP:
    /*-----------------------------------------------------------------------------*/
    /*                     dcfetch 4$line from 4 rows                              */
    /*                     dczero  1$line for output                               */
    /*-----------------------------------------------------------------------------*/
    { LOOP0(.DownSampleToQuarter_innerLOOP,R20)
                                            //[ setup loop0:lc0 = min(cnt,16)       ]
      R12 = R0                              //[ address of line 0                   ]
      R19 = ADD(R19,#-16)                   //[ cnt -= 16                           ]
      R18 = ASRRND(R20,#1)                  //[ # of dcfetch lines per row          ]
    }

    .falign
.DownSampleToQuarter_prefetch:
    { DCFETCH(R12+#DQ_PREFETCH_AHEAD)       //[ dcfetch                             ]
      R12 = ADD(R12,#32)                    //[ update to next $line                ]
      R13 = ADD(R12,R28)                    //[ address of line 1                   ]
    }
    { DCFETCH(R13+#DQ_PREFETCH_AHEAD)       //[ dcfetch                             ]
      R13 = ADD(R13,R28)                    //[ address of line 2                   ]
      R18 = ADD(R18,#-1)                    //[                                     ]
    }
    { DCFETCH(R13+#DQ_PREFETCH_AHEAD)       //[ dcfetch                             ]
      R13 = ADD(R13,R28)                    //[ address of line 3                   ]
      P0 = CMP.GT(R18,#0)                   //[                                     ]
    }
    { DCFETCH(R13+#DQ_PREFETCH_AHEAD)       //[ dcfetch                             ]
      IF P0 JUMP .DownSampleToQuarter_prefetch
                                            //[ if P0 contine prefetch loop         ] 
    }
    { P0 = CMP.GTU(R21,R22)                 //[ will dczero out of boundary?        ]
      IF P0.new JUMP:nt .DownSampleToQuarter_innerLOOP
                                            //[ if P0 then skip dczero              ]
      R20 = MIN(R19,R20)                    //[ LC = min(cnt,16)                    ]
    }
    { DCZEROA(R21)                          //[ dc zeor output                      ]
      R21 = ADD(R21,#32)                    //[ update to next output $line         ]
    }
    { DCZEROA(R21)                          //[ dc zeor output                      ]
      R21 = ADD(R21,#32)                    //[ update to next output $line         ]
    }
#else
    { MEMD(R29+#24) = R23:22                //[ callee-saved regs                   ]
      R24 = LSR(R6,#4)                      //[ n16= ceil(width/16)                 ]
      R5 = ADDASL(R1,R1,#1)                 //[ 3*width                             ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved regs                   ]
      R5 = SUB(#16,R5)                      //[ 16 - 3*width                        ]
    }
    { MEMD(R29+#8)  = R19:18                //[ callee-saved regs                   ]
      M1 = R5                               //[ 16 - 3*width                        ]
      R1 = ADDASL(R0,R28,#2)                //[ img + 4*width                       ]
    }
    { MEMD(R29+#0)  = R17:16                //[ callee-saved regs                   ]
      P3 = SP1LOOP0(.DownSampleToQuarter_innerLOOP,R24)
                                            //[ setup loop0:lc0=n16                 ]
    }
    .falign
.DownSampleToQuarter_outerLOOP:             //[ P3 must be clean @here              ]
#endif

    .falign
.DownSampleToQuarter_innerLOOP:
    { R11:10 = MEMD(R0++M0)                 //[[1]load from line #0                 ]
      R13:12 = MEMD(R0+#8)                  //[[1]                                  ]
      R5:4 += VRADDUB(R11:10,R15:14)        //[[2]sum+=p[0]+p[1]+p[2]+p[3] (line2,3)]
      R7:6 += VRADDUB(R9:8,  R17:16)        //[[2]sum+=p[0]+p[1]+p[2]+p[3] (line2,3)]
    }
    { R15:14 = MEMD(R0++M0)                 //[[1]load from line #1                 ]
      R17:16 = MEMD(R0+#8)                  //[[1]                                  ]
      R9 = VASRW(R7:6,#4)                   //[[2] sum>>4                           ]
      R8 = VASRW(R5:4,#4)                   //[[2]                                  ]
    }
    { R5:4 = VRADDUB(R11:10,R15:14)         //[[1]sum+=p[0]+p[1]+p[2]+p[3] (line0,1)]
      R11:10 = MEMD(R0++M0)                 //[[1]load from line #2                 ]
      R9:8   = MEMD(R0+#8)                  //[[1]                                  ]
      R18 = VTRUNEHB(R9:8)                  //[[2]truncate to 8-bit                 ]
    }
    { R7:6 = VRADDUB(R13:12,R17:16)         //[[1]sum+=p[0]+p[1]+p[2]+p[3] (line0,1)]
      R17:16 = MEMD(R0+#8)                  //[[1]load from line #3                 ]
      IF P3 MEMH(R3++#2) = R18              //[[2]save 2 output pixels              ]
    }
    { R15:14 = MEMD(R0++M1)                 //[[1]load from line #3                 ]
      IF P3 MEMH(R3++#2) = R18.H            //[[2]save 2 output pixels              ]
#ifdef ENABLE_PREFETCH
      P3 = CMP.EQ(R0,R0)                    //[[1]set P3                            ]
    }:endloop0:endloop1
#else
    }:endloop0
#endif
    { R5:4 += VRADDUB(R11:10,R15:14)        //[[e]sum+=p[0]+p[1]+p[2]+p[3] (line2,3)]
      R7:6 += VRADDUB(R9:8,  R17:16)        //[[e]sum+=p[0]+p[1]+p[2]+p[3] (line2,3)]
      R2 = ADD(R2,#-4)                      //[ i-= 4                               ]
    }
    { R9 = VASRW(R7:6,#4)                   //[[e] sum>>4                           ]
      R8 = VASRW(R5:4,#4)                   //[[e]                                  ]
    }
    { R18 = VTRUNEHB(R9:8)                  //[[e]truncate to 8-bit                 ]
      P0 = CMP.GT(R2,#0)                    //[ P0 = (i>0)                          ]
#ifdef ENABLE_PREFETCH
      P3 = CMP.GT(R0,R0)                    //[ clean P3                            ]
#else
      P3 = SP1LOOP0(.DownSampleToQuarter_innerLOOP,R24)
                                            //[ setup loop0:lc0=n16                 ]
#endif
    }
    { MEMH(R3++#2) = R18                    //[[e]save 2 output pixels              ]
      R21 = ADD(R3,#(31+2))                 //[                                     ]
      IF P0 R0 = R1                         //[                                     ]
      IF P0 R20 = #16                       //[                                     ]
    }
    { IF !P2 MEMH(R3++#2) = R18.H           //[[e]save 2 output pixels              ]
      IF !P2 R21 = ADD(R21,#2)              //[                                     ]
      R1 += ASL(R28,#2)                     //[                                     ]
      IF P0 JUMP .DownSampleToQuarter_outerLOOP
                                            //[ IF P0 continue loop                 ]
    }

.DownSampleToQuarter_END:
    { R25:24 = MEMD(R29+#32)                //[ restore callee-saved regs           ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved regs           ]
    }
    { R21:20 = MEMD(R29+#16)                //[ restore callee-saved regs           ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved regs           ]
    }
    { R17:16 = MEMD(R29+#0)                 //[ restore callee-saved regs           ]
      R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      R0 = #1                               //[ return value = 1                    ]
      JUMPR R31                             //[ return                              ]
    }
    .size       scale_DownSampleToQuarter, .-scale_DownSampleToQuarter
