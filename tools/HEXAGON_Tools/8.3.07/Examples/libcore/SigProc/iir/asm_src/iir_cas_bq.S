/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "iir_cas_bq.S"
    /*[*****************************************************************************]*/
    /*[  Function   : void IIR_casBiquad()                                          ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Implements a cascade of biquads IIR filter                    ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 *input                                            ]*/
    /*[               R1 : Word16 *coef                                             ]*/
    /*[               R2 : Word16 *states                                           ]*/
    /*[               R3 : int    nsec     -- number of biquad sections             ]*/
    /*[               R4 : int    nsamples -- number of samples                     ]*/
    /*[               R2 : Word16 *output                                           ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - nsamp * (3 *nsec + 2) + 4                                       ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl IIR_casBiquad
    .type    IIR_casBiquad, @function
IIR_casBiquad:
    { P0 = CMP.GT(R4,#0)                    //[ P0 = (nsamp  >0 )                   ]
      IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
    }
    { P0 = CMP.GT(R3,#0)                    //[ P1 = (nSec > 0)                     ]
      IF !P0.new JUMPR:nt R31               //[ IF !P1 return                       ]
    }
    { LOOP1(.IIR_sampLoop,R4)               //[ setup loop1: lc1 = nsamp            ]
      R11 = MEMH(R0++#2)                    //[[p1] load xin[i]                     ]
      R15:14 = COMBINE(R2,R1)               //[[p1] coef = pCoef; state = pState    ]
      P2 = CMP.GT(R0,R0)                    //[ clean P2                            ]
    }


    .falign
.IIR_sampLoop:
    { IF P2 MEMH(R5++#2) = R10.H            //[ out[i] = extract_h(x)               ]
      R6 = MEMW(R15)                        //[ load state[0-1]                     ]
      R10 = ASLH(R11)                       //[  xin[i] << 16                       ]
      P3 = SP1LOOP0(.IIR_filtLoop,R3)       //[ setup loop0 : lc0 = nsec            ]
    }

    .falign
.IIR_filtLoop:
    { IF P3 MEMW(R15++#4) = R12             //[ [2] save state[0-1]                 ]
      R9:8 = MEMD(R14++#8)                  //[ load coefs[0-3]                     ]
      R7 = R6                               //[ R7=R6 = s1|s0                       ]
      R11 = #0                              //[ R11 = 0                             ]
    }
    { R11:10 += VDMPY(R7:6,R9:8):sat        //[ R10 = t + c0*s0 + c1*s1             ]
                                            //[ R11 = c2*s0 + c3*s1                 ]
    }
    { R12 = COMBINE(R6.L,R10.H)             //[ state[1] = state[0];                ]
                                            //[ state[0] = extract_h(t)             ]
      R6 = MEMW(R15+#4)                     //[[2] load next state[0-1]             ]
      R10 = ADD(R10,R11):sat                //[ x = t + c2*s0 + c3*s1               ]
    }:endloop0

    { MEMW(R15) = R12                       //[ save state[]                        ]
      R11 = MEMH(R0++#2)                    //[ load xin[i]                         ]
      R15:14 = COMBINE(R2,R1)               //[ coef = pCoef; state = pState        ]
      P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
    }:endloop1

    { MEMH(R5) = R10.H                      //[ [e1]out[i] = extract_h(x)           ]
      JUMPR R31                             //[ return                              ]
    }
    .size    IIR_casBiquad, .-IIR_casBiquad
