/**=============================================================================
@file
   qhdsp_r1difft_h.S

@brief
   Performs real 2^N FFT 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part)
   fixed-point. Scale factor [1/N] absent since scaling was done in FFT function.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

.file   "qhdsp_r1difft_h.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_r1difft_h
    .type    qhdsp_r1difft_h, @function

/*============================================================================*/

#define MAX_BYTES_ON_STACK          2048
#define MAX_COMPLEX_FXPs_ON_STACK   (MAX_BYTES_ON_STACK / (4*2))// 4 = sizeof(int32_t)
                                                                // 4*2 - because input buffer
                                                                // has to be 8 bytes aligned

/*============================================================================*/

/*
 * R0 -> @param[in]       input - input samples in frequency domain
 * R1 -> @param[in]       N - number of samples on which FFT is performed
 * R2 -> @param[in]       Wt1 - twiddle factors - for N/2-point complex FFT
 * R3 -> @param[in]       Wt2 - twiddle factors - for last stage
 * R4 -> @param[out]      output - IFFT output buffer
*/

qhdsp_r1difft_h:
{
    P0 = cmp.gt(R1, ##MAX_COMPLEX_FXPs_ON_STACK)
}
{
    R3 = R4; R4 = R3                    // switch &Wt2 and &output input params
    memd(R29+#-16) = R1:0               //[ save input params
    allocframe(#7*8)                    //[ reserve stack - implicitly allocates 8bytes extra for R31:30
}
{
    memd(R30+#-3*8) = R3:2              //[ save input params
    memd(R30+#-4*8) = R17:16            //[ callee-saved registers              ]
    R7 = asl(R1,#1)                     // N/2*sizeof(CWord2x16) = N/2 * 4 = 2*N
    R5 = P0
}
{
    R17 = R4                            // save &Wt2 to R17
    memw(R30+#-2*8) = R5                // save p0 predicate value
    memd(R30+#-5*8) = R19:18            //[ callee-saved registers              ]
}
{
    R0 = asl(R1,#2)                     // alignment - align to [8 bytes]*N/2 = [sizeof(CWord2x16)*2]*N/2
                                        // N*sizeof(int32_t)
    R1 = R7                             // size
                                        // int32_t alignment = N/2*sizeof(int32_t);
    if(P0) jump:t .L_do_memalign
}
{
    R1 = asl(R0,#1)                     // size = alignment*2 in order to be able to cover alignment constraint
}
{

    R29 = sub(R29, R1)                  // complex_ifft_in_buff = __builtin_alloca(N*sizeof(float complex));
    R0 = add(R0, #-1)                   // int32_t offset = alignment - 1;
}
{
    R19 = add(R29, R0)                  // (int32_t)complex_ifft_in_buff + offset
}
{
    R0 = and(R19,~R0)                   // complex_ifft_in_buff = (int32_t *)(((int32_t)complex_ifft_in_buff + offset) & ~(alignment - 1));
    if(!P0) jump .L_skip_memalign
}
    .falign
.L_do_memalign:
{

    call memalign                       // allocate temp buffer
}
    .falign
.L_skip_memalign:
{
    R4 = R17                            // restore &Wt2 to R4
    R6 = R0                             // save allocated buffer pointer to R6
    R1:0 = memd(R30+#-1*8)              // R0 = &input[0]; R1 = N
    memd(R30+#-6*8) = R21:20            //[ callee-saved registers              ]
}
{
    memd(R30+#-7*8) = R23:22            //[ callee-saved registers              ]
    R7 = addasl(R0, R1, #1)             // R7 = &input[N/2]
    R5 = R6                             // save allocated buffer pointer to R5
}
{
    R1 = asr(R1,#2)                     // set loop count in R1 - N/4
    R18 = #0x7fff0000                   // Y_multiplier = complex(0,0x7fff);
    R10 = memw(R7++#-4)                 // R11 = input[N/2-i-1]        // prefetch inputs, i=0
}
{
    R11 = memw(R7++#-4)                 // R10 = input[N/2-i];        // prefetch inputs, i=0
}
{
    R11:10 = vconj(R11:10):sat          // R10 = conjugate(input[N/2-i]); R11 = conjugate(input[N/2-i-1]); i=0
    R9:8 = memd(R0++#8)                 // R8 = input[i]; R9 = input[i+1]                // prefetch inputs, i=0
    R19 = memw(R4++#4)                  // read Y_multiplier = Wt2[i]; for next iteration
    LOOP0(.L_prepare_qhdsp_c1difft_h_inputs_loop,R1)
}
    .falign
.L_prepare_qhdsp_c1difft_h_inputs_loop:
{
    R13:12 = vaddh(R9:8,R11:10):sat     // R12 = X[i] = cadd(input[i], conjugate(input[N/2-i]));
                                        // R13 = X[i+1] = cadd(input[i+1], conjugate(input[N/2-i-1]));
    R15:14 = vsubh(R9:8,R11:10):sat     // R14 = Y[i] = csub(input[i], conjugate(input[N/2-i]));
                                        // R15 = Y[i+1] = csub(input[i+1], conjugate(input[N/2-i-1]));
    R10 = memw(R7++#-4)                 // R10 = input[N/2-i]
}
{
    R16 = cmpy(R14,R18*):<<1:rnd:sat    // Y[i] = cmult_cr(Y[i],Y_multiplier);
    R17 = cmpy(R15,R19*):<<1:rnd:sat    // Y[i+1] = cmult_cr(Y[i+1],Y_multiplier);
    R18 = memw(R4++#4)                  // read Y_multiplier = Wt2[i]; for next iteration
}
{
    R15:14 = vsubh(R13:12,R17:16):sat   // R14 = csub(X,Y)[i]; R15 = csub(X,Y)[i+1];
    R9:8 = memd(R0++#8)                 // R8 = input[i]; R9 = input[i+1], prefetch inputs, i=0
    R11 = memw(R7++#-4)                 // R11 = input[N/2-i-1]
}
{
    memd(R6++#8) = R15:14
    R11:10 = vconj(R11:10):sat          // R10 = conjugate(input[N/2-i]); R11 = conjugate(input[N/2-i-1])
    R19 = memw(R4++#4)                  // read Y_multiplier = Wt2[i]; for next iteration
}:endloop0

/* --------------------------------------------------------------------------- */
/*                    Call N/2-point FFT                                       */
/* --------------------------------------------------------------------------- */
{
    R0 = R5                             // R1 - already set
    R1 = asl(R1,#1)                     // set loop count in R1 - N/4*2 = N/2
    R3:2 = memd(R30+#-3*8)              //[ restore callee-saved registers              ]
    R17 = R5
}
{
    call qhdsp_c1difft_h
}
{
    R23:22 = memd(R30+#-7*8)            //[ restore callee-saved registers              ]
    R21:20 = memd(R30+#-6*8)            //[ restore callee-saved registers              ]
}
{
    R19:18 = memd(R30+#-5*8)            //[ restore callee-saved registers              ]
    R17:16 = memd(R30+#-4*8)            //[ restore callee-saved registers              ]
    R0 = R17                            // restore allocated buffer pointer to R0 - input param for "free"
}
{
    R5 = memw(R30+#-2*8)                // restore p0 predicate value
}
{
    P0 = R5
}
{
    if(!P0) jump:nt .L_skip_free        // if !(N > MAX_COMPLEX_FXPs_ON_STACK)
}
{
    call free
}
    .falign
.L_skip_free:
{
    dealloc_return                  //[ pop stack and return               ]
}
    .size    qhdsp_r1difft_h, .-qhdsp_r1difft_h
