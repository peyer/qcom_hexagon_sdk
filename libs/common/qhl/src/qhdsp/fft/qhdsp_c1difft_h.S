/**=============================================================================
@file
   qhdsp_c1difft_h.S

@brief
   Performs complex 2^N IFFT 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part)
   fixed-point. Scale factor [1/N] absent since scaling was done in FFT function.

   [=============================================================================]
   [  Assumptions:                                                               ]
   [           - complex number: interleaved real/image with real part           ]
   [             at low memory address                                           ]
   [           - input  is aligned by size of the array                          ]
   [           - output is aligned by 8bytes                                     ]
   [           - N >  4                                                          ]
   [=============================================================================]
   [  Inputs     : R0 : short complex *input                                     ]
   [               R1 : int32_t       N                                          ]
   [               R2 : int32_t *twiddles                                        ]
   [               R3 : short complex *output                                    ]
   [=============================================================================]
   [  Register Usage: R0-R27, R28                                                ]
   [  Hardware Loops affected: Loop0, Loop1                                      ]
   [                                                                             ]
   [  Stack Memory Frame Allocated (in Bytes): 48                                ]
   [=============================================================================]
   [  Implementation:                                                            ]
   [           - Radix-4 DIF                                                     ]
   [           - INPUT: LOAD IN BIT-REVERSE ORDER                                ]
   [           - TWIDDLE FACTOR ARRAY: IN BIT-REVERSE ORDER & INTERLEAVED        ]
   [=============================================================================]
   [  Cycle Count:                                                               ]
   [                                                                             ]
   [=============================================================================]

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

.text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_c1difft_h
    .type qhdsp_c1difft_h, @function
qhdsp_c1difft_h:
{
    P0 = CMP.GT(R1,#4)                    //[ P0 = N > 4                          ]
    IF !P0.new JUMPR:nt R31               //[ IF !P0 return                       ]
}
{
    R29 = ADD(R29, #-6*8)                 //[ allocate stack                      ]
    MEMD(R29+#(0-6*8)) = R17:16           //[ callee-saved registers              ]
    R4 = #0                               //[ R4 = #0                             ]
    R5 = BREV(R0)                         //[ to bit reverse R0.L                 ]
}
{
    MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
    R0 = COMBINE(R0.H,R5.H)               //[ bit reverse R0.L                    ]
    R6 = CL0(R1)                          //[ used to calculate log2(N)           ]
    R28 = #0x05                           //[ used for multiplying with -j        ]
}
{
    MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
    R5 = ASR(R1, #3)                      //[ LC = N/8 for FIRST_STAGE_START loop ]
    R7 = ADD(R6,#-17)                     //[ 16-log2(4N) = cl0(N)-17             ]
}
{
    MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
    R4 = SETBIT(R4,R7)                    //[ 1<<(16 - log2(4N)) = 1<<(14-LOG2N)  ]
    P3 = SP1LOOP0(.qhdsp_c1difft_h_1stStageLoop,R5)  //[ setup loop: lc0=N/8       ]
}
{
    MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
    M0 = R4                               //[ used for bit-reversed addressing    ]
    R25:24 = COMBINE(R2,R2)               //[ set R24=twiddle, R25=twiddle+3*N/8  ]
}
{
    MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
    R25 += MPYI(R5,#12)                   //[ set R25 = twiddle + 3*N/8           ]
    R26 = R3                              //[ R26 = output                        ]
    R27 = ADDASL(R3,R1,#1)                //[ R27 = output + N/2                  ]
}

    .falign
.qhdsp_c1difft_h_1stStageLoop:
{
    R11:10 = MEMD(R0++M0:brev)            //[ [1]load A                           ]
    R19 = CMPY(R12,R8*):<<1:rnd:sat       //[ [2]B" = B"*Wa                       ]
    R12 = R19                             //[ [2]                                 ]
    R13 = CMPY(R13,R9*):<<1:rnd:sat       //[ [2]B" = B"*Wa                       ]
}
{
    R13:12 = MEMD(R0++M0:brev)            //[ [1]load B                           ]
    R20 = CMPY(R14,R6*):<<1:rnd:sat       //[ [2]C" = C"*Wb                       ]
    R22 = CMPY(R15,R7*):<<1:rnd:sat       //[ [2]C" = C"*Wb                       ]
    IF (P3) MEMD(R27++#8) = R13:12        //[ [2]save A"B"(2)                     ]
}
{
    R15:14 = MEMD(R0++M0:brev)            //[ [1]load C                           ]
    R11:10= VADDH(R11:10,R13:12)          //[ [1]A'= (A+B)/2                      ]
    R13:12=VSUBH(R11:10,R13:12):sat       //[ [1]B'= (A-B)/2                      ]
    IF (P3) MEMD(R26++#8) = R19:18        //[ [2]save A"B"(1)                     ]
}
{
    R17:16 = MEMD(R0++M0:brev)            //[ [1]load D                           ]
    R8 = MEMW(R24+#4)                     //[ [1]load Wa                          ]
    R21 = CMPY(R16,R4*):<<1:rnd:sat       //[ [2]D" = C"*Wc                       ]
    R23 = CMPY(R17,R5*):<<1:rnd:sat       //[ [2]D" = D"*Wc                       ]
}
{
    R6 = MEMW(R24++#12)                   //[ [1]load Wb                          ]
    R4 = MEMW(R24+#8)                     //[ [1]load Wc                          ]
    R15:14= VADDH(R15:14,R17:16)          //[ [1]C'= (C+D)/2                      ]
    R17:16=VSUBH(R15:14,R17:16):sat       //[ [1]D'= (C-D)/2                      ]
}
{
    R9 = MEMW(R25+#4)                     //[ [1]load Wa                          ]
    R13:12 = VCROTATE(R13:12,R28)         //[ [1]B' = -jB'                        ]
    IF (P3) MEMD(R26++#8) = R21:20        //[ [2]save C"D"(1)                     ]
}
{
    R19:18= VADDH(R11:10,R15:14)          //[ [1]A"= (A'+C')/2                    ]
    R15:14=VSUBH(R11:10,R15:14):sat       //[ [1]C"= (A'-C')/2                    ]
    IF (P3) MEMD(R27++#8) = R23:22        //[ [2]save C"D"(2)                     ]
}
{
    R7 = MEMW(R25++#12)                   //[ [1]load Wb                          ]
    R5 = MEMW(R25+#8)                     //[ [1]load Wc                          ]
    R13:12= VADDH(R13:12,R17:16)          //[ [1]B"= (B'+D')/2                    ]
    R17:16=VSUBH(R13:12,R17:16):sat       //[ [1]D"= (B'-D')/2                    ]
}:endloop0
{
    R19 = CMPY(R12,R8*):<<1:rnd:sat       //[[e]B" = B"*Wa                        ]
    R12 = R19                             //[[e]R12 = A"                          ]
    R13 = CMPY(R13,R9*):<<1:rnd:sat       //[[e]B" = B"*Wa                        ]
}
{
    MEMD(R27++#8) = R13:12                //[[e] save A"B"(2)                     ]
    R6 = CMPY(R14,R6*):<<1:rnd:sat        //[[e] C" = C"*Wb                       ]
    R8 = CMPY(R15,R7*):<<1:rnd:sat        //[[e] C" = C"*Wb                       ]
    R25 = R1                              //[ save R1 (i.e., N) in R25            ]
}
{
    MEMD(R26++#8) = R19:18                //[[e] save A"B"(1)                     ]
    R7 = CMPY(R16,R4*):<<1:rnd:sat        //[[e] D" = C"*Wc                       ]
    R9 = CMPY(R17,R5*):<<1:rnd:sat        //[[e] D" = D"*Wc                       ]
    R23:22 = COMBINE(#-48,#16)            //[ initial k1=4, R23:22= -3*k1*4: k1*4 ]
}
.qhdsp_c1difft_h_1stStage_END:
{
    MEMD(R26) = R7:6                      //[[e] save C"D"(1)                     ]
    R5 = ASR(R1,#4)                       //[ initial k2 = N/16                   ]
    M0 = R22                              //[ M0 = 4*sizeof(CWord2x16)            ]
    P0 = CMP.GT(R1,#16)                   //[ N > 16 ?                            ]
}
{
    MEMD(R27) = R9:8                      //[[e] save C"D"(2)                     ]
    R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
    R15 = ADD(R23,#8)                     //[ -3*M0+8                             ]
    IF !P0 JUMP .qhdsp_c1difft_h_LastStage //[ If !(N>16) go to last stage         ]
}

    .falign
.qhdsp_c1difft_h_StagesLoop:
{
    R11:10 = MEMD(R0++M0)                 //[ [p]load A                           ]
    R24 = #1                              //[ R24 = i+1, initial i = 0            ]
    R4 = ASR(R22,#3)                      //[ k1/2                                ]
    LOOP1(.qhdsp_c1difft_h_mainLoop,R5)    //[ setup loop1: lc1 = k2               ]
}
{
    R13:12 = MEMD(R0++M0)                 //[ [p]load B                           ]
    M1 = R15                              //[ M1                                  ]
    R4 = ADD(R4,#-1)                      //[ R4 = k1/2 - 1                       ]
    R6 = R2                               //[ R6 = *Wtwiddles                     ]
}
{
    R15:14 = MEMD(R0++M0)                 //[ [p]load C                           ]
    R11:10= VADDH(R11:10,R13:12)          //[ [p]A'= (A+B)/2                      ]
    R13:12=VSUBH(R11:10,R13:12):sat       //[ [p]B'= (A-B)/2                      ]
}

    .falign
.qhdsp_c1difft_h_mainLoop:
{
    R17:16 = MEMD(R0++M1)                 //[ [p]load D                           ]
    R13:12 = VCROTATE(R13:12,R28)         //[ [p] B'= jB'                         ]
    P1 = CMP.EQ(R24,R5)                   //[ P1 = (i+1 == k2)                    ]
    LOOP0(.qhdsp_c1difft_h_innerLoop,R4)   //[ setup loop0: lc0 = k1/2-1           ]
}
{
    R15:14= VADDH(R15:14,R17:16)          //[ [p]C'= (C+D)/2                      ]
    R17:16=VSUBH(R15:14,R17:16):sat       //[ [p]D'= (C-D)/2                      ]
}
{
    R11:10= VADDH(R11:10,R15:14)          //[ [p]A"= (A'+C')/2                    ]
    R15:14=VSUBH(R11:10,R15:14):sat       //[ [p]C"= (A'-C')/2                    ]
    R7 = MEMW(R6+#4)                      //[ [p]load Wa                          ]
    R9 = MEMW(R6+#8)                      //[ [p]load Wc                          ]
}
{
    R8 = MEMW(R6++#12)                    //[ [p]load Wb                          ]
    R13:12= VADDH(R13:12,R17:16)          //[ [p]B"= (B'+D')/2                    ]
    R21:20=VSUBH(R13:12,R17:16):sat       //[ [p]D"= (B'-D')/2                    ]
    R24 = ADD(R24,#1)                     //[ i++                                 ]
}

    .falign
.qhdsp_c1difft_h_innerLoop:
{
    R11:10 = MEMD(R0++M0)                 //[ [1]load A                           ]
    R13 = CMPY(R13,R7*):<<1:rnd:sat       //[ [2]B"=B"*Wa                         ]
    R12 = CMPY(R12,R7*):<<1:rnd:sat       //[ [2]B"=B"*Wa                         ]
    MEMD(R1++M0) = R11:10                 //[ [2]save A"                          ]
}
{
    R13:12 = MEMD(R0++M0)                 //[ [1]load B                           ]
    R15 = CMPY(R15,R8*):<<1:rnd:sat       //[ [2]C"=C"*Wb                         ]
    R14 = CMPY(R14,R8*):<<1:rnd:sat       //[ [2]C"=C"*Wb                         ]
    MEMD(R1++M0) = R13:12                 //[ [2]save B"                          ]
}
{
    R15:14 = MEMD(R0++M0)                 //[ [1]load C                           ]
    R11:10= VADDH(R11:10,R13:12)          //[ [1]A'= (A+B)/2                      ]
    R13:12=VSUBH(R11:10,R13:12):sat       //[ [1]B'= (A-B)/2                      ]
    MEMD(R1++M0) = R15:14                 //[ [2]save C"                          ]
}
{
    R17:16 = MEMD(R0++M1)                 //[ [1]load D                           ]
    R18 = #0                              //[ [1]set R18= 0                       ]
    R20 = CMPY(R20,R9*):<<1:rnd:sat       //[ [2]D"=D"*Wc                         ]
    R21 = CMPY(R21,R9*):<<1:rnd:sat       //[ [2]D"=D"*Wc                         ]
}
{
    R15:14= VADDH(R15:14,R17:16)          //[ [1]C'= (C+D)/2                      ]
    R17:16=VSUBH(R15:14,R17:16):sat       //[ [1]D'= (C-D)/2                      ]
    R19 = VSUBH(R18,R13):sat              //[ [1]R19:18 = -B'                     ]
    R18 = VSUBH(R18,R12):sat              //[ [1]R19:18 = -B'                     ]
}
{
    R11:10= VADDH(R11:10,R15:14)          //[ [1]A"= (A'+C')/2                    ]
    R15:14=VSUBH(R11:10,R15:14):sat       //[ [1]C"= (A'-C')/2                    ]
    R12 = COMBINE(R18.L,R12.H)            //[ [1]B'=-jB'                          ]
    R13 = COMBINE(R19.L,R13.H)            //[ [1]B'=-jB'                          ]
}
{
    R13:12= VADDH(R13:12,R17:16)          //[ [1]B"= (B'+D')/2                    ]
    R21:20=VSUBH(R13:12,R17:16):sat       //[ [1]D"= (B'-D')/2                    ]
    MEMD(R1++M1) = R21:20                 //[ [2]save D"                          ]
}:endloop0
{
    MEMD(R1++M0) = R11:10                 //[ [e]save A"                          ]
    R13 = CMPY(R13,R7*):<<1:rnd:sat       //[ [e]B"=B"*Wa                         ]
    R12 = CMPY(R12,R7*):<<1:rnd:sat       //[ [e]B"=B"*Wa                         ]
    IF (!P1) R0 = SUB(R0,R23)             //[ [p]R0 = R0+3*M0                     ]
}
{
    MEMD(R1++M0) = R13:12                 //[ [e]save B"                          ]
    R15 = CMPY(R15,R8*):<<1:rnd:sat       //[ [e]C"=C"*Wb                         ]
    R14 = CMPY(R14,R8*):<<1:rnd:sat       //[ [e]C"=C"*Wb                         ]
    R11:10 = MEMD(R0++M0)                 //[ [p]load A                           ]
}
{
    MEMD(R1++M0) = R15:14                 //[ [e]save C"                          ]
    R20 = CMPY(R20,R9*):<<1:rnd:sat       //[ [e] D"=D"*Wc                        ]
    R21 = CMPY(R21,R9*):<<1:rnd:sat       //[ [e]D"=D"*Wc                         ]
    R13:12 = MEMD(R0++M0)                 //[ [p]load B                           ]
}
{
    MEMD(R1++#8) = R21:20                 //[ [e]save D"                          ]
    R15:14 = MEMD(R0++M0)                 //[ [p]load C                           ]
    R11:10= VADDH(R11:10,R13:12)          //[ [p]A'= (A+B)/2                      ]
    R13:12=VSUBH(R11:10,R13:12):sat       //[ [p]B'= (A-B)/2                      ]
}:endloop1
{
    R23:22 = VASLW(R23:22,#2)             //[ update -3*k1*4 : k1*4  (k1<<2)      ]
    R5 = ASR(R5,#2)                       //[ k2 >>=2                             ]
    P0 = CMP.GT(R5,#4)                    //[ P0=( (k2>>2)>1 )                    ]
}
{
    IF P0 JUMP .qhdsp_c1difft_h_StagesLoop //[ IF P0 continue next stage           ]
    R1:0 = COMBINE(R3,R3)                 //[ R1 = R0 = output                    ]
    M0 = R22                              //[ set M0 = k1*4                       ]
    R15 = ADD(R23,#8)                     //[ -3*M0+8                             ]
}

.qhdsp_c1difft_h_LastStage:
{
    P0 = CMP.EQ(R5,#0)                    //[ P0 = (k2 == 0)                      ]
    IF P0.new JUMP:nt .qhdsp_c1difft_h_RADIX2_LastStage
                                          //[ IF P0 last stage is Radix2          ]
}
/*-----------------------------------------------------------------------------*/
/*                       Last Stage  (Radix-4)                                 */
/*-----------------------------------------------------------------------------*/
{
    R2 = ADD(R1,R22)                      //[ R2 = &output[1*N/4]                 ]
    R5 = ASR(R22,#3)                      //[ N/8 (R22= k1*4, k1=N/4)             ]
    M1 = R15                              //[ M1                                  ]
}
{
    R3 = ADD(R2,R22)                      //[ R3 = &output[2*N/4]                 ]
    R4 = ADDASL(R2,R22,#1)                //[ R4 = &output[3*N/4                  ]
    P3 = SP1LOOP0(.qhdsp_c1difft_h_RADIX4_LastStageLoop,R5)
                                          //[ setup inner loop: lc0 = N/8         ]
    R11:10 = MEMD(R0++M0)                 //[ [p]load A                           ]
}

    .falign
.qhdsp_c1difft_h_RADIX4_LastStageLoop:
{
    R13:12 = MEMD(R0++M0)                 //[ [2]load B                           ]
    R17:16 = VCROTATE(R17:16,R28)         //[ [3]-j*D'                            ]
}
{
    R15:14 = MEMD(R0++M0)                 //[ [2]load C                           ]
    R19:18 = VSUBH(R9:8,R17:16):sat       //[ [3]B"=(B'+jD')/2                    ]
    R17:16 =  VADDH(R9:8,R17:16)          //[ [3]D"=(B'-jD')/2                    ]
    IF P3 MEMD(R3++#8) = R15:14           //[ [3]save C"                          ]
}
{
    R17:16 = MEMD(R0++M1)                 //[ [2]load D                           ]
    R11:10=  VADDH(R11:10,R13:12)         //[ [2]A'= (A+B)/2                      ]
    R9:8  = VSUBH(R11:10,R13:12):sat      //[ [2]B'= (A-B)/2                      ]
    IF P3 MEMD(R4++#8) = R17:16           //[ [3]save D"                          ]
}
{
    R15:14=  VADDH(R15:14,R17:16)         //[ [2]C'= (C+D)/2                      ]
    R17:16= VSUBH(R15:14,R17:16):sat      //[ [2]D'= (C-D)/2                      ]
    IF P3 MEMD(R2++#8) = R19:18           //[ [3]save B"                          ]
}
{
    R11:10 = MEMD(R0++M0)                 //[ [1]load A                           ]
    R21:20=  VADDH(R11:10,R15:14)         //[ [2]A"=(A'+C')/2                     ]
    R15:14= VSUBH(R11:10,R15:14):sat      //[ [2]C"=(A'-C')/2                     ]
    IF P3 MEMD(R1++#8) = R21:20           //[ [3]save A"                          ]
}:endloop0

{
    MEMD(R1) = R21:20                     //[ [e]save A"                          ]
    R17:16 = VCROTATE(R17:16,R28)         //[ [e]-j*D'                            ]
    R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
}
{
    MEMD(R3) = R15:14                     //[ [e]save C"                          ]
    R19:18 = VSUBH(R9:8,R17:16):sat       //[ [e]B"=(B'+jD')/2                    ]
    R17:16 =  VADDH(R9:8,R17:16)          //[ [e]D"=(B'-jD')/2                    ]
    R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
}
{
    MEMD(R2) = R19:18                     //[ [e]save B"                          ]
    R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
}
{
    MEMD(R4) = R17:16                     //[ [e]save D"                          ]
    R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
    JUMP  .qhdsp_c1difft_h_DONE            //[ If FALSE Return                     ]
}

/*-----------------------------------------------------------------------------*/
/*                       Last Stage  (Radix-2)                                 */
/*-----------------------------------------------------------------------------*/
    .falign
.qhdsp_c1difft_h_RADIX2_LastStage:
{
    R27:26 = MEMD(R29+#40)                //[ restore callee-saved registers      ]
    R0 += ASL(R25,#1)                     //[ R0 = &output[N/2]                   ]
    R4 = ASR(R25,#2)                      //[ N/4                                 ]
}
{
    R25:24 = MEMD(R29+#32)                //[ restore callee-saved registers      ]
    R2 = R0                               //[ R2 = &output[N/2]                   ]
    P3 = SP1LOOP0(.qhdsp_c1difft_h_RADIX2_LastStageLoop,R4)
                                          //[ setup loop: lc0 = N/4               ]
    R7:6 = MEMD(R1++#8)                   //[ [p]load A                           ]
}

    .falign
.qhdsp_c1difft_h_RADIX2_LastStageLoop:
{
    R9:8 = MEMD(R0++#8)                   //[ [1]load B                           ]
    IF (P3) MEMD(R2++#8) = R13:12         //[ [2]save B'                          ]
}
{
    R11:10 =  VADDH(R7:6,R9:8)            //[ [1]A'= (A+B)/2                      ]
    R13:12 = VSUBH(R7:6,R9:8):sat         //[ [1]B'= (A-B)/2                      ]
    IF (P3) MEMD(R3++#8) = R11:10         //[ [2]save A'                          ]
    R7:6 = MEMD(R1++#8)                   //[ [3]load A                           ]
}:endloop0

{
    MEMD(R2) = R13:12                     //[ [e] save results                    ]
    R23:22 = MEMD(R29+#24)                //[ restore callee-saved registers      ]
}
{
    MEMD(R3) = R11:10                     //[ [e] save results                    ]
    R21:20 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
}
    .falign
.qhdsp_c1difft_h_DONE:
{
    R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
    R29 = ADD(R29,#6*8)                   //[ pop stack                           ]
    JUMPR R31                             //[ return                              ]
}

    .size    qhdsp_c1difft_h, .-qhdsp_c1difft_h

