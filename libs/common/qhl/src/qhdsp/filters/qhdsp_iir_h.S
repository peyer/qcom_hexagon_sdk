/**=============================================================================
@file
    qhdsp_iir_h.S

@brief

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

    .file   "qhdsp_iir_h.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_iir_h
    .type    qhdsp_iir_h, @function

/*============================================================================*/

#define L2FETCH_CONFIG    0x0100FF00    // initial L2FETCH config [stride = 256 : width = 255 : height = 0]

/*============================================================================*/

qhdsp_iir_h:
{
    p0 = !cmp.eq(r0, #0)                       // check NULL pointer - in_samples
    p0 = !cmp.eq(r1, #0)                       // check NULL pointer - coefs
    p0 = !cmp.eq(r2, #0)                       // check NULL pointer - states
    p0 = cmp.gtu(r3, #0)                       // check num_biquads > 0
    if (!p0.new) jump:nt .L_input_error
}
{
    p0 = cmp.gtu(r4, #0)                       // check length > 0
    p0 = !cmp.eq(r5, #0)                       // check NULL pointer - out_samples
    if (!p0.new) jump:nt .L_input_error
}
{
    r6 = lsr(r4, #7)                           // calculate lines to prefetch (length/(256/sizeof(int16_t)))
    r7 = #1                                    // minimum lines to prefetch
    memd(r29+#-2*8) = r17:16
    allocframe(#6*8)
}
{
    r6 = max(r6, r7)                           // avoid zero lines to prefetch
    memd(r29+#4*8) = r19:18
    memd(r29+#3*8) = r21:20
}
{
    r6 = add(r6, ##L2FETCH_CONFIG)             // add lines to prefetch to initial L2FETCH config
    memd(r29+#2*8) = r23:22
    memd(r29+#1*8) = r25:24
}
{
    r4 = asr(r4, #1)                           // loop_counter / 2 -> i+=2
    l2fetch(r0, r6)                            // fetch input samples
}
{
    loop1(.L_qhdsp_iir_h_outer_loop, r4)
    memd(r29+#0*8) = r27:26
}
    .falign
.L_qhdsp_iir_h_outer_loop:
{
    r4 = r1                                    // coef_ptr = coefs;
    r28 = r2                                   // state_ptr = states;
    r7 = memh(r0+#2)                           // r7 = x[i+1]
    r6 = memh(r0++#4)                          // r6 = x[i]
}
{
    r7:6 = vaslw(r7:6, #16)                    // r6 -> x_n = (int32_t)Q6_R_aslh_R(*in_samples++);
                                               // r7 -> x_n_p1 = (int32_t)Q6_R_aslh_R(*in_samples++);
    r23 = memh(r1)                             // b0 = *coef_ptr++;
    r27 = memh(r1+#2)                          // b1 = *coef_ptr++;

    loop0(.L_qhdsp_iir_h_inner_loop, r3)
}
{
    r25 = memh(r1+#4)                          // a1 = *coef_ptr++;
    r26 = memh(r1+#6)                          // b2 = *coef_ptr++;
}
{
    r24 = memh(r1+#8)                          // a2 = *coef_ptr++;
    r9 = memh(r1+#10)                          // shift_amt = *coef_ptr++;
}
    .falign
.L_qhdsp_iir_h_inner_loop:
{
    r15:14 = memd(r28)                         // s1_n = *state_ptr++;      y_n = s1_n;
    r17:16 = combine(#0, r6)                   // x_n
    r19:18 = combine(#0, r23)                  // b0
}
{
    r15:14 += vrmpyweh(r17:16, r19:18):<<1     // y_n = Q6_P_vrmpywehacc_PP_s1(y_n, x_n, (int32_t) b0);
    r19:18 = combine(#0, r27)                  // b1
}
{
    r13:12 = memd(r28+#8)                      // s2_n = *state_ptr--;
    r15:14 = asr(r15:14, #16)                  // y_n = Q6_P_extract_PII(y_n, 32, 16);
    r11:10 = vrmpyweh(r17:16, r19:18)          // s1_n = Q6_P_vrmpyweh_PP(x_n, (int32_t) b1);
    r19:18 = combine(#0, r25)                  // a1
}
{
    r15 = #0
    r4 = add(r4, #6*2)
}
{
    r11:10 += vrmpyweh(r15:14, r19:18):<<1     // r11:10 = s1_n = Q6_P_vrmpywehacc_PP_s1(s1_n, y_n, (int32_t) a1);    // b1 * x[n] + (-a1 * y[n])<<1
    r19:18 = combine(#0, r23)                  // b0
}
{
    r13:12 += asl(r11:10, #1)                  // s1_n = Q6_P_asl_PI(s1_n, 1);      s1_n += s2_n;       y_n_p1 = s1_n;
    r11:10 = combine(#0, r7)                   // x_n_p1
    r15 = lc0                                  // check when last loop iteration
}
{
    r13:12 += vrmpyweh(r11:10, r19:18):<<1     // y_n_p1 = Q6_P_vrmpywehacc_PP_s1(y_n_p1, x_n_p1, (int32_t) b0);
                                               // r7:6 = a_tmp = Q6_P_combine_RR(x_n_p1, x_n);
                                               // r27:26 = b_tmp = Q6_P_combine_RR((int32_t) b1, (int32_t) b2);
    p3 = !cmp.eq(r15, #1)                      // if last loop iteration, do not read next coeffs (reading out of buffer bounds)
}
{
    r13:12 = asr(r13:12, #16)                  // y_n_p1 = Q6_P_extract_PII(y_n_p1, 32, 16);
    r11:10 = vrmpyweh(r7:6, r27:26)            // s1_n = Q6_P_vrmpyweh_PP(a_tmp, b_tmp);
    r19:18 = combine(#0, r7)                   // x_n_p1
    r27 = #0
}
{
    r27:26 = combine(r12, r14)                 // r27:26 = c_tmp = Q6_P_combine_RR(y_n_p1, y_n);
                                               // r25:24 = d_tmp = Q6_P_combine_RR((int32_t) a1, (int32_t) a2);
    r21:20 = vrmpyweh(r19:18, r27:26)          // s2_n = Q6_P_vrmpyweh_PP(x_n_p1, (int32_t) b2);
    r19:18 = combine(#0, r24)                  // a2
    r13 = #0
}
{
    r11:10 += vrmpyweh(r27:26, r25:24):<<1     // s1_n = Q6_P_vrmpywehacc_PP_s1(s1_n, c_tmp, d_tmp);
    r21:20 += vrmpyweh(r13:12, r19:18):<<1     // s2_n = Q6_P_vrmpywehacc_PP_s1(s2_n, y_n_p1, (int32_t) a2);
    if(p3) r23 = memh(r4)                      // b0 = *coef_ptr++;
    if(p3) r27 = memh(r4+#2)                   // b1 = *coef_ptr++;
}
{
    r6 = asl(r14, r9)                          // r6 -> x_n = (int32_t)Q6_P_asl_PR(y_n, shift_amt);
    r7 = asl(r12, r9)                          // r7 -> x_n_p1 = (int32_t)Q6_P_asl_PR(y_n_p1, shift_amt);
    if(p3) r25 = memh(r4+#4)                   // a1 = *coef_ptr++;
    if(p3) r26 = memh(r4+#6)                   // b2 = *coef_ptr++;
}
{
    r11:10 = asl(r11:10, #1)                   // s1_n = Q6_P_asl_PI(s1_n, 1);
    r21:20 = asl(r21:20, #1)                   // s2_n = Q6_P_asl_PI(s2_n, 1);
    if(p3) r24 = memh(r4+#8)                   // a2 = *coef_ptr++;
    if(p3) r9 = memh(r4+#10)                   // shift_amt = *coef_ptr++;
}
{
    memd(r28+#8) = r21:20                      // *state_ptr++ = s2_n;
    memd(r28++#16) = r11:10                    // *state_ptr++ = s1_n;
}:endloop0                                     // .L_qhdsp_iir_h_inner_loop
{
    r6 = add(r6, #0x8000)                      // r6 -> x_n
    r7 = add(r7, #0x8000)                      // r7 -> x_n_p1
}
{
    memh(r5++#4) = r6.h                        // *out_samples++ = (int16_t)Q6_R_asrh_R(x_n + 0x8000);
    memh(r5+#2) = r7.h                         // *out_samples++ = (int16_t)Q6_R_asrh_R(x_n_p1 + 0x8000);
}:endloop1                                     // .L_qhdsp_iir_h_outer_loop
{
    r0 = #0
    r17:16 = memd(r29+#5*8)
    r19:18 = memd(r29+#4*8)
}
{
    r21:20 = memd(r29+#3*8)
    r23:22 = memd(r29+#2*8)
}
{
    r25:24 = memd(r29+#1*8)
    r27:26 = memd(r29+#0*8)
}
{
    r31:30 = dealloc_return(r30):raw
}
.L_input_error:
{
    r0 = #-1
    jumpr r31
}
    .size    qhdsp_iir_h, .-qhdsp_iir_h
