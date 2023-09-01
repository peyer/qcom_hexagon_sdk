/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "tcin.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void cal_input1()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calcuates soft inputs to 1st MAP decoder                      ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - xs, L2, xLin are aligned by 8-bytes                             ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *xs                                               ]*/
    /*[               R1 : Word16 *L2                                               ]*/
    /*[               R2 : int    nFrameSize                                        ]*/
    /*[               R3 : Word16 *xLin                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R13                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 3*ceil(nFrameSize/8) + (nFrameSize%4) + 7                       ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl cal_input1
    .type	cal_input1, @function
cal_input1:
    { R4 = ADD(R2,#7)                       //[ (nFrameSize+7)                      ]
      R2 = AND(R2,#7)                       //[ nFrameSize%8                        ]
    }
    { R4 = ASR(R4,#3)                       //[ ceil(nFrameSize/8)                  ]
      P0 = CMP.EQ(R2,#0)                    //[ if nFrameSize%8 == 0                ]
      IF P0.new R2 = #8                     //[ then set R2 = 8                     ]
    }
    { P3=SP1LOOP0(.cal_input1_LOOP,R4)      //[ setup loop0: lc0=ceil(nFrameSize/8) ]
      P0  = CMP.GT(R2,#4)                   //[ P0= (nFrameSize%8 > 4)              ]
      IF P0.new R2 = ADD(R2,#-4)            //[ R2= (nFrameSize%4)? (nFrameSize%4):4]
    }
    
    .falign
.cal_input1_LOOP:
    { R7:6 = MEMD(R0++#16)                  //[ [1]load xs[i                        ]
      R9:8 = MEMD(R0+#8)                    //[ [1]                                 ]
      R11:10 = VADDH(R11:10,R7:6):sat       //[ [2]saturate(xs[i]+L1[i])            ]
      R13:12 = VADDH(R13:12,R9:8):sat       //[ [2]                                 ]
    }
    { R11:10 = MEMD(R1++#8)                 //[ [1]load L1[i]                       ]
      IF P3 MEMD(R3++#8) = R11:10           //[ [2]save xLin[i]                     ]
    }
    { R13:12 = MEMD(R1++#8)                 //[ [1]load L1[i]                       ]
      IF P3 MEMD(R3++#8) = R13:12           //[ [2]save xLin[i]                     ]
    }:endloop0

    { R11:10 = VADDH(R11:10,R7:6):sat       //[ [e]saturate(xs[i]+L1[i])            ]
      IF !P0 JUMP .cal_input1_CONT
    }
    { MEMD(R3++#8) = R11:10                 //[ [e]save xLin[i]                     ]
      R11:10 = VADDH(R13:12,R9:8):sat       //[ [e]saturate(xs[i]+L1[i])            ]
    }
.cal_input1_CONT:
    { LOOP0(.cal_input1_restLOOP,R2)        //[ setup loop0:                        ]
    }

    .falign
.cal_input1_restLOOP:
    { MEMH(R3++#2)   = R10                  //[ save one result                     ]
      R11:10 = ASR(R11:10,#16)              //[ shift down result to 16LSB          ]
    }:endloop0

    { JUMPR R31                             //[ return                              ]
    }
    .size	cal_input1, .-cal_input1




    /*[*****************************************************************************]*/
    /*[  Function   : void cal_input2()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Calcuates and interleaves soft inputs to 2nd MAP decoder      ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *xs                                               ]*/
    /*[               R1 : Word16 *L2                                               ]*/
    /*[               R2 : Word16 *intrlve                                          ]*/
    /*[               R3 : int    nFrameSize                                        ]*/
    /*[               R4 : Word16 *xLin                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R10                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 2*nFrameSize + 4                                                ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl cal_input2
    .type	cal_input2, @function
cal_input2:
    { P3 = SP1LOOP0(.cal_input2_LOOP,R3)    //[ setup loop0:lc0=nFrameSize          ]
      R5 = MEMW(R2++#4)                     //[ [p]j = intrlve[i]                   ]
    }
    { R6 = ADDASL(R0,R5,#1)                 //[ [p]&xs[j]                           ]
      R7 = ADDASL(R1,R5,#1)                 //[ [p]&L2[j]                           ]
    }

    .falign
.cal_input2_LOOP:
    { R5 = MEMW(R2++#4)                     //[[1]j = intrlve[i]                    ]
      R8 = MEMH(R6)                         //[[2]xs[j]                             ]
      R10 = ADD(R8.L,R9.L):sat              //[[3]saturate(xs[j]+L2[j])             ]
    }
    { R6 = ADDASL(R0,R5,#1)                 //[[1]&xs[j]                            ]
      R7 = ADDASL(R1,R5,#1)                 //[[1]&L2[j]                            ]
      R9 = MEMH(R7)                         //[[2]L2[j]                             ]
      IF P3 MEMH(R4++#2) = R10              //[[3]save                              ]
    }:endloop0
    
    { R10 = ADD(R8.L,R9.L):sat              //[ [e]saturate(xs[j]+L2[j])            ]
    }
    { MEMH(R4) = R10                        //[ [e]save                             ]
      JUMPR R31                             //[ return                              ]
    }
    .size	cal_input2, .-cal_input2




    /*[*****************************************************************************]*/
    /*[  Function   : void deinterleave()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Deinterleave input array based on look-up table               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *in                                               ]*/
    /*[               R2 : Word16 *intrlve                                          ]*/
    /*[               R3 : int    nFrameSize                                        ]*/
    /*[               R4 : Word16 *out                                              ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R8                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - nFrameSize + 4                                                  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl deinterleave
    .type	deinterleave, @function
deinterleave:
    { P0 = TSTBIT(R2,#0)                    //[ nFrameSize%2 == 1                   ]
      R2 = ASRRND(R2,#1)                    //[ (nFrameSize+1)/2                    ]
    }
    { P3 = SP1LOOP0(.deinterleave_LOOP,R2)  //[ setup loop0:lc0=ceil(nFrameSize/2)  ]
      R7:6 = COMBINE(R3,R3)                 //[ used for stores in 1st              ]
    }
    .falign
.deinterleave_LOOP:
    { R5:4 = MEMD(R1++#8)                   //[[1]j = intrlv[i]   (1,2)             ]
      IF P3 MEMH(R6) = R8                   //[[2]out[j] = in[i]  (1)               ]
    }
    { R8 = MEMW(R0++#4)                     //[[1]in[i]           (1,2)             ]
      R6 = ADDASL(R3,R4,#1)                 //[[1]&out[j]         (1)               ]
      R7 = ADDASL(R3,R5,#1)                 //[[1]&out[j]         (2)               ]
      IF P3 MEMH(R7) = R8.H                 //[[2]out[j] = in[i]  (2)               ]
    }:endloop0

    { MEMH(R6) = R8                         //[ [e]out[j] = in[i]                   ]
      IF P0 JUMPR R31                       //[ if nFrameSize%2==1 then DONE        ]
    }
    { MEMH(R7) = R8.H                       //[ [e]out[j] = in[i]                   ]
      JUMPR R31
    }
    .size	deinterleave, .-deinterleave


