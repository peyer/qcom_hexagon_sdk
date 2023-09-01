/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file    "yuv2rgb.S"

#define  ENABLE_PREFETCH           // enable inlined dcfetch and dczero
#define  PREFETCH_AHEAD        32  

    /*[*****************************************************************************]*/            
    /*[  Function   : void YUVConvert()                                             ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: convert color form 24-bit YUV-plane to 24-bit RGB-plane       ]*/ 
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - in and out aligned by 2bytes                                    ]*/
    /*[           - width and height are multiple of 2                              ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *in                                               ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : int height                                               ]*/
    /*[               R3 : UWord8 *out                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl YUVConvert
    .type    YUVConvert, @function
YUVConvert:
    { P0 = CMP.GT(R2,#0)                    //[ P0 = (height>0)                     ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 then return                  ]
      R4.H = #HI(LUT_yuv2rgb)               //[ R4 = LUT_yuv2rgb                    ]
      R28 = ASR(R1,#0):rnd                  //[ R28 = ceil(width/2)                 ]
    }
    { P0 = CMP.GT(R1,#0)                    //[ P0 = (width>0)                      ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 return                       ]
      R4.L = #LO(LUT_yuv2rgb)               //[ R4 = LUT_yuv2rgb                    ]
      R5 = ADD(R28,#15)                     //[ (width/2)+15                        ]
    }
    { R29 = ADD(R29,#-5*8)                  //[ allocate stack                      ]
      MEMD(R29+#(0-5*8)) = R17:16           //[ callee-saved registers              ]
      R6 = MPYI(R1,R2)                      //[ width*height                        ]
      R16 = ASR(R5,#4)                      //[ R16 = ceil( (width/2)/16 )          ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R17 = R2                              //[ R17 = main loop cnt = height        ]
      R19 = ADDASL(R1,R1,#1)                //[ 3*width                             ]
      R2 = ADD(R6,R0)                       //[ read PTR uvbuf= in + width*height   ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R18 = R1                              //[ R18 = width                         ]
      R5 = ADD(R3,R19)                      //[ write PTR: out2= out + 3*width      ]
      R9 = MEMUB(R2+#1)                     //[ [p] u = *uvbuf++                    ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R1 = ADD(R0,R1)                       //[ read PTR ybuf2= in + width          ]
      R6 += ADD(R6,R6)                      //[ 3*width*height                      ]
      R8 = MEMUB(R2++#2)                    //[ [p] v = *uvbuf++                    ]
    }
    { R23 = ADD(R3,R6)                      //[ out + 3*width*height (dczero)       ]
      R21 = MEMUBH(R0++#2)                  //[ [p] y = *ybuf++ (1st row)           ]
      R8 = ADD(R4,R8)                       //[ [p] &LUT_yuv2rgb[u]                 ]
      R10 = ADD(R9,#-256)                   //[ [p] u -256                          ]
    }
    { R22 = ADD(R5,#31)                     //[ out2 + 31            (dczero)       ]
      R6 = MEMB(R8)                         //[ [p] vg = LUT_yuv2rgb[v]             ]
      R10 = ADD(R10,R9)                     //[ [p] buv = u+u-256                   ]
      R9 = ADD(R4,R9)                       //[ [p] &LUT_yuv2rgb[u]                 ]
    }
    { R23 = ADD(R23,#-6*32)                 //[ dczero Boundary=ArrayBoundary-6*32  ]
      R22 = AND(R22,#-32)                   //[ next $-aligned addr of out2 (dczero)]
      R7 = MEMB(R9)                         //[ [p] LUT_yuv2rgb[u]                  ]
      R14 = ADD(R6,R6)                      //[ [p] ruv = vg+vg                     ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R6 += ASR(R7,#1)                      //[ [p] -guv = vg + vyuv_tab[u]>>1      ]
      R9:8 = VSPLATH(R10)                   //[ [p] duplicate buv                   ]
      R14 = COMBINE(R14.L,R14.L)            //[ [p] ruv|ruv                         ]
    }
    
    .falign
.YUVConvert_mainLOOP:
    { R24 = R28                             //[ set prefetch loop cnt = width/2     ]
      R25 = #16                             //[ constant 16 (for inner loop control)]
      R20 = R21                             //[ [p] y = *ybuf++ (1st row)           ]
      R21 = MEMUBH(R1++#2)                  //[ [p] y = *ybuf++ (2nd row)           ]
    }
    { R17 = ADD(R17,#-2)                    //[ main loop cnt++                     ]
      LOOP1(.YUVConvert_prefetchLOOP,R16)   //[ setup prefetch loop                 ]
      R11:10 = VADDH(R21:20,R9:8)           //[p] y + buv (re-do)
    }

    .falign
.YUVConvert_prefetchLOOP:                   
    { R25 = MIN(R25,R24)                    //[ lc0 = min(cnt, 16)                  ]
#ifdef ENABLE_PREFETCH
      DCFETCH(R0+#PREFETCH_AHEAD)           //[ prefetch y (1)                      ]
      LOOP0(.YUVConvert_DCZERO,#6)          //[ setup dczero loop in case           ]
    }
    { R24 = ADD(R24,#-16)                   //[ prefetch loop cnt -= 16             ]
      DCFETCH(R1+#PREFETCH_AHEAD)           //[ prefetch y (2)                      ]
      P0 = CMP.GTU(R23,R22)                 //[ dczero will be within boundary      ]
      IF !P0.new JUMP:nt .YUVConvert_SKIP_DCZERO
    }
                                            //[ if not skip dczero                  ]
    .falign
.YUVConvert_DCZERO:
    { DCZEROA(R22)                          //[ dczero @out2                        ]
      R22 = ADD(R22,#32)                    //[ next $line                          ]
    }:endloop0

.YUVConvert_SKIP_DCZERO:
    { DCFETCH(R2+#PREFETCH_AHEAD)           //[ prefetch uv                         ]
#else
    }
    { R24 = ADD(R24,#-16)                   //[ prefetch loop cnt -= 16             ]
#endif
      LOOP0(.YUVConvert_innerLOOP,R25)      //[ setup inner loop                    ]
    }

    .falign
.YUVConvert_innerLOOP:
    { R8 = MEMUB(R2++#1)                    //[[1]v = *uvbuf++                      ]
      R14 = VADDH(R20,R14)                  //[[2]y + ruv  (1)                      ]
      R15 = VADDH(R21,R14)                  //[[2]y + ruv  (2)                      ]
      R13 = COMBINE(R6.L,R6.L)              //[[2]
    }
    { R9 = MEMUB(R2++#1)                    //[[1]u = *uvbuf++                      ]
      R12 = VSUBH(R20,R13)                  //[[2]y + guv (1)                       ]
      R6 = COMBINE(R14.H,R10.L)             //[[2]r2|b1   (1)                       ]
      R7 = COMBINE(R15.H,R11.L)             //[[2]r2|b1   (2)                       ]
    }
    { R20 = MEMUBH(R0++#2)                  //[[1]y = *ybuf++ (1)                   ]
      R7 = COMBINE(R10.H,R12.H)             //[[2]b2|g2                             ]
      R6 = COMBINE(R12.L,R14.L)             //[[2]g1|r1                             ]
      R12 = VSATHUB(R7:6)                   //[[2]clamp to 8bits                    ]
    }
    { R21 = MEMUBH(R1++#2)                  //[[1]y = *ybuf++ (2)                   ]
      MEMH(R3+#2) = R12                     //[[2]save r2|b1                        ]
      R6 = VSUBH(R21,R13)                   //[[2]y + guv (2)                       ]
      R13 = VSATHUB(R7:6)                   //[[2]clamp to 8-bit                    ]
    }
    { R8 = ADD(R4,R8)                       //[[1]&LUT_yuv2rgb[v]                   ]
      MEMH(R3+#4) = R13.H                   //[[2]save b2|g2                        ]
      R6 = COMBINE(R6.L,R15.L)              //[[2]g1|r1                             ]
      R7 = COMBINE(R11.H,R6.H)              //[[2]b2|g2                             ]
    }
    { R10 = ADD(R9,#-256)                   //[[1]u-256                             ]
      MEMH(R3++#6) = R13                    //[[2]save g1|r1                        ]
      R13 = VSATHUB(R7:6)                   //[[2]clamp to 8-bit                    ]
    }
    { R6 = MEMB(R8)                         //[[1]vg = LUT_yuv2rgb[v]               ]
      R10 = ADD(R10,R9)                     //[[1]buv = u+u-256                     ]
      R9 = ADD(R4,R9)                       //[[1]&LUT_yuv2rgb[u]                   ]
      MEMH(R5+#2) = R12.H                   //[[2]save r2|b1                        ]
    }
    { R7 = MEMB(R9)                         //[[1]LUT_yuv2rgb[u]                    ]
      R9:8 = VSPLATH(R10)                   //[[1]duplicate buv                     ]
      R14 = ADD(R6,R6)                      //[[1]ruv = vg+vg                       ]
      MEMH(R5+#4) = R13.H                   //[[2]save b2|g2                        ]
    }
    { R11:10 = VADDH(R21:20,R9:8)           //[[1]y + buv                           ]
      R6 += ASR(R7,#1)                      //[[1]-guv = vg + vyuv_tab[u]>>1        ]
      R14 = COMBINE(R14.L,R14.L)            //[[1]R14 = ruv|urv                     ]
      MEMH(R5++#6) = R13                    //[[2]save g1|r1                        ]
    }:endloop0:endloop1
    
    { P0 = CMP.GT(R17,#0)                   //[ lpcnt > 0 ?                         ]
      IF P0.new R3 = R5                     //[ update R3= out + (2*i  )* (3*width) ]
      IF P0.new R0 = R1                     //[ update R0= y + (2*i)* width + 2     ]
      IF P0.new R1 = ADD(R1,#-2)            //[ R1= y + (2*i)* width                ]
    }
    { IF P0 R5 = ADD(R5,R19)                //[ update R5= out + (2*i+1)* (3*width) ]
      IF P0 R1 = ADD(R1,R18)                //[ update R1= y + (2*i+1)* width       ]
      IF P0 JUMP .YUVConvert_mainLOOP       //[ if p0 continue main loop            ]
    }


    { R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    }
    { R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
      R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
    }
    { R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
      R29 = ADD(R29,#5*8)                   //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    YUVConvert, .-YUVConvert


    /*-----------------------------------------------------------------------------*/
    /*                         Color conversion lookup Table                       */
    /*-----------------------------------------------------------------------------*/
    .globl LUT_yuv2rgb
    .data
    .p2align 3
    .type    LUT_yuv2rgb, @object
    .size    LUT_yuv2rgb, 256
