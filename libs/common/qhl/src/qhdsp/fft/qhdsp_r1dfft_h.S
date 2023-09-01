/**=============================================================================
@file
   qhdsp_r1dfft_h.S

@brief
   Performs real 2^N FFT 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part)
   fixed-point with a scaling factor of 1/N.

   [=============================================================================]
   [  Assumptions:                                                               ]
   [           - complex number: interleaved real/image with real part           ]
   [             at low memory address                                           ]
   [           - input  is aligned by size of the array                          ]
   [           - output is aligned by 8bytes                                     ]
   [           - N >  8                                                          ]
   [=============================================================================]
   [  Inputs     : R0 : short complex *input                                     ]
   [               R1 : int32_t  N                                               ]
   [               R2 : int32_t *Wt1 - twiddle factors for N/2-point complex FFT ]
   [               R3 : int32_t *Wt2 - twiddle factors for last stage            ]
   [               R4 : short complex *output                                    ]
   [=============================================================================]
   [  Register Usage: R0-R17, R28                                                ]
   [  Hardware Loops affected: Loop0                                             ]
   [                                                                             ]
   [  Stack Memory Frame Allocated (in Bytes): 16+8                              ]
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

    .file   "qhdsp_r1dfft_h.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_r1dfft_h
    .type    qhdsp_r1dfft_h, @function
qhdsp_r1dfft_h:
{
    P0 = CMP.GT(R1,#8)                    //[ if !(N>8)                           ]
    IF !P0.new JUMPR:nt R31               //[ then return                         ]
}
{
    ALLOCFRAME(#3*8)                      //[ reserve stack                       ]
    MEMD(R29+#-16) = R17:16               //[ callee-saved registers              ]
}
{
    MEMD(R29+#8) = R21:20                 //[ callee-saved registers              ]
    MEMD(R29+#0) = R19:18                 //[ callee-saved registers              ]
    R19:18 = COMBINE(R3,R1)               //[ R19 = wt2,  R18 = N                 ]
    R3 = R4                               //[ func call: R3 = output              ]
}
/* --------------------------------------------------------------------------- */
/*                    Call N/2-point FFT                                       */
/* --------------------------------------------------------------------------- */
{
    R21 = R4                              //[ R21 = output                        ]
    R1 = LSR(R1,#1)                       //[ func call: R1 = N/2                 ]
    R16 = R4                              //[ R16 = output                        ]
    CALL qhdsp_c1dfft_h
}
    .falign
.qhdsp_r1dfft_h_LastStage:
/* --------------------------------------------------------------------------- */
/*                    Calcuate last stage butterflies                          */
/* --------------------------------------------------------------------------- */
{
    R0 = ADD(R16,#16)                     //[ set R0 = &Z[4]                      ]
    R1 = ADDASL(R16,R18,#1)               //[ &Z[N/2]                             ]
    R7:6 = MEMD(R16+#0)                   //[ load Z[0],Z[1]                      ]
    R5:4 = MEMD(R16+#8)                   //[[p]load Z[2*i+2],Z[2*i+3]            ]
}
{   
    R14 = LSR(R6,#16)                     //[ complex(imag(Z[0]),0)               ]
    R12 = ZXTH(R6)                        //[ complex(real(Z[0]),0)               ]
    R1 = ADD(R1,#-8)                      //[ set R1 = &Z[N/2-2]                  ]
    R28 = R19                             //[ R28= &Wt2                           ]
}
{
    R20 = R18                             // N
    R10 = ASR(R18,#3)                     //[ N/8
    R17 = ASR(R18,#1)                     //[ N/2 - for last loop
    R19:18 = MEMD(R29+#0)                 //[ restore callee-saved registers      ]
}
{
    R15:14=  VAVGH(R13:12,R15:14):crnd    //[ output[0]                           ]
    R13:12= VNAVGH(R13:12,R15:14):crnd:sat//[ output[N/2]                         ]
    R9:8 = MEMD(R1++#-8)                  //[[p]load Z[N/2-(2*i+1)]:Z[N/2-(2*i+2)]]
    R6 = R4                               //[[p]R7:6 = Z[2*i+1] : Z[2*i+2]        ]
}
{
    MEMW(R1+#16) = R12                    //[ save output[N/2]                    ]
    R7:6 =  VAVGH(R7:6,R9:8):crnd         //[[p](Z[k]+Z[N/2-k])/2                 ]
    R9:8 = VNAVGH(R7:6,R9:8):crnd:sat     //[[p](Z[k]-Z[N/2-k])/2                 ]
    R3:2 = MEMD(R28++#8)                  //[[p]load Wt2[k]                       ]
}
{
    R16 = R0
    LOOP0(.qhdsp_r1dfft_h_LastStage_LOOP,R10)   //[ setup loop0: lc0 = N/8         ]
    R10 = COMBINE(R6.H,R8.L)              //[[p]Y = (Z[k]-Z*[N/2-k])/2            ]
    R11 = COMBINE(R7.H,R9.L)              //[[p]Y = (Z[k]-Z*[N/2-k])/2            ]
}

    .falign
.qhdsp_r1dfft_h_LastStage_LOOP:
{
    R10 = CMPY(R10,R3):<<1:rnd:sat        //[[2]Wt2[k]*Y                          ]
    R11 = CMPY(R11,R2):<<1:rnd:sat        //[[2]Wt2[k]*Y                          ]
    R12 = COMBINE(R8.H,R6.L)              //[[2]X = (Z[k]+Z*[N/2-k])/2            ]
    R13 = COMBINE(R9.H,R7.L)              //[[2]X = (Z[k]+Z*[N/2-k])/2            ]
}
{
    R5:4 = MEMD(R0++#8)                   //[[1]load Z[2*i+2],Z[2*i+3]            ]
    R7:6 = COMBINE(R5,R14)                //[[2]save previous output[k] in R6     ]
    R15:14= VNAVGH(R13:12,R11:10):crnd:sat//[[2]output[k]                         ]
    R11:10=  VAVGH(R13:12,R11:10):crnd    //[[2]for output[N/2-k]                 ]
}
{
    R9:8 = MEMD(R1++#-8)                  //[[1]load Z[N/2-(2*i+1)]:Z[N/2-(2*i+2)]]
    R6 = R4                               //[[1]R7:6 = Z[2*i+1] : Z[2*i+2]        ]
    R13:12 = COMBINE(R15,R6)              //[[2]align output[k] for save          ]
    R11:10 = VCONJ(R11:10):sat            //[[2] output[N/2-k]                    ]
}
{
    R7:6 = VAVGH(R7:6,R9:8):crnd          //[[1](Z[k]+Z[N/2-k])/2                 ]
    R9:8 = VNAVGH(R7:6,R9:8):crnd:sat     //[[1](Z[k]-Z[N/2-k])/2                 ]
    R3:2 = MEMD(R28++#8)                  //[[1]load Wt2[k]                       ]
    MEMD(R1+#16) = R11:10                 //[[2]save                              ]
}
{
    R10 = COMBINE(R6.H,R8.L)              //[[1]Y = (Z[k]-Z*[N/2-k])/2            ]
    R11 = COMBINE(R7.H,R9.L)              //[[1]Y = (Z[k]-Z*[N/2-k])/2            ]
    MEMD(R0+#-24) = R13:12                //[[2]save                              ]
}:endloop0
/*
    for (i = 1; i < N / 2; i++)
    {
        output[N - i] = conjugate(output[i]);
    }
 */
{
    R21 = ADDASL(R21,R20,#2)              // &output[N]
    R16 = ADD(R21,#4)                     // &output[i], i=1
    R17 = ADD(R17,#-1)                    // N/4 - 1
}
{
    R21 = ADD(R21,#-4)                    // &output[N-i], i=1
    P3 = SP2LOOP0(.qhdsp_r1dfft_h_conj_LOOP,R17) //[ setup loop0: lc0 = N/4-1      ]
}
    .falign
.qhdsp_r1dfft_h_conj_LOOP:
{
    R2 = MEMW(R16++#4)
    R5:4 = VCONJ(R3:2):SAT                // R3&R5 - don't care registers
    IF P3 MEMW(R21++#-4) = R4
}:endloop0
{
    R5:4 = VCONJ(R3:2):SAT                // R3&R5 - don't care registers; flushing out pipeline
    MEMW(R21++#-4) = R4                   // flushing out pipeline
}
{
    MEMW(R21++#-4) = R4                   // flushing out pipeline
    R17:16 = MEMD(R29+#16)                //[ restore callee-saved registers      ]
}
{
    R21:20 = MEMD(R29+#8)                 //[ restore callee-saved registers      ]
    DEALLOCFRAME                          //[ pop stack                           ]
}
{
    JUMPR R31                             //[ return                              ]
}

    .size    qhdsp_r1dfft_h, .-qhdsp_r1dfft_h

