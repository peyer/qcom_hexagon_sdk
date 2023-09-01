/**=============================================================================
@file
    qhblas_matrix_inverse4x4_ah.S

@brief
    Calculates matrix inversion for 4x4 matrix of 16-bit fixed-point values.

    Function prototype

        int32_t __qhblas_matrix_inverse4x4_ah(int16_t *input_matrix, int32_t *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse4x4_ah(int16_t *input_matrix, int32_t *output_matrix)
        {
            int32_t det =   A[0]*A[5]*A[10]*A[15] + A[0]*A[6]*A[11]*A[13] + A[0]*A[7]*A[9]*A[14]   \
                            + A[1]*A[4]*A[11]*A[14] + A[1]*A[6]*A[8]*A[15] + A[1]*A[7]*A[10]*A[12] \
                            + A[2]*A[4]*A[9]*A[15] + A[2]*A[5]*A[11]*A[12] + A[2]*A[7]*A[8]*A[13]  \
                            + A[3]*A[4]*A[10]*A[13] + A[3]*A[5]*A[8]*A[14] + A[3]*A[6]*A[9]*A[12]  \
                            - A[0]*A[5]*A[11]*A[14] - A[0]*A[6]*A[9]*A[15] - A[0]*A[7]*A[10]*A[13] \
                            - A[1]*A[4]*A[10]*A[15] - A[1]*A[6]*A[11]*A[12] - A[1]*A[7]*A[8]*A[14] \
                            - A[2]*A[4]*A[11]*A[13] - A[2]*A[5]*A[8]*A[15] - A[2]*A[7]*A[9]*A[12]  \
                            - A[3]*A[4]*A[9]*A[14] - A[3]*A[5]*A[10]*A[12] - A[3]*A[6]*A[8]*A[13];

            if (det == 0)
                return -1;

            det = 1/det;

            B[0] =  det * (A[6]*A[11]*A[13] - A[7]*A[10]*A[13] + A[7]*A[9]*A[14] - A[5]*A[11]*A[14] - A[6]*A[9]*A[15] + A[5]*A[10]*A[15]);
            B[1] =  det * (A[3]*A[10]*A[13] - A[2]*A[11]*A[13] - A[3]*A[9]*A[14] + A[1]*A[11]*A[14] + A[2]*A[9]*A[15] - A[1]*A[10]*A[15]);
            B[2] =  det * (A[2]*A[7]*A[13] - A[3]*A[6]*A[13] + A[3]*A[5]*A[14] - A[1]*A[7]*A[14] - A[2]*A[5]*A[15] + A[1]*A[6]*A[15]);
            B[3] =  det * (A[3]*A[6]*A[9] - A[2]*A[7]*A[9] - A[3]*A[5]*A[10] + A[1]*A[7]*A[10] + A[2]*A[5]*A[11] - A[1]*A[6]*A[11]);
            B[4] =  det * (A[7]*A[10]*A[12] - A[6]*A[11]*A[12] - A[7]*A[8]*A[14] + A[4]*A[11]*A[14] + A[6]*A[8]*A[15] - A[4]*A[10]*A[15]);
            B[5] =  det * (A[2]*A[11]*A[12] - A[3]*A[10]*A[12] + A[3]*A[8]*A[14] - A[0]*A[11]*A[14] - A[2]*A[8]*A[15] + A[0]*A[10]*A[15]);
            B[6] =  det * (A[3]*A[6]*A[12] - A[2]*A[7]*A[12] - A[3]*A[4]*A[14] + A[0]*A[7]*A[14] + A[2]*A[4]*A[15] - A[0]*A[6]*A[15]);
            B[7] =  det * (A[2]*A[7]*A[8] - A[3]*A[6]*A[8] + A[3]*A[4]*A[10] - A[0]*A[7]*A[10] - A[2]*A[4]*A[11] + A[0]*A[6]*A[11]);
            B[8] =  det * (A[5]*A[11]*A[12] - A[7]*A[9]*A[12] + A[7]*A[8]*A[13] - A[4]*A[11]*A[13] - A[5]*A[8]*A[15] + A[4]*A[9]*A[15]);
            B[9] =  det * (A[3]*A[9]*A[12] - A[1]*A[11]*A[12] - A[3]*A[8]*A[13] + A[0]*A[11]*A[13] + A[1]*A[8]*A[15] - A[0]*A[9]*A[15]);
            B[10] = det *  (A[1]*A[7]*A[12] - A[3]*A[5]*A[12] + A[3]*A[4]*A[13] - A[0]*A[7]*A[13] - A[1]*A[4]*A[15] + A[0]*A[5]*A[15]);
            B[11] = det *  (A[3]*A[5]*A[8] - A[1]*A[7]*A[8] - A[3]*A[4]*A[9] + A[0]*A[7]*A[9] + A[1]*A[4]*A[11] - A[0]*A[5]*A[11]);
            B[12] = det *  (A[6]*A[9]*A[12] - A[5]*A[10]*A[12] - A[6]*A[8]*A[13] + A[4]*A[10]*A[13] + A[5]*A[8]*A[14] - A[4]*A[9]*A[14]);
            B[13] = det *  (A[1]*A[10]*A[12] - A[2]*A[9]*A[12] + A[2]*A[8]*A[13] - A[0]*A[10]*A[13] - A[1]*A[8]*A[14] + A[0]*A[9]*A[14]);
            B[14] = det *  (A[2]*A[5]*A[12] - A[1]*A[6]*A[12] - A[2]*A[4]*A[13] + A[0]*A[6]*A[13] + A[1]*A[4]*A[14] - A[0]*A[5]*A[14]);
            B[15] = det *  (A[1]*A[6]*A[8] - A[2]*A[5]*A[8] + A[2]*A[4]*A[9] - A[0]*A[6]*A[9] - A[1]*A[4]*A[10] + A[0]*A[5]*A[10]);

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .hidden	__qhblas_matrix_inverse4x4_ah
    .global __qhblas_matrix_inverse4x4_ah
    .type __qhblas_matrix_inverse4x4_ah, @function

/*============================================================================*/

