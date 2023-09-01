/**=============================================================================
@file
    qhdsp_fir_f.S

@brief
    FIR filtering on a bulk of data.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

    .file   "qhdsp_fir_f.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_fir_f
    .type    qhdsp_fir_f, @function

/*============================================================================*/

#define L2FETCH_CONFIG    0x0100FF00    // initial L2FETCH config [stride = 256 : width = 255 : height = 0]

/*============================================================================*/

// r0 -> const float *in_samples
// r1 -> float *delay_line
// r2 -> const float *coefs
// r3 -> uint32_t taps
// r4 -> uint32_t length
// r5 -> float_a8_t *out_samples
qhdsp_fir_f:
{
    p0 = !cmp.eq(r0, #0)                       // check NULL pointer - in_samples
    p0 = !cmp.eq(r1, #0)                       // check NULL pointer - delay_line
    p0 = !cmp.eq(r2, #0)                       // check NULL pointer - coefs
    p0 = cmp.gtu(r3, #0)                       // check taps > 0

    if (!p0.new) jump:nt .L_input_error
}
{
    r10 = addasl(r2, r3, #2)                   // coefs[taps]
    r4 = asr(r4, #1)                           // length / 2
    p0 = cmp.gtu(r4, #0)                       // check length > 0
    p0 = !cmp.eq(r5, #0)                       // check NULL pointer - out_samples
    if (!p0.new) jump:nt .L_input_error
}
{
    r10 = add(r10, #-4)                        // coefs[taps-1]
    r28 = r2                                   // save: r28 - coefs
    loop1(.L_qhdsp_fir_f_outer_loop,r4)
}
{
    r12 = #1                                   // minimum lines to prefetch
    r13 = lsr(r4, #5)                          // calculate lines to prefetch for in_samples (length/(256/sizeof(float)))
    r14 = lsr(r3, #6)                          // calculate lines to prefetch for coefs (length/(256/sizeof(float)))
}
{
    r6 = asl(r3, #2)                           // sizeof modulo buffer in bytes = taps * 4 (sizeof(float))
    cs0 = r1                                   // set base address for modulo buffer - write/read
}
{
    r13 = max(r13, r12)                        // avoid zero lines to prefetch
    r14 = max(r14, r12)                        // avoid zero lines to prefetch
    r6.h = #0
}
{
    r13 = add(r13, ##L2FETCH_CONFIG)           // add lines to prefetch to initial L2FETCH config
    r3 = add(r3, #-1)                          // taps - 1
    m0 = r6                                    // set modulo size - write/read
}
{
    r14 = add(r14, ##L2FETCH_CONFIG)           // add lines to prefetch to initial L2FETCH config
    l2fetch(r0, r13)
}
{
    r8 = r5                                    // r8 = out_samples
    l2fetch(r2, r14)
}
{
    r5:4 = #0                                  // neutral element for multiplication for inner loop iteration=0
    r11 = memw(r10)                            // coefs[taps-1]
}
    .falign
.L_qhdsp_fir_f_outer_loop:
{
    r12 = r1                                   // dly_rd1_idx
    r15 = #0                                   // r15 -> sum2 = 0;
    r6 = memw(r0++#4)                          // r6 = in_samples[i]
    memw(r1++#4:circ(m0)) = r6.new             // store in_samples[i] to delay_line buffer
}
{
    r13 = r1                                   // dly_rd2_idx
    r9 = memw(r1)                              // delay_line[dly_wr_idx]
    r7 = memw(r0++#4)                          // r7 = in_samples[i+1]
    loop0(.L_qhdsp_fir_f_inner_loop,r3)
}
{
    r14 = sfmpy(r9, r11)                       // sum1 = coefs[taps-1] * delay_line[dly_wr_idx];
    memw(r1++#4:circ(m0)) = r7                 // store in_samples[i+1] to delay_line buffer
}
    .falign
.L_qhdsp_fir_f_inner_loop:
{
    r5 = memw(r2)                              // coefs[j]
    r14 += sfmpy(r5,r6)                        // sum1 += coefs[j] * delay_line[dly_rd1_idx];
    r6 = memw(r12++#-4:circ(m0))               // read input samples from delay_line buffer
}
{
    r4 = memw(r2++#4)                          // coefs[j]
    r15 += sfmpy(r4,r7)                        // sum2 += coefs[j] * delay_line[dly_rd2_idx];
    r7 = memw(r13++#-4:circ(m0))
}:endloop0
{
    r14 += sfmpy(r5,r6)                        // sum1 += coefs[j] * delay_line[dly_rd1_idx];
    r15 += sfmpy(r5,r7)                        // sum2 += coefs[j] * delay_line[dly_rd2_idx];
    r7 = memw(r13++#-4:circ(m0))
}
{
    r15 += sfmpy(r11,r7)                       // sum2 += coefs[taps-1] * delay_line[dly_rd2_idx];
}
{
    memd(r8++#8) = r15:14                      // out_samples[i] = sum1; out_samples[i+1] = sum2;
    r5:4 = #0                                  // neutral element for multiplication for inner loop iteration=0
    r2 = r28                                   // restore: r28 - coefs
}:endloop1
{
    r0 = #0
    jumpr r31
}
    .falign
.L_input_error:
{
    r0 = #-1
    jumpr r31
}
    .size    qhdsp_fir_f, .-qhdsp_fir_f
