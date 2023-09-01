/**=============================================================================
@file
    qhdsp_iir_f.S

@brief

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

    .file   "qhdsp_iir_f.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_iir_f
    .type    qhdsp_iir_f, @function

/*============================================================================*/

#define L2FETCH_CONFIG    0x0100FF00    // initial L2FETCH config [stride = 256 : width = 255 : height = 0]

/*============================================================================*/

qhdsp_iir_f:
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
    r6 = lsr(r4,#6)                            // calculate lines to prefetch (length/(256/sizeof(float)))
    r7 = #1                                    // minimum lines to prefetch
}
{
    r6 = max(r6,r7)                            // avoid zero lines to prefetch
    memd(r29+#-16) = r17:16
    allocframe(#8)
}
{
    r6 = add(r6,##L2FETCH_CONFIG)              // add lines to prefetch to initial L2FETCH config
    r4 = add(r4,#-1)                           // r4 = length - 1
    loop1(.L_qhdsp_iir_f_outer_loop, r3)
}
{
    p0 = cmp.gtu(r4,#1)
    p3 = cmp.gt(r2,r2)                         // clean p3
    l2fetch(r0,r6)                             // fetch input samples
}
    .falign
.L_qhdsp_iir_f_outer_loop:
{
    r17 = r5                                   // r17 = &out_samples
    r7 = memw(r1+#0)                           // r7 = b0
    r6 = memw(r0++#4)                          // r6 = x[0]
}
{
    if(p3) memd(r2++#8) = r15:14               // save s2:s1
    r8 = memw(r1+#4)                           // r8 = b1
    p3 = cmp.eq(r2,r2)                         // set p3 (for conditional write)
}
{
    r15:14 = memd(r2)                          // r15 = s2[0]; r14 = s1[0]
    r9 = memw(r1+#8)                           // r9 = a1
}
{
    r14 += sfmpy(r7,r6)                        // r14 = s1[0] + b0 * x[0]
    r10 = memw(r1+#12)                         // r10 = b2
    r11 = memw(r1+#16)                         // r11 = a2
}
{
    loop0(.L_qhdsp_iir_f_inner_loop, r4)
    if (!p0) jump:nt .L_skip_inner_loop
}
    .falign
.L_qhdsp_iir_f_inner_loop:
{
    r28 = sfmpy(r9,r14)                        // r28 = a1 * y[n] (a1 * y[0] first time)
    r15 += sfmpy(r8,r6)                        // r15 = s2[n] + b1 * x[n] (s2[0] + b1 * x[0] first time)
}
{
    r12 = sfmpy(r10,r6)                        // r12 = b2 * x[n] (b2 * x[0] first time)
    r14 = sfmpy(r11,r14)                       // r14 = a2 * y[n] (a2 * y[0] first time)
    memw(r17++#4) = r14                        // y[n] = b0 * x[n] + s1[n-1]
    r6 = memw(r0++#4)                          // r6 = x[n+1]
}
{
    r13 = sfmpy(r7,r6)                         // r13 = b0 * x[n+1]
    r16 = sfsub(r15,r28)                       // r16 = s1[n] = s2[n] + b1 * x[n] - a1 * y[n]
}
{
    r15 = sfsub(r12,r14)                       // r15 = s2[n] = b2 * x[n] - a2 * y[n]
    r14 = sfadd(r16,r13)                       // b0 * x[n+1] + s1[n]
}:endloop0                                     // .L_qhdsp_iir_f_inner_loop
    .falign
.L_skip_inner_loop:
{
    r15 += sfmpy(r8,r6)                        // r15 = s2[n] + b1 * x[n]
    r9 = sfmpy(r9,r14)                         // r9 = a1 * y[n]
    r1 = add(r1,#20)                           // move coefs ptr
    r0 = r5                                    // in_samples_ptr = out_samples;
}
{
    r6 = sfmpy(r10,r6)                         // r6 = b2 * x[n]
    r11 = sfmpy(r11,r14)                       // r11 = a2 * y[n]
    memw(r17++#4) = r14                        // y[n] = b0 * x[n] + s1[n-1]
}
{
    r14 = sfsub(r15,r9)                        // r14 = s1[n] = s2[n] + b1 * x[n] - a1 * y[n]
    r15 = sfsub(r6,r11)                        // r15 = s2[n] = b2 * x[n] - a2 * y[n]
}:endloop1                                     // .L_qhdsp_iir_f_outer_loop
{
    memd(r2++#8) = r15:14                      // save s2:s1
    r0 = #0
    r17:16 = memd(r29+#0)
}
{
    dealloc_return
}
    .falign
.L_input_error:
{
    r0 = #-1
    jumpr r31
}
    .size    qhdsp_iir_f, .-qhdsp_iir_f