__qhblas_matrix_inverse4x4_ah:
{
    allocframe(#216)
}
{
    memd(r29+#192) = r17:16
}
{
    r2 = memh(r0+#0)
    memw(r29+#116) = r2.new
}
{
    r5 = memh(r0+#10)
    memw(r29+#100) = r5.new
}
{
    r14 = memh(r0+#12)
    memw(r29+#32) = r14.new
}
{
    r28 = mpyi(r5,r2)
    r25 = memh(r0+#20)
    memd(r29+#160) = r25:24
}
{
    r4 = mpyi(r14,r2)
    r22 = memh(r0+#22)
    memd(r29+#168) = r23:22
}
{
    r6 = mpyi(r28,r25)
    r11 = memh(r0+#14)
    memw(r29+#24) = r11.new
}
{
    r26 = memh(r0+#18)
    memd(r29+#152) = r27:26
}
{
    r2 = mpyi(r11,r2)
    memw(r29+#60) = r2.new
}
{
    r6 = mpyi(r4,r26)
    memd(r29+#184) = r19:18
    memw(r29+#148) = r6
}
{
    r7 = mpyi(r2,r25)
    r18 = memh(r0+#30)
    memw(r29+#56) = r6
}
{
    r4 = mpyi(r4,r22)
    r3 = memh(r0+#2)
    memw(r29+#136) = r4
}
{
    r13 = mpyi(r2,r26)
    r2 = mpyi(r6,r18)
    r6 = memh(r0+#4)
    memw(r29+#112) = r3
}
{
    memw(r29+#132) = r6
    memd(r29+#176) = r21:20
}
{
    r23 = mpyi(r3,r11)
    r17 = mpyi(r3,r14)
    r20 = memh(r0+#26)
    r8 = memh(r0+#6)
}
{
    r16 = mpyi(r6,r11)
    r12 = mpyi(r6,r5)
    memw(r29+#88) = r8
    memw(r29+#128) = r4
}
{
    r2 += mpyi(r7,r20)
    r14 = mpyi(r8,r14)
    r4 = memh(r0+#8)
    memw(r29+#48) = r7
}
{
    r27 = mpyi(r12,r22)
    memw(r29+#108) = r4
}
{
    r7 = mpyi(r8,r4)
    r15 = mpyi(r6,r4)
    r21 = memh(r0+#28)
    memw(r29+#104) = r7.new
}
{
    r19 = mpyi(r4,r3)
    r4 = mpyi(r16,r20)
    r9 = memh(r0+#16)
    memw(r29+#76) = r19.new
}
{
    r4 = mpyi(r7,r25)
    r6 = mpyi(r8,r5)
    memw(r29+#124) = r4
}
{
    r4 = mpyi(r28,r22)
    r8 = mpyi(r23,r21)
    memw(r29+#140) = r4
}
{
    r3 = mpyi(r17,r18)
    r11 = mpyi(r15,r26)
    r10 = memh(r0+#24)
    memw(r29+#16) = r4
}
{
    r17 = mpyi(r17,r22)
    r2 += mpyi(r4,r21)
    memw(r29+#144) = r3
    memw(r29+#92) = r17
}
{
    r3 = mpyi(r19,r22)
    r2 += mpyi(r8,r9)
    memw(r29+#40) = r3.new
}
{
    r19 = mpyi(r19,r25)
    r2 += mpyi(r17,r10)
    memw(r29+#84) = r23
    memw(r29+#64) = r15
}
{
    r5 = mpyi(r10,r26)
    r4 = mpyi(r9,r18)
    memw(r29+#80) = r28
    memw(r29+#44) = r13
}
{
    r0 = mpyi(r23,r25)
    r2 += mpyi(r19,r18)
    memw(r29+#12) = r17
    memw(r29+#96) = r11
}
{
    r2 += mpyi(r5,r16)
    r28 = mpyi(r7,r26)
    memw(r29+#36) = r0
    memw(r29+#120) = r6
}
{
    r23 = mpyi(r15,r22)
    r15 = add(r3,r13)
    r13 = r12
    r3 = memw(r29+#148)
}
{
    r24 = mpyi(r6,r21)
    r2 += mpyi(r4,r12)
    r7 = add(r11,r3)
    memw(r29+#20) = r24.new
}
{
    r4 = mpyi(r7,r18)
    r5 = add(r27,r0)
    r12 = memw(r29+#124)
    r3 = memw(r29+#144)
}
{
    r17 = mpyi(r6,r25)
    r4 += mpyi(r15,r21)
    r15 = memw(r29+#140)
    memw(r29+#28) = r27
}
{
    r24 += add(r12,r3)
    r2 += mpyi(r23,r20)
    r3 = memw(r29+#128)
    memw(r29+#68) = r8
}
{
    r6 = mpyi(r9,r20)
    r27 = r23
    r23 = r3
    r3 = add(r15,r3)
}
{
    r5 += mpyi(r14,r26)
    r4 += mpyi(r3,r20)
    memw(r29+#72) = r16
    memw(r29+#124) = r14
}
{
    r2 += mpyi(r17,r10)
    memw(r29+#52) = r10
    memw(r29+#8) = r9
}
{
    r2 += mpyi(r6,r14)
    r4 += mpyi(r5,r10)
}
{
    r4 += mpyi(r24,r9)
    r2 += mpyi(r28,r21)
    r0 = #-1
}
{
    r2 = sub(r4,r2)
    if (cmp.eq(r2.new,#0)) jump:nt ##.L_SINGULAR
    r3 = #0
}
{
    p0 = cmp.gt(r2,r3)
    if (!p0.new) r10 = #1
    if (p0.new) r10 = #0
    if (!p0.new) r2 = sub(r3,r2)
}
{
    memw(r29+#200) = r10
    r16 = r1
    memw(r29+#0) = r19
    r0 = asr(r2,#15)
}
{
    call qhmath_inv_h
    r24 = r28
    r19 = r13
    memw(r29+#4) = r13
}
{
    r10 = memw(r29+#200)
    r4 = #0
    r1 = add(r1,#-15)
}
{
    r0 = asr(r0,r1)
}
{
    p0 = cmp.eq(r10,#1)
    if(p0.new) r0 = sub(r4,r0)
}
{
    r2 = memw(r29+#144)
    r8 = memw(r29+#68)
}
{
    r8 += mpyi(r19,r18)
    r3 = memw(r29+#20)
    r6 = memw(r29+#28)
}
{
    r1 = add(r3,r2)
    r3 = memw(r29+#12)
    r2 = memw(r29+#36)
}
{
    r2 = sub(r2,r3)
    r3 = memw(r29+#48)
    r4 = memw(r29+#44)
}
{
    r3 = sub(r23,r3)
    r2 = add(r2,r6)
    r5 = memw(r29+#16)
    r6 = memw(r29+#56)
}
{
    r10 = sub(r2,r17)
    r3 = sub(r3,r27)
    r5 = sub(r4,r5)
    r2 = memw(r29+#140)
}
{
    r2 = add(r3,r2)
    r4 = memw(r29+#148)
    memw(r29+#48) = r2.new
}
{
    r6 = sub(r4,r6)
    r2 = memw(r29+#40)
    r3 = memw(r29+#0)
}
{
    r2 = add(r5,r2)
    r3 = sub(r6,r3)
    r4 = memw(r29+#32)
    memw(r29+#148) = r3.new
}
{
    r2 = sub(r2,r24)
    r17 = memw(r29+#8)
    memw(r29+#144) = r2.new
}
{
    r5 = mpyi(r22,r4)
    r2 = memw(r29+#24)
    r15 = memw(r29+#100)
}
{
    r9 = mpyi(r17,r4)
    r23 = memw(r29+#52)
    memw(r29+#36) = r9.new
}
{
    r6 = mpyi(r2,r25)
    r3 = mpyi(r26,r2)
    r7 = memw(r29+#88)
    memw(r29+#44) = r3.new
}
{
    r13 = mpyi(r17,r2)
    r2 = mpyi(r26,r4)
    r28 = sub(r5,r6)
    memw(r29+#28) = r13.new
}
{
    r12 = mpyi(r22,r15)
    r5 = sub(r6,r5)
    memw(r29+#40) = r12.new
}
{
    r11 = mpyi(r5,r23)
    r5 = mpyi(r3,r21)
    memw(r29+#128) = r2
}
{
    r3 = mpyi(r25,r15)
    r2 = mpyi(r2,r18)
    memw(r29+#56) = r3.new
}
{
    r2 += mpyi(r12,r21)
    r4 = mpyi(r7,r26)
    r12 = memw(r29+#132)
    memw(r29+#32) = r4.new
}
{
    r5 += mpyi(r3,r18)
    r19 = mpyi(r7,r25)
}
{
    r5 += mpyi(r28,r20)
    r3 = mpyi(r12,r26)
    r28 = memw(r29+#112)
    memw(r29+#140) = r3.new
}
{
    r27 = mpyi(r28,r22)
    r14 = mpyi(r12,r22)
    r2 = sub(r5,r2)
    r5 = memw(r29+#72)
}
{
    r2 = mpyi(r0,r2)
    r24 = sub(r19,r14)
    memw(r16+#0) = r2.new
}
{
    r2 = mpyi(r3,r18)
    r6 = mpyi(r28,r25)
    memw(r29+#68) = r6.new
}
{
    r3 = mpyi(r4,r21)
    r2 += mpyi(r27,r21)
    r4 = memw(r29+#124)
}
{
    r2 += mpyi(r24,r20)
    r3 += mpyi(r6,r18)
    r24 = sub(r5,r4)
    r6 = sub(r4,r5)
}
{
    r1 += mpyi(r24,r20)
    r4 = mpyi(r13,r21)
    r2 = sub(r2,r3)
    r5 = memw(r29+#108)
}
{
    r2 = mpyi(r0,r2)
    r10 += mpyi(r6,r26)
    memw(r16+#4) = r2.new
}
{
    r3 = mpyi(r5,r25)
    r12 = mpyi(r5,r22)
    r2 = sub(r1,r8)
    memw(r29+#124) = r3.new
}
{
    r2 = mpyi(r0,r2)
    r8 = r17
    r1 = memw(r29+#104)
    memw(r16+#8) = r2.new
}
{
    r11 += mpyi(r12,r21)
    r4 += mpyi(r3,r18)
    r3 = memw(r29+#116)
}
{
    r11 += mpyi(r9,r18)
    r13 = mpyi(r7,r8)
    r7 = sub(r14,r19)
    memw(r29+#72) = r13.new
}
{
    r2 = mpyi(r25,r3)
    r17 = sub(r11,r4)
    r4 = memw(r29+#132)
    memw(r29+#88) = r2.new
}
{
    r14 = mpyi(r13,r21)
    r10 = mpyi(r0,r10)
    r9 = memw(r29+#60)
    memw(r16+#12) = r10.new
}
{
    r14 += mpyi(r2,r18)
    r4 = mpyi(r4,r8)
    r19 = sub(r9,r1)
    memw(r29+#132) = r4.new
}
{
    r19 = mpyi(r19,r21)
    r14 += mpyi(r7,r23)
    r11 = memw(r29+#64)
    r7 = memw(r29+#136)
}
{
    r2 = mpyi(r4,r18)
    r19 += mpyi(r6,r23)
    r4 = sub(r11,r7)
    r7 = memw(r29+#48)
}
{
    r19 += mpyi(r4,r18)
    r22 = mpyi(r22,r3)
    r6 = memw(r29+#40)
    r4 = memw(r29+#44)
}
{
    r10 = mpyi(r8,r15)
    r17 = mpyi(r0,r17)
    memw(r16+#16) = r17.new
}
{
    r17 = mpyi(r8,r28)
    r28 = mpyi(r26,r3)
    r13 = sub(r6,r4)
    r6 = memw(r29+#32)
}
{
    r2 += mpyi(r22,r21)
    r25 = mpyi(r10,r18)
    r4 = sub(r6,r27)
    r3 = memw(r29+#28)
}
{
    r25 += mpyi(r12,r20)
    r14 = mpyi(r5,r26)
    r2 = sub(r14,r2)
    r27 = memw(r29+#76)
}
{
    r12 = mpyi(r3,r20)
    r15 = mpyi(r17,r18)
    r3 = memw(r29+#72)
    r26 = memw(r29+#80)
}
{
    r6 = mpyi(r27,r18)
    r12 += mpyi(r14,r18)
}
{
    r5 = mpyi(r3,r20)
    r6 += mpyi(r9,r20)
}
{
    r3 = mpyi(r26,r18)
    r12 += mpyi(r13,r23)
}
{
    r15 += mpyi(r22,r20)
    r5 += mpyi(r28,r18)
    r13 = sub(r3,r6)
    r6 = memw(r29+#56)
}
{
    r15 += mpyi(r4,r23)
    r13 += mpyi(r1,r20)
    r3 = memw(r29+#128)
    r22 = memw(r29+#120)
}
{
    r7 += mpyi(r24,r8)
    r4 = sub(r3,r6)
    r5 = sub(r15,r5)
    r3 = memw(r29+#84)
}
{
    r2 = mpyi(r0,r2)
    r4 = mpyi(r4,r23)
    r6 = sub(r3,r22)
    memw(r16+#20) = r2.new
}
{
    r13 += mpyi(r6,r23)
    r7 = mpyi(r0,r7)
    r6 = sub(r22,r3)
    r3 = memw(r29+#36)
}
{
    r5 = mpyi(r0,r5)
    r12 = sub(r12,r25)
    r22 = memw(r29+#144)
    memw(r16+#36) = r5.new
}
{
    r15 = mpyi(r3,r20)
    r22 += mpyi(r6,r8)
    r6 = sub(r28,r17)
    r3 = memw(r29+#124)
}
{
    r15 += mpyi(r14,r21)
    r14 = mpyi(r0,r19)
    r2 = memw(r29+#140)
    memw(r16+#24) = r14.new
}
{
    r4 += mpyi(r3,r20)
    r6 = mpyi(r6,r21)
    r3 = memw(r29+#68)
    memw(r16+#28) = r7
}
{
    r7 = mpyi(r0,r13)
    r2 = sub(r3,r2)
    r3 = memw(r29+#132)
    memw(r16+#40) = r7.new
}
{
    r6 += mpyi(r2,r23)
    r4 += mpyi(r10,r21)
    r2 = memw(r29+#88)
    r14 = memw(r29+#136)
}
{
    r3 = mpyi(r11,r20)
    r5 = mpyi(r27,r21)
    r2 = sub(r3,r2)
    r7 = memw(r29+#92)
}
{
    r6 += mpyi(r2,r20)
    r5 += mpyi(r14,r20)
    r2 = sub(r4,r15)
    r11 = memw(r29+#4)
}
{
    r3 += mpyi(r26,r21)
    r2 = mpyi(r0,r2)
    r4 = sub(r11,r7)
    r15 = memw(r29+#148)
}
{
    r5 += mpyi(r4,r23)
    r4 = sub(r7,r11)
    r7 = memw(r29+#96)
    memw(r16+#48) = r2
}
{
    r12 = mpyi(r0,r12)
    r7 = add(r15,r7)
    r3 = sub(r5,r3)
    memw(r16+#32) = r12.new
}
{
    r7 += mpyi(r4,r8)
    r2 = mpyi(r0,r3)
    memw(r16+#56) = r2.new
}
{
    r4 = mpyi(r0,r22)
    r3 = mpyi(r0,r6)
    memw(r16+#44) = r4.new
}
{
    r2 = mpyi(r0,r7)
    r0 = #0
    memw(r16+#60) = r2.new
}
{
    memw(r16+#52) = r3
}
.L_SINGULAR:
{
    r17:16 = memd(r29+#192)
    r19:18 = memd(r29+#184)
}
{
    r21:20 = memd(r29+#176)
    r23:22 = memd(r29+#168)
}
{
    r25:24 = memd(r29+#160)
    r27:26 = memd(r29+#152)
}
{
    dealloc_return
}
    .size   __qhblas_matrix_inverse4x4_ah, .-__qhblas_matrix_inverse4x4_ah
