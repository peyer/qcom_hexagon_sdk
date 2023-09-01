/**=============================================================================
@file
    qhblas_matrix_inverse4x4_acf.S

@brief
    Calculates matrix inversion for 4x4 matrix of complex float values.

    Function prototype

        int32_t __qhblas_matrix_inverse4x4_acf(complex float *input_matrix, complex float *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse4x4_acf(complex float *input_matrix, complex float *output_matrix);
        {
            complex float det =   A[0]*A[5]*A[10]*A[15] + A[0]*A[6]*A[11]*A[13] + A[0]*A[7]*A[9]*A[14]   \
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
    .hidden	__qhblas_matrix_inverse4x4_acf
    .global __qhblas_matrix_inverse4x4_acf
    .type __qhblas_matrix_inverse4x4_acf, @function

/*============================================================================*/

__qhblas_matrix_inverse4x4_acf:
{
    r16 = r0
    memd(r29+#-16) = r17:16
    allocframe(#208)
}
{
    r14 = memw(r0+#0)
    memd(r29+#184) = r21:20
}
{
    r21 = memw(r16+#40)
    memd(r29+#168) = r25:24
}
{
    r13 = memw(r16+#80)
    memw(r29+#144) = r13.new
}
{
    r2 = sfmpy(r14,r21)
    r4 = memw(r16+#44)
    memw(r29+#52) = r4.new
}
{
    r0 = memw(r0+#4)
    memd(r29+#160) = r27:26
}
{
    r6 = sfmpy(r14,r4)
    r9 = memw(r16+#84)
    memw(r29+#152) = r9.new
}
{
    r3 = sfmpy(r0,r4)
    r7 = sfmpy(r0,r21)
    r17 = memw(r16+#48)
    memw(r29+#12) = r17.new
}
{
    r20 = memw(r16+#52)
    memw(r29+#36) = r20.new
}
{
    r8 = sfsub(r2,r3)
    r3 = sfadd(r7,r6)
    r12 = memw(r16+#120)
    memw(r29+#136) = r8.new
}
{
    r6 = sfmpy(r14,r17)
    r7 = sfmpy(r0,r20)
    r24 = memw(r16+#124)
    memw(r29+#156) = r24.new
}
{
    r27 = sfmpy(r14,r20)
    r10 = sfmpy(r13,r8)
    r28 = memw(r16+#88)
    memw(r29+#24) = r28.new
}
{
    r6 = sfsub(r6,r7)
    r7 = sfmpy(r0,r17)
    memw(r29+#116) = r6.new
}
{
    r11 = sfmpy(r8,r9)
    r18 = memw(r16+#92)
    memd(r29+#192) = r19:18
}
{
    r25 = sfmpy(r3,r9)
    r19 = sfmpy(r13,r3)
    memw(r29+#132) = r18
}
{
    r7 = sfadd(r7,r27)
    memw(r29+#112) = r7.new
}
{
    r26 = sfmpy(r28,r6)
    r6 = sfmpy(r6,r18)
    r4 = memw(r16+#104)
    memw(r29+#40) = r3
}
{
    r2 = sfmpy(r7,r18)
    r3 = sfmpy(r28,r7)
    memw(r29+#32) = r4
    memd(r29+#176) = r23:22
}
{
    r10 = sfsub(r10,r25)
    r11 = sfadd(r11,r19)
    r22 = memw(r16+#56)
    r5 = memw(r16+#108)
}
{
    r2 = sfsub(r26,r2)
    r3 = sfadd(r6,r3)
    r23 = memw(r16+#60)
    memw(r29+#128) = r5
}
{
    r8 = memw(r16+#72)
}
{
    r19 = sfmpy(r12,r10)
    r25 = sfmpy(r11,r24)
    r9 = memw(r16+#76)
    memw(r29+#120) = r9.new
}
{
    r6 = sfmpy(r2,r5)
    r7 = sfmpy(r4,r3)
    r13 = memw(r16+#112)
    memw(r29+#16) = r13.new
}
{
    r2 = sfmpy(r4,r2)
    r4 = sfmpy(r0,r23)
    r15 = memw(r16+#116)
    memw(r29+#28) = r15.new
}
{
    r26 = sfmpy(r14,r22)
    r10 = sfmpy(r10,r24)
    memw(r29+#140) = r8
    memw(r29+#124) = r12
}
{
    r11 = sfmpy(r12,r11)
    r27 = sfmpy(r14,r23)
}
{
    r0 = sfmpy(r0,r22)
    r3 = sfmpy(r3,r5)
}
{
    r5 = sfadd(r6,r7)
    r4 = sfsub(r26,r4)
    r6 = memw(r16+#8)
    memw(r29+#104) = r4.new
}
{
    r7 = sfadd(r0,r27)
    r11 = sfadd(r10,r11)
    r10 = memw(r16+#12)
    memw(r29+#100) = r7.new
}
{
    r19 = sfsub(r19,r25)
    r2 = sfsub(r2,r3)
    r26 = memw(r16+#36)
    r25 = memw(r16+#32)
}
{
    r3 = sfadd(r11,r5)
    r0 = sfmpy(r7,r9)
}
{
    r5 = sfmpy(r8,r4)
    r11 = sfmpy(r4,r9)
    r14 = memw(r16+#64)
    memw(r29+#8) = r14.new
}
{
    r7 = sfmpy(r8,r7)
    r9 = sfmpy(r6,r25)
}
{
    r5 = sfsub(r5,r0)
    r0 = sfmpy(r10,r26)
}
{
    r11 = sfadd(r11,r7)
    r2 = sfadd(r19,r2)
}
{
    r4 = sfsub(r9,r0)
    r7 = sfmpy(r5,r15)
    memw(r29+#108) = r4.new
}
{
    r0 = sfmpy(r13,r11)
    r19 = sfmpy(r6,r26)
}
{
    r5 = sfmpy(r13,r5)
    r11 = sfmpy(r11,r15)
}
{
    r27 = sfmpy(r10,r25)
    r8 = sfmpy(r28,r4)
}
{
    r5 = sfsub(r5,r11)
    r9 = sfmpy(r18,r4)
}
{
    r4 = sfadd(r27,r19)
    r7 = sfadd(r7,r0)
    memw(r29+#88) = r4.new
}
{
    r2 = sfadd(r2,r5)
    r24 = sfmpy(r17,r6)
}
{
    r5 = sfmpy(r18,r4)
    r4 = sfmpy(r28,r4)
}
{
    r0 = sfmpy(r20,r10)
    r3 = sfadd(r3,r7)
}
{
    r7 = sfmpy(r20,r6)
    r11 = sfmpy(r17,r10)
    r17 = memw(r16+#68)
}
{
    r5 = sfsub(r8,r5)
    r4 = sfadd(r9,r4)
}
{
    r24 = sfsub(r24,r0)
    r11 = sfadd(r7,r11)
    memw(r29+#96) = r24.new
}
{
    r7 = sfmpy(r15,r5)
    r8 = sfmpy(r13,r4)
    memw(r29+#92) = r11
}
{
    r5 = sfmpy(r13,r5)
    r4 = sfmpy(r15,r4)
    r13 = r14
}
{
    r9 = sfmpy(r24,r14)
    r7 = sfadd(r7,r8)
    r14 = r11
}
{
    r4 = sfsub(r5,r4)
    r11 = sfmpy(r11,r17)
}
{
    r15 = sfmpy(r24,r17)
    r5 = sfmpy(r14,r13)
}
{
    r20 = sfmpy(r22,r6)
    r3 = sfadd(r3,r7)
}
{
    r7 = sfsub(r9,r11)
    r2 = sfadd(r2,r4)
    r11 = memw(r29+#156)
}
{
    r4 = sfmpy(r23,r10)
    r5 = sfadd(r5,r15)
}
{
    r6 = sfmpy(r23,r6)
    r10 = sfmpy(r22,r10)
}
{
    r15 = sfsub(r20,r4)
    r9 = sfmpy(r11,r7)
    memw(r29+#84) = r15.new
}
{
    r4 = sfmpy(r12,r5)
    r14 = sfadd(r6,r10)
    r10 = memw(r29+#144)
    memw(r29+#80) = r14.new
}
{
    r5 = sfmpy(r11,r5)
    r7 = sfmpy(r12,r7)
    r8 = memw(r16+#28)
    r11 = memw(r29+#152)
}
{
    r6 = sfmpy(r10,r15)
    r4 = sfadd(r9,r4)
    r9 = memw(r16+#24)
}
{
    r5 = sfsub(r7,r5)
    r15 = sfmpy(r11,r15)
}
{
    r7 = sfmpy(r10,r14)
    r13 = sfmpy(r11,r14)
    r11 = memw(r16+#96)
    r14 = memw(r29+#136)
}
{
    r2 = sfadd(r5,r2)
    r3 = sfadd(r4,r3)
    r10 = memw(r16+#100)
    r4 = memw(r16+#16)
}
{
    r5 = sfsub(r6,r13)
    r7 = sfadd(r15,r7)
    r6 = memw(r16+#20)
}
{
    r15 = sfmpy(r14,r28)
    r12 = sfmpy(r14,r18)
    memw(r29+#4) = r15.new
}
{
    r24 = sfmpy(r5,r10)
    r15 = sfmpy(r7,r11)
    memw(r29+#20) = r12
}
{
    r5 = sfmpy(r5,r11)
    r7 = sfmpy(r7,r10)
}
{
    r24 = sfadd(r15,r24)
    r20 = sfmpy(r25,r4)
}
{
    r7 = sfsub(r5,r7)
    r14 = sfmpy(r26,r6)
}
{
    r15 = sfmpy(r26,r4)
    r5 = sfmpy(r25,r6)
}
{
    r0 = sfadd(r7,r2)
    r12 = sfsub(r20,r14)
    r2 = memw(r29+#52)
    memw(r29+#68) = r12.new
}
{
    r27 = sfadd(r15,r5)
    r19 = sfmpy(r21,r4)
    r15 = memw(r29+#140)
    memw(r29+#64) = r27.new
}
{
    r7 = sfmpy(r2,r6)
    r5 = sfmpy(r2,r4)
}
{
    r20 = sfmpy(r21,r6)
    r3 = sfadd(r24,r3)
}
{
    r18 = sfsub(r19,r7)
    r19 = sfmpy(r22,r4)
    memw(r29+#76) = r18.new
}
{
    r13 = sfadd(r5,r20)
    r5 = sfmpy(r23,r6)
    r20 = memw(r29+#120)
    memw(r29+#44) = r13.new
}
{
    r4 = sfmpy(r23,r4)
    r6 = sfmpy(r22,r6)
}
{
    r23 = sfmpy(r25,r9)
    r7 = sfmpy(r15,r12)
}
{
    r4 = sfadd(r4,r6)
    r6 = sfmpy(r26,r8)
    memw(r29+#136) = r4.new
}
{
    r22 = sfmpy(r20,r27)
    r14 = sfsub(r19,r5)
    memw(r29+#56) = r14.new
}
{
    r5 = sfmpy(r20,r12)
    r12 = sfsub(r23,r6)
    memw(r29+#60) = r12.new
}
{
    r6 = sfmpy(r15,r27)
    r4 = sfsub(r7,r22)
    r15 = memw(r29+#132)
}
{
    r7 = sfmpy(r26,r9)
    r24 = sfmpy(r25,r8)
    r26 = memw(r29+#156)
}
{
    r5 = sfadd(r5,r6)
    r22 = sfmpy(r21,r9)
    r25 = memw(r29+#124)
}
{
    r27 = sfadd(r7,r24)
    r19 = sfmpy(r26,r4)
    memw(r29+#72) = r27.new
}
{
    r24 = sfmpy(r25,r5)
    r5 = sfmpy(r26,r5)
    r26 = memw(r29+#152)
}
{
    r4 = sfmpy(r25,r4)
    r23 = sfmpy(r2,r8)
}
{
    r7 = sfmpy(r28,r18)
    r6 = sfmpy(r15,r13)
}
{
    r4 = sfsub(r4,r5)
    r20 = sfmpy(r2,r9)
}
{
    r21 = sfmpy(r21,r8)
    r25 = sfsub(r22,r23)
    memw(r29+#52) = r25.new
}
{
    r6 = sfsub(r7,r6)
    r7 = sfadd(r19,r24)
}
{
    r22 = sfadd(r20,r21)
    r2 = sfadd(r0,r4)
    r0 = memw(r29+#12)
    memw(r29+#48) = r22.new
}
{
    r3 = sfadd(r3,r7)
    r5 = sfmpy(r15,r18)
    r20 = memw(r29+#36)
}
{
    r4 = sfmpy(r0,r9)
    r19 = sfmpy(r10,r6)
}
{
    r7 = sfmpy(r20,r9)
    r9 = sfmpy(r20,r8)
}
{
    r8 = sfmpy(r0,r8)
    r6 = sfmpy(r11,r6)
    r0 = memw(r29+#40)
}
{
    r24 = sfsub(r4,r9)
    r4 = sfmpy(r28,r13)
    memw(r29+#40) = r24.new
}
{
    r23 = sfmpy(r0,r15)
    r15 = sfmpy(r0,r28)
    r28 = memw(r29+#132)
    memw(r29+#12) = r15.new
}
{
    r13 = sfadd(r7,r8)
    r4 = sfadd(r5,r4)
    r15 = memw(r29+#8)
    memw(r29+#36) = r13.new
}
{
    r5 = sfmpy(r17,r14)
    r21 = sfmpy(r26,r27)
    r0 = memw(r29+#136)
    r7 = memw(r29+#4)
}
{
    r18 = sfsub(r7,r23)
    r7 = sfmpy(r15,r14)
    r14 = r15
    r23 = memw(r29+#128)
}
{
    r8 = sfmpy(r17,r0)
    r9 = sfmpy(r15,r0)
    r15 = memw(r29+#32)
    r0 = memw(r29+#144)
}
{
    r7 = sfsub(r7,r8)
    r8 = sfmpy(r11,r4)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfadd(r5,r9)
}
{
    r9 = sfmpy(r23,r7)
    r8 = sfadd(r19,r8)
}
{
    r4 = sfsub(r6,r4)
    r20 = sfmpy(r0,r12)
}
{
    r6 = sfmpy(r26,r12)
    r12 = sfmpy(r0,r27)
    r0 = r17
}
{
    r2 = sfadd(r4,r2)
    r4 = sfmpy(r15,r5)
}
{
    r3 = sfadd(r8,r3)
    r7 = sfmpy(r15,r7)
}
{
    r5 = sfmpy(r23,r5)
    r4 = sfadd(r9,r4)
}
{
    r6 = sfadd(r6,r12)
    r21 = sfsub(r20,r21)
}
{
    r5 = sfsub(r7,r5)
    r19 = sfadd(r4,r3)
}
{
    r8 = sfmpy(r14,r25)
    r12 = sfmpy(r17,r22)
}
{
    r7 = sfmpy(r17,r25)
    r3 = sfmpy(r14,r22)
    r25 = memw(r29+#140)
}
{
    r4 = sfsub(r8,r12)
    r20 = sfadd(r5,r2)
    r22 = memw(r29+#120)
    r12 = memw(r29+#28)
}
{
    r3 = sfadd(r7,r3)
    r7 = sfmpy(r15,r6)
}
{
    r2 = sfmpy(r23,r21)
    r5 = sfmpy(r15,r21)
}
{
    r6 = sfmpy(r23,r6)
    r9 = sfmpy(r12,r4)
    r23 = memw(r29+#16)
}
{
    r2 = sfadd(r2,r7)
    r8 = sfmpy(r25,r24)
}
{
    r5 = sfsub(r5,r6)
    r6 = sfmpy(r23,r3)
    r27 = memw(r29+#116)
}
{
    r4 = sfmpy(r23,r4)
    r3 = sfmpy(r12,r3)
}
{
    r7 = sfmpy(r22,r13)
    r21 = sfmpy(r22,r24)
    r26 = memw(r29+#144)
}
{
    r3 = sfsub(r4,r3)
    r4 = sfmpy(r25,r13)
}
{
    r7 = sfsub(r8,r7)
    r5 = sfadd(r5,r20)
}
{
    r4 = sfadd(r21,r4)
    r6 = sfadd(r9,r6)
    r24 = memw(r29+#152)
}
{
    r8 = sfmpy(r10,r7)
    r7 = sfmpy(r11,r7)
    r21 = memw(r29+#112)
}
{
    r13 = sfmpy(r11,r4)
    r4 = sfmpy(r10,r4)
}
{
    r2 = sfadd(r2,r19)
    r9 = sfmpy(r18,r12)
}
{
    r4 = sfsub(r7,r4)
    r8 = sfadd(r8,r13)
    r7 = memw(r29+#20)
    r13 = memw(r29+#12)
}
{
    r17 = sfmpy(r18,r23)
    r3 = sfadd(r3,r5)
}
{
    r7 = sfadd(r13,r7)
    r2 = sfadd(r6,r2)
    r13 = r12
}
{
    r20 = sfmpy(r27,r22)
    r5 = sfmpy(r27,r25)
}
{
    r18 = sfmpy(r21,r22)
    r21 = sfmpy(r21,r25)
    r27 = memw(r29+#124)
    r25 = memw(r29+#156)
}
{
    r19 = sfmpy(r7,r23)
    r7 = sfmpy(r7,r12)
}
{
    r12 = sfadd(r21,r20)
    r3 = sfadd(r4,r3)
    r21 = memw(r29+#104)
}
{
    r5 = sfsub(r5,r18)
    r7 = sfsub(r17,r7)
    r18 = memw(r29+#88)
}
{
    r2 = sfadd(r8,r2)
    r4 = sfmpy(r27,r12)
}
{
    r8 = sfmpy(r25,r12)
    r6 = sfadd(r19,r9)
    r12 = memw(r29+#100)
}
{
    r17 = sfmpy(r25,r5)
    r5 = sfmpy(r27,r5)
}
{
    r3 = sfsub(r3,r7)
    r7 = sfmpy(r26,r21)
}
{
    r9 = sfmpy(r24,r12)
    r2 = sfsub(r2,r6)
}
{
    r5 = sfsub(r5,r8)
    r21 = sfmpy(r24,r21)
}
{
    r20 = sfmpy(r26,r12)
    r6 = sfsub(r7,r9)
}
{
    r3 = sfsub(r3,r5)
    r4 = sfadd(r17,r4)
}
{
    r7 = sfadd(r21,r20)
    r5 = sfmpy(r15,r6)
    r20 = r15
    r21 = memw(r29+#128)
}
{
    r17 = sfmpy(r24,r18)
    r2 = sfsub(r2,r4)
    r15 = memw(r29+#108)
}
{
    r8 = sfmpy(r21,r7)
}
{
    r12 = sfmpy(r26,r15)
}
{
    r4 = sfsub(r5,r8)
    r5 = sfmpy(r21,r6)
}
{
    r6 = sfmpy(r20,r7)
    r7 = sfsub(r12,r17)
}
{
    r12 = sfmpy(r24,r15)
    r15 = sfmpy(r26,r18)
}
{
    r3 = sfsub(r3,r4)
    r5 = sfadd(r5,r6)
    r18 = memw(r29+#92)
}
{
    r4 = sfadd(r12,r15)
    r6 = sfmpy(r25,r7)
    r15 = memw(r29+#24)
    r12 = memw(r29+#96)
}
{
    r7 = sfmpy(r27,r7)
    r2 = sfsub(r2,r5)
}
{
    r8 = sfmpy(r15,r12)
    r19 = sfmpy(r27,r4)
}
{
    r9 = sfmpy(r28,r18)
    r17 = sfmpy(r28,r12)
}
{
    r18 = sfmpy(r15,r18)
    r4 = sfmpy(r25,r4)
}
{
    r8 = sfsub(r8,r9)
    r5 = sfadd(r6,r19)
    r9 = memw(r29+#84)
}
{
    r12 = sfadd(r17,r18)
    r4 = sfsub(r7,r4)
}
{
    r6 = sfmpy(r8,r11)
    r2 = sfsub(r2,r5)
}
{
    r7 = sfmpy(r12,r10)
    r3 = sfsub(r3,r4)
}
{
    r5 = sfmpy(r8,r10)
}
{
    r4 = sfsub(r6,r7)
    r6 = sfmpy(r12,r11)
    r18 = memw(r29+#44)
    r12 = memw(r29+#80)
}
{
    r7 = sfmpy(r9,r14)
    r9 = sfmpy(r9,r0)
}
{
    r8 = sfmpy(r12,r0)
    r17 = sfmpy(r12,r14)
    r12 = memw(r29+#68)
}
{
    r5 = sfadd(r6,r5)
    r3 = sfsub(r3,r4)
}
{
    r4 = sfsub(r7,r8)
    r6 = sfadd(r17,r9)
    r17 = r13
}
{
    r7 = sfmpy(r15,r12)
    r8 = sfmpy(r28,r12)
    r12 = memw(r29+#64)
}
{
    r19 = sfmpy(r13,r6)
    r6 = sfmpy(r23,r6)
}
{
    r9 = sfmpy(r28,r12)
    r12 = sfmpy(r15,r12)
}
{
    r15 = sfmpy(r23,r4)
    r4 = sfmpy(r13,r4)
    r13 = r14
    r14 = memw(r29+#76)
}
{
    r2 = sfsub(r2,r5)
    r7 = sfsub(r7,r9)
}
{
    r5 = sfsub(r15,r19)
    r8 = sfadd(r8,r12)
}
{
    r4 = sfadd(r4,r6)
    r6 = sfmpy(r20,r7)
    r19 = memw(r29+#136)
}
{
    r3 = sfsub(r3,r5)
    r5 = sfmpy(r21,r7)
}
{
    r7 = sfmpy(r20,r8)
    r8 = sfmpy(r21,r8)
}
{
    r9 = sfmpy(r13,r14)
    r15 = sfmpy(r0,r14)
    r14 = r13
}
{
    r28 = sfmpy(r13,r18)
    r12 = sfmpy(r0,r18)
    r13 = memw(r29+#140)
}
{
    r2 = sfsub(r2,r4)
    r4 = sfsub(r6,r8)
}
{
    r6 = sfsub(r9,r12)
    r8 = sfadd(r15,r28)
    r15 = memw(r29+#56)
}
{
    r5 = sfadd(r5,r7)
    r3 = sfsub(r3,r4)
}
{
    r4 = sfmpy(r27,r6)
    r7 = sfmpy(r25,r8)
}
{
    r9 = sfmpy(r13,r15)
    r12 = sfmpy(r22,r19)
}
{
    r4 = sfsub(r4,r7)
    r2 = sfsub(r2,r5)
}
{
    r7 = sfsub(r9,r12)
    r12 = sfmpy(r22,r15)
}
{
    r15 = sfmpy(r13,r19)
    r3 = sfsub(r3,r4)
}
{
    r5 = sfmpy(r25,r6)
    r6 = sfmpy(r27,r8)
}
{
    r4 = sfadd(r12,r15)
    r12 = memw(r29+#60)
    r15 = memw(r29+#72)
}
{
    r5 = sfadd(r5,r6)
    r6 = sfmpy(r11,r7)
}
{
    r8 = sfmpy(r13,r12)
    r9 = sfmpy(r22,r12)
}
{
    r13 = sfmpy(r13,r15)
    r12 = sfmpy(r22,r15)
}
{
    r15 = sfmpy(r10,r4)
    r7 = sfmpy(r10,r7)
}
{
    r4 = sfmpy(r11,r4)
    r13 = sfadd(r9,r13)
}
{
    r2 = sfsub(r2,r5)
    r5 = sfsub(r6,r15)
    r15 = memw(r29+#48)
}
{
    r4 = sfadd(r7,r4)
    r6 = sfsub(r8,r12)
}
{
    r7 = sfmpy(r23,r13)
    r8 = sfmpy(r17,r13)
    r13 = memw(r29+#52)
}
{
    r3 = sfsub(r3,r5)
    r5 = sfmpy(r17,r6)
}
{
    r9 = sfmpy(r26,r13)
    r6 = sfmpy(r23,r6)
}
{
    r12 = sfmpy(r24,r15)
    r13 = sfmpy(r24,r13)
}
{
    r15 = sfmpy(r26,r15)
    r2 = sfsub(r2,r4)
}
{
    r4 = sfsub(r6,r8)
    r6 = sfsub(r9,r12)
}
{
    r15 = sfadd(r13,r15)
    r5 = sfadd(r5,r7)
    r13 = memw(r29+#40)
}
{
    r3 = sfsub(r3,r4)
    r4 = sfmpy(r10,r6)
}
{
    r8 = sfmpy(r10,r15)
    r6 = sfmpy(r11,r6)
    r10 = memw(r29+#36)
}
{
    r7 = sfmpy(r11,r15)
    r9 = sfmpy(r14,r13)
}
{
    r11 = sfmpy(r0,r10)
    r13 = sfmpy(r0,r13)
    r0 = #-1
}
{
    r14 = sfmpy(r14,r10)
    r2 = sfsub(r2,r5)
}
{
    r4 = sfadd(r4,r7)
    r5 = sfsub(r9,r11)
}
{
    r7 = sfadd(r13,r14)
    r6 = sfsub(r6,r8)
}
{
    r14 = sfmpy(r21,r5)
    r5 = sfmpy(r20,r5)
}
{
    r15 = sfmpy(r20,r7)
    r7 = sfmpy(r21,r7)
}
{
    r3 = sfsub(r3,r6)
    r4 = sfsub(r2,r4)
}
{
    r2 = sfsub(r5,r7)
    r6 = sfadd(r14,r15)
}
{
    r2 = sfsub(r3,r2)
    r3 = sfsub(r4,r6)
    r6 = #0
}
{
    p0 = sfcmp.eq(r2,r6)
    p1 = sfcmp.eq(r3,r6)
}
{
    p0 = and(p0,p1)
    if (p0.new) jump:t .L_SINGULAR
}
// %bb.1:                               // %if.end
{
    call __divsc3
    r1:0 = combine(#0,##1065353216)
    memw(r29+#156) = r1
}
{
    r18 = memw(r16+#48)
    memd(r29+#144) = r1:0
}
{
    r11 = memw(r16+#88)
    r19 = memw(r16+#52)
}
{
    r2 = sfmpy(r18,r11)
    r20 = memw(r16+#92)
    r10 = memw(r16+#104)
}
{
    r3 = sfmpy(r19,r20)
    r5 = sfmpy(r19,r11)
    r6 = memw(r16+#108)
    r13 = memw(r16+#56)
}
{
    r3 = sfsub(r2,r3)
    r4 = sfmpy(r18,r20)
    r9 = memw(r16+#80)
    r22 = memw(r16+#72)
}
{
    r26 = sfmpy(r13,r22)
    r28 = sfmpy(r13,r9)
    r23 = memw(r16+#76)
    r21 = memw(r16+#60)
}
{
    r25 = sfmpy(r21,r9)
    r5 = sfadd(r5,r4)
    r12 = memw(r16+#84)
    r14 = memw(r16+#40)
}
{
    r24 = sfmpy(r21,r12)
    r17 = sfmpy(r21,r23)
    r15 = memw(r16+#44)
    r7 = memw(r16+#112)
}
{
    r2 = sfmpy(r10,r3)
    r27 = sfmpy(r13,r23)
    r8 = memw(r16+#116)
    memw(r29+#152) = r2.new
}
{
    r2 = sfmpy(r13,r12)
    r4 = sfmpy(r5,r6)
}
{
    r28 = sfsub(r28,r24)
    r5 = sfmpy(r10,r5)
}
{
    r24 = sfadd(r25,r2)
    r25 = sfmpy(r20,r15)
}
{
    r2 = sfmpy(r18,r22)
    r18 = sfmpy(r18,r23)
}
{
    r23 = sfmpy(r19,r23)
    r19 = sfmpy(r19,r22)
}
{
    r21 = sfmpy(r21,r22)
    r22 = sfsub(r26,r17)
}
{
    r26 = sfmpy(r11,r14)
    r13 = sfmpy(r6,r28)
}
{
    r20 = sfmpy(r20,r14)
    r11 = sfmpy(r11,r15)
}
{
    r3 = sfmpy(r3,r6)
    r28 = sfmpy(r10,r28)
}
{
    r6 = sfmpy(r6,r24)
    r10 = sfmpy(r10,r24)
}
{
    r17 = sfmpy(r12,r15)
    r12 = sfmpy(r12,r14)
}
{
    r14 = sfmpy(r9,r14)
    r9 = sfmpy(r9,r15)
}
{
    r15 = sfadd(r21,r27)
    r21 = sfsub(r26,r25)
    r26 = memw(r29+#152)
}
{
    r11 = sfadd(r20,r11)
    r20 = sfmpy(r22,r8)
}
{
    r24 = sfmpy(r7,r15)
    r22 = sfmpy(r7,r22)
}
{
    r3 = sfadd(r3,r5)
    r5 = sfadd(r13,r10)
}
{
    r15 = sfmpy(r15,r8)
    r4 = sfsub(r26,r4)
}
{
    r6 = sfsub(r28,r6)
    r18 = sfadd(r19,r18)
}
{
    r25 = sfmpy(r8,r21)
    r21 = sfmpy(r7,r21)
}
{
    r7 = sfmpy(r7,r11)
    r8 = sfmpy(r8,r11)
    r11 = memw(r16+#120)
}
{
    r2 = sfsub(r2,r23)
    r4 = sfsub(r4,r6)
    r23 = memw(r16+#124)
}
{
    r10 = sfsub(r22,r15)
    r3 = sfsub(r3,r5)
}
{
    r6 = sfadd(r20,r24)
    r14 = sfsub(r14,r17)
}
{
    r9 = sfadd(r12,r9)
    r12 = sfmpy(r2,r23)
}
{
    r17 = sfmpy(r18,r11)
    r7 = sfadd(r25,r7)
}
{
    r3 = sfadd(r3,r6)
    r18 = sfmpy(r18,r23)
}
{
    r2 = sfmpy(r2,r11)
    r5 = sfsub(r21,r8)
}
{
    r4 = sfadd(r4,r10)
    r6 = sfadd(r17,r12)
}
{
    r19 = sfmpy(r14,r23)
    r14 = sfmpy(r11,r14)
}
{
    r4 = sfsub(r4,r5)
    r2 = sfsub(r2,r18)
}
{
    r23 = sfmpy(r9,r23)
    r3 = sfsub(r3,r7)
}
{
    r5 = sfmpy(r11,r9)
}
{
    r2 = sfsub(r4,r2)
    r7 = sfsub(r14,r23)
}
{
    r4 = sfadd(r19,r5)
    r3 = sfsub(r3,r6)
}
{
    r2 = sfadd(r7,r2)
}
{
    r3 = sfadd(r4,r3)
}
{
    r7 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    r3 = memw(r29+#156)
}
{
    r27 = r3
}
{
    r2 = sfadd(r6,r7)
    memw(r3+#0) = r2
}
{
    memw(r3+#4) = r2
    r8 = memw(r16+#24)

} :mem_noshuf
{
    r2 = memw(r16+#80)
    r3 = memw(r16+#84)
}
{
    r4 = sfmpy(r8,r3)
    r5 = sfmpy(r8,r2)
    r9 = memw(r16+#28)
    r28 = memw(r16+#76)
}
{
    r6 = sfmpy(r9,r2)
    r7 = sfmpy(r9,r3)
    r12 = memw(r16+#16)
    r13 = memw(r16+#88)
}
{
    r10 = sfmpy(r12,r13)
    r21 = sfmpy(r9,r28)
    r17 = memw(r16+#72)
    r20 = memw(r16+#8)
}
{
    r5 = sfsub(r5,r7)
    r4 = sfadd(r6,r4)
    r14 = memw(r16+#92)
    r15 = memw(r16+#20)
}
{
    r22 = sfmpy(r8,r28)
    r8 = sfmpy(r8,r17)
    r23 = memw(r16+#12)
    r7 = memw(r16+#108)
}
{
    r11 = sfmpy(r12,r14)
    r18 = sfmpy(r15,r14)
}
{
    r19 = sfmpy(r15,r13)
    r9 = sfmpy(r9,r17)
}
{
    r24 = sfmpy(r13,r20)
    r13 = sfmpy(r13,r23)
}
{
    r25 = sfmpy(r14,r20)
    r14 = sfmpy(r14,r23)
}
{
    r26 = sfmpy(r12,r17)
    r8 = sfsub(r8,r21)
    r21 = memw(r16+#116)
}
{
    r12 = sfmpy(r12,r28)
    r28 = sfmpy(r15,r28)
}
{
    r15 = sfmpy(r15,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfmpy(r2,r23)
    r6 = sfsub(r10,r18)
    r23 = memw(r16+#104)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r5)
}
{
    r18 = sfmpy(r4,r7)
    r19 = sfmpy(r7,r6)
}
{
    r5 = sfmpy(r5,r7)
    r4 = sfmpy(r23,r4)
}
{
    r6 = sfmpy(r23,r6)
    r7 = sfmpy(r7,r10)
}
{
    r28 = sfsub(r26,r28)
    r23 = sfmpy(r23,r10)
    r10 = memw(r16+#112)
    r26 = memw(r16+#124)
}
{
    r9 = sfadd(r9,r22)
    r14 = sfsub(r24,r14)
}
{
    r13 = sfadd(r25,r13)
    r4 = sfadd(r5,r4)
}
{
    r22 = sfmpy(r8,r21)
    r24 = sfmpy(r10,r9)
}
{
    r5 = sfadd(r19,r23)
    r8 = sfmpy(r10,r8)
}
{
    r9 = sfmpy(r9,r21)
    r6 = sfsub(r6,r7)
}
{
    r11 = sfsub(r11,r18)
    r25 = sfmpy(r21,r14)
}
{
    r14 = sfmpy(r10,r14)
    r10 = sfmpy(r10,r13)
}
{
    r13 = sfmpy(r21,r13)
    r6 = sfsub(r11,r6)
    r21 = memw(r16+#120)
}
{
    r7 = sfsub(r8,r9)
    r12 = sfadd(r15,r12)
}
{
    r4 = sfsub(r4,r5)
    r11 = sfadd(r22,r24)
}
{
    r3 = sfsub(r17,r3)
    r15 = sfmpy(r28,r26)
}
{
    r2 = sfadd(r20,r2)
    r17 = sfmpy(r12,r21)
}
{
    r6 = sfsub(r6,r7)
    r5 = sfsub(r14,r13)
}
{
    r28 = sfmpy(r28,r21)
    r12 = sfmpy(r12,r26)
}
{
    r7 = sfadd(r25,r10)
    r4 = sfsub(r4,r11)
}
{
    r20 = sfmpy(r3,r26)
    r26 = sfmpy(r2,r26)
}
{
    r3 = sfmpy(r21,r3)
    r5 = sfadd(r6,r5)
}
{
    r2 = sfmpy(r21,r2)
    r10 = sfsub(r28,r12)
}
{
    r6 = sfadd(r17,r15)
    r4 = sfadd(r4,r7)
}
{
    r3 = sfsub(r3,r26)
    r5 = sfadd(r10,r5)
}
{
    r2 = sfadd(r20,r2)
}
{
    r4 = sfadd(r6,r4)
}
{
    r3 = sfsub(r5,r3)
}
{
    r2 = sfsub(r4,r2)
}
{
    r7 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r6 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#8) = r2.new
}
{
    r2 = sfadd(r6,r7)
    memw(r27+#12) = r2.new
}
{
    r2 = memw(r16+#16)
    r3 = memw(r16+#56)
}
{
    r9 = sfmpy(r2,r3)
    r4 = memw(r16+#60)
    r5 = memw(r16+#20)
}
{
    r15 = sfmpy(r5,r3)
    r14 = sfmpy(r5,r4)
    r28 = memw(r16+#44)
    r6 = memw(r16+#24)
}
{
    r22 = sfmpy(r6,r28)
    r12 = sfmpy(r2,r4)
    r7 = memw(r16+#48)
    r17 = memw(r16+#40)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#8)
    r13 = memw(r16+#28)
}
{
    r12 = sfadd(r15,r12)
    r26 = sfmpy(r2,r17)
    r8 = memw(r16+#52)
    r23 = memw(r16+#12)
}
{
    r19 = sfmpy(r13,r7)
    r24 = sfmpy(r3,r20)
    r14 = memw(r16+#108)
}
{
    r11 = sfmpy(r6,r8)
    r18 = sfmpy(r13,r8)
}
{
    r21 = sfmpy(r13,r28)
    r6 = sfmpy(r6,r17)
}
{
    r13 = sfmpy(r13,r17)
    r25 = sfmpy(r4,r20)
}
{
    r4 = sfmpy(r4,r23)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfmpy(r2,r28)
    r28 = sfmpy(r5,r28)
}
{
    r5 = sfmpy(r5,r17)
    r17 = sfmpy(r7,r20)
}
{
    r20 = sfmpy(r8,r20)
    r7 = sfmpy(r7,r23)
}
{
    r6 = sfsub(r6,r21)
    r8 = sfmpy(r8,r23)
    r23 = memw(r16+#104)
    r21 = memw(r16+#116)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r13 = sfadd(r13,r22)
    r10 = memw(r16+#112)
}
{
    r4 = sfsub(r24,r4)
    r9 = sfadd(r9,r12)
}
{
    r3 = sfadd(r25,r3)
    r22 = sfmpy(r6,r21)
}
{
    r24 = sfmpy(r10,r13)
    r12 = sfadd(r19,r23)
}
{
    r6 = sfmpy(r10,r6)
    r13 = sfmpy(r13,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r4)
    r6 = sfsub(r6,r13)
}
{
    r4 = sfmpy(r10,r4)
    r10 = sfmpy(r10,r3)
}
{
    r3 = sfmpy(r21,r3)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r2 = sfadd(r5,r2)
    r5 = sfsub(r17,r8)
}
{
    r13 = sfsub(r11,r14)
    r11 = sfadd(r22,r24)
}
{
    r7 = sfadd(r20,r7)
    r8 = sfmpy(r28,r26)
}
{
    r17 = sfmpy(r2,r21)
    r3 = sfsub(r4,r3)
}
{
    r4 = sfadd(r13,r6)
    r28 = sfmpy(r28,r21)
}
{
    r2 = sfmpy(r2,r26)
    r6 = sfadd(r25,r10)
}
{
    r9 = sfadd(r9,r11)
    r20 = sfmpy(r5,r26)
}
{
    r3 = sfsub(r4,r3)
    r2 = sfsub(r28,r2)
}
{
    r5 = sfmpy(r21,r5)
    r26 = sfmpy(r7,r26)
}
{
    r4 = sfmpy(r21,r7)
    r7 = sfadd(r17,r8)
}
{
    r6 = sfsub(r9,r6)
    r2 = sfsub(r3,r2)
}
{
    r5 = sfsub(r5,r26)
    r3 = sfadd(r20,r4)
}
{
    r7 = sfsub(r6,r7)
}
{
    r2 = sfadd(r5,r2)
}
{
    r3 = sfadd(r3,r7)
}
{
    r6 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r7 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    memw(r27+#16) = r2.new
}
{
    r2 = sfadd(r7,r6)
    memw(r27+#20) = r2.new
}
{
    r4 = memw(r16+#24)
    r2 = memw(r16+#48)
}
{
    r9 = sfmpy(r4,r2)
    r3 = memw(r16+#52)
    r5 = memw(r16+#28)
}
{
    r15 = sfmpy(r5,r2)
    r14 = sfmpy(r5,r3)
    r28 = memw(r16+#44)
    r6 = memw(r16+#16)
}
{
    r21 = sfmpy(r5,r28)
    r12 = sfmpy(r4,r3)
    r7 = memw(r16+#56)
    r17 = memw(r16+#40)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#8)
    r8 = memw(r16+#60)
}
{
    r12 = sfadd(r15,r12)
    r22 = sfmpy(r4,r28)
    r13 = memw(r16+#20)
    r23 = memw(r16+#12)
}
{
    r11 = sfmpy(r6,r8)
    r4 = sfmpy(r4,r17)
    r14 = memw(r16+#76)
}
{
    r18 = sfmpy(r13,r8)
    r19 = sfmpy(r13,r7)
}
{
    r5 = sfmpy(r5,r17)
    r24 = sfmpy(r7,r20)
}
{
    r25 = sfmpy(r8,r20)
    r8 = sfmpy(r8,r23)
}
{
    r7 = sfmpy(r7,r23)
    r26 = sfmpy(r6,r17)
}
{
    r6 = sfmpy(r6,r28)
    r28 = sfmpy(r13,r28)
}
{
    r13 = sfmpy(r13,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r4 = sfsub(r4,r21)
    r2 = sfmpy(r2,r23)
    r23 = memw(r16+#72)
    r21 = memw(r16+#84)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#92)
}
{
    r23 = sfmpy(r23,r10)
    r5 = sfadd(r5,r22)
    r10 = memw(r16+#80)
}
{
    r8 = sfsub(r24,r8)
    r7 = sfadd(r25,r7)
}
{
    r9 = sfadd(r9,r12)
    r22 = sfmpy(r4,r21)
}
{
    r24 = sfmpy(r10,r5)
    r12 = sfadd(r19,r23)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfmpy(r5,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r8)
    r6 = sfadd(r13,r6)
}
{
    r8 = sfmpy(r10,r8)
    r10 = sfmpy(r10,r7)
}
{
    r7 = sfmpy(r21,r7)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#88)
}
{
    r4 = sfsub(r4,r5)
    r5 = sfsub(r11,r14)
}
{
    r11 = sfadd(r22,r24)
    r13 = sfmpy(r28,r26)
}
{
    r3 = sfsub(r17,r3)
    r17 = sfmpy(r6,r21)
}
{
    r2 = sfadd(r20,r2)
    r7 = sfsub(r8,r7)
}
{
    r4 = sfsub(r5,r4)
    r5 = sfadd(r25,r10)
}
{
    r10 = sfsub(r9,r11)
    r28 = sfmpy(r28,r21)
}
{
    r6 = sfmpy(r6,r26)
    r20 = sfmpy(r3,r26)
}
{
    r26 = sfmpy(r2,r26)
    r3 = sfmpy(r21,r3)
}
{
    r2 = sfmpy(r21,r2)
    r4 = sfadd(r4,r7)
}
{
    r6 = sfsub(r28,r6)
    r7 = sfadd(r17,r13)
}
{
    r5 = sfadd(r10,r5)
    r3 = sfsub(r3,r26)
}
{
    r4 = sfadd(r6,r4)
    r2 = sfadd(r20,r2)
}
{
    r5 = sfadd(r7,r5)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r5,r2)
}
{
    r4 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#24) = r2.new
}
{
    r2 = sfadd(r5,r4)
    memw(r27+#28) = r2.new
}
{
    r4 = memw(r16+#56)
    r2 = memw(r16+#80)
}
{
    r9 = sfmpy(r4,r2)
    r3 = memw(r16+#84)
    r5 = memw(r16+#60)
}
{
    r15 = sfmpy(r5,r2)
    r14 = sfmpy(r5,r3)
    r28 = memw(r16+#68)
    r6 = memw(r16+#48)
}
{
    r21 = sfmpy(r5,r28)
    r12 = sfmpy(r4,r3)
    r7 = memw(r16+#88)
    r17 = memw(r16+#64)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#32)
    r8 = memw(r16+#92)
}
{
    r12 = sfadd(r15,r12)
    r22 = sfmpy(r4,r28)
    r13 = memw(r16+#52)
    r23 = memw(r16+#36)
}
{
    r11 = sfmpy(r6,r8)
    r4 = sfmpy(r4,r17)
    r14 = memw(r16+#100)
}
{
    r18 = sfmpy(r13,r8)
    r19 = sfmpy(r13,r7)
}
{
    r5 = sfmpy(r5,r17)
    r24 = sfmpy(r7,r20)
}
{
    r25 = sfmpy(r8,r20)
    r8 = sfmpy(r8,r23)
}
{
    r7 = sfmpy(r7,r23)
    r26 = sfmpy(r6,r17)
}
{
    r6 = sfmpy(r6,r28)
    r28 = sfmpy(r13,r28)
}
{
    r13 = sfmpy(r13,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r4 = sfsub(r4,r21)
    r2 = sfmpy(r2,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#116)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r5 = sfadd(r5,r22)
    r10 = memw(r16+#112)
}
{
    r8 = sfsub(r24,r8)
    r7 = sfadd(r25,r7)
}
{
    r9 = sfadd(r9,r12)
    r22 = sfmpy(r4,r21)
}
{
    r24 = sfmpy(r10,r5)
    r12 = sfadd(r19,r23)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfmpy(r5,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r8)
    r6 = sfadd(r13,r6)
}
{
    r8 = sfmpy(r10,r8)
    r10 = sfmpy(r10,r7)
}
{
    r7 = sfmpy(r21,r7)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r4 = sfsub(r4,r5)
    r5 = sfsub(r11,r14)
}
{
    r11 = sfadd(r22,r24)
    r13 = sfmpy(r28,r26)
}
{
    r3 = sfsub(r17,r3)
    r17 = sfmpy(r6,r21)
}
{
    r2 = sfadd(r20,r2)
    r7 = sfsub(r8,r7)
}
{
    r4 = sfsub(r5,r4)
    r5 = sfadd(r25,r10)
}
{
    r10 = sfsub(r9,r11)
    r28 = sfmpy(r28,r21)
}
{
    r6 = sfmpy(r6,r26)
    r20 = sfmpy(r3,r26)
}
{
    r26 = sfmpy(r2,r26)
    r3 = sfmpy(r21,r3)
}
{
    r2 = sfmpy(r21,r2)
    r4 = sfadd(r4,r7)
}
{
    r6 = sfsub(r28,r6)
    r7 = sfadd(r17,r13)
}
{
    r5 = sfadd(r10,r5)
    r3 = sfsub(r3,r26)
}
{
    r4 = sfadd(r6,r4)
    r2 = sfadd(r20,r2)
}
{
    r5 = sfadd(r7,r5)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r5,r2)
}
{
    r4 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#32) = r2.new
}
{
    r2 = sfadd(r5,r4)
    memw(r27+#36) = r2.new
}
{
    r2 = memw(r16+#16)
    r3 = memw(r16+#88)
}
{
    r9 = sfmpy(r2,r3)
    r4 = memw(r16+#92)
    r5 = memw(r16+#20)
}
{
    r15 = sfmpy(r5,r3)
    r14 = sfmpy(r5,r4)
    r28 = memw(r16+#68)
    r6 = memw(r16+#24)
}
{
    r22 = sfmpy(r6,r28)
    r12 = sfmpy(r2,r4)
    r7 = memw(r16+#80)
    r17 = memw(r16+#64)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#0)
    r13 = memw(r16+#28)
}
{
    r12 = sfadd(r15,r12)
    r26 = sfmpy(r2,r17)
    r8 = memw(r16+#84)
    r23 = memw(r16+#4)
}
{
    r19 = sfmpy(r13,r7)
    r24 = sfmpy(r3,r20)
    r14 = memw(r16+#100)
}
{
    r11 = sfmpy(r6,r8)
    r18 = sfmpy(r13,r8)
}
{
    r21 = sfmpy(r13,r28)
    r6 = sfmpy(r6,r17)
}
{
    r13 = sfmpy(r13,r17)
    r25 = sfmpy(r4,r20)
}
{
    r4 = sfmpy(r4,r23)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfmpy(r2,r28)
    r28 = sfmpy(r5,r28)
}
{
    r5 = sfmpy(r5,r17)
    r17 = sfmpy(r7,r20)
}
{
    r20 = sfmpy(r8,r20)
    r7 = sfmpy(r7,r23)
}
{
    r6 = sfsub(r6,r21)
    r8 = sfmpy(r8,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#116)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r13 = sfadd(r13,r22)
    r10 = memw(r16+#112)
}
{
    r4 = sfsub(r24,r4)
    r9 = sfadd(r9,r12)
}
{
    r3 = sfadd(r25,r3)
    r22 = sfmpy(r6,r21)
}
{
    r24 = sfmpy(r10,r13)
    r12 = sfadd(r19,r23)
}
{
    r6 = sfmpy(r10,r6)
    r13 = sfmpy(r13,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r4)
    r6 = sfsub(r6,r13)
}
{
    r4 = sfmpy(r10,r4)
    r10 = sfmpy(r10,r3)
}
{
    r3 = sfmpy(r21,r3)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r2 = sfadd(r5,r2)
    r5 = sfsub(r17,r8)
}
{
    r13 = sfsub(r11,r14)
    r11 = sfadd(r22,r24)
}
{
    r7 = sfadd(r20,r7)
    r8 = sfmpy(r28,r26)
}
{
    r17 = sfmpy(r2,r21)
    r3 = sfsub(r4,r3)
}
{
    r4 = sfadd(r13,r6)
    r28 = sfmpy(r28,r21)
}
{
    r2 = sfmpy(r2,r26)
    r6 = sfadd(r25,r10)
}
{
    r9 = sfadd(r9,r11)
    r20 = sfmpy(r5,r26)
}
{
    r3 = sfsub(r4,r3)
    r2 = sfsub(r28,r2)
}
{
    r5 = sfmpy(r21,r5)
    r26 = sfmpy(r7,r26)
}
{
    r4 = sfmpy(r21,r7)
    r7 = sfadd(r17,r8)
}
{
    r6 = sfsub(r9,r6)
    r2 = sfsub(r3,r2)
}
{
    r5 = sfsub(r5,r26)
    r3 = sfadd(r20,r4)
}
{
    r6 = sfsub(r6,r7)
}
{
    r2 = sfadd(r5,r2)
}
{
    r3 = sfadd(r3,r6)
}
{
    r7 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    memw(r27+#40) = r2.new
}
{
    r2 = sfadd(r6,r7)
    memw(r27+#44) = r2.new
}
{
    r4 = memw(r16+#24)
    r2 = memw(r16+#48)
}
{
    r9 = sfmpy(r4,r2)
    r3 = memw(r16+#52)
    r5 = memw(r16+#28)
}
{
    r15 = sfmpy(r5,r2)
    r14 = sfmpy(r5,r3)
    r28 = memw(r16+#36)
    r6 = memw(r16+#16)
}
{
    r21 = sfmpy(r5,r28)
    r12 = sfmpy(r4,r3)
    r7 = memw(r16+#56)
    r17 = memw(r16+#32)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#0)
    r8 = memw(r16+#60)
}
{
    r12 = sfadd(r15,r12)
    r22 = sfmpy(r4,r28)
    r13 = memw(r16+#20)
    r23 = memw(r16+#4)
}
{
    r11 = sfmpy(r6,r8)
    r4 = sfmpy(r4,r17)
    r14 = memw(r16+#100)
}
{
    r18 = sfmpy(r13,r8)
    r19 = sfmpy(r13,r7)
}
{
    r5 = sfmpy(r5,r17)
    r24 = sfmpy(r7,r20)
}
{
    r25 = sfmpy(r8,r20)
    r8 = sfmpy(r8,r23)
}
{
    r7 = sfmpy(r7,r23)
    r26 = sfmpy(r6,r17)
}
{
    r6 = sfmpy(r6,r28)
    r28 = sfmpy(r13,r28)
}
{
    r13 = sfmpy(r13,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r4 = sfsub(r4,r21)
    r2 = sfmpy(r2,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#116)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r5 = sfadd(r5,r22)
    r10 = memw(r16+#112)
}
{
    r8 = sfsub(r24,r8)
    r7 = sfadd(r25,r7)
}
{
    r9 = sfadd(r9,r12)
    r22 = sfmpy(r4,r21)
}
{
    r24 = sfmpy(r10,r5)
    r12 = sfadd(r19,r23)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfmpy(r5,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r8)
    r6 = sfadd(r13,r6)
}
{
    r8 = sfmpy(r10,r8)
    r10 = sfmpy(r10,r7)
}
{
    r7 = sfmpy(r21,r7)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r4 = sfsub(r4,r5)
    r5 = sfsub(r11,r14)
}
{
    r11 = sfadd(r22,r24)
    r13 = sfmpy(r28,r26)
}
{
    r3 = sfsub(r17,r3)
    r17 = sfmpy(r6,r21)
}
{
    r2 = sfadd(r20,r2)
    r7 = sfsub(r8,r7)
}
{
    r4 = sfsub(r5,r4)
    r5 = sfadd(r25,r10)
}
{
    r10 = sfsub(r9,r11)
    r28 = sfmpy(r28,r21)
}
{
    r6 = sfmpy(r6,r26)
    r20 = sfmpy(r3,r26)
}
{
    r26 = sfmpy(r2,r26)
    r3 = sfmpy(r21,r3)
}
{
    r2 = sfmpy(r21,r2)
    r4 = sfadd(r4,r7)
}
{
    r6 = sfsub(r28,r6)
    r7 = sfadd(r17,r13)
}
{
    r5 = sfadd(r10,r5)
    r3 = sfsub(r3,r26)
}
{
    r4 = sfadd(r6,r4)
    r2 = sfadd(r20,r2)
}
{
    r5 = sfadd(r7,r5)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r5,r2)
}
{
    r4 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#48) = r2.new
}
{
    r2 = sfadd(r5,r4)
    memw(r27+#52) = r2.new
}
{
    r2 = memw(r16+#16)
    r3 = memw(r16+#56)
}
{
    r9 = sfmpy(r2,r3)
    r4 = memw(r16+#60)
    r5 = memw(r16+#20)
}
{
    r15 = sfmpy(r5,r3)
    r14 = sfmpy(r5,r4)
    r28 = memw(r16+#36)
    r6 = memw(r16+#24)
}
{
    r22 = sfmpy(r6,r28)
    r12 = sfmpy(r2,r4)
    r7 = memw(r16+#48)
    r17 = memw(r16+#32)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#0)
    r13 = memw(r16+#28)
}
{
    r12 = sfadd(r15,r12)
    r26 = sfmpy(r2,r17)
    r8 = memw(r16+#52)
    r23 = memw(r16+#4)
}
{
    r19 = sfmpy(r13,r7)
    r24 = sfmpy(r3,r20)
    r14 = memw(r16+#68)
}
{
    r11 = sfmpy(r6,r8)
    r18 = sfmpy(r13,r8)
}
{
    r21 = sfmpy(r13,r28)
    r6 = sfmpy(r6,r17)
}
{
    r13 = sfmpy(r13,r17)
    r25 = sfmpy(r4,r20)
}
{
    r4 = sfmpy(r4,r23)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfmpy(r2,r28)
    r28 = sfmpy(r5,r28)
}
{
    r5 = sfmpy(r5,r17)
    r17 = sfmpy(r7,r20)
}
{
    r20 = sfmpy(r8,r20)
    r7 = sfmpy(r7,r23)
}
{
    r6 = sfsub(r6,r21)
    r8 = sfmpy(r8,r23)
    r23 = memw(r16+#64)
    r21 = memw(r16+#84)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#92)
}
{
    r23 = sfmpy(r23,r10)
    r13 = sfadd(r13,r22)
    r10 = memw(r16+#80)
}
{
    r4 = sfsub(r24,r4)
    r9 = sfadd(r9,r12)
}
{
    r3 = sfadd(r25,r3)
    r22 = sfmpy(r6,r21)
}
{
    r24 = sfmpy(r10,r13)
    r12 = sfadd(r19,r23)
}
{
    r6 = sfmpy(r10,r6)
    r13 = sfmpy(r13,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r4)
    r6 = sfsub(r6,r13)
}
{
    r4 = sfmpy(r10,r4)
    r10 = sfmpy(r10,r3)
}
{
    r3 = sfmpy(r21,r3)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#88)
}
{
    r2 = sfadd(r5,r2)
    r5 = sfsub(r17,r8)
}
{
    r13 = sfsub(r11,r14)
    r11 = sfadd(r22,r24)
}
{
    r7 = sfadd(r20,r7)
    r8 = sfmpy(r28,r26)
}
{
    r17 = sfmpy(r2,r21)
    r3 = sfsub(r4,r3)
}
{
    r4 = sfadd(r13,r6)
    r28 = sfmpy(r28,r21)
}
{
    r2 = sfmpy(r2,r26)
    r6 = sfadd(r25,r10)
}
{
    r9 = sfadd(r9,r11)
    r20 = sfmpy(r5,r26)
}
{
    r3 = sfsub(r4,r3)
    r2 = sfsub(r28,r2)
}
{
    r5 = sfmpy(r21,r5)
    r26 = sfmpy(r7,r26)
}
{
    r4 = sfmpy(r21,r7)
    r7 = sfadd(r17,r8)
}
{
    r6 = sfsub(r9,r6)
    r2 = sfsub(r3,r2)
}
{
    r5 = sfsub(r5,r26)
    r3 = sfadd(r20,r4)
}
{
    r7 = sfsub(r6,r7)
}
{
    r2 = sfadd(r5,r2)
}
{
    r3 = sfadd(r3,r7)
}
{
    r6 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r7 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    memw(r27+#56) = r2.new
}
{
    r2 = sfadd(r7,r6)
    memw(r27+#60) = r2.new
}
{
    r2 = memw(r16+#40)
    r3 = memw(r16+#88)
}
{
    r9 = sfmpy(r2,r3)
    r4 = memw(r16+#92)
    r5 = memw(r16+#44)
}
{
    r15 = sfmpy(r5,r3)
    r14 = sfmpy(r5,r4)
    r28 = memw(r16+#68)
    r6 = memw(r16+#56)
}
{
    r22 = sfmpy(r6,r28)
    r12 = sfmpy(r2,r4)
    r7 = memw(r16+#72)
    r17 = memw(r16+#64)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#32)
    r13 = memw(r16+#60)
}
{
    r12 = sfadd(r15,r12)
    r26 = sfmpy(r2,r17)
    r8 = memw(r16+#76)
    r23 = memw(r16+#36)
}
{
    r19 = sfmpy(r13,r7)
    r24 = sfmpy(r3,r20)
    r14 = memw(r16+#100)
}
{
    r11 = sfmpy(r6,r8)
    r18 = sfmpy(r13,r8)
}
{
    r21 = sfmpy(r13,r28)
    r6 = sfmpy(r6,r17)
}
{
    r13 = sfmpy(r13,r17)
    r25 = sfmpy(r4,r20)
}
{
    r4 = sfmpy(r4,r23)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfmpy(r2,r28)
    r28 = sfmpy(r5,r28)
}
{
    r5 = sfmpy(r5,r17)
    r17 = sfmpy(r7,r20)
}
{
    r20 = sfmpy(r8,r20)
    r7 = sfmpy(r7,r23)
}
{
    r6 = sfsub(r6,r21)
    r8 = sfmpy(r8,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#108)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r13 = sfadd(r13,r22)
    r10 = memw(r16+#104)
}
{
    r4 = sfsub(r24,r4)
    r9 = sfadd(r9,r12)
}
{
    r3 = sfadd(r25,r3)
    r22 = sfmpy(r6,r21)
}
{
    r24 = sfmpy(r10,r13)
    r12 = sfadd(r19,r23)
}
{
    r6 = sfmpy(r10,r6)
    r13 = sfmpy(r13,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r4)
    r6 = sfsub(r6,r13)
}
{
    r4 = sfmpy(r10,r4)
    r10 = sfmpy(r10,r3)
}
{
    r3 = sfmpy(r21,r3)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r2 = sfadd(r5,r2)
    r5 = sfsub(r17,r8)
}
{
    r13 = sfsub(r11,r14)
    r11 = sfadd(r22,r24)
}
{
    r7 = sfadd(r20,r7)
    r8 = sfmpy(r28,r26)
}
{
    r17 = sfmpy(r2,r21)
    r3 = sfsub(r4,r3)
}
{
    r4 = sfadd(r13,r6)
    r28 = sfmpy(r28,r21)
}
{
    r2 = sfmpy(r2,r26)
    r6 = sfadd(r25,r10)
}
{
    r9 = sfadd(r9,r11)
    r20 = sfmpy(r5,r26)
}
{
    r3 = sfsub(r4,r3)
    r2 = sfsub(r28,r2)
}
{
    r5 = sfmpy(r21,r5)
    r26 = sfmpy(r7,r26)
}
{
    r4 = sfmpy(r21,r7)
    r7 = sfadd(r17,r8)
}
{
    r6 = sfsub(r9,r6)
    r2 = sfsub(r3,r2)
}
{
    r5 = sfsub(r5,r26)
    r3 = sfadd(r20,r4)
}
{
    r6 = sfsub(r6,r7)
}
{
    r2 = sfadd(r5,r2)
}
{
    r3 = sfadd(r3,r6)
}
{
    r7 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    memw(r27+#64) = r2.new
}
{
    r2 = sfadd(r6,r7)
    memw(r27+#68) = r2.new
}
{
    r4 = memw(r16+#24)
    r2 = memw(r16+#72)
}
{
    r9 = sfmpy(r4,r2)
    r3 = memw(r16+#76)
    r5 = memw(r16+#28)
}
{
    r15 = sfmpy(r5,r2)
    r14 = sfmpy(r5,r3)
    r28 = memw(r16+#68)
    r6 = memw(r16+#8)
}
{
    r21 = sfmpy(r5,r28)
    r12 = sfmpy(r4,r3)
    r7 = memw(r16+#88)
    r17 = memw(r16+#64)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#0)
    r8 = memw(r16+#92)
}
{
    r12 = sfadd(r15,r12)
    r22 = sfmpy(r4,r28)
    r13 = memw(r16+#12)
    r23 = memw(r16+#4)
}
{
    r11 = sfmpy(r6,r8)
    r4 = sfmpy(r4,r17)
    r14 = memw(r16+#100)
}
{
    r18 = sfmpy(r13,r8)
    r19 = sfmpy(r13,r7)
}
{
    r5 = sfmpy(r5,r17)
    r24 = sfmpy(r7,r20)
}
{
    r25 = sfmpy(r8,r20)
    r8 = sfmpy(r8,r23)
}
{
    r7 = sfmpy(r7,r23)
    r26 = sfmpy(r6,r17)
}
{
    r6 = sfmpy(r6,r28)
    r28 = sfmpy(r13,r28)
}
{
    r13 = sfmpy(r13,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r4 = sfsub(r4,r21)
    r2 = sfmpy(r2,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#108)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r5 = sfadd(r5,r22)
    r10 = memw(r16+#104)
}
{
    r8 = sfsub(r24,r8)
    r7 = sfadd(r25,r7)
}
{
    r9 = sfadd(r9,r12)
    r22 = sfmpy(r4,r21)
}
{
    r24 = sfmpy(r10,r5)
    r12 = sfadd(r19,r23)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfmpy(r5,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r8)
    r6 = sfadd(r13,r6)
}
{
    r8 = sfmpy(r10,r8)
    r10 = sfmpy(r10,r7)
}
{
    r7 = sfmpy(r21,r7)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r4 = sfsub(r4,r5)
    r5 = sfsub(r11,r14)
}
{
    r11 = sfadd(r22,r24)
    r13 = sfmpy(r28,r26)
}
{
    r3 = sfsub(r17,r3)
    r17 = sfmpy(r6,r21)
}
{
    r2 = sfadd(r20,r2)
    r7 = sfsub(r8,r7)
}
{
    r4 = sfsub(r5,r4)
    r5 = sfadd(r25,r10)
}
{
    r10 = sfsub(r9,r11)
    r28 = sfmpy(r28,r21)
}
{
    r6 = sfmpy(r6,r26)
    r20 = sfmpy(r3,r26)
}
{
    r26 = sfmpy(r2,r26)
    r3 = sfmpy(r21,r3)
}
{
    r2 = sfmpy(r21,r2)
    r4 = sfadd(r4,r7)
}
{
    r6 = sfsub(r28,r6)
    r7 = sfadd(r17,r13)
}
{
    r5 = sfadd(r10,r5)
    r3 = sfsub(r3,r26)
}
{
    r4 = sfadd(r6,r4)
    r2 = sfadd(r20,r2)
}
{
    r5 = sfadd(r7,r5)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r5,r2)
}
{
    r4 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#72) = r2.new
}
{
    r2 = sfadd(r5,r4)
    memw(r27+#76) = r2.new
}
{
    r2 = memw(r16+#8)
    r3 = memw(r16+#56)
}
{
    r9 = sfmpy(r2,r3)
    r4 = memw(r16+#60)
    r5 = memw(r16+#12)
}
{
    r15 = sfmpy(r5,r3)
    r14 = sfmpy(r5,r4)
    r28 = memw(r16+#36)
    r6 = memw(r16+#24)
}
{
    r22 = sfmpy(r6,r28)
    r12 = sfmpy(r2,r4)
    r7 = memw(r16+#40)
    r17 = memw(r16+#32)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#0)
    r13 = memw(r16+#28)
}
{
    r12 = sfadd(r15,r12)
    r26 = sfmpy(r2,r17)
    r8 = memw(r16+#44)
    r23 = memw(r16+#4)
}
{
    r19 = sfmpy(r13,r7)
    r24 = sfmpy(r3,r20)
    r14 = memw(r16+#100)
}
{
    r11 = sfmpy(r6,r8)
    r18 = sfmpy(r13,r8)
}
{
    r21 = sfmpy(r13,r28)
    r6 = sfmpy(r6,r17)
}
{
    r13 = sfmpy(r13,r17)
    r25 = sfmpy(r4,r20)
}
{
    r4 = sfmpy(r4,r23)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfmpy(r2,r28)
    r28 = sfmpy(r5,r28)
}
{
    r5 = sfmpy(r5,r17)
    r17 = sfmpy(r7,r20)
}
{
    r20 = sfmpy(r8,r20)
    r7 = sfmpy(r7,r23)
}
{
    r6 = sfsub(r6,r21)
    r8 = sfmpy(r8,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#108)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#124)
}
{
    r23 = sfmpy(r23,r10)
    r13 = sfadd(r13,r22)
    r10 = memw(r16+#104)
}
{
    r4 = sfsub(r24,r4)
    r9 = sfadd(r9,r12)
}
{
    r3 = sfadd(r25,r3)
    r22 = sfmpy(r6,r21)
}
{
    r24 = sfmpy(r10,r13)
    r12 = sfadd(r19,r23)
}
{
    r6 = sfmpy(r10,r6)
    r13 = sfmpy(r13,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r4)
    r6 = sfsub(r6,r13)
}
{
    r4 = sfmpy(r10,r4)
    r10 = sfmpy(r10,r3)
}
{
    r3 = sfmpy(r21,r3)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#120)
}
{
    r2 = sfadd(r5,r2)
    r5 = sfsub(r17,r8)
}
{
    r13 = sfsub(r11,r14)
    r11 = sfadd(r22,r24)
}
{
    r7 = sfadd(r20,r7)
    r8 = sfmpy(r28,r26)
}
{
    r17 = sfmpy(r2,r21)
    r3 = sfsub(r4,r3)
}
{
    r4 = sfadd(r13,r6)
    r28 = sfmpy(r28,r21)
}
{
    r2 = sfmpy(r2,r26)
    r6 = sfadd(r25,r10)
}
{
    r9 = sfadd(r9,r11)
    r20 = sfmpy(r5,r26)
}
{
    r3 = sfsub(r4,r3)
    r2 = sfsub(r28,r2)
}
{
    r5 = sfmpy(r21,r5)
    r26 = sfmpy(r7,r26)
}
{
    r4 = sfmpy(r21,r7)
    r7 = sfadd(r17,r8)
}
{
    r6 = sfsub(r9,r6)
    r2 = sfsub(r3,r2)
}
{
    r5 = sfsub(r5,r26)
    r3 = sfadd(r20,r4)
}
{
    r7 = sfsub(r6,r7)
}
{
    r2 = sfadd(r5,r2)
}
{
    r3 = sfadd(r3,r7)
}
{
    r6 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r7 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    memw(r27+#80) = r2.new
}
{
    r2 = sfadd(r7,r6)
    memw(r27+#84) = r2.new
}
{
    r4 = memw(r16+#24)
    r2 = memw(r16+#40)
}
{
    r9 = sfmpy(r4,r2)
    r3 = memw(r16+#44)
    r5 = memw(r16+#28)
}
{
    r15 = sfmpy(r5,r2)
    r14 = sfmpy(r5,r3)
    r28 = memw(r16+#36)
    r6 = memw(r16+#8)
}
{
    r21 = sfmpy(r5,r28)
    r12 = sfmpy(r4,r3)
    r7 = memw(r16+#56)
    r17 = memw(r16+#32)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#0)
    r8 = memw(r16+#60)
}
{
    r12 = sfadd(r15,r12)
    r22 = sfmpy(r4,r28)
    r13 = memw(r16+#12)
    r23 = memw(r16+#4)
}
{
    r11 = sfmpy(r6,r8)
    r4 = sfmpy(r4,r17)
    r14 = memw(r16+#68)
}
{
    r18 = sfmpy(r13,r8)
    r19 = sfmpy(r13,r7)
}
{
    r5 = sfmpy(r5,r17)
    r24 = sfmpy(r7,r20)
}
{
    r25 = sfmpy(r8,r20)
    r8 = sfmpy(r8,r23)
}
{
    r7 = sfmpy(r7,r23)
    r26 = sfmpy(r6,r17)
}
{
    r6 = sfmpy(r6,r28)
    r28 = sfmpy(r13,r28)
}
{
    r13 = sfmpy(r13,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r4 = sfsub(r4,r21)
    r2 = sfmpy(r2,r23)
    r23 = memw(r16+#64)
    r21 = memw(r16+#76)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#92)
}
{
    r23 = sfmpy(r23,r10)
    r5 = sfadd(r5,r22)
    r10 = memw(r16+#72)
}
{
    r8 = sfsub(r24,r8)
    r7 = sfadd(r25,r7)
}
{
    r9 = sfadd(r9,r12)
    r22 = sfmpy(r4,r21)
}
{
    r24 = sfmpy(r10,r5)
    r12 = sfadd(r19,r23)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfmpy(r5,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r8)
    r6 = sfadd(r13,r6)
}
{
    r8 = sfmpy(r10,r8)
    r10 = sfmpy(r10,r7)
}
{
    r7 = sfmpy(r21,r7)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#88)
}
{
    r4 = sfsub(r4,r5)
    r5 = sfsub(r11,r14)
}
{
    r11 = sfadd(r22,r24)
    r13 = sfmpy(r28,r26)
}
{
    r3 = sfsub(r17,r3)
    r17 = sfmpy(r6,r21)
}
{
    r2 = sfadd(r20,r2)
    r7 = sfsub(r8,r7)
}
{
    r4 = sfsub(r5,r4)
    r5 = sfadd(r25,r10)
}
{
    r10 = sfsub(r9,r11)
    r28 = sfmpy(r28,r21)
}
{
    r6 = sfmpy(r6,r26)
    r20 = sfmpy(r3,r26)
}
{
    r26 = sfmpy(r2,r26)
    r3 = sfmpy(r21,r3)
}
{
    r2 = sfmpy(r21,r2)
    r4 = sfadd(r4,r7)
}
{
    r6 = sfsub(r28,r6)
    r7 = sfadd(r17,r13)
}
{
    r5 = sfadd(r10,r5)
    r3 = sfsub(r3,r26)
}
{
    r4 = sfadd(r6,r4)
    r2 = sfadd(r20,r2)
}
{
    r5 = sfadd(r7,r5)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r5,r2)
}
{
    r4 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#88) = r2.new
}
{
    r2 = sfadd(r5,r4)
    memw(r27+#92) = r2.new
}
{
    r4 = memw(r16+#48)
    r2 = memw(r16+#72)
}
{
    r9 = sfmpy(r4,r2)
    r3 = memw(r16+#76)
    r5 = memw(r16+#52)
}
{
    r15 = sfmpy(r5,r2)
    r14 = sfmpy(r5,r3)
    r28 = memw(r16+#68)
    r6 = memw(r16+#40)
}
{
    r21 = sfmpy(r5,r28)
    r12 = sfmpy(r4,r3)
    r7 = memw(r16+#80)
    r17 = memw(r16+#64)
}
{
    r9 = sfsub(r9,r14)
    r10 = sfmpy(r6,r7)
    r20 = memw(r16+#32)
    r8 = memw(r16+#84)
}
{
    r12 = sfadd(r15,r12)
    r22 = sfmpy(r4,r28)
    r13 = memw(r16+#44)
    r23 = memw(r16+#36)
}
{
    r11 = sfmpy(r6,r8)
    r4 = sfmpy(r4,r17)
    r14 = memw(r16+#100)
}
{
    r18 = sfmpy(r13,r8)
    r19 = sfmpy(r13,r7)
}
{
    r5 = sfmpy(r5,r17)
    r24 = sfmpy(r7,r20)
}
{
    r25 = sfmpy(r8,r20)
    r8 = sfmpy(r8,r23)
}
{
    r7 = sfmpy(r7,r23)
    r26 = sfmpy(r6,r17)
}
{
    r6 = sfmpy(r6,r28)
    r28 = sfmpy(r13,r28)
}
{
    r13 = sfmpy(r13,r17)
    r17 = sfmpy(r2,r20)
}
{
    r20 = sfmpy(r3,r20)
    r3 = sfmpy(r3,r23)
}
{
    r4 = sfsub(r4,r21)
    r2 = sfmpy(r2,r23)
    r23 = memw(r16+#96)
    r21 = memw(r16+#108)
}
{
    r15 = sfsub(r10,r18)
    r18 = sfmpy(r12,r14)
}
{
    r10 = sfadd(r19,r11)
    r11 = sfmpy(r23,r9)
}
{
    r19 = sfmpy(r14,r15)
    r9 = sfmpy(r9,r14)
}
{
    r12 = sfmpy(r23,r12)
    r15 = sfmpy(r23,r15)
}
{
    r14 = sfmpy(r14,r10)
    r28 = sfsub(r26,r28)
    r26 = memw(r16+#116)
}
{
    r23 = sfmpy(r23,r10)
    r5 = sfadd(r5,r22)
    r10 = memw(r16+#104)
}
{
    r8 = sfsub(r24,r8)
    r7 = sfadd(r25,r7)
}
{
    r9 = sfadd(r9,r12)
    r22 = sfmpy(r4,r21)
}
{
    r24 = sfmpy(r10,r5)
    r12 = sfadd(r19,r23)
}
{
    r4 = sfmpy(r10,r4)
    r5 = sfmpy(r5,r21)
}
{
    r11 = sfsub(r11,r18)
    r14 = sfsub(r15,r14)
}
{
    r25 = sfmpy(r21,r8)
    r6 = sfadd(r13,r6)
}
{
    r8 = sfmpy(r10,r8)
    r10 = sfmpy(r10,r7)
}
{
    r7 = sfmpy(r21,r7)
    r9 = sfsub(r9,r12)
    r21 = memw(r16+#112)
}
{
    r4 = sfsub(r4,r5)
    r5 = sfsub(r11,r14)
}
{
    r11 = sfadd(r22,r24)
    r13 = sfmpy(r28,r26)
}
{
    r3 = sfsub(r17,r3)
    r17 = sfmpy(r6,r21)
}
{
    r2 = sfadd(r20,r2)
    r7 = sfsub(r8,r7)
}
{
    r4 = sfsub(r5,r4)
    r6 = sfmpy(r6,r26)
}
{
    r28 = sfmpy(r28,r21)
    r5 = sfadd(r25,r10)
}
{
    r15 = sfsub(r9,r11)
    r20 = sfmpy(r3,r26)
}
{
    r26 = sfmpy(r2,r26)
    r3 = sfmpy(r21,r3)
}
{
    r2 = sfmpy(r21,r2)
    r4 = sfadd(r4,r7)
}
{
    r6 = sfsub(r28,r6)
    r7 = sfadd(r17,r13)
}
{
    r5 = sfadd(r15,r5)
    r3 = sfsub(r3,r26)
}
{
    r4 = sfadd(r6,r4)
    r2 = sfadd(r20,r2)
}
{
    r5 = sfadd(r7,r5)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r5,r2)
}
{
    r4 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r27+#96) = r2.new
}
{
    r2 = sfadd(r5,r4)
    memw(r27+#100) = r2.new
}
{
    r7 = memw(r16+#8)
    r3 = memw(r16+#80)
}
{
    r9 = sfmpy(r7,r3)
    r4 = memw(r16+#84)
    r5 = memw(r16+#12)
}
{
    r15 = sfmpy(r5,r3)
    r2 = sfmpy(r5,r4)
    r6 = memw(r16+#16)
    r0 = memw(r16+#72)
}
{
    r10 = sfmpy(r6,r0)
    r12 = sfmpy(r7,r4)
    r28 = memw(r16+#68)
    r13 = memw(r16+#20)
}
{
    r12 = sfadd(r15,r12)
    r19 = sfmpy(r13,r0)
    r8 = memw(r16+#76)
    r17 = memw(r16+#64)
}
{
    r18 = sfmpy(r13,r8)
    r21 = sfmpy(r13,r28)
    r27 = memw(r16+#108)
    r20 = memw(r16+#0)
}
{
    r15 = sfsub(r10,r18)
    r11 = sfmpy(r6,r8)
    r23 = memw(r16+#4)
    r14 = memw(r16+#96)
}
{
    r22 = sfmpy(r6,r28)
    r6 = sfmpy(r6,r17)
    r26 = memw(r16+#100)
}
{
    r13 = sfmpy(r13,r17)
    r9 = sfsub(r9,r2)
}
{
    r10 = sfadd(r19,r11)
    r18 = sfmpy(r12,r26)
    r19 = memw(r16+#104)
}
{
    r2 = sfmpy(r26,r15)
    r12 = sfmpy(r14,r12)
}
{
    r11 = sfmpy(r14,r9)
    r9 = sfmpy(r9,r26)
}
{
    r15 = sfmpy(r14,r15)
    r1 = sfmpy(r26,r10)
}
{
    r6 = sfsub(r6,r21)
    r13 = sfadd(r13,r22)
}
{
    r14 = sfmpy(r14,r10)
    r24 = sfmpy(r3,r20)
}
{
    r25 = sfmpy(r4,r20)
    r4 = sfmpy(r4,r23)
}
{
    r3 = sfmpy(r3,r23)
    r10 = sfmpy(r6,r27)
}
{
    r21 = sfmpy(r19,r13)
    r9 = sfadd(r9,r12)
}
{
    r6 = sfmpy(r19,r6)
    r13 = sfmpy(r13,r27)
}
{
    r11 = sfsub(r11,r18)
    r12 = sfsub(r15,r1)
}
{
    r2 = sfadd(r2,r14)
    r4 = sfsub(r24,r4)
}
{
    r3 = sfadd(r25,r3)
    r6 = sfsub(r6,r13)
}
{
    r12 = sfsub(r11,r12)
    r14 = sfadd(r10,r21)
    r21 = memw(r29+#156)
}
{
    r2 = sfsub(r9,r2)
    r13 = sfmpy(r27,r4)
}
{
    r9 = sfmpy(r19,r3)
    r10 = sfmpy(r7,r17)
}
{
    r11 = sfmpy(r5,r28)
    r4 = sfmpy(r19,r4)
}
{
    r3 = sfmpy(r27,r3)
    r7 = sfmpy(r7,r28)
}
{
    r5 = sfmpy(r5,r17)
    r6 = sfadd(r12,r6)
    r12 = memw(r16+#116)
}
{
    r3 = sfsub(r4,r3)
    r4 = sfsub(r10,r11)
}
{
    r11 = sfmpy(r8,r23)
    r5 = sfadd(r5,r7)
    r7 = memw(r16+#112)
}
{
    r10 = sfmpy(r0,r20)
    r8 = sfmpy(r8,r20)
}
{
    r23 = sfmpy(r0,r23)
    r2 = sfadd(r2,r14)
}
{
    r9 = sfadd(r13,r9)
    r3 = sfsub(r6,r3)
}
{
    r6 = sfmpy(r4,r12)
    r13 = sfmpy(r5,r7)
}
{
    r8 = sfadd(r8,r23)
    r4 = sfmpy(r4,r7)
    r23:22 = memd(r29+#144)
}
{
    r5 = sfmpy(r5,r12)
    r14 = sfsub(r10,r11)
}
{
    r6 = sfadd(r13,r6)
    r2 = sfsub(r2,r9)
}
{
    r4 = sfsub(r4,r5)
    r13 = sfmpy(r7,r8)
}
{
    r5 = sfmpy(r14,r12)
    r7 = sfmpy(r7,r14)
}
{
    r8 = sfmpy(r8,r12)
    r3 = sfsub(r3,r4)
}
{
    r2 = sfsub(r2,r6)
    r4 = sfadd(r5,r13)
}
{
    r6 = sfsub(r7,r8)
}
{
    r2 = sfadd(r4,r2)
}
{
    r3 = sfadd(r6,r3)
}
{
    r4 = sfmpy(r2,r22)
    r2 = sfmpy(r2,r23)
}
{
    r5 = sfmpy(r3,r23)
    r3 = sfmpy(r3,r22)
}
{
    r2 = sfsub(r3,r2)
    memw(r21+#104) = r2.new
}
{
    r2 = sfadd(r4,r5)
    memw(r21+#108) = r2.new
}
{
    r4 = memw(r16+#16)
    r2 = memw(r16+#40)
}
{
    r6 = sfmpy(r4,r2)
    r5 = memw(r16+#20)
    r3 = memw(r16+#44)
}
{
    r7 = sfmpy(r5,r3)
    r13 = memw(r16+#48)
    r12 = memw(r16+#96)
}
{
    r6 = sfsub(r6,r7)
    r15 = sfmpy(r4,r3)
    r25 = memw(r16+#0)
    r14 = memw(r16+#52)
}
{
    r10 = sfmpy(r5,r2)
    r7 = memw(r16+#8)
    r17 = memw(r16+#4)
}
{
    r24 = sfmpy(r12,r6)
    r9 = memw(r16+#12)
}
{
    r8 = sfadd(r10,r15)
    r0 = sfmpy(r7,r13)
    r15 = memw(r16+#100)
}
{
    r10 = sfmpy(r7,r14)
    r11 = sfmpy(r9,r13)
}
{
    r1 = sfmpy(r9,r14)
    r18 = sfmpy(r8,r15)
}
{
    r0 = sfsub(r0,r1)
    r1 = sfadd(r11,r10)
    r10 = memw(r16+#32)
}
{
    r28 = sfsub(r24,r18)
    r6 = sfmpy(r6,r15)
    r18 = memw(r16+#36)
}
{
    r8 = sfmpy(r12,r8)
    r11 = sfmpy(r15,r0)
}
{
    r20 = sfmpy(r12,r1)
    r12 = sfmpy(r12,r0)
}
{
    r15 = sfmpy(r15,r1)
    r0 = sfmpy(r4,r10)
}
{
    r1 = sfmpy(r5,r18)
    r4 = sfmpy(r4,r18)
}
{
    r5 = sfmpy(r5,r10)
    r6 = sfadd(r6,r8)
}
{
    r8 = sfsub(r12,r15)
    r12 = sfsub(r0,r1)
    r15 = memw(r16+#108)
}
{
    r4 = sfadd(r5,r4)
    r0 = sfadd(r11,r20)
    r5 = memw(r16+#104)
}
{
    r1 = sfmpy(r12,r15)
    r19 = sfmpy(r13,r25)
}
{
    r11 = sfmpy(r5,r4)
    r20 = sfmpy(r14,r17)
}
{
    r12 = sfmpy(r5,r12)
    r4 = sfmpy(r4,r15)
}
{
    r14 = sfmpy(r14,r25)
    r13 = sfmpy(r13,r17)
}
{
    r4 = sfsub(r12,r4)
    r8 = sfsub(r28,r8)
}
{
    r13 = sfadd(r14,r13)
    r12 = sfsub(r19,r20)
}
{
    r6 = sfsub(r6,r0)
    r14 = sfadd(r1,r11)
}
{
    r4 = sfsub(r8,r4)
    r0 = sfmpy(r5,r13)
}
{
    r8 = sfmpy(r15,r12)
    r5 = sfmpy(r5,r12)
}
{
    r12 = sfmpy(r15,r13)
    r13 = sfmpy(r7,r10)
}
{
    r11 = sfmpy(r9,r18)
    r7 = sfmpy(r7,r18)
}
{
    r9 = sfmpy(r9,r10)
    r5 = sfsub(r5,r12)
}
{
    r12 = sfsub(r13,r11)
    r8 = sfadd(r8,r0)
    r13 = memw(r16+#116)
}
{
    r7 = sfadd(r9,r7)
    r11 = sfmpy(r2,r25)
    r9 = memw(r16+#112)
}
{
    r0 = sfmpy(r3,r17)
    r3 = sfmpy(r3,r25)
}
{
    r2 = sfmpy(r2,r17)
    r4 = sfadd(r4,r5)
}
{
    r5 = sfmpy(r12,r13)
    r10 = sfmpy(r7,r9)
}
{
    r6 = sfsub(r6,r14)
    r2 = sfadd(r3,r2)
}
{
    r12 = sfmpy(r12,r9)
    r7 = sfmpy(r7,r13)
}
{
    r15 = sfsub(r11,r0)
    r3 = sfadd(r6,r8)
}
{
    r5 = sfadd(r10,r5)
    r6 = sfsub(r12,r7)
}
{
    r10 = sfmpy(r9,r2)
    r7 = sfmpy(r15,r13)
}
{
    r9 = sfmpy(r9,r15)
    r2 = sfmpy(r2,r13)
}
{
    r4 = sfadd(r6,r4)
    r3 = sfadd(r5,r3)
}
{
    r2 = sfsub(r9,r2)
    r7 = sfadd(r7,r10)
}
{
    r3 = sfsub(r3,r7)
    r2 = sfsub(r4,r2)
}
{
    r6 = sfmpy(r3,r22)
    r7 = sfmpy(r2,r23)
}
{
    r2 = sfmpy(r2,r22)
    r3 = sfmpy(r3,r23)
}
{
    r2 = sfsub(r2,r3)
    memw(r21+#112) = r2.new
}
{
    r2 = sfadd(r6,r7)
    memw(r21+#116) = r2.new
}
{
    r2 = memw(r16+#8)
    r6 = memw(r16+#48)
}
{
    r4 = sfmpy(r2,r6)
    r7 = memw(r16+#52)
    r3 = memw(r16+#12)
}
{
    r5 = sfmpy(r3,r7)
    r14 = memw(r16+#16)
    r12 = memw(r16+#64)
}
{
    r13 = sfsub(r4,r5)
    r11 = sfmpy(r2,r7)
    r9 = memw(r16+#20)
    r15 = memw(r16+#68)
}
{
    r10 = sfmpy(r3,r6)
    r17 = memw(r16+#4)
    r27 = memw(r16+#0)
}
{
    r26 = sfmpy(r12,r13)
    r13 = sfmpy(r13,r15)
    r4 = memw(r16+#40)
    r5 = memw(r16+#44)
}
{
    r8 = sfadd(r10,r11)
}
{
    r0 = sfmpy(r14,r4)
    r11 = sfmpy(r9,r4)
}
{
    r1 = sfmpy(r9,r5)
    r10 = sfmpy(r14,r5)
}
{
    r18 = sfmpy(r8,r15)
    r8 = sfmpy(r12,r8)
}
{
    r0 = sfsub(r0,r1)
    r1 = sfadd(r11,r10)
    r10 = memw(r16+#32)
}
{
    r28 = sfsub(r26,r18)
    r19 = sfmpy(r6,r27)
    r18 = memw(r16+#36)
}
{
    r11 = sfmpy(r15,r0)
    r15 = sfmpy(r15,r1)
}
{
    r20 = sfmpy(r12,r1)
    r12 = sfmpy(r12,r0)
}
{
    r0 = sfmpy(r14,r10)
    r14 = sfmpy(r14,r18)
}
{
    r1 = sfmpy(r9,r18)
    r9 = sfmpy(r9,r10)
}
{
    r12 = sfsub(r12,r15)
    r6 = sfmpy(r6,r17)
    r15 = memw(r16+#76)
}
{
    r8 = sfadd(r13,r8)
    r13 = sfsub(r0,r1)
}
{
    r9 = sfadd(r9,r14)
    r0 = sfadd(r11,r20)
    r14 = memw(r16+#72)
}
{
    r1 = sfmpy(r13,r15)
    r20 = sfmpy(r7,r17)
}
{
    r11 = sfmpy(r14,r9)
    r13 = sfmpy(r14,r13)
}
{
    r9 = sfmpy(r9,r15)
    r12 = sfsub(r28,r12)
}
{
    r7 = sfmpy(r7,r27)
    r8 = sfsub(r8,r0)
}
{
    r9 = sfsub(r13,r9)
    r13 = sfsub(r19,r20)
}
{
    r6 = sfadd(r7,r6)
    r7 = sfadd(r1,r11)
}
{
    r9 = sfadd(r12,r9)
    r12 = sfmpy(r15,r13)
}
{
    r0 = sfmpy(r14,r6)
    r13 = sfmpy(r14,r13)
}
{
    r6 = sfmpy(r15,r6)
    r14 = sfmpy(r2,r10)
}
{
    r11 = sfmpy(r3,r18)
    r2 = sfmpy(r2,r18)
}
{
    r3 = sfmpy(r3,r10)
    r6 = sfsub(r13,r6)
    r13 = memw(r16+#84)
}
{
    r7 = sfadd(r8,r7)
    r8 = sfsub(r14,r11)
}
{
    r2 = sfadd(r3,r2)
    r12 = sfadd(r12,r0)
    r3 = memw(r16+#80)
}
{
    r11 = sfmpy(r4,r27)
    r0 = sfmpy(r5,r17)
}
{
    r5 = sfmpy(r5,r27)
    r4 = sfmpy(r4,r17)
}
{
    r6 = sfsub(r9,r6)
    r9 = sfmpy(r8,r13)
}
{
    r4 = sfadd(r5,r4)
    r10 = sfmpy(r2,r3)
}
{
    r15 = sfsub(r11,r0)
    r8 = sfmpy(r8,r3)
    r0 = #0
}
{
    r2 = sfmpy(r2,r13)
    r5 = sfsub(r7,r12)
}
{
    r14 = sfmpy(r3,r4)
    r7 = sfadd(r10,r9)
}
{
    r2 = sfsub(r8,r2)
    r12 = sfmpy(r15,r13)
}
{
    r3 = sfmpy(r3,r15)
    r4 = sfmpy(r4,r13)
}
{
    r2 = sfsub(r6,r2)
    r5 = sfsub(r5,r7)
}
{
    r3 = sfsub(r3,r4)
    r6 = sfadd(r12,r14)
}
{
    r2 = sfadd(r3,r2)
    r3 = sfadd(r6,r5)
}
{
    r6 = sfmpy(r2,r22)
    r5 = sfmpy(r3,r23)
}
{
    r3 = sfmpy(r3,r22)
    r2 = sfmpy(r2,r23)
}
{
    r4 = sfsub(r6,r5)
    memw(r21+#120) = r4.new
}
{
    r2 = sfadd(r3,r2)
    memw(r21+#124) = r2.new
}
.L_SINGULAR:
{
    r17:16 = memd(r29+#200)
    r19:18 = memd(r29+#192)
}
{
    r21:20 = memd(r29+#184)
    r23:22 = memd(r29+#176)
}
{
    r25:24 = memd(r29+#168)
    r27:26 = memd(r29+#160)
}
{
    r31:30 = dealloc_return(r30):raw
}
    .size   __qhblas_matrix_inverse4x4_acf, .-__qhblas_matrix_inverse4x4_acf