LUT_yuv2rgb:
    .byte -89,-89,-89,-89,-89,-89,-89,-89
    .byte -89,-89,-89,-89,-89,-89,-89,-89
    .byte -89,-88,-88,-87,-86,-85,-84,-84
    .byte -83,-82,-81,-80,-80,-79,-78,-77
    .byte -76,-76,-75,-74,-73,-72,-72,-71
    .byte -70,-69,-68,-68,-67,-66,-65,-64
    .byte -64,-63,-62,-61,-60,-60,-59,-58
    .byte -57,-56,-56,-55,-54,-53,-52,-52
    .byte -51,-50,-49,-48,-48,-47,-46,-45
    .byte -44,-44,-43,-42,-41,-40,-40,-39
    .byte -38,-37,-36,-36,-35,-34,-33,-32
    .byte -32,-31,-30,-29,-28,-28,-27,-26
    .byte -25,-24,-24,-23,-22,-21,-20,-20
    .byte -19,-18,-17,-16,-16,-15,-14,-13
    .byte -12,-12,-11,-10,-9,-8,-8,-7
    .byte -6,-5,-4,-4,-3,-2,-1,0
    .byte 0,0,1,2,3,4,4,5
    .byte 6,7,8,8,9,10,11,12
    .byte 12,13,14,15,16,16,17,18
    .byte 19,20,20,21,22,23,24,24
    .byte 25,26,27,28,28,29,30,31
    .byte 32,32,33,34,35,36,36,37
    .byte 38,39,40,40,41,42,43,44
    .byte 44,45,46,47,48,48,49,50
    .byte 51,52,52,53,54,55,56,56
    .byte 57,58,59,60,60,61,62,63
    .byte 64,64,65,66,67,68,68,69
    .byte 70,71,72,72,73,74,75,76
    .byte 76,77,78,79,80,80,81,82
    .byte 83,84,84,85,86,87,88,88
    .byte 89,89,89,89,89,89,89,89
    .byte 89,89,89,89,89,89,89,89

