/**=============================================================================
@file
   qhdsp_r1difft_f.S

@brief
   Performs real 2^N IFFT for single precision float point.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

.file   "qhdsp_r1difft_f.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_r1difft_f
    .type    qhdsp_r1difft_f, @function

/*============================================================================*/

#define MAX_BYTES_ON_STACK          2048
#define MAX_COMPLEX_FLOATS_ON_STACK (MAX_BYTES_ON_STACK / 8) // 8 = sizeof(float complex)

/*============================================================================*/
    
/*
 * R0 -> @param[in]       input - input samples in frequency domain
 * R1 -> @param[in]       N - number of samples on which FFT is performed
 * R2 -> @param[in]       Wt1 - twiddle factors - for N/2-point complex FFT
 * R3 -> @param[in]       Wt2 - twiddle factors - for last stage
 * R4 -> @param[out]      output - IFFT output buffer
*/

qhdsp_r1difft_f:
{
    P0 = CMP.GT(R1, ##MAX_COMPLEX_FLOATS_ON_STACK)
}
{
    R3 = R4; R4 = R3                // switch &Wt2 and &output input params
    MEMD(R29+#-16) = R1:0           // save input params
    ALLOCFRAME(#7*8)                // reserve stack - implicitly allocates 8bytes extra for R31:30
}
{
    MEMD(R30+#-3*8) = R3:2          // save input params
    MEMD(R30+#-4*8) = R17:16        // callee-saved registers
    R7 = ASL(R1,#2)                 // N/2*sizeof(float complex) = N/2 * 8 = 4*N
    R5 = P0
}
{
    R17 = R4                        // save &Wt2 to R17
    MEMW(R30+#-2*8) = R5            // save p0 predicate value
    MEMD(R30+#-5*8) = R19:18        // callee-saved registers
}
{
    R16 = ASL(R1, #3)               // N*sizeof(float complex)
    R18 = ASL(R1, #2)               // int32_t alignment = N/2*sizeof(float complex);
    IF(P0) JUMP:T .L_do_memalign
}
{   
    R29 = SUB(R29, R16)             // c1difft_f_input_ptr = __builtin_alloca(N*sizeof(float complex));
    R18 = ADD(R18, #-1)             // int32_t offset = alignment - 1;
}
{
    R19 = ADD(R29, R18)             // (int32_t)c1difft_f_input_ptr + offset
}
{
    R0 = AND(R19,~R18)              // c1difft_f_input_ptr = (float complex *)(((int32_t)c1difft_f_input_ptr + offset) & ~(alignment - 1));
    IF(!P0) JUMP .L_skip_memalign
}
    .falign
.L_do_memalign:
{
    R1:0 = COMBINE(R7, R7)          // r1 -> size; r0 -> alignment
    call memalign                   // allocate temp buffer
}
    .falign
.L_skip_memalign:
{
    R4 = R17                        // restore &Wt2 to R4
    R6 = R0                         // save allocated buffer pointer to R6
    R1:0 = MEMD(R30+#-1*8)          // R0 = &input[0]; R1 = N
    MEMD(R30+#-6*8) = R21:20        // callee-saved registers
}
{
    MEMD(R30+#-7*8) = R23:22        // callee-saved registers
    R7 = ADDASL(R0, R1, #2)         // R7 = &input[N/2]
    R28 = #0x80000000               // mask for XOR operation - changing sign
}
{
    R5 = R6                         // save allocated buffer pointer to R5
    R21 = SFMAKE(#320):NEG          //#0x3f000000            // 0.5 - (0 + 0.5*1i).im - :neg because of conjugation
    R18 = SFMAKE(#320):POS          //#0x3f000000            // 0.5
}
{
    R1 = ASR(R1,#1)                 // set loop count in R1 - N/2
    R20 = #0                        // (0 + 0.5*1i).re
}
{
    R9:8 = MEMD(R0++#8)             // R8 = input[0].re; R9 = input[0].im            // prefetch inputs
    R11:10 = MEMD(R7++#-8)          // R10 = input[N/2].re; R11 = input[N/2].im      // prefetch inputs
    P3 = SP1LOOP0(.L_prepare_qhdsp_c1difft_f_inputs_loop,R1)
}
    .falign
.L_prepare_qhdsp_c1difft_f_inputs_loop:
// do not touch R18 register - it has 0.5 constant
{
    R11 = XOR(R11,R28)              // R11 = togglebit(R11,#31)    // conjugate(input[N/2]) - change sign of imag part
    R12 = SFADD(R8,R10)             // X.re = input[0].re + conjugate(input[N/2]).re;
    R13 = SFSUB(R8,R10)             // Y.re = input[0].re - conjugate(input[N/2]).re;
    IF P3 MEMD(R6++#8) = R17:16     // save outputs from previous iteration
}
{
    R14 = SFADD(R9,R11)             // X.im = input[0].im + conjugate(input[N/2]).im;
    R15 = SFSUB(R9,R11)             // Y.im = input[0].im - conjugate(input[N/2]).im;
    R21 = XOR(R21,R28)              // conjugate(Wt2[i-1]) - change sign of imag part - for next iteration
}
{
    R16 = SFMPY(R12,R18)            // X'.re = X.re * 0.5
    R22 = SFMPY(R13,R20)            // Y.re * conjugate(Wt2[i-1]).re;        (in first iteration (0 + 0.5*1i).re )
}
{
    R22 -= SFMPY(R15,R21)           // Y'.re = Y.re * conjugate(Wt2[i-1]).re - Y.im * conjugate(Wt2[i-1]).im;
                                    // (in first iteration (0 + 0.5*1i).re )
    R17 = SFMPY(R15,R20)            // Y.im * conjugate(Wt2[i-1]).re;
}
{
    R17 += SFMPY(R13,R21)           // Y'.im = Y.im * conjugate(Wt2[i-1]).re + Y.re * conjugate(Wt2[i-1]).im;
    R21:20 = MEMD(R4++#8)           // read Wt2[i-1] for next iteration;     R20=Wt2[i-1].re, R21=Wt2[i-1].im
}
{
    R16 = SFADD(R16,R22)            // c1difft_f_input_ptr[i].re = X'.re + Y'.re
    R17 += SFMPY(R14,R18)           // R17 = Y'im + X'.im -> (X'.im = X.im * 0.5)
    R9:8 = MEMD(R0++#8)             // R8 = input[i].re; R9 = input[i].im            // prefetch inputs
    R11:10 = MEMD(R7++#-8)          // R10 = input[N/2-i].re; R11 = input[N/2-i].im  // prefetch inputs
}:endloop0

/* --------------------------------------------------------------------------- */
/*                    Call N/2-point FFT                                       */
/* --------------------------------------------------------------------------- */
{
    MEMD(R6++#8) = R17:16           // save output from last iteration
    R0 = R5                         // R1 - already set
    R3:2 = MEMD(R30+#-3*8)
    R17 = R5
}
{
    call qhdsp_c1difft_f
}
{
    R23:22 = MEMD(R30+#-7*8)        // restore callee-saved registers
    R21:20 = MEMD(R30+#-6*8)        // restore callee-saved registers
}
{
    R19:18 = MEMD(R30+#-5*8)        // restore callee-saved registers
    R17:16 = MEMD(R30+#-4*8)        // restore callee-saved registers
    R0 = R17                        // restore allocated buffer pointer to R0 - input param for "free"
}
{
    R5 = MEMW(R30+#-2*8)            // restore p0 predicate value
}
{
    P0 = R5
}
{
    IF(!P0) JUMP:NT .L_skip_free    // if !(N > MAX_COMPLEX_FLOATS_ON_STACK)
}
{
    call free
}
    .falign
.L_skip_free:
{
    dealloc_return                  // pop stack and return
}
    .size    qhdsp_r1difft_f, .-qhdsp_r1difft_f
