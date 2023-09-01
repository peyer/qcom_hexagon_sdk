/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file "luma_filt.S"

#define    OFFSET_HP_COEF              0
#define    OFFSET_HP_EPSILON           20
#define    OFFSET_HP_SHIFT             22
#define    OFFSET_LP_COEF              24
#define    OFFSET_LP_EPSILON           44
#define    OFFSET_LP_SHIFT             46
#define    OFFSET_MASKSEL              68

    /*[*****************************************************************************]*/            
    /*[  Function   : void luma_filt()                                              ]*/ 
    /*[*****************************************************************************]*/            
    /*[  Description: epsilon filtering luma component                              ]*/ 
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - filter_memory is aligned to 8-byte boundary                     ]*/
    /*[           - the most current is the first in filtMem                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8 *ch                                               ]*/
    /*[               R1 : int width                                                ]*/
    /*[               R2 : int height                                               ]*/
    /*[               R3 : LumaFilterInStruct *param                                ]*/
    /*[               R4 : UWord8 *out                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R27                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 72                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - [12*(W-1)+3]*(H-2) + 14                                         ]*/
    /*[*****************************************************************************]*/            
    .text
    .p2align 2
    .p2align 4,,15
    .globl luma_filt
    .type    luma_filt, @function
luma_filt:
    { P0 = CMP.GT(R1,#2)                    //[ P0 = (width  > 2)                   ]
      P1 = CMP.GT(R2,#2)                    //[ P1 = (height > 2)                   ]
      R4 += ADD(R1,#-1)                     //[ R4 = out + width-1                  ]
    }
    {
      P0 = AND(P0,P1)                       //[ P0 = (widht>2)&(height>2)           ]
    }
    {
      IF !P0 JUMPR R31                      //[ IF !P0 return                       ]
      R9 = #0                               //[ R9 = 0                              ]
      R8 = ADD(R3,#48)                      //[ &params->identity_coeff             ]
    }
    /*-----------------------------------------------------------------------------*/
    /* - Stack:                                                                    */
    /*    R29 + 48:  *identity_coeff                                               */
    /*    R29 + 52:  0                                                             */
    /*    R29 + 56:  *hp_coeff                                                     */
    /*    R29 + 60:  hp_shift                                                      */
    /*    R29 + 64:  *lp_coeff                                                     */
    /*    R29 + 68:  lp_shift                                                      */
    /*-----------------------------------------------------------------------------*/
    { R29 = ADD(R29, #-9*8)                 //[ allocate stack                      ]
      MEMD(R29+#(0-9*8)) = R17:16           //[ callee-saved registers              ]
      R6 = ADD(R1,R1)                       //[ 2*width                             ]
      M0 = R1                               //[ M0 = width                          ]
    }
    { MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
      R6 = SUB(#1,R6)                       //[ 1-2*width                           ]
      R10 = ADD(R3,#OFFSET_HP_COEF)         //[ &params->hp_coeff[0]                ]
      R11 = MEMH(R3+#OFFSET_HP_SHIFT)       //[ params->hp_shift                    ]
    }
    { MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
      R1 = ADD(R1,#-1)                      //[ for loop counter: width -1          ]
      M1 = R6                               //[ M1 = 1-2*width                      ]
      R21 = MEMH(R3+#OFFSET_HP_EPSILON)     //[ R21 = params->hp_epsilon            ]
    }
    { MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
      R12 = ADD(R3,#OFFSET_LP_COEF)         //[ &params->lp_coeff[0]                ]
      R13 = MEMH(R3+#OFFSET_LP_SHIFT)       //[ get params->lp_shift                ]
    }
    { MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
      R20 = MEMH(R3+#OFFSET_LP_EPSILON)     //[ R20 = params->lp_epsilon            ]
      R0 = ADD(R0,#1)                       //[ R0 = ch+1                           ]
      R2 = ADD(R2,#-2)                      //[ for colum loop counter = height-2   ]
    }
    { MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
      R14 = MEMUH(R3+#OFFSET_MASKSEL)       //[ read params->mask_sel               ]
    }
    { MEMD(R29+#48) = R9:8                  //[ save parameters to stack            ]
      P0 = CMP.EQ(R14,#0)                   //[ mask_sel == 0?                      ]
    }
    { MEMD(R29+#56) = R11:10                //[ save HP filter parameters to stack  ]
      IF !P0 JUMP .MASK_SEL_EQ_1            //[ JUMP TO mode 1                      ]
    }
.MASK_SEL_EQ_0:
    /*-----------------------------------------------------------------------------*/
    /*                             Mode 0                                          */
    /*-----------------------------------------------------------------------------*/
    { MEMD(R29+#64) = R13:12                //[ save LP filter parameters to stack  ]
      LOOP1(.lumafilt_i_loop1,R2)           //[ setup outer loop with lc1=height-2  ]
    }

    .falign
.lumafilt_i_loop1:    
    { R16 = MEMUB(R0+#-1)                   //[ load rio[0]                         ]
      R17 = MEMUB(R0++M0)                   //[ load rio[1]                         ]
      R4 = ADD(R4,#2)                       //[ update R4 = &out[i*width+1]         ]
      P3 = SP1LOOP0(.lumafilt_j_loop1,R1)   //[ setup inner loop with lc0 = width-1 ]
    }
    { R12 = MEMUB(R0+#-1)                   //[ load rio[3]                         ]
      R13 = MEMUB(R0++M0)                   //[ load rio[4]                         ]
      R16 = ASLH(R16)                       //[ R16 = rio[0]|x                      ]
    }
    { R7 = MEMUB(R0+#-1)                    //[ load rio[6]                         ]
      R15 = MEMUB(R0++M1)                   //[ load rio[7]                         ]
      R18 = COMBINE(R13.L,R12.L)            //[ R17 = rio[4]|rio[3]                 ]
    }                                       
    /*-----------------------------------------------------------------------------*/
    /* In inner loop, only 3 pixels will be read, the rest 6 are re-used from      */ 
    /* previous computation                                                        */
    /*-----------------------------------------------------------------------------*/
    .falign
.lumafilt_j_loop1:
    { R12 = MEMUB(R0++M0)                   //[[1]load pixel roi[2]                 ]
      R6 = ASRH(R16)                        //[[1]load rio[0]                       ]
      R3 = ZXTH(R17)                        //[[1]load rio[1]                       ]
      R9:8 = VMAXW(R9:8,R11:10)             //[[2] max(s3:2, s2: s1)                ]
    }
    { R13 = MEMUB(R0++M0)                   //[[1]load pixel roi[5]                 ]
      R2 = ZXTH(R18)                        //[[1]rio[3]                            ]
      R5 = ASRH(R18)                        //[[1]rio[4]                            ]
      R8 = MAX(R8,R9)                       //[[2]smax                              ]
    }
    { R27:26 = MEMD(R29+#48)                //[[2]R26 = *identity_coef, R27 = 0     ]
      R23:22 = MEMD(R29+#56)                //[[2]*hp_coef and hp_shift             ]
      P0 = CMP.GT(R20,R8)                   //[[2]lp_flag = (smax<lp_epsilon)? 1:0  ]
      P1 = CMP.GT(R8,R21)                   //[[2]hp_flag = (smax>hp_epsilon)? 1:0  ]
    }
    { R8 = ADD(R3,R12)                      //[[1]rio[1]+rio[2] (s1,s4)             ]
      P2 = AND(P1,!P0)                      //[[2](hp_flag==1) && (lp_flag==0)      ]
      IF P2.new R27:26 = COMBINE(R23,R22)   //[[2]use hpfilter                      ]
      R23:22 = MEMD(R29+#64)                //[[2]*lp_coef and lp_shift             ]
    }
    { R9 = ADD(R6,R2)                       //[[1]rio[0] + rio[3]        (s2,s3)    ]
      R11 = ADD(R8,R13)                     //[[1]rio[1]+rio[2]+rio[5]   (s4)       ]
      P1 = AND(P0,!P1)                      //[[2](lp_flag==1) && (hp_flag==0)      ]
      IF P1.new R27:26 = COMBINE(R23,R22)   //[[2]use lpfilter                      ]
    }                                       //[[1]R9 = rio[0]+rio[3]+rio[6]  (s2)   ]
    { R9:8 = VADDW(R9:8,R7:6)               //[[1]R8 = rio[0]+rio[1]+rio[2]  (s1)   ]
      R10 = ADD(R9,R3)                      //[[1]rio[0]+rio[3]+rio[1]       (s3)   ]
      R11 = SUB(R11,R6)                     //[[1]rio[1]+rio[2]+rio[5]-rio[0](s4)   ]
      R23:22 = MEMD(R26+#0)                 //[[2]load coeff                        ]
    }                                       //[[1]R9=rio[0]+rio[3]+rio[6]-rio[1](s2)]
    { R9:8 = VSUBW(R9:8,R3:2)               //[[1]R8=rio[0]+rio[1]+rio[2]-rio[3](s1)]
      R10 = SUB(R10,R7)                     //[[1]rio[0]+rio[3]+rio[1]-rio[6]   (s4)]
      R25:24 = VRMPYH(R23:22,R17:16)        //[[2]sum += coef[] *roi[]              ]
      R23:22 = MEMD(R26+#8)                 //[[2]load coeff                        ]
    }
    { R8 -= ADD(R5,R13)                     //[[1]s1= rio[0]+rio[1]+rio[2]-rio[3]   ]
                                            //[             -rio[4]-rio[5]          ]
      R10 = SUB(R10,R5)                     //[[1]rio[0]+rio[3]+rio[1]-rio[6]-rio[4]]
      R22 = MEMW(R26+#16)                   //[[2]load coeff                        ]
      R25:24 += VRMPYH(R23:22,R19:18)       //[[2]sum += coef[] * roi[]             ]
    }
    { R14 = MEMUB(R0++M1)                   //[[1]load pixel roi[8]                 ]
      R9 -= ADD(R5,R15)                     //[[1]s2= rio[0]+rio[3]+rio[6]-rio[1]   ]
                                            //[       -rio[4]-rio[7]                ]
      R17 = COMBINE(R2.L,R12.L)             //[[1]pack rio[3]|rio[2] in R17         ]
      R24 += MPY(R22.L,R15.L)               //[[2]R24 = sum                         ]
    }
    { R10 = SUB(R10,R12)                    //[[1]s3 = rio[0]+rio[3]+rio[1]-rio[6]  ]
                                            //[              -rio[4]-rio[2]         ]
      R11 -= ADD(R5,R14)                    //[[1]s4 = rio[1]+rio[2]+rio[5]-rio[0]  ]
                                            //[              -rio[4]-rio[8]         ]
      R19 = COMBINE(R15.L,R7.L)             //[[1]pack rio[7]|rio[6] in R19         ]
      R24 = ASR(R24,R27)                    //[[2]sum >> bitshift                   ]
    }
    { R9:8 = VABSW(R9:8)                    //[[1]abs(s2:s1)                        ]
      R16 = COMBINE(R3.L,R6.L)              //[[1]put new rio[1]|rio[0] in R16      ]
      R7 = R15                              //[[1]put new rio[6] in R7              ]
      R24 = SATUB(R24)                      //[[2]clip to (0,255)                   ]
    }
    { R11:10 = VABSW(R11:10)                //[[1]abs(s4 : s3)                      ]
      R18 = COMBINE(R13.L,R5.L)             //[[1]pack rio[5]|rio[4] in R18         ]
      R15 = R14                             //[[1]put new rio[7] in R15             ]
      IF P3 MEMB(R4++#1) = R24              //[[2]save output                       ]
    }:endloop0:endloop1

    { JUMP .lumafilt_end                    //[ jump to END                         ]
    }

    /*-----------------------------------------------------------------------------*/
    /*                             Mode 1                                          */
    /*-----------------------------------------------------------------------------*/
    .falign
.MASK_SEL_EQ_1:
    { MEMD(R29+#64) = R13:12                //[ save LP filter parameters to stack  ]
      LOOP1(.lumafilt_i_loop2,R2)           //[ setup outer loop with lc1=height-2  ]
    } 
    .falign
.lumafilt_i_loop2:    
    { R16 = MEMUB(R0+#-1)                   //[ load rio[0]                         ]
      R17 = MEMUB(R0++M0)                   //[ load rio[1]                         ]
      R4 = ADD(R4,#2)                       //[ update R4 = &out[i*width+1]         ]
    }
    { R12 = MEMUB(R0+#-1)                   //[ load rio[3]                         ]
      R13 = MEMUB(R0++M0)                   //[ load rio[4]                         ]
      R16 = ASLH(R16)                       //[ R16 = rio[0]|x                      ]
    }
    { R7 = MEMUB(R0+#-1)                    //[ load rio[6]                         ]
      R15 = MEMUB(R0++M1)                   //[ load rio[7]                         ]
      R18 = COMBINE(R13.L,R12.L)            //[ R17 = rio[4]|rio[3]                 ]
      P3 = SP1LOOP0(.lumafilt_j_loop2,R1)   //[ setup inner loop with lc0=width-1   ]
    }                                       
    /*-----------------------------------------------------------------------------*/
    /* In inner loop, only 3 pixels will be read, the rest 6 are re-used from      */ 
    /* previous computation.                                                       */
    /*-----------------------------------------------------------------------------*/
    .falign
.lumafilt_j_loop2:
    { R12 = MEMUB(R0++M0)                   //[[1]load pixel roi[2]                 ]
      R6 = ASRH(R16)                        //[[1]rio[0]                            ]
      R3 = ZXTH(R17)                        //[[1]rio[1]                            ]
      R9:8 = VMAXW(R9:8,R11:10)             //[[2]max(s4:s3, s2:s1)                 ]
    }
    { R13 = MEMUB(R0++M0)                   //[[1]load pixel roi[5]                 ]
      R2 = ZXTH(R18)                        //[[1]rio[3]                            ]
      R5 = ASRH(R18)                        //[[1]rio[4]                            ]
      R8 = MAX(R8,R9)                       //[[2]smax                              ]
    }
    { R27:26 = MEMD(R29+#48)                //[[2]R26 = *identity_coef, R27 = 0     ]
      R23:22 = MEMD(R29+#56)                //[[2]*hp_coef and hp_shift             ]
      P0 = CMP.GT(R20,R8)                   //[[2]lp_flag = (smax<lp_epsilon)? 1:0  ]
      P1 = CMP.GT(R8,R21)                   //[[2]hp_flag = (smax>hp_epsilon)? 1:0  ]
    }
    { R8 = ADD(R3,R12)                      //[[1]rio[1] + rio[2] (s1,s4)           ]
      P2 = AND(P1,!P0)                      //[[2] (hp_flag==1) && (lp_flag==0)     ]
      IF P2.new R27:26 = COMBINE(R23,R22)   //[[2]use hpfilter                      ]
      R23:22 = MEMD(R29+#64)                //[[2]*lp_coef and lp_shift             ]
    }
    { R9 = ADD(R6,R2)                       //[[1]rio[0] + rio[3]           (s2,s3) ]
      R11 = ADD(R8,R13)                     //[[1]rio[1]+rio[2]+rio[5]      (s4     ])
      P1 = AND(P0,!P1)                      //[[1](lp_flag==1) && (hp_flag==0)      ]
      IF P1.new R27:26 = COMBINE(R23,R22)   //[[2] use lpfilter                     ]
    }         
    { R8 += SUB(R6,R7)                      //[[1]rio[0]+rio[1]+rio[2]-rio[6] (s1)  ]
      R10 = ADD(R9,R3)                      //[[1]rio[0]+rio[3]+rio[1]        (s3)  ]
      R11 = SUB(R11,R2)                     //[[1]rio[1]+rio[2]+rio[5]-rio[3] (s4)  ]
      R23:22 = MEMD(R26+#0)                 //[[2]load coeff                        ]
    }
    { R14 = MEMUB(R0++M1)                   //[[1]load pixel roi[8]                 ]
      R9 += SUB(R7,R12)                     //[[1]rio[0]+rio[3]+rio[6]-rio[2] (s2)  ]
      R25:24 = VRMPYH(R23:22,R17:16)        //[[2] sum += coef[] *roi[]             ]
      R23:22 = MEMD(R26+#8)                 //[[2]load coeff                        ]
    }
    { R9 = SUB(R9,R13)                      //[[1]rio[0]+rio[3]+rio[6]-rio[2]-rio[5]]
                                            //[                                (s2) ]
      R10 -= ADD(R15,R14)                   //[[1]rio[0]+rio[3]+rio[1]-rio[7]-rio[8]]
                                            //[                                (s4) ]
      R25:24 += VRMPYH(R23:22,R19:18)       //[[2] sum += coef[] *roi[]             ]
      R22 = MEMW(R26+#16)                   //[[2]load coeff                        ]
    }
    { R8 -= ADD(R15,R14)                    //[[1]s1= rio[0]+rio[1]+rio[2]-rio[6]   ]
                                            //[[1]          -rio[7]-rio[8]          ]
      R9 = SUB(R9,R14)                      //[[1]s2= rio[0]+rio[3]+rio[6]-rio[2]   ]
                                            //[[1]          -rio[5]-rio[8]          ]
      R17 = COMBINE(R2.L,R12.L)             //[[1]pack rio[3]|rio[2] in R17         ]
      R24 += MPY(R22.L,R15.L)               //[[2] R24 = sum                        ]
    }
    { R10 = SUB(R10,R13)                    //[[1]s3 = rio[0]+rio[3]+rio[1]-rio[7]  ]
                                            //[              -rio[8]-rio[5]         ]
      R11 -= ADD(R7,R15)                    //[[1]s4 = rio[1]+rio[2]+rio[5]-rio[3]  ]
                                            //[              -rio[6]-rio[7]         ]
      R19 = COMBINE(R15.L,R7.L)             //[[1]pack rio[7]|rio[6] in R19         ]
      R24 = ASR(R24,R27)                    //[[2]sum >> bitshift                   ]
    }
    { R9:8 = VABSW(R9:8)                    //[[1]abs(s2:s1)                        ]
      R16 = COMBINE(R3.L,R6.L)              //[[1]put new rio[1]|rio[0] in R16      ]
      R7 = R15                              //[[1] put new rio[6] in R7             ]
      R24 = SATUB(R24)                      //[[2] clip to (0,255)                  ]
    }
    { R11:10 = VABSW(R11:10)                //[[1]abs(s4:s3)                        ]
      R18 = COMBINE(R13.L,R5.L)             //[[1]pack rio[5]|rio[4] in R18         ]
      R15 = R14                             //[[1]put new rio[7] in R15             ]
      IF P3 MEMB(R4++#1) = R24              //[[2]save output                       ]
    }:endloop0:endloop1

.lumafilt_end:
    { R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
      R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    }
    { R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
      R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    }
    { R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
      R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
      R29 = ADD(R29,#9*8)                   //[ pop stack                           ]
      JUMPR R31
    }
    .size    luma_filt, .-luma_filt
