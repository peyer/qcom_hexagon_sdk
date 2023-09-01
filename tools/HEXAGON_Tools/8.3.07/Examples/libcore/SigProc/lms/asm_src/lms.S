/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "lms.S"
    /*[*****************************************************************************]*/
    /*[  Function   : Word16 lms()                                                  ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Implements adaptive filter with least mean square (LMS)       ]*/
    /*[               algorithm                                                     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - ntap is a multiple of 8                                         ]*/
    /*[           - coefficients and states arrays are aligned by 8bytes            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 input --  an input sample                         ]*/
    /*[               R1 : Word16 ydes   -- the desired output                      ]*/
    /*[               R2 : Word16 rate   -- adaptive rate                           ]*/
    /*[               R3 : int    ntap   -- number of taps                          ]*/
    /*[               R4 : Word16 *state -- Ptr to state buffer                     ]*/
    /*[               R5 : Word16 *coef  -- Ptr to coefficients                     ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 3*T/4 + 11                                                      ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl lms
    .type    lms, @function
lms:
    { P0 = CMP.GT(R3,#7)                    //[ P0 = (ntap >= 8)                    ]
      IF !P0.new JUMPR:nt R31               //[ if !P0 return                       ]
      R3  = ASR(R3, #2)                     //[ R3 = ntap/4                         ]
    }
    { MEMH(R4) = R0                         //[ states[0] = xin                     ]
      LOOP0(.lms_filtLOOP, R3)              //[ setup loop0: lc0 = ntap/4           ]
      R11:10 = #0                           //[ sum = 0                             ]
      R9:8   = #0                           //[ use to mute 1st MAC                 ]
    }

    .falign
.lms_filtLOOP:
    { R7:6 = MEMD(R5++#8)                   //[[1]load coef[]                       ]
      R9:8 = MEMD(R4++#8)                   //[[1]load states[]                     ]
      R11:10 += VRMPYH(R9:8, R7:6)          //[[2]sum += coef[] * states[]          ]
    }:endloop0

.lms_filtLOOPEND:
    { R11:10 += VRMPYH(R9:8, R7:6)          //[ sum += coef[] * states[]            ]
    }
    { R11:10 = ASR(R11:10,#15)              //[ sum >> 15                           ]
    }
    { R0 = SATH(R10)                        //[ output = sature(sum >> 15)          ]
    }
    { R1 = SUB(R1.L,R0.L):sat               //[ error = sub(ydes, output)           ]
    }
    { R1 = MPY(R2.L, R1.L):<<1:rnd:sat      //[ er = mult_r(rate,error)             ]
      R11 = ASR(R3,#1)                      //[ R11 = ntap/8                        ]
      R2 = ADD(R4,#-8)                      //[ R2 = &states[ntap-4] (write ptr)    ]
      R3 = ADD(R5,#-8)                      //[ R3 = &coef[ntap-4] (write ptr)      ]
    }                                       //[ first load is already done in R6-R9 ]
    { R4 = ADD(R4,#-16)                     //[ R4 = &state[ntap-8] (load ptr)      ]
      R5 = ADD(R5,#-16)                     //[ R5 = &coef[ntap-8]  (load ptr)      ]
      R1 = COMBINE(R1.H,R1.H)               //[ R1 = er|er                          ]
      P3 = SP1LOOP0(.lms_updateLOOP,R11)    //[ setup loop0: lc0 = ntap/8           ]
    }

    .falign
.lms_updateLOOP:
    { IF P3 R7:6 = MEMD(R5++#-8)            //[2] Load c(n)                         ]
      R14 = VMPYH(R1,R8):<<1:rnd:sat        //[2] er*s(n)                           ]
      R15 = VMPYH(R1,R9):<<1:rnd:sat        //[2] er*s(n)                           ]
      IF P3 MEMD(R3++#-8) = R7:6            //[3] save c'(n-1)                      ]
    }
    { R11:10 = MEMD(R4++#-8)                //[2] Load s(n-4)                       ]
      R7:6 = VADDH(R7:6,R15:14):sat         //[2] c(n) += er*s(n)                   ]
      IF P3 MEMD(R2++#-8) = R13:12          //[3] save updated s(n)                 ]
      R13:12 = VALIGNB(R11:10,R9:8,#6)      //[3] update state buffer               ]
    }
    { MEMD(R3++#-8) = R7:6                  //[2] Save c'(n)                        ]
      R7:6 = MEMD(R5++#-8)                  //[2] Load c(n-4)                       ]
      R14 = VMPYH(R1,R10):<<1:rnd:sat       //[2] er*s(n-4)                         ]
      R15 = VMPYH(R1,R11):<<1:rnd:sat       //[2] er*s(n-4)                         ]
    }
    { R9:8 = MEMD(R4++#-8)                  //[1] Load s(n)                         ]
      R7:6 = VADDH(R7:6,R15:14):sat         //[2] c(n-4) += er*s(n-4)               ]
      R13:12 = VALIGNB(R9:8,R11:10,#6)      //[2] update state buffer               ]
      IF P3 MEMD(R2++#-8) = R13:12          //[3] save updated s(n-4)               ]
    }:endloop0

    { MEMD(R3) = R7:6                       //[e] save c'(n-1)                      ]
    }
    { MEMD(R2++#-8) = R13:12                //[e] save updated s(n)                 ]
      R13:12 = VALIGNB(R11:10,R9:8,#6)      //[e] update state buffer               ]
    }
    { MEMD(R2) = R13:12                     //[e] save updated s(n)                 ]
      JUMPR R31                             //[ return                              ]
    }
    .size    lms, .-lms

