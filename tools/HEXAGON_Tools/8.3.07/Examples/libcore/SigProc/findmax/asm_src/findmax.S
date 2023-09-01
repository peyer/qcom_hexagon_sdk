/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 


    .file    "findmax.S"
    /*[*****************************************************************************]*/
    /*[  Function   : Word32 findmax()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  Description: find maximum value and its index  from an array               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 *input                                            ]*/
    /*[               R1 : int    length                                            ]*/
    /*[               R2 : int    *index                                            ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : maximum value                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - length + 3                                                      ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl findmax
    .type    findmax, @function
findmax:
    { P0 = CMP.GT(R1,#1)                    //[ P0 =(length > 1)                    ]
      R3 = ADD(R1,#-1)                      //[ length -1                           ]
      R4 = MEMW(R0++#4)                     //[ max_val = xin[0]                    ]
      R5 = #0                               //[ index = 0                           ]
    }
    { IF !P0 JUMP .findmax_END              //[ if (length <=1) return              ]
    }
    { LOOP0(.findmax_loop,R3)               //[ setup loop: lc0 = length-1          ]
      R1 = MEMW(R0++#4)                     //[ [p] load x[i]                       ]
      R3 = #1                               //[ i = 1                               ]
    }

    .falign
.findmax_loop:
    { R1 = MEMW(R0++#4)                     //[[1] load x[i]                        ]
#ifdef FIRST_PREFERED 
      P1 = CMP.GT(R1,R4)                    //[[2] P0 = (x > max_val)               ]
      IF (P1.new) R5:4 = COMBINE(R3,R1)     //[[2] if !P0 { max_val = xin[i] ;      ]
                                            //[             index = i; }            ]
#else
      P1 = CMP.GT(R4,R1)                    //[[2] P0 = (x < max_val)               ]
      IF (!P1.new) R5:4 = COMBINE(R3,R1)    //[[2] if !P0 { max_val = xin[i] ;      ]
                                            //[             index = i; }            ]
#endif
      R3 = ADD(R3,#1)                       //[[2] i++                              ]
    }:endloop0 

.findmax_END:
    { MEMW(R2) = R5                         //[ save *index                         ]
      R0 = R4                               //[ return value = max_val              ]
      JUMPR R31                             //[ return                              ]
    }
    .size    findmax, .-findmax

