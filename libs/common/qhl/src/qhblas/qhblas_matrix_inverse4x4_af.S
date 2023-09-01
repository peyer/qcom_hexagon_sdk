/**=============================================================================
@file
    qhblas_matrix_inverse4x4_af.S

@brief
    Calculates matrix inversion for 4x4 matrix of float values.

    Function prototype

        int32_t __qhblas_matrix_inverse4x4_af(float_a8_t *input_matrix, float_a8_t *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse4x4_af(float_a8_t *input_matrix, float_a8_t *output_matrix)
        {
            float det =   A[0]*A[5]*A[10]*A[15] + A[0]*A[6]*A[11]*A[13] + A[0]*A[7]*A[9]*A[14]   \
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
    .hidden	__qhblas_matrix_inverse4x4_af
    .global __qhblas_matrix_inverse4x4_af
    .type __qhblas_matrix_inverse4x4_af, @function

/*============================================================================*/

__qhblas_matrix_inverse4x4_af:
{
    r16 = r0
    memd(r29+#-16) = r17:16
    allocframe(#48)
}
{
    memd(r29+#32) = r19:18
    memd(r29+#24) = r21:20
}
{
    r2 = memw(r0+#0)
    memd(r29+#16) = r23:22
}
{
    r18 = memw(r0+#20)
    memd(r29+#8) = r25:24
}
{
    r20 = memw(r0+#24)
    memd(r29+#0) = r27:26
}
{
    r4 = sfmpy(r2,r20)
    r3 = sfmpy(r2,r18)
    r19 = memw(r0+#40)
    r24 = memw(r0+#28)
}
{
    r5 = sfmpy(r2,r24)
    r22 = memw(r0+#44)
    r21 = memw(r0+#36)
}
{
    r8 = sfmpy(r4,r22)
    r2 = sfmpy(r3,r19)
    r7 = memw(r0+#16)
    r6 = memw(r0+#4)
}
{
    r9 = sfmpy(r5,r21)
    r3 = sfmpy(r3,r22)
    r23 = memw(r0+#60)
    r26 = memw(r0+#52)
}
{
    r13 = sfmpy(r2,r23)
    r12 = sfmpy(r6,r7)
    r25 = memw(r0+#56)
    r28 = memw(r0+#8)
}
{
    r15 = sfmpy(r20,r6)
    r8 = sfmpy(r8,r26)
    r2 = memw(r0+#32)
    r10 = memw(r16+#48)
}
{
    r14 = sfmpy(r22,r12)
    r9 = sfmpy(r9,r25)
    r17 = memw(r16+#12)
}
{
    r8 = sfadd(r13,r8)
    r6 = sfmpy(r24,r6)
}
{
    r13 = sfmpy(r25,r14)
    r11 = sfmpy(r15,r2)
}
{
    r8 = sfadd(r8,r9)
    r14 = sfmpy(r7,r28)
}
{
    r0 = sfmpy(r19,r6)
    r9 = sfmpy(r23,r11)
}
{
    r8 = sfadd(r8,r13)
    r11 = sfmpy(r18,r28)
}
{
    r13 = sfmpy(r21,r14)
    r28 = sfmpy(r24,r28)
}
{
    r8 = sfadd(r9,r8)
    r0 = sfmpy(r0,r10)
}
{
    r9 = sfmpy(r23,r13)
    r13 = sfmpy(r22,r11)
}
{
    r8 = sfadd(r0,r8)
    r7 = sfmpy(r7,r17)
}
{
    r13 = sfmpy(r10,r13)
    r4 = sfmpy(r4,r21)
}
{
    r8 = sfadd(r8,r9)
    r0 = sfmpy(r2,r28)
}
{
    r9 = sfmpy(r19,r7)
    r27 = sfmpy(r18,r17)
}
{
    r8 = sfadd(r13,r8)
    r0 = sfmpy(r26,r0)
}
{
    r13 = sfmpy(r2,r27)
    r9 = sfmpy(r26,r9)
}
{
    r8 = sfadd(r0,r8)
    r17 = sfmpy(r20,r17)
}
{
    r13 = sfmpy(r25,r13)
    r3 = sfmpy(r3,r25)
}
{
    r8 = sfadd(r9,r8)
    r0 = sfmpy(r21,r17)
}
{
    r5 = sfmpy(r19,r5)
    r4 = sfmpy(r23,r4)
}
{
    r9 = sfmpy(r10,r0)
    r8 = sfadd(r13,r8)
    r0 = #-1
}
{
    r5 = sfmpy(r26,r5)
    r13 = sfmpy(r19,r12)
}
{
    r8 = sfadd(r9,r8)
}
{
    r3 = sfsub(r8,r3)
    r8 = sfmpy(r23,r13)
}
{
    r3 = sfsub(r3,r4)
    r4 = sfmpy(r22,r15)
}
{
    r3 = sfsub(r3,r5)
    r5 = sfmpy(r6,r2)
}
{
    r4 = sfmpy(r4,r10)
    r6 = sfmpy(r22,r14)
}
{
    r3 = sfsub(r3,r8)
    r5 = sfmpy(r25,r5)
}
{
    r3 = sfsub(r3,r4)
    r4 = sfmpy(r26,r6)
}
{
    r6 = sfmpy(r2,r11)
    r2 = sfmpy(r2,r17)
}
{
    r3 = sfsub(r3,r5)
    r5 = sfmpy(r21,r28)
    r17 = r1
}
{
    r6 = sfmpy(r23,r6)
    r2 = sfmpy(r26,r2)
}
{
    r3 = sfsub(r3,r4)
    r4 = sfmpy(r10,r5)
}
{
    r5 = sfmpy(r21,r7)
}
{
    r3 = sfsub(r3,r6)
    r6 = sfmpy(r19,r27)
}
{
    r5 = sfmpy(r25,r5)
}
{
    r3 = sfsub(r3,r4)
    r6 = sfmpy(r10,r6)
}
{
    r3 = sfsub(r3,r5)
}
{
    r3 = sfsub(r3,r6)
}
{
    r2 = sfsub(r3,r2)
    r3 = #0
}
{
    p0 = sfcmp.uo(r2,r3)
    p1 = sfcmp.eq(r2,r3)
}
{
    p0 = and(p1,!p0)
    if (p0.new) jump:t .L_SINGULAR
}
// %bb.1:                               // %if.end
{
    call qhmath_div_f
    r1:0 = combine(r2,##1065353216)
}
{
    r2 = sfmpy(r20,r22)
    r3 = sfmpy(r19,r24)
}
{
    r4 = sfmpy(r24,r21)
    r5 = sfmpy(r18,r22)
}
{
    r2 = sfmpy(r2,r26)
    r3 = sfmpy(r26,r3)
}
{
    r4 = sfmpy(r4,r25)
    r5 = sfmpy(r5,r25)
}
{
    r2 = sfsub(r2,r3)
    r3 = sfmpy(r20,r21)
}
{
    r6 = sfmpy(r18,r19)
}
{
    r2 = sfadd(r2,r4)
    r3 = sfmpy(r23,r3)
}
{
    r4 = sfmpy(r6,r23)
}
{
    r2 = sfsub(r2,r5)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfmpy(r2,r0)
    memw(r17+#0) = r2.new
}
{
    r2 = memw(r16+#12)
    r6 = memw(r16+#36)
}
{
    r3 = memw(r16+#40)
    r7 = memw(r16+#52)
}
{
    r8 = sfmpy(r2,r3)
    r4 = memw(r16+#8)
    r12 = memw(r16+#4)
}
{
    r2 = sfmpy(r2,r6)
    r5 = memw(r16+#44)
    r13 = memw(r16+#56)
}
{
    r3 = sfmpy(r3,r12)
    r8 = sfmpy(r8,r7)
}
{
    r9 = sfmpy(r4,r5)
    r5 = sfmpy(r5,r12)
}
{
    r2 = sfmpy(r2,r13)
}
{
    r7 = sfmpy(r7,r9)
    r4 = sfmpy(r4,r6)
    r6 = memw(r16+#60)
}
{
    r5 = sfmpy(r13,r5)
}
{
    r7 = sfsub(r8,r7)
    r4 = sfmpy(r4,r6)
}
{
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#4) = r2.new
}
{
    r2 = memw(r16+#8)
    r3 = memw(r16+#28)
}
{
    r11 = sfmpy(r2,r3)
    r5 = memw(r16+#24)
    r4 = memw(r16+#12)
}
{
    r14 = sfmpy(r4,r5)
    r6 = memw(r16+#20)
    r15 = memw(r16+#4)
}
{
    r2 = sfmpy(r2,r6)
    r7 = memw(r16+#52)
    r10 = memw(r16+#56)
}
{
    r4 = sfmpy(r4,r6)
    r3 = sfmpy(r3,r15)
    r6 = memw(r16+#60)
}
{
    r8 = sfmpy(r11,r7)
    r7 = sfmpy(r7,r14)
}
{
    r4 = sfmpy(r4,r10)
    r3 = sfmpy(r10,r3)
}
{
    r7 = sfsub(r8,r7)
    r5 = sfmpy(r5,r15)
}
{
    r2 = sfmpy(r2,r6)
}
{
    r4 = sfadd(r7,r4)
    r7 = sfmpy(r5,r6)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
}
{
    r2 = sfadd(r7,r2)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#8) = r2.new
}
{
    r2 = memw(r16+#12)
    r3 = memw(r16+#24)
}
{
    r11 = sfmpy(r2,r3)
    r4 = memw(r16+#8)
    r5 = memw(r16+#28)
}
{
    r14 = sfmpy(r4,r5)
    r7 = memw(r16+#36)
    r6 = memw(r16+#20)
}
{
    r4 = sfmpy(r4,r6)
    r15 = memw(r16+#4)
    r10 = memw(r16+#40)
}
{
    r2 = sfmpy(r2,r6)
    r8 = sfmpy(r11,r7)
    r6 = memw(r16+#44)
}
{
    r7 = sfmpy(r7,r14)
    r5 = sfmpy(r5,r15)
}
{
    r2 = sfmpy(r2,r10)
    r3 = sfmpy(r3,r15)
}
{
    r7 = sfsub(r8,r7)
    r5 = sfmpy(r10,r5)
}
{
    r4 = sfmpy(r4,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#12) = r2.new
}
{
    r2 = memw(r16+#28)
    r3 = memw(r16+#40)
}
{
    r11 = sfmpy(r2,r3)
    r4 = memw(r16+#24)
    r5 = memw(r16+#44)
}
{
    r14 = sfmpy(r4,r5)
    r7 = memw(r16+#48)
    r6 = memw(r16+#32)
}
{
    r4 = sfmpy(r4,r6)
    r15 = memw(r16+#16)
    r10 = memw(r16+#56)
}
{
    r2 = sfmpy(r2,r6)
    r8 = sfmpy(r11,r7)
    r6 = memw(r16+#60)
}
{
    r7 = sfmpy(r7,r14)
    r5 = sfmpy(r5,r15)
}
{
    r2 = sfmpy(r2,r10)
    r3 = sfmpy(r3,r15)
}
{
    r7 = sfsub(r8,r7)
    r5 = sfmpy(r10,r5)
}
{
    r4 = sfmpy(r4,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#16) = r2.new
}
{
    r2 = memw(r16+#8)
    r6 = memw(r16+#32)
}
{
    r3 = memw(r16+#44)
    r7 = memw(r16+#48)
}
{
    r11 = sfmpy(r2,r3)
    r4 = memw(r16+#12)
    r15 = memw(r16+#0)
}
{
    r2 = sfmpy(r2,r6)
    r5 = memw(r16+#40)
    r10 = memw(r16+#56)
}
{
    r3 = sfmpy(r3,r15)
    r8 = sfmpy(r11,r7)
}
{
    r14 = sfmpy(r4,r5)
    r4 = sfmpy(r4,r6)
    r6 = memw(r16+#60)
}
{
    r3 = sfmpy(r10,r3)
    r5 = sfmpy(r5,r15)
}
{
    r7 = sfmpy(r7,r14)
    r4 = sfmpy(r4,r10)
}
{
    r2 = sfmpy(r2,r6)
}
{
    r7 = sfsub(r8,r7)
}
{
    r6 = sfmpy(r5,r6)
}
{
    r4 = sfadd(r7,r4)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
}
{
    r2 = sfadd(r6,r2)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#20) = r2.new
}
{
    r2 = memw(r16+#12)
    r3 = memw(r16+#24)
}
{
    r11 = sfmpy(r2,r3)
    r5 = memw(r16+#28)
    r4 = memw(r16+#8)
}
{
    r14 = sfmpy(r4,r5)
    r6 = memw(r16+#16)
    r15 = memw(r16+#0)
}
{
    r4 = sfmpy(r4,r6)
    r7 = memw(r16+#48)
    r10 = memw(r16+#56)
}
{
    r2 = sfmpy(r2,r6)
    r5 = sfmpy(r5,r15)
    r6 = memw(r16+#60)
}
{
    r8 = sfmpy(r11,r7)
    r7 = sfmpy(r7,r14)
}
{
    r2 = sfmpy(r2,r10)
    r5 = sfmpy(r10,r5)
}
{
    r7 = sfsub(r8,r7)
    r3 = sfmpy(r3,r15)
}
{
    r4 = sfmpy(r4,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#24) = r2.new
}
{
    r2 = memw(r16+#8)
    r3 = memw(r16+#28)
}
{
    r11 = sfmpy(r2,r3)
    r4 = memw(r16+#12)
    r5 = memw(r16+#24)
}
{
    r14 = sfmpy(r4,r5)
    r7 = memw(r16+#32)
    r6 = memw(r16+#16)
}
{
    r2 = sfmpy(r2,r6)
    r15 = memw(r16+#0)
    r10 = memw(r16+#40)
}
{
    r4 = sfmpy(r4,r6)
    r8 = sfmpy(r11,r7)
    r6 = memw(r16+#44)
}
{
    r7 = sfmpy(r7,r14)
    r3 = sfmpy(r3,r15)
}
{
    r4 = sfmpy(r4,r10)
    r5 = sfmpy(r5,r15)
}
{
    r7 = sfsub(r8,r7)
    r3 = sfmpy(r10,r3)
}
{
    r2 = sfmpy(r2,r6)
}
{
    r4 = sfadd(r7,r4)
    r7 = sfmpy(r5,r6)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
}
{
    r2 = sfadd(r7,r2)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#28) = r2.new
}
{
    r2 = memw(r16+#20)
    r3 = memw(r16+#44)
}
{
    r11 = sfmpy(r2,r3)
    r5 = memw(r16+#36)
    r4 = memw(r16+#28)
}
{
    r7 = memw(r16+#48)
    r6 = memw(r16+#32)
}
{
    r8 = sfmpy(r11,r7)
    r14 = sfmpy(r4,r5)
    r15 = memw(r16+#16)
    r10 = memw(r16+#52)
}
{
    r4 = sfmpy(r4,r6)
    r11 = memw(r16+#60)
}
{
    r7 = sfmpy(r7,r14)
    r3 = sfmpy(r3,r15)
}
{
    r4 = sfmpy(r4,r10)
    r2 = sfmpy(r2,r6)
}
{
    r7 = sfsub(r8,r7)
    r3 = sfmpy(r10,r3)
}
{
    r5 = sfmpy(r5,r15)
    r2 = sfmpy(r2,r11)
}
{
    r4 = sfadd(r7,r4)
}
{
    r6 = sfmpy(r5,r11)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
}
{
    r2 = sfadd(r6,r2)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#32) = r2.new
}
{
    r2 = memw(r16+#12)
    r3 = memw(r16+#36)
}
{
    r14 = sfmpy(r2,r3)
    r4 = memw(r16+#4)
    r5 = memw(r16+#44)
}
{
    r15 = sfmpy(r4,r5)
    r6 = memw(r16+#32)
    r7 = memw(r16+#48)
}
{
    r2 = sfmpy(r2,r6)
    r10 = memw(r16+#0)
    r11 = memw(r16+#52)
}
{
    r8 = sfmpy(r14,r7)
    r7 = sfmpy(r7,r15)
    r14 = memw(r16+#60)
}
{
    r5 = sfmpy(r5,r10)
    r4 = sfmpy(r4,r6)
}
{
    r2 = sfmpy(r2,r11)
    r7 = sfsub(r8,r7)
}
{
    r5 = sfmpy(r11,r5)
    r3 = sfmpy(r3,r10)
}
{
    r4 = sfmpy(r4,r14)
}
{
    r2 = sfsub(r7,r2)
}
{
    r3 = sfmpy(r3,r14)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#36) = r2.new
}
{
    r2 = memw(r16+#4)
    r3 = memw(r16+#28)
}
{
    r15 = sfmpy(r2,r3)
    r5 = memw(r16+#20)
    r4 = memw(r16+#12)
}
{
    r6 = memw(r16+#16)
    r11 = memw(r16+#0)
}
{
    r10 = sfmpy(r4,r5)
    r14 = memw(r16+#52)
    r7 = memw(r16+#48)
}
{
    r4 = sfmpy(r4,r6)
    r3 = sfmpy(r3,r11)
}
{
    r8 = sfmpy(r15,r7)
    r7 = sfmpy(r7,r10)
    r15 = memw(r16+#60)
}
{
    r4 = sfmpy(r4,r14)
    r2 = sfmpy(r2,r6)
}
{
    r7 = sfsub(r8,r7)
    r3 = sfmpy(r14,r3)
}
{
    r2 = sfmpy(r2,r15)
    r5 = sfmpy(r5,r11)
}
{
    r4 = sfadd(r7,r4)
}
{
    r7 = sfmpy(r5,r15)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
}
{
    r2 = sfadd(r7,r2)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#40) = r2.new
}
{
    r3 = memw(r16+#20)
    r2 = memw(r16+#12)
}
{
    r10 = sfmpy(r2,r3)
    r4 = memw(r16+#4)
    r5 = memw(r16+#28)
}
{
    r7 = memw(r16+#32)
    r6 = memw(r16+#16)
}
{
    r8 = sfmpy(r10,r7)
    r11 = sfmpy(r4,r5)
    r14 = memw(r16+#0)
    r15 = memw(r16+#36)
}
{
    r4 = sfmpy(r4,r6)
    r2 = sfmpy(r2,r6)
    r6 = memw(r16+#44)
}
{
    r7 = sfmpy(r7,r11)
    r5 = sfmpy(r5,r14)
}
{
    r2 = sfmpy(r2,r15)
    r3 = sfmpy(r3,r14)
}
{
    r7 = sfsub(r8,r7)
    r5 = sfmpy(r15,r5)
}
{
    r4 = sfmpy(r4,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#44) = r2.new
}
{
    r3 = memw(r16+#36)
    r2 = memw(r16+#24)
}
{
    r10 = sfmpy(r2,r3)
    r4 = memw(r16+#20)
    r5 = memw(r16+#40)
}
{
    r11 = sfmpy(r4,r5)
    r7 = memw(r16+#48)
    r6 = memw(r16+#32)
}
{
    r4 = sfmpy(r4,r6)
    r14 = memw(r16+#52)
    r15 = memw(r16+#16)
}
{
    r2 = sfmpy(r2,r6)
    r8 = sfmpy(r10,r7)
    r6 = memw(r16+#56)
}
{
    r7 = sfmpy(r7,r11)
    r5 = sfmpy(r5,r15)
}
{
    r2 = sfmpy(r2,r14)
    r3 = sfmpy(r3,r15)
}
{
    r7 = sfsub(r8,r7)
    r5 = sfmpy(r14,r5)
}
{
    r4 = sfmpy(r4,r6)
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#48) = r2.new
}
{
    r2 = memw(r16+#4)
    r3 = memw(r16+#40)
}
{
    r10 = sfmpy(r2,r3)
    r5 = memw(r16+#36)
    r4 = memw(r16+#8)
}
{
    r11 = sfmpy(r4,r5)
    r6 = memw(r16+#32)
    r7 = memw(r16+#48)
}
{
    r2 = sfmpy(r2,r6)
    r15 = memw(r16+#0)
    r14 = memw(r16+#52)
}
{
    r4 = sfmpy(r4,r6)
    r8 = sfmpy(r10,r7)
    r6 = memw(r16+#56)
}
{
    r7 = sfmpy(r7,r11)
    r3 = sfmpy(r3,r15)
}
{
    r4 = sfmpy(r4,r14)
    r5 = sfmpy(r5,r15)
}
{
    r7 = sfsub(r8,r7)
    r3 = sfmpy(r14,r3)
}
{
    r2 = sfmpy(r2,r6)
}
{
    r4 = sfadd(r7,r4)
}
{
    r6 = sfmpy(r5,r6)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
}
{
    r2 = sfadd(r6,r2)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#52) = r2.new
}
{
    r3 = memw(r16+#20)
    r2 = memw(r16+#8)
}
{
    r10 = sfmpy(r2,r3)
    r4 = memw(r16+#4)
    r5 = memw(r16+#24)
}
{
    r11 = sfmpy(r4,r5)
    r6 = memw(r16+#16)
    r15 = memw(r16+#0)
}
{
    r4 = sfmpy(r4,r6)
    r7 = memw(r16+#48)
    r14 = memw(r16+#52)
}
{
    r2 = sfmpy(r2,r6)
    r5 = sfmpy(r5,r15)
    r6 = memw(r16+#56)
}
{
    r8 = sfmpy(r10,r7)
    r7 = sfmpy(r7,r11)
}
{
    r2 = sfmpy(r2,r14)
    r5 = sfmpy(r14,r5)
}
{
    r7 = sfsub(r8,r7)
    r3 = sfmpy(r3,r15)
}
{
    r4 = sfmpy(r4,r6)
}
{
    r2 = sfsub(r7,r2)
}
{
    r3 = sfmpy(r3,r6)
}
{
    r2 = sfadd(r2,r5)
}
{
    r2 = sfadd(r4,r2)
}
{
    r2 = sfsub(r2,r3)
}
{
    r2 = sfmpy(r0,r2)
    memw(r17+#56) = r2.new
}
{
    r2 = memw(r16+#4)
    r3 = memw(r16+#24)
}
{
    r10 = sfmpy(r2,r3)
    r5 = memw(r16+#20)
    r4 = memw(r16+#8)
}
{
    r7 = memw(r16+#32)
    r6 = memw(r16+#16)
}
{
    r2 = sfmpy(r2,r6)
    r11 = sfmpy(r4,r5)
    r15 = memw(r16+#0)
    r14 = memw(r16+#36)
}
{
    r4 = sfmpy(r4,r6)
    r8 = sfmpy(r10,r7)
    r6 = memw(r16+#40)
}
{
    r7 = sfmpy(r7,r11)
    r3 = sfmpy(r3,r15)
}
{
    r4 = sfmpy(r4,r14)
}
{
    r7 = sfsub(r8,r7)
    r2 = sfmpy(r2,r6)
}
{
    r3 = sfmpy(r14,r3)
}
{
    r4 = sfadd(r7,r4)
    r7 = sfmpy(r5,r15)
}
{
    r3 = sfsub(r4,r3)
}
{
    r2 = sfsub(r3,r2)
    r3 = sfmpy(r7,r6)
}
{
    r2 = sfadd(r3,r2)
}
{
    r2 = sfmpy(r0,r2)
    r0 = #0
    memw(r17+#60) = r2.new
}
    .falign
.L_SINGULAR:                                // %cleanup
{
    r17:16 = memd(r29+#40)
    r19:18 = memd(r29+#32)
}
{
    r21:20 = memd(r29+#24)
    r23:22 = memd(r29+#16)
}
{
    r25:24 = memd(r29+#8)
    r27:26 = memd(r29+#0)
}
{
    dealloc_return
}
.size   __qhblas_matrix_inverse4x4_af, .-__qhblas_matrix_inverse4x4_af
