/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "vitgsm_fr.S"

#define    LENGTH                               189
#define    OFST_SAVEDREGS                       ((2*LENGTH+7)&(-8))
#define    BYTES_ON_STACK                       (OFST_SAVEDREGS+4*8)

    /*[*****************************************************************************]*/
    /*[  Function   : void vitgsm()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  Description: viterbi decoding of covoluational code used in GSM full-rate  ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Decoded bits are packed as                                      ]*/
    /*[              Word 0 = I15I14...I0                                           ]*/
    /*[              Word 1 = I31I30...I16                                          ]*/
    /*[              ....                                                           ]*/
    /*[              Word 11= xxxI188.. I177I176                                    ]*/
    /*[                                                                             ]*/
    /*[           - Overflow is not considered                                      ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word8   *INsoft   - pointer to soft-decision input bytes ]*/
    /*[               R1 : UWord16 *OutWord  - pointer to packed decoded bits       ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R23, R28                                                ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 32+384                            ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -  11*N + 1*ceil(N/16) + 10                                       ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl vitgsm
    .type  vitgsm, @function
vitgsm:
    { R29 = ADD(R29, #-BYTES_ON_STACK)      //[ allocate stack                      ]
      R6 = MEMB(R0++#2)                     //[[p] load INsoft[2*i  ]               ]
      R7 = MEMB(R0+#1)                      //[[p] load INsoft[2*i+1]               ]
      R3:2 = #0                             //[ initialization BMs                  ]
    }
    { MEMD(R29+#(OFST_SAVEDREGS+0))= R17:16 //[ callee-saved registers              ]
      R5:4 = #0                             //[ initialization BMs                  ]
      R16 = ADD(R6,R7)                      //[[p] m0 = INsoft[2*i] + INsoft[2*i+1] ]
      R17 = SUB(R6,R7)                      //[[p] m1 = INsoft[2*i] - INsoft[2*i+1] ]
    }
    { MEMD(R29+#(OFST_SAVEDREGS+8))= R19:18 //[ callee-saved registers              ]
      R7:6 = #0                             //[ initialization BMs                  ]
      R9:8 = #0                             //[ initialization BMs                  ]
    }
    { MEMD(R29+#(OFST_SAVEDREGS+16))= R21:20//[ callee-saved registers              ]
      LOOP0(.vitgsm_FORWARD_ACSLOOP,#LENGTH)//[ setup loop0: lc0 = LENGTH           ]
      R18 = COMBINE(R17.L,R16.L)            //[p] R18 = m1|m0                       ]
      R19 = COMBINE(R17.L,R16.L)            //[p] R19 = m1|m0                       ]
    }
    { MEMD(R29+#(OFST_SAVEDREGS+24))= R23:22//[ callee-saved registers              ]
      R28 = R29                             //[ set up PTR to write transit words   ]
      R11:10 = VADDH(R3:2,R19:18)           //[[p]a = PM + BM                       ]
                                            //[   from state 3| 2|1|0 to 6|4|2|0    ]
      R13:12 = VSUBH(R7:6,R19:18)           //[[p]b = PM + BM                       ]
                                            //[   from state 11|10|9|8 to 6|4|2|0   ]
    }
    { R15:14 = VMAXH(R11:10,R13:12)         //[[p] new PM[6|4|2|0]                  ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[[p] transit bits for state 6|4|2|0   ]
    }

    .falign
.vitgsm_FORWARD_ACSLOOP:
    { R11:10 = VSUBH(R3:2,R19:18)           //[ a = PM + BM                         ]
                                            //[     from state  3| 2|1|0 to 7|5|3|1 ]
      R13:12 = VADDH(R7:6,R19:18)           //[ b = PM + BM                         ]
                                            //[     from state 11|10|9|8 to 7|5|3|1 ]
    }
    { R16 = MEMB(R0++#2)                    //[[1]load INsoft[2*i  ]                ]
      R17 = MEMB(R0+#1)                     //[[1]load INsoft[2*i+1]                ]
      R7:6 = VMAXH(R11:10,R13:12)           //[ new PM[7|5|3|1]                     ]
      P1 = VCMPH.GT(R13:12,R11:10)          //[ transit bits for state 7|5|3|1      ]
    }
    { R20 = VITPACK(P0,P1)                  //[ pack transit bits as 7|6|5|4|3|2|1|0]
      R11 = VSUBH(R5,R19)                   //[ a = PM + BM from state  7| 6| 5| 4  ]
      R10 = VSUBH(R4,R18)                   //[             to state 14|12|10|8     ]
      R13:12 = VADDH(R9:8,R19:18)           //[ b = PM + BM from state 15|14|13|12  ]
                                            //[             to state 14|12|10|8     ]
    }
    { R23:22 = VMAXH(R11:10,R13:12)         //[ new PM[14|12|10|8]                  ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[ transit bits for state 14|12|10|8   ]
      R3 = COMBINE(R6.H,R14.H)              //[ pack R3:2 = new PM[3|2|1|0]         ]
      R2 = COMBINE(R6.L,R14.L)              //[ pack R3:2 = new PM[3|2|1|0]         ]
    }
    { R11:10 = VADDH(R5:4,R19:18)           //[ a = PM + BM from state  7| 6| 5| 4  ]
                                            //[             to state 15|13|11|9     ]
      R13:12 = VSUBH(R9:8,R19:18)           //[ b = PM + BM from state 15|14|13|12  ]
                                            //[             to state 15|13|11|9     ]
      R5 = COMBINE(R7.H,R15.H)              //[ pack R5:4 = new PM[7|6|5|4]         ]
      R4 = COMBINE(R7.L,R15.L)              //[ pack R5:4 = new PM[7|6|5|4]         ]
    }
    { R16 = ADD(R16,R17)                    //[[1] m0 = INsoft[2*i] + INsoft[2*i+1] ]
      R17 = SUB(R16,R17)                    //[[1] m1 = INsoft[2*i] - INsoft[2*i+1] ]
      R9:8 = VMAXH(R11:10,R13:12)           //[ new PM[15|13|11|9]                  ]
      P1 = VCMPH.GT(R13:12,R11:10)          //[ transit bits for state 15|13|11|9   ]
    }
    { R18 = COMBINE(R17.L,R16.L)            //[[1] R18 = m1|m0                      ]
      R19 = COMBINE(R17.L,R16.L)            //[[1] R19 = m1|m0                      ]
      R21 = VITPACK(P0,P1)                  //[ pack flags                          ]
      R7:6 = PACKHL(R8,R22)                 //[ pack R7:6 = new PM[11|10|9|8]       ]
    }
    { R11:10 = VADDH(R3:2,R19:18)           //[[1]a = PM + BM from state  3| 2|1|0  ]
                                            //[               to 6|4|2|0            ]
      R13:12 = VSUBH(R7:6,R19:18)           //[[1]b = PM + BM from state 11|10|9|8  ]
                                            //[               to 6|4|2|0            ]
      MEMB(R28++#1) = R20                   //[ save transit bits 0-7               ]
      R8 = COMBINE(R9.L,R23.L)              //[ pack R9:8 = new PM[15|14|13|12]     ]
    }
    { R15:14 = VMAXH(R11:10,R13:12)         //[[1] new PM[6|4|2|0]                  ]
      P0 = VCMPH.GT(R13:12,R11:10)          //[[1] transit bits for state 6|4|2|0   ]
      MEMB(R28++#1) = R21                   //[ save transit bit 8-15               ]
      R9 = COMBINE(R9.H,R23.H)              //[ pack R9:8 = new PM[15|14|13|12]     ]
    }:endloop0

.vitgsm_FORWARD_LOOPEND:
    { R0 = ADD(R29,#(LENGTH-1)*2)           //[ points to the last transit word     ]
      R1 = ADD(R1,#((LENGTH+15)/16-1)*2)    //[ points to last output word          ]
      R2 = #0                               //[ set state = 0                       ]
      LOOP1(.vitgsm_TRACEBACK_LOOPS,#(LENGTH+15)/16)    
                                            //[ setup loop1: lc1=ceil(LENGTH/16)    ]
    }
    { R4 = MEMUH(R0++#-2)                   //[ read transit word                   ]
      R5 = #1                               //[ constant 1                          ]
      R6 = #0                               //[ Output_word=0                       ]
      LOOP0(.vitgsm_TRACEBACK_LOOPS,#(16-(-LENGTH)&15))    
                                            //[ LC0 = 189%16 for first iteration    ]
    }

    .falign
.vitgsm_TRACEBACK_LOOPS:
    { R5 &= LSR(R4,R2)                      //[ bit = (trans[i]>>bitpos) & 1        ]
      R2 = LSR(R2,#1)                       //[ state>>1                            ]
      R6 = ADD(R6,R6)                       //[ Output_word << 1                    ]
      R7 = AND(R2,#1)                       //[ state &1                            ]
    }
    { R6 = OR(R6,R7)                        //[ Outword=(Outword << 1)|(state &1)   ]
      R2 |= ASL(R5,#3)                      //[ state = (state>>1)|(bit<<3)         ]
      R4 = MEMUH(R0++#-2)                   //[ read next transit word              ]
      R5 = #1                               //[ constant 1                          ]
    }:endloop0

    { MEMH(R1++#-2) = R6                    //[ save output word                    ]
      R5 = #1                               //[ R5 = 1                              ]
      LOOP0(.vitgsm_TRACEBACK_LOOPS,#16)    //[ setup loop0:lc0=16 after 1st iter   ]
    }:endloop1

.vitgsm_END:
    { R17:16= MEMD(R29+#(OFST_SAVEDREGS+0)) //[ restore callee-saved registers      ]
      R19:18= MEMD(R29+#(OFST_SAVEDREGS+8)) //[ restore callee-saved registers      ]
    }
    { R21:20= MEMD(R29+#(OFST_SAVEDREGS+16))//[ restore callee-saved registers      ]
      R23:22= MEMD(R29+#(OFST_SAVEDREGS+24))//[ restore callee-saved registers      ]
      R29 = ADD(R29, #BYTES_ON_STACK)       //[ pop stack                           ]
      JUMPR R31                             //[ return                              ]
    }
    .size   vitgsm, .-vitgsm


