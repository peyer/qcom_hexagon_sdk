/**=============================================================================
@file
   qhdsp_r1dfft_f.S

@brief
   Performs real 2^N FFT for single precision float point.

   [=============================================================================]
   [  Assumptions:                                                               ]
   [           - complex number: interleaved real/image with real part           ]
   [             at low memory address                                           ]
   [           - input  is aligned by size of the array                          ]
   [           - output is aligned by 8bytes                                     ]
   [           - N >= 8                                                          ]
   [=============================================================================]
   [  Inputs     : R0 : float *input                                             ]
   [               R1 : int       N                                              ]
   [               R2 : cfloat *Wt1    -- twiddle factors for N/2-point cxFFT    ]
   [               R3 : cfloat *Wt2    -- twiddel factors for last stage         ]
   [               R4 : cfloat *output                                           ]
   [=============================================================================]
   [  Register Usage: R0-R19, R28                                                ]
   [  Hardware Loops affected: Loop0                                             ]
   [                                                                             ]
   [  Stack Memory Frame Allocated (in Bytes): 16+16                             ]
   [=============================================================================]
   [  Implementation:                                                            ]
   [           - Radix-4 DIF                                                     ]
   [           - INPUT: LOAD IN BIT-REVERSE ORDER                                ]
   [=============================================================================]
   [  Cycle Count:                                                               ]
   [                                                                             ]
   [=============================================================================]

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

    .file   "qhdsp_r1dfft_f.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_r1dfft_f
    .type    qhdsp_r1dfft_f, @function

qhdsp_r1dfft_f:
{
    P0 = CMP.GT(R1,#7)                    //[ if !(N>=8)                          ]
    IF !P0.new JUMPR:nt R31               //[ then return                         ]
}
{
    ALLOCFRAME(#2*8)                      //[ reserve stack                       ]
}
{
    MEMD(R29+#0) = R17:16                 //[ callee-saved registers              ]
    MEMD(R29+#8) = R19:18                 //[ callee-saved registers              ]
    R19:18 = COMBINE(R3,R1)               //[ R19 = wt2,  R18 = N                 ]
    R3 = R4                               //[ func call: R3 = output              ]
}
/* --------------------------------------------------------------------------- */
/*                    Call N/2-point FFT                                       */
/* --------------------------------------------------------------------------- */
{
    R1 = LSR(R1,#1)                       //[ func call: R1 = N/2                 ]
    R16 = R4                              //[ R16 = output                        ]
    CALL qhdsp_c1dfft_f                    //[ qhdsp_c1dfft_f()                     ]
}
    .falign
.qhdsp_r1dfft_f_LastStage:
/* --------------------------------------------------------------------------- */
/*                    Calcuate last stage butterflies                          */
/* --------------------------------------------------------------------------- */
{
    R0 = R16                              //[ R0 = &Z[0]                          ]
    R1 = ADDASL(R16,R18,#2)               //[ &Z[N/2]                             ]
    R6 = #0                               //[ constant 0                          ]
    R7 = ASR(R18,#2)                      //[ N/4
}
{
    R3:2 = R1:0                           //[ R2 = &Z[0]; R3 = &Z[N/2]            ]
    R1 = ADD(R1,#-8)                      //[ R1 = &Z[N/2-1]                      ]
    R5:4 = MEMD(R0++#8)                   //[ load Z[0]                           ]
    R28 = ADDASL(R16,R18,#3)              //[ R28 = &output[N]                    ]
}
{
    R9:8   = COMBINE(R6,R4)               //[ X = Z.r + 1i*0                      ]
    R15:14 = COMBINE(R6,R5)               //[ Y = Z.i + 1i*0                      ]
    R17 = ##0x3F000000                    //[ constant 0.5                        ]
}
{
    LOOP0(.qhdsp_r1dfft_f_LastStage_LOOP,R7)     //[ setup loop0: lc0 = N/4        ]
    R18 = ASR(R18,#1)                     //[ N/2                                 ]
}

    .falign
.qhdsp_r1dfft_f_LastStage_LOOP:
{
    R5:4 = MEMD(R0++#8)                   //[[1]load Z[i]                         ]
    R10 = SFADD(R8,R14)                   //[[2]X + Y                             ]
    R11 = SFADD(R9,R15)                   //[[2]                                  ]
}
{
    R7:6 = MEMD(R1++#-8)                  //[[1]load Z[N/2-i]                     ]
    R14 = SFSUB(R8,R14)                   //[[2]conj(X-Y)                         ]
    R15 = SFSUB(R15,R9)                   //[[2]                                  ]
}
{
    R8 = SFADD(R4,R6)                     //[[1]X= Z[i] + Z*[N/2-i]               ]
    R9 = SFSUB(R5,R7)                     //[[1]                                  ]
    MEMD(R2++#8) = R11:10                 //[[2]save output[i]                    ]
}
{
    R10= SFSUB(R4,R6)                     //[[1]Y=Z[i] - Z*[N/2-i]                ]
    R11= SFADD(R5,R7)                     //[[1]                                  ]
    MEMD(R3++#-8)= R15:14                 //[[2]save output[N/2-i]                ]
}
{
    R8 = SFMPY(R8,R17)                    //[[1]X = 0.5*X                         ]
    R9 = SFMPY(R9,R17)                    //[[1]                                  ]
    R13:12 = MEMD(R19++#8)                //[[1]load W                            ]
}
{
    R14 = SFMPY(R12,R10)                  //[[1]W.r * Y.r                         ]
    R15 = SFMPY(R12,R11)                  //[[1]W.r * Y.i                         ]
}
{
    R14 -= SFMPY(R13,R11)                 //[[1]Y = W*Y = W.r*Y.r-W.i*Y.i         ]
    R15 += SFMPY(R13,R10)                 //[[1]Y = W*Y = W.r*Y.i+W.i*Y.r         ]
}:endloop0

{
    R10 = SFADD(R8,R14)                   //[[e]X + Y                             ]
    R11 = SFADD(R9,R15)                   //[[e]                                  ]
    R0 = ADD(R16,#8)                      //[ R0 =&output[1]                      ]
}
{
    MEMD(R2+#0) = R11:10                  //[[e]save output[N/4]                  ]
    R1 = ADD(R28,#-8)                     //[ R1 =&output[N-1]                    ]
    R13 = ASL(R17,#7)                     //[ 0x80000000                          ]
    P3 = SP2LOOP0(.qhdsp_r1dfft_f_CONJ_LOOP,R18)  //[ setup loop0 with lc0 = N/2   ]
}

    .falign
.qhdsp_r1dfft_f_CONJ_LOOP:
{
    R5:4 = MEMD(R0++#8)                   //[[1]load output[i]                    ]
    R6 = R4                               //[[2]conj(output[i]                    ]
    R7 = XOR(R5,R13)                      //[[2]                                  ]
    IF P3 MEMD(R1++#-8) = R7:6            //[[3]save output[N-i]                  ]
}:endloop0

{
    MEMD(R1+#0) = R7:6                    //[[e]save output[N-i]                  ]
    R17:16 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
}
{
    R19:18 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    DEALLOC_RETURN                        //[ pop stack  and return               ]
}
    .size    qhdsp_r1dfft_f, .-qhdsp_r1dfft_f
