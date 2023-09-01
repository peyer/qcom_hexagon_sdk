/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "pnseq.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void PNGeneration()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Generate PN sequence with generator polynomial of             ]*/
    /*[               h(D)=D^20+D^19+D^16+D^14+1                                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Generate 8-bits at once. If length is not a multiple of 8,      ]*/
    /*[             then last byte is filled as xxxxPPP, where xxxx are             ]*/
    /*[             DO-NOT-CARE bits.                                               ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 seed                                             ]*/
    /*[               R1 : int     length                                           ]*/
    /*[               R2 : UWord8  *output                                          ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 3*ceil(length/8) + 4                                            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl PNGeneration
    .type    PNGeneration, @function
PNGeneration:
    { P0 = CMP.GT(R1,#0)                    //[ P0 = (length > 0) ?                 ]
      IF !P0.new JUMPR:nt R31               //[ if (length <=0 ) return             ]
      R1 = ADD(R1,#7)                       //[ length + 7                          ]
    }
    { R1 = ASR(R1,#3)                       //[ LC = (length+7)>>3                  ]
      R3 = LSR(R0,#1)                       //[ [p]D^19                             ]
    }
    { LOOP0(.PNgen_LOOP,R1)                 //[ setup loop0: lc0 = ceil(length/8)   ]
    }

    .falign
.PNgen_LOOP:
    { MEMB(R2++#1) = R0                     //[ output[i] = (uint8)LFSRvalue        ]
      R3 = XOR(R0,R3)                       //[ D^20 + D^19                         ]
      R4 = LSR(R0,#4)                       //[ D^16                                ]
      R5 = LSR(R0,#6)                       //[ D^14                                ]
    }
    { R5 ^= XOR(R4,R3)                      //[ shiftBackbits                       ]
      R0 = LSR(R0,#8)                       //[ LFSRvalue >> 8                      ]
      R3 = R0                               //[ use old LFSRvalue to get D^19       ]
                                            //[ (because only need 8 valid bits)    ]
    }
    { R0 = INSERT(R5,#8,#12)                //[ update LFSRvalue                    ]
      R3 = LSR(R3,#9)                       //[ D^19                                ]
    }:endloop0

    { JUMPR R31                             //[ return                              ]
    }
    .size    PNGeneration, .-PNGeneration

