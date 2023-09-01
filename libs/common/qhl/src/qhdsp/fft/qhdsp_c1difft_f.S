/**=============================================================================
@file
   qhdsp_c1difft_f.S

@brief
   Performs complex 2^N IFFT for single precision float point.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

.text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_c1difft_f
    .type qhdsp_c1difft_f, @function
qhdsp_c1difft_f:
{
    P0 = CMP.GT(R1,#3)                    //[ N > 3 ?                             ]
    IF !P0.new JUMPR:nt R31               //[ if !(N>=4) return                   ]
}
{
    R29 = ADD(R29, #-8*8)                 //[ allocate stack                      ]
    R7 = CL0(R1)                          //[ used to calculate log2(N)           ]
    R4 = #0                               //[ R4 = 0                              ]
    R5 = BREV(R0)                         //[ to bit reverse R0.L                 ]
}
{
    MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
    MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
    R0 = COMBINE(R0.H,R5.H)               //[ bit reverse R0.L (bit-reversed addr)]
    R7 = ADD(R7,#-18)                     //[ 16-log2(8N) = cl0(N)-18             ]
}
{
    MEMD(R29+#16) = R21:20                //[ callee-saved registers              ]
    R4 = SETBIT(R4,R7)                    //[ R4= 1<<(15-log2(4N))= 1<<(13-LOG2N) ]
    R6 = ASR(R1, #2)                      //[ N/4 (for qhdsp_c1difft_f_1stStageLoop)       ]
    MEMW(R29+#60) = R1
}
{
    MEMD(R29+#24) = R23:22                //[ callee-saved registers              ]
    MEMD(R29+#32) = R25:24                //[ callee-saved registers              ]
    M0 = R4                               //[ set M0 for bit-reversed addressing  ]
    R4 = R2                               //[ set R6 = twiddle                    ]
}
{
    MEMD(R29+#40) = R27:26                //[ callee-saved registers              ]
    MEMD(R29+#48) = R31:30                //[ callee-saved registers              ]
    R30 = ADD(R1,R1)                      //[ R30 = 2*N                           ]
    R1 = R3                               //[ set R1=  Output                     ]
}
{
    MEMW(R29+#56) = R3
}
{
    R28 = #0x80000000                       // mask for XOR operation - changing sign
    P3 = SP1LOOP0(.qhdsp_c1difft_f_1stStageLoop,R6)  //[ setup loop0: lc0 = N/4    ]
}

    .falign
.qhdsp_c1difft_f_1stStageLoop:
{
    R9 = XOR(R9,R28)                      //                                      ]        R9 = conj(Wc.i) = change sign of imag part
    R11:10 = MEMD(R0++M0:brev)            //[[1] load A                           ]        R10 = x[0].real; R11 = x[0].imag
    R18 = SFMPY(R8,R16)                   //[[2]Wc.r * D".r                       ]        R18 = Wc.r * (b-d).real;    R8 - Wc.r; R16 - x[3](output).real = (b-d)
    R19 = SFMPY(R8,R17)                   //[[2]Wc.r * D".i                       ]        R19 = Wc.r * (b-d).imag;    R8 - Wc.r; R17 - x[3](output).imag = (b-d)
}
{
    R13:12 = MEMD(R0++M0:brev)            //[[1]load B                            ]        R12 = x[1].real; R13 = x[1].imag
    R18 -= SFMPY(R9,R17)                  //[[2]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]        R18 = x[3](output) = Wc.r * (b-d).real - conj(Wc.i) * x[3](output).imag;
    R19 += SFMPY(R9,R16)                  //[[2]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]        R19 = x[3](output) = Wc.r * (b-d).imag - conj(Wc.i) * x[3](output).real;
}
{
    R15:14 = MEMD(R0++M0:brev)            //[[1]load C                            ]        R14 = x[2].real; R15 = x[2].imag
    R6 = SFADD(R10,R12)                   //[[1]A'= A + B                         ]        R6 = a.real = x[0].real + x[1].real
    R7 = SFADD(R11,R13)                   //[[1]A'= A + B                         ]        R7 = a.imag = x[0].imag + x[1].imag
    IF P3 MEMD(R1++#8) = R15:14           //[[2]save C"                           ]        save x[2](output) * Wb
}
{
    R17:16 = MEMD(R0++M0:brev)            //[[1]load D                            ]        R16 = x[3].real; R17 = x[3].imag
    R12 = SFSUB(R10,R12)                  //[[1]B'= A - B                         ]        R12 = b.real = x[0].real - x[1].real
    R13 = SFSUB(R11,R13)                  //[[1]B'= A - B                         ]        R13 = b.imag = x[0].imag - x[1].imag
    IF P3 MEMD(R1++#8) = R19:18           //[[2]save D"                           ]        save x[3](output) * Wc
}
{
    R8 = SFADD(R14,R16)                   //[[1]C'= C + D                         ]        R8 = c.real = x[2].real + x[3].real
    R9 = SFADD(R15,R17)                   //[[1]C'= C + D                         ]        R9 = c.imag = x[2].imag + x[3].imag
}
{
    R16 = SFSUB(R14,R16)                  //[[1]D'= C - D                         ]        R16 = d.real = x[2].real - x[3].real
    R17 = SFSUB(R15,R17)                  //[[1]D'= C - D                         ]        R17 = d.imag = x[2].imag - x[3].imag
}
{
    R10 = SFADD(R6,R8)                    //[[1]A"= A'+ C'                        ]        R10 = x[0](output).real = a.real + c.real
    R11 = SFADD(R7,R9)                    //[[1]A"= A'+ C'                        ]        R11 = x[0](output).imag = a.imag + c.imag
}
{
    R8 = SFSUB(R6,R8)                     //[[1]C"= A'- C'                        ]        R8 = x[2](output).real = a.real - c.real
    R9 = SFSUB(R7,R9)                     //[[1]C"= A'- C'                        ]        R9 = x[2](output).imag = a.imag - c.imag
    R7:6 = MEMD(R4+#8)                    //[[1]load Wa                           ]        R7 = Wa.i; R6 = Wa.r
}
{
    R7 = XOR(R7,R28)                      //                                      ]        R7 = conj(Wa.i) = change sign of imag part
    R18 = SFSUB(R12,R17)                  //[[1]B"= B' - j*D'                     ]        R18 = x[1](output).real = b.real - d.imag (b+(j*d))
    R19 = SFADD(R13,R16)                  //[[1]B"= B' - j*D'                     ]        R19 = x[1](output).imag = b.imag + d.real (b+(j*d))
    MEMD(R1++#8) = R11:10                 //[[1]save A"                           ]        save x[0](output)
}
{
    R16 = SFADD(R12,R17)                  //[[1]D"= B'+ j*D'                      ]        R16 = x[3](output).real = b.real + d.imag (b-(j*d))
    R17 = SFSUB(R13,R16)                  //[[1]D"= B'+ j*D'                      ]        R17 = x[3](output).imag = b.imag - d.real (b-(j*d))
}
{
    R10 = SFMPY(R6,R18)                   //[[1]Wa.r * B".r                       ]        R10 = Wa.r * x[1](output).real
    R11 = SFMPY(R6,R19)                   //[[1]Wa.r * B".i                       ]        R11 = Wa.r * x[1](output).imag
}
{
    R10 -= SFMPY(R7,R19)                  //[[1]Wa*B" = Wa.r*B".r-Wa.i*B".i       ]        R10 = (Wa.r * x[1](output).real) - (conj(Wa.i) * x[1](output).imag)
    R11 += SFMPY(R7,R18)                  //[[1]Wa*B" = Wa.r*B".i+Wa.i*B".r       ]        R11 = (Wa.r * x[1](output).real) + (conj(Wa.i) * x[1](output).real)
    R7:6 = MEMD(R4++#16)                  //[[1]load Wb                           ]        R7 = Wb.i; R6 = Wb.r
}
{
    R7 = XOR(R7,R28)                      //                                      ]        R7 = conj(Wb.i) = change sign of imag part
    R14 = SFMPY(R6,R8)                    //[[1]Wb.r * C".r                       ]        R14 = Wb.r * x[2](output).real
    R15 = SFMPY(R6,R9)                    //[[1]Wb.r * C".i                       ]        R15 = Wb.r * x[2](output).imag
    MEMD(R1++#8) = R11:10                 //[[1]save B"                           ]        save x[1](output) * Wa
}
{
    R14 -= SFMPY(R7,R9)                   //[[1]Wb*C" = Wb.r*C".r-Wb.i*C".i       ]        R14 = Wb.r * x[2](output).real - conj(Wb.i) * x[2](output).imag
    R15 += SFMPY(R7,R8)                   //[[1]Wb*C" = Wb.r*C".i+Wb.i*C".r       ]        R15 = Wb.r * x[2](output).real + conj(Wb.i) * x[2](output).real
    R9:8 = MEMD(R4++#8)                   //[[1]load Wc                           ]        R9 = Wc.i; R8 = Wc.r
}:endloop0


.qhdsp_c1difft_f_Stage1_END:
{
    R9 = XOR(R9,R28)                      // R9 = conj(Wc.i) = change sign of imag part
}
{
    R18 = SFMPY(R8,R16)                   //[[e]Wc.r * D".r                       ]        R18 = Wc.r * x[3](output).real
    R19 = SFMPY(R8,R17)                   //[[e]Wc.r * D".i                       ]        R19 = Wc.r * x[3](output).imag
    MEMD(R1++#8) = R15:14                 //[[e]save C"                           ]        save x[2](output) * Wb
    R21:20 = COMBINE(#-3*4*8,#4*8)        //[ -3*k1*8 :k1*8, where initial k1=4   ]
}
{
    R18 -= SFMPY(R9,R17)                  //[[e]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]        R18 = x[3](output) = Wc.r * x[3](output).real - conj(Wc.i) * x[3](output).imag;
    R19 += SFMPY(R9,R16)                  //[[e]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]        R19 = x[3](output) = Wc.r * x[3](output).imag - conj(Wc.i) * x[3](output).real;
    P0 = CMP.GT(R30,#8)                   //[ P0 = (N > 4)                        ]
    P1 = CMP.GT(R30,#32)                  //[ P1 = (N>16)                         ]
}
{
    MEMD(R1++#8) = R19:18                 //[[e]save D"                           ]        save x[3](output) * Wc
    R5 = ASR(R30,#5)                      //[ initial k2 = N/16                   ]
    IF !P0 JUMP .qhdsp_c1difft_f_DONE              //[ If !(N>4) then return       ]
    IF !P0 R31:30 = MEMD(R29+#48)         //[ and restore callee-saved registers  ]
}
{
    M0 = R20                              //[ set M0 = 8*k1                       ]
    IF !P1 JUMP .qhdsp_c1difft_f_LastStage         //[ if !(N>16) go to last stage ]
}

    .falign
.qhdsp_c1difft_f_StagesLOOP:
{
    R31 = ASR(R20,#3)                     //[ R31 = k1                            ]
    R15 = ADD(R21,#8)                     //[ -3*k1*8 + 8                         ]
    LOOP1(.qhdsp_c1difft_f_mainLOOP,R5)            //[ setup loop1: lc1 = k2       ]
}
{
    M1 = R15                              //[ set M1 = -3*k1*8 + 8                ]
    R4 = R2                               //[ set R4 = *Wtwiddles                 ]
    R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
    P2 = CMP.GT(R0,R0)                    //[ clean P2                            ]
}

    .falign
.qhdsp_c1difft_f_mainLOOP:
{
    R18 -= SFMPY(R27,R17)                 //[[e1]Wc*D" = Wc.r*D".r-Wc.i*D".i      ]
    R19 += SFMPY(R27,R16)                 //[[e1]Wc*D" = Wc.r*D".i+Wc.i*D".r      ]
    R27:26 = MEMD(R4+#16)                 //[ load Wc                             ]
    R23:22 = MEMD(R4++#8)                 //[ load Wb                             ]
}
{
    R27 = XOR(R27,R28)                    // R27 = conj(Wc.i) = change sign of imag part
    R23 = XOR(R23,R28)                    // R23 = conj(Wb.i) = change sign of imag part
}
{
    IF P2 MEMD(R1++#8) = R19:18           //[[e1]save D"                          ]
    R25:24 = MEMD(R4++#16)                //[ load Wa                             ]
    P2 = CMP.EQ(R0,R0)                    //[ set P2                              ]
}
{
    R25 = XOR(R25,R28)                    // R25 = conj(Wa.i) = change sign of imag part
    P3=SP1LOOP0(.qhdsp_c1difft_f_innerLOOP,R31)    //[ setup loop0 with LC= k1     ]
}

    .falign
.qhdsp_c1difft_f_innerLOOP:
{
    R11:10 = MEMD(R0++M0)                 //[[1]load A                            ]
    R18 = SFMPY(R26,R16)                  //[[2]Wc.r * D".r                       ]
    R19 = SFMPY(R26,R17)                  //[[2]Wc.r * D".i                       ]
}
{
    R13:12 = MEMD(R0++M0)                 //[[1]load B                            ]
    R18 -= SFMPY(R27,R17)                 //[[2]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]
    R19 += SFMPY(R27,R16)                 //[[2]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]
}
{
    R15:14 = MEMD(R0++M0)                 //[[1]load C                            ]
    R6 = SFADD(R10,R12)                   //[[1]A'= A + B                         ]
    R7 = SFADD(R11,R13)                   //[[1]A'= A + B                         ]
    IF P3 MEMD(R1+#0) = R15:14            //[[2]save C"                           ]
}
{
    R17:16 = MEMD(R0++M1)                 //[[1]load D                            ]
    R12 = SFSUB(R10,R12)                  //[[1]B'= A - B                         ]
    R13 = SFSUB(R11,R13)                  //[[1]B'= A - B                         ]
    IF P3 R1 = ADD(R1,R20)                //[[2]update write pointer              ]
}
{
    R8 = SFADD(R14,R16)                   //[[1]C'= C + D                         ]
    R9 = SFADD(R15,R17)                   //[[1]C'= C + D                         ]
    IF P3 MEMD(R1++#8) = R19:18           //[[2]save D"                           ]
}
{
    R16 = SFSUB(R14,R16)                  //[[1]D'= C - D                         ]
    R17 = SFSUB(R15,R17)                  //[[1]D'= C - D                         ]
    IF P3 R1 = ADD(R1,R21)                //[[2]update write pointer              ]
}
{
    R10 = SFADD(R6,R8)                    //[[1]A"= A'+ C'                        ]
    R11 = SFADD(R7,R9)                    //[[1]A"= A'+ C'                        ]
}
{
    R8 = SFSUB(R6,R8)                     //[[1]C"= A'- C'                        ]
    R9 = SFSUB(R7,R9)                     //[[1]C"= A'- C'                        ]
}
{
    R18 = SFSUB(R12,R17)                  //[[1]B"= B'- j*D'                      ]
    R19 = SFADD(R13,R16)                  //[[1]B"= B'- j*D'                      ]
    MEMD(R1++M0) = R11:10                 //[[1]save A"
}
{
    R16 = SFADD(R12,R17)                  //[[1]D"= B'+ j*D'                      ]
    R17 = SFSUB(R13,R16)                  //[[1]D"= B'+ j*D'                      ]
}
{
    R10 = SFMPY(R24,R18)                  //[[1]Wa.r * B".r                       ]
    R11 = SFMPY(R24,R19)                  //[[1]Wa.r * B".i                       ]
}
{
    R10 -= SFMPY(R25,R19)                 //[[1]Wa*B" = Wa.r*B".r-Wa.i*B".i       ]
    R11 += SFMPY(R25,R18)                 //[[1]Wa*B" = Wa.r*B".i+Wa.i*B".r       ]
}
{
    R14 = SFMPY(R22,R8)                   //[[1]Wb.r * C".r                       ]
    R15 = SFMPY(R22,R9)                   //[[1]Wb.r * C".i                       ]
    MEMD(R1++M0) = R11:10                 //[[1]save B"
}
{
    R14 -= SFMPY(R23,R9)                  //[[1]Wb*C" = Wb.r*C".r-Wb.i*C".i       ]
    R15 += SFMPY(R23,R8)                  //[[1]Wb*C" = Wb.r*C".i+Wb.i*C".r       ]
}:endloop0

{
    R18 = SFMPY(R26,R16)                  //[[e]Wc.r * D".r                       ]
    R19 = SFMPY(R26,R17)                  //[[e]Wc.r * D".i                       ]
    MEMD(R1++M0) = R15:14                 //[[e]save C"                           ]
    R0 = SUB(R0,R21)                      //[ R0 = R0 + 3*M0                      ]
}:endloop1

{
    R18 -= SFMPY(R27,R17)                 //[[e]Wc*D" = Wc.r*D".r-Wc.i*D".i       ]
    R19 += SFMPY(R27,R16)                 //[[e]Wc*D" = Wc.r*D".i+Wc.i*D".r       ]
}
{
    MEMD(R1) = R19:18                     //[[e]save D"                           ]
    R21:20 = VASLW(R21:20,#2)             //[ update -3*8*k1 : 8*k1               ]
    R5 = ASR(R5,#2)                       //[ k2 >>= 2                            ]
    P0 = CMP.GT(R5,#4)                    //[ P0 = ((k2 >>2)> 1)                  ]
}
{
    M0 = R20                              //[ update M0 = 8*k1                    ]
    IF P0 JUMP .qhdsp_c1difft_f_StagesLOOP         //[ IF P0 continue next radix-4 stage   ]
}

.qhdsp_c1difft_f_LastStage:
{
    P0 = CMP.EQ(R5,#0)                    //[ P0=(k2==0),i.e.,last stage is Radix2]
    R1:0 = COMBINE(R3,R3)                 //[ set read/write pointer R0/R1        ]
    R15 = ADD(R21,#8)                     //[ -3*k1*8 + 8                         ]
    IF P0.new JUMP:nt  .qhdsp_c1difft_f_RADIX2_LastStage
                                          //[ If P0 do Radix2                     ]
}
/*-----------------------------------------------------------------------------*/
/*                       Last Radix-4 stage                                    */
/*-----------------------------------------------------------------------------*/
{
    R2 = ADD(R1,R30)                      //[ R2 = &output[1*N/4]                 ]
    R5 = ASR(R30,#3)                      //[ N/4                                 ]
    M1 = R15                              //[ set M1
}
{
    R3 = ADD(R2,R30)                      //[ R3 = &output[2*N/4]                 ]
    R4 = ADDASL(R2,R30,#1)                //[ R4 = &output[3*N/4]                 ]
    P3 = SP1LOOP0(.qhdsp_c1difft_f_RADIX4_LastStageLoop,R5)
                                          //[ setup loop0: lc0 = N/4              ]
}

    .falign
.qhdsp_c1difft_f_RADIX4_LastStageLoop:
{
    R11:10 = MEMD(R0++M0)                 //[[1] load A                           ]
    R18 = SFSUB(R12,R17)                  //[[2]B"= B'- j*D'                      ]
    R19 = SFADD(R13,R16)                  //[[2]B"= B'- j*D'                      ]
}
{
    R13:12 = MEMD(R0++M0)                 //[[1]load B                            ]
    R16 = SFADD(R12,R17)                  //[[2]D"= B'+ j*D'                      ]
    R17 = SFSUB(R13,R16)                  //[[2]D"= B'+ j*D'                      ]
    IF P3 MEMD(R2++#8) = R19:18           //[[2]save B"                           ]
}
{
    R15:14 = MEMD(R0++M0)                 //[[1]load C                            ]
    R6 = SFADD(R10,R12)                   //[[1]A'= A + B                         ]
    R7 = SFADD(R11,R13)                   //[[1]A'= A + B                         ]
    IF P3 MEMD(R3++#8) = R9:8             //[[2]save C"                           ]
}
{
    R17:16 = MEMD(R0++M1)                 //[[1]load D                            ]
    R12 = SFSUB(R10,R12)                  //[[1]B'= A - B                         ]
    R13 = SFSUB(R11,R13)                  //[[1]B'= A - B                         ]
    IF P3 MEMD(R4++#8) = R17:16           //[[2]save D"                           ]
}
{
    R8 = SFADD(R14,R16)                   //[[1]C'= C + D                         ]
    R9 = SFADD(R15,R17)                   //[[1]C'= C + D                         ]
}
{
    R16 = SFSUB(R14,R16)                  //[[1]D'= C - D                         ]
    R17 = SFSUB(R15,R17)                  //[[1]D'= C - D                         ]
}
{
    R10 = SFADD(R6,R8)                    //[[1]A"= A'+ C'                        ]
    R11 = SFADD(R7,R9)                    //[[1]A"= A'+ C'                        ]
}
{
    R8 = SFSUB(R6,R8)                     //[[1]C"= A'- C'                        ]
    R9 = SFSUB(R7,R9)                     //[[1]C"= A'- C'                        ]
    MEMD(R1++#8) = R11:10                 //[[1]save A"                           ]
}:endloop0

{
    R18 = SFSUB(R12,R17)                  //[[e]B"= B'- j*D'                      ]
    R19 = SFADD(R13,R16)                  //[[e]B"= B'- j*D'                      ]
    MEMD(R3) = R9:8                       //[[e]save C"                           ]
}
{
    R16 = SFADD(R12,R17)                  //[[e]D"= B'+ j*D'                      ]
    R17 = SFSUB(R13,R16)                  //[[e]D"= B'+ j*D'                      ]
    MEMD(R2) = R19:18                     //[[e]save B"                           ]
}
{
    MEMD(R4) = R17:16                     //[[e]save D"                           ]
    R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
    JUMP .qhdsp_c1difft_f_DONE                     //[ go to return                ]
}

/*-----------------------------------------------------------------------------*/
/*                       Last Radix-2 stage                                    */
/*-----------------------------------------------------------------------------*/
    .falign
.qhdsp_c1difft_f_RADIX2_LastStage:
{
    R0 += ASL(R30,#1)                     //[ R0 = &output[N/2] , R1=&output[0]   ]
    R5 = ASR(R30,#3)                      //[ N/4                                 ]
}
{
    R2 = R0                               //[ R2 points to output[N/2]            ]
    P3 = SP1LOOP0(.qhdsp_c1difft_f_RADIX2_LOOP,R5) //[ setup loop0: lc0 = N/4      ]
    R7:6 = MEMD(R1++#8)                   //[[p]load A                            ]
}

    .falign
.qhdsp_c1difft_f_RADIX2_LOOP:
{
    R9:8 = MEMD(R0++#8)                   //[[2]load B      (1)                   ]
    R10 = SFADD(R10,R12)                  //[[3]A'= A+B     (2)                   ]
    R11 = SFADD(R11,R13)                  //[[3]A'= A+B                           ]
    IF P3 MEMD(R2++#8) = R9:8             //[[3]save B'     (2)                   ]
}
{
    R11:10 = MEMD(R1++#8)                 //[[2]load A      (2)                   ]
    R12 = SFSUB(R6,R8)                    //[[2]B'= A-B     (1)                   ]
    R13 = SFSUB(R7,R9)                    //[[2]B'= A-B                           ]
    IF P3 MEMD(R3++#8) = R11:10           //[[3]save A'     (2)                   ]
}
{
    MEMD(R2++#8) = R13:12                 //[[2]save B'     (1)                   ]
    R13:12 = MEMD(R0++#8)                 //[[2]load B                            ]
    R6 = SFADD(R6,R8)                     //[[2]A'= A+B     (1)                   ]
    R7 = SFADD(R7,R9)                     //[[2]A'= A+B                           ]
}
{
    R7:6 = MEMD(R1++#8)                   //[[1]load A       (1)                  ]
    MEMD(R3++#8) = R7:6                   //[[2]save A'      (1)                  ]
    R8 = SFSUB(R10,R12)                   //[[2]B'= A-B      (2)                  ]
    R9 = SFSUB(R11,R13)                   //[[2]B'= A-B                           ]
}:endloop0

{
    R10 = SFADD(R10,R12)                  //[[e]A'= A+B     (2)                   ]
    R11 = SFADD(R11,R13)                  //[[e]A'= A+B                           ]
    MEMD(R2) = R9:8                       //[[e]save B'     (2)                   ]
}
{
    MEMD(R3) = R11:10                     //[[e]save A'     (2)                   ]
    R31:30 = MEMD(R29+#48)                //[ restore callee-saved registers      ]
}

    .falign
.qhdsp_c1difft_f_DONE:
    // do:
    /*
        for (int i = 0; i < nPoints; i += 1)
        {
            IFFT_output[i] *= 1/nPoints;
        }
     */

{
    R0 = MEMW(R29+#56)                      // output buffer pointer
    R1 = MEMW(R29+#60)                      // nPoints
}
{
    R6 = CL0(R1)                            //[ used to calculate log2(N)
    P0 = CMP.GT(R1,#0); if (!P0.new) jump:nt .skipForLoop
}
{
    R7 = SUB(#31,R6)                        // R7 = LOG2(N)
    R2 = R0
    R3 = ADD(PC,##fft_window_reciprocal@PCREL)
}
{
    R5:4 = MEMD(R0++#8)                     // R5 = output.imag; R4 = output.real
    R6 = ADDASL(R3,R7,#2)
}
{
    R3 = MEMW(R6+#-4)                       // -4 since we need N-1
    P3 = SP1LOOP0(.forLoopStart,R1)
}
.falign
.forLoopStart:
{
    r6 = SFMPY(r4,r3)
    r7 = SFMPY(r5,r3)
    IF P3 MEMD(R2++#8) = R7:6
    R5:4 = MEMD(R0++#8)                     // R5 = output.imag; R4 = output.real
} :endloop0
{
    MEMD(R2++#8) = R7:6                     // save results from last iteration
}
.skipForLoop:


{
    R27:26 = MEMD(R29+#40)                  //[ restore callee-saved registers      ]
    R25:24 = MEMD(R29+#32)                  //[ restore callee-saved registers      ]
}
{
    R23:22 = MEMD(R29+#24)                  //[ restore callee-saved registers      ]
    R21:20 = MEMD(R29+#16)                  //[ restore callee-saved registers      ]
}
{
    R19:18 = MEMD(R29+#8)                   //[ restore callee-saved registers      ]
    R17:16 = MEMD(R29+#0)                   //[ restore callee-saved registers      ]
    R29 = ADD(R29,#8*8)                     //[ pop stack                           ]
    JUMPR R31                               //[ return                              ]
}
    .size    qhdsp_c1difft_f, .-qhdsp_c1difft_f


    .type    fft_window_reciprocal,@object // @fft_window_reciprocal
    .section    .rodata,"a",@progbits
    .p2align    3
fft_window_reciprocal:
    .word    1056964608              // float 0.5
    .word    1048576000              // float 0.25
    .word    1040187392              // float 0.125
    .word    1031798784              // float 0.0625
    .word    1023410176              // float 0.03125
    .word    1015021568              // float 0.015625
    .word    1006632960              // float 0.0078125
    .word    998244352               // float 0.00390625
    .word    989855744               // float 0.001953125
    .word    981467136               // float 9.765625E-4
    .word    973078528               // float 4.8828125E-4
    .word    964689920               // float 2.44140625E-4
    .word    956301312               // float 1.22070313E-4
    .word    947912704               // float 6.10351563E-5
    .word    939524096               // float 3.05175781E-5
    .word    931135488               // float 1.52587891E-5
    .size    fft_window_reciprocal, 64
