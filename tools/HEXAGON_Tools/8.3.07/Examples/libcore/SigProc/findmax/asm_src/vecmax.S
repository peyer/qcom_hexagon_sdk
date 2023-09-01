/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "vecmax.S"
    /*[*****************************************************************************]*/
    /*[  Function   : Word32 VecMax()                                               ]*/
    /*[*****************************************************************************]*/
    /*[  Description: find maximum value and its index from an array                ]*/
    /*[               NOTE: This is fast version of findmax(). BUT they will not    ]*/
    /*[                     be equivalent when there are multiple-maximum values    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Array is aligned by 8bytes                                      ]*/
    /*[           - length is a multipe of 4                                        ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 *input                                            ]*/
    /*[               R1 : int    N                                                 ]*/
    /*[               R2 : int    *index                                            ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : maximum value                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R13                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - N/2 + 10  (worst case)                                          ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl VecMax
    .type    VecMax, @function
VecMax:
    { P0 = CMP.GT(R1,#3)                    //[ P0 = N >= 4                         ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
      R1 = ASR(R1,#2)                       //[ N/4                                 ]
      R11:10 = #0                           //[ R11 = R10= #0                       ]
    }
    { R3 = ADD(R0,#8)                       //[ R3 = &x[2]                          ]
      R10.H = #0x8000                       //[ set R10 = smallest number           ]
      R11.H = #0x8000                       //[ set R11 = smallest number           ]
      R4 = R0                               //[ save xin in R4                      ]
    }
    { R13:12 = R11:10                       //[ set R13 = R12 = smallest number     ]
      R7:6   = R11:10                       //[ set R7:6 = smallest number          ]
      R9:8   = R11:10                       //[ set R9:8 = smallest number          ]
      LOOP0(.vecMax_find_maxval_Loop,R1)    //[ setup loop: lc0 = N/4               ]
    }

    .falign
.vecMax_find_maxval_Loop:
    { R7:6 = MEMD(R0++#16)                  //[ [1]load x[4*k+1]:x[4*k+0]           ]
      R9:8 = MEMD(R3++#16)                  //[ [1]load x[4*k+3]:x[4*k+2]           ]
      R11:10 = VMAXW(R11:10,R7:6)           //[ [2]compare within x[4*k+1],x[4*k+0] ]
      R13:12 = VMAXW(R13:12,R9:8)           //[ [2]compare within x[4*k+3],x[4*k+2] ]
    }:endloop0

    { R11:10 = VMAXW(R11:10,R7:6)           //[ [e]compare within x[4*k+1],x[4*k+0] ]
      R13:12 = VMAXW(R13:12,R9:8)           //[ [e]compare within x[4*k+3],x[4*k+2] ]
      R7:6 = COMBINE(#1,#0)                 //[ group index 1 : 0                   ]
      R9:8 = COMBINE(#3,#2)                 //[ group index 1 : 0                   ]
    }
    { P0 = VCMPW.GT(R13:12,R11:10)          //[compare max of Group1/0 with Group3/2]
      R11:10 = VMAXW(R11:10,R13:12)         //[compare max of Group1/0 with Group3/2]
    }
    { R7:6 = VMUX(P0,R9:8,R7:6)             //[ choose right group index            ]
      P1 = CMP.GT(R11,R10)                  //[ final compare                       ]
    }
    { R0 = MUX(P1,R11,R10)                  //[ maximum value                       ]
      R1 = MUX(P1,R7, R6)                   //[ group index GI of maximum value     ]
    }
    { R4 += ASL(R1,#2)                      //[ &x[GI]                              ]
    }
    { R6 = MEMW(R4++#16)                    //[ [p]load x[4*0 + GI]                 ]
    }
    .falign
.vecMax_find_index:                    
    { R6 = MEMW(R4++#16)                    //[ [1]load x[4*k + A]                  ]
      P0 = CMP.EQ(R6,R0)                    //[ [2] x[4*k+A] == max ?               ]
      IF !P0.new R1 = ADD(R1,#4)            //[ [2]if !P0 updat R1 = 4*k +A         ]
      IF !P0.new JUMP:t .vecMax_find_index  //[ [2]if !P0 continue loop             ]
    }

    { MEMW(R2+#0) = R1                      //[ *index  =                           ]
      JUMPR R31                             //[ Return                              ]
    }
    .size VecMax, .-VecMax

