/**=============================================================================
@file
    qhblas_matrix_inverse3x3_acf.S

@brief
    Calculates matrix inversion for 3x3 matrix of complex float values.

    Function prototype

        int32_t __qhblas_matrix_inverse3x3_acf(complex float *input_matrix, complex float *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse3x3_acf(complex float *input_matrix, complex float *output_matrix)
        {
            complex float det = A[0]*A[4]*A[8] - A[0]*A[5]*A[7] - A[1]*A[3]*A[8] + A[1]*A[5]*A[6] + A[2]*A[3]*A[7] - A[2]*A[4]*A[6];
            if (det == 0)
                return -1;

            det = 1/det;

            B[0] = det * (A[4]*A[8] - A[5]*A[7]);
            B[1] = -det * (A[1]*A[8] - A[2]*A[7]);
            B[2] = det * (A[1]*A[5] - A[2]*A[4]);
            B[3] = -det * (A[3]*A[8] - A[5]*A[6]);
            B[4] = det * (A[0]*A[8] - A[2]*A[6]);
            B[5] = -det * (A[0]*A[5] - A[2]*A[3]);
            B[6] = det * (A[3]*A[7] - A[4]*A[6]);
            B[7] = -det * (A[0]*A[7] - A[1]*A[6]);
            B[8] = det * (A[0]*A[4] - A[1]*A[3]);

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .hidden	__qhblas_matrix_inverse3x3_acf
    .global __qhblas_matrix_inverse3x3_acf
    .type __qhblas_matrix_inverse3x3_acf, @function

/*============================================================================*/

__qhblas_matrix_inverse3x3_acf:
{
    r16 = r0
    memd(r29+#-16) = r17:16
    allocframe(#40)
}
{
    r5 = memw(r0+#0)
    memd(r29+#24) = r19:18
}
{
    r6 = memw(r0+#4)
    memd(r29+#16) = r21:20
}
{
    r2 = memw(r0+#32)
    memd(r29+#8) = r23:22
}
{
    r3 = memw(r0+#36)
    memd(r29+#0) = r25:24
}
{
    r28 = sfmpy(r5,r3)
    r13 = sfmpy(r5,r2)
    r12 = memw(r0+#40)
    r8 = memw(r0+#64)
}
{
    r14 = sfmpy(r6,r3)
    r0 = sfmpy(r6,r2)
    r15 = memw(r0+#44)
    r9 = memw(r0+#68)
}
{
    r28 = sfadd(r0,r28)
    r20 = sfmpy(r6,r12)
    r7 = memw(r16+#56)
    r10 = memw(r16+#8)
}
{
    r13 = sfsub(r13,r14)
    r19 = sfmpy(r5,r15)
    r4 = memw(r16+#60)
    r11 = memw(r16+#12)
}
{
    r17 = sfmpy(r6,r15)
    r14 = sfmpy(r5,r12)
    r0 = memw(r16+#24)
    r5 = memw(r16+#48)
}
{
    r21 = sfmpy(r10,r0)
    r25 = sfmpy(r15,r11)
    r18 = memw(r16+#28)
    r6 = memw(r16+#52)
}
{
    r19 = sfadd(r20,r19)
    r14 = sfsub(r14,r17)
    r20 = memw(r16+#16)
}
{
    r22 = sfmpy(r11,r18)
    r23 = sfmpy(r10,r18)
}
{
    r15 = sfmpy(r15,r10)
    r10 = sfmpy(r12,r10)
}
{
    r12 = sfmpy(r12,r11)
    r24 = sfmpy(r11,r0)
}
{
    r11 = sfsub(r21,r22)
    r21 = sfmpy(r8,r13)
}
{
    r22 = sfmpy(r28,r9)
    r12 = sfadd(r15,r12)
}
{
    r17 = sfadd(r24,r23)
    r15 = sfmpy(r14,r4)
    r24 = memw(r16+#20)
}
{
    r21 = sfsub(r21,r22)
    r14 = sfmpy(r7,r14)
}
{
    r22 = sfmpy(r7,r19)
    r13 = sfmpy(r13,r9)
}
{
    r19 = sfmpy(r19,r4)
    r23 = sfmpy(r9,r11)
}
{
    r28 = sfmpy(r8,r28)
    r11 = sfmpy(r8,r11)
}
{
    r14 = sfsub(r14,r19)
    r9 = sfmpy(r9,r17)
}
{
    r10 = sfsub(r10,r25)
    r13 = sfadd(r13,r28)
}
{
    r8 = sfmpy(r8,r17)
    r17 = sfmpy(r0,r20)
}
{
    r25 = sfmpy(r18,r20)
    r0 = sfmpy(r0,r24)
}
{
    r15 = sfadd(r15,r22)
    r28 = sfmpy(r10,r6)
}
{
    r22 = sfmpy(r12,r5)
    r9 = sfsub(r11,r9)
}
{
    r18 = sfmpy(r18,r24)
    r14 = sfsub(r21,r14)
}
{
    r10 = sfmpy(r10,r5)
    r12 = sfmpy(r12,r6)
}
{
    r8 = sfadd(r23,r8)
    r13 = sfsub(r13,r15)
}
{
    r11 = sfsub(r17,r18)
    r12 = sfsub(r10,r12)
}
{
    r9 = sfsub(r14,r9)
    r14 = sfmpy(r2,r20)
    r17 = r1
}
{
    r10 = sfmpy(r3,r24)
    r3 = sfmpy(r3,r20)
}
{
    r0 = sfadd(r25,r0)
    r2 = sfmpy(r2,r24)
}
{
    r15 = sfmpy(r4,r11)
    r28 = sfadd(r22,r28)
}
{
    r8 = sfsub(r13,r8)
    r13 = sfmpy(r7,r0)
}
{
    r7 = sfmpy(r7,r11)
    r4 = sfmpy(r4,r0)
    r0 = #-1
}
{
    r11 = sfsub(r14,r10)
    r2 = sfadd(r3,r2)
}
{
    r14 = sfadd(r15,r13)
    r3 = sfadd(r28,r8)
}
{
    r4 = sfsub(r7,r4)
    r15 = sfmpy(r5,r2)
}
{
    r9 = sfadd(r12,r9)
    r7 = sfmpy(r6,r11)
}
{
    r2 = sfmpy(r6,r2)
    r5 = sfmpy(r5,r11)
}
{
    r4 = sfadd(r9,r4)
    r3 = sfadd(r3,r14)
}
{
    r6 = sfadd(r7,r15)
}
{
    r2 = sfsub(r5,r2)
}
{
    r3 = sfsub(r3,r6)
    r6 = #0
}
{
    r2 = sfsub(r4,r2)
}
{
    p1 = sfcmp.eq(r3,r6)
}
{
    p0 = sfcmp.eq(r2,r6)
}
{
    p0 = and(p0,p1)
    if (p0.new) jump:nt .L_SINGULAR
}
{
    call __divsc3
    r1:0 = combine(#0,##1065353216)
}
{
    r2 = memw(r16+#32)
    r6 = memw(r16+#40)
}
{
    r3 = memw(r16+#36)
    r7 = memw(r16+#56)
}
{
    r13 = sfmpy(r6,r7)
    r4 = memw(r16+#64)
    r8 = memw(r16+#44)
}
{
    r9 = memw(r16+#60)
    r5 = memw(r16+#68)
}
{
    r12 = sfmpy(r2,r4)
}
{
    r2 = sfmpy(r2,r5)
    r5 = sfmpy(r3,r5)
}
{
    r3 = sfmpy(r3,r4)
    r4 = sfmpy(r6,r9)
}
{
    r14 = sfmpy(r8,r9)
    r6 = sfmpy(r8,r7)
}
{
    r2 = sfadd(r3,r2)
    r5 = sfsub(r12,r5)
}
{
    r7 = sfsub(r13,r14)
    r3 = sfadd(r6,r4)
}
{
    r7 = sfsub(r5,r7)
    r2 = sfsub(r2,r3)
}
{
    r6 = sfmpy(r2,r0)
    r4 = sfmpy(r7,r0)
}
{
    r2 = sfmpy(r2,r1)
    r3 = sfmpy(r7,r1)
}
{
    r2 = sfsub(r4,r2)
    memw(r17+#0) = r2.new
}
{
    r2 = sfadd(r6,r3)
    memw(r17+#4) = r2.new
}
{
    r2 = memw(r16+#8)
    r4 = memw(r16+#64)
}
{
    r12 = sfmpy(r2,r4)
    r6 = memw(r16+#16)
    r7 = memw(r16+#56)
}
{
    r10 = sfmpy(r6,r7)
    r3 = memw(r16+#12)
    r5 = memw(r16+#68)
}
{
    r8 = memw(r16+#20)
    r9 = memw(r16+#60)
}
{
    r2 = sfmpy(r2,r5)
    r5 = sfmpy(r3,r5)
}
{
    r3 = sfmpy(r3,r4)
    r4 = sfmpy(r6,r9)
}
{
    r6 = sfmpy(r8,r7)
    r11 = sfmpy(r8,r9)
}
{
    r3 = sfadd(r3,r2)
    r5 = sfsub(r12,r5)
}
{
    r7 = sfsub(r10,r11)
    r4 = sfadd(r6,r4)
}
{
    r2 = togglebit(r0,#31)
}
{
    r3 = sfsub(r3,r4)
    r5 = sfsub(r5,r7)
}
{
    r4 = sfmpy(r5,r1)
    r6 = sfmpy(r3,r1)
}
{
    r5 = sfmpy(r5,r0)
    r3 = sfmpy(r3,r2)
}
{
    r5 = sfsub(r6,r5)
    r3 = sfsub(r3,r4)
    memw(r17+#8) = r5.new
}
{
    memw(r17+#12) = r3
    r3 = memw(r16+#8)

} :mem_noshuf
{
    r7 = memw(r16+#16)
    r4 = memw(r16+#12)
}
{
    r8 = memw(r16+#32)
    r5 = memw(r16+#40)
}
{
    r11 = sfmpy(r7,r8)
    r9 = memw(r16+#20)
    r12 = memw(r16+#36)
}
{
    r10 = sfmpy(r3,r5)
    r6 = memw(r16+#44)
}
{
    r15 = sfmpy(r9,r12)
}
{
    r3 = sfmpy(r3,r6)
    r6 = sfmpy(r4,r6)
}
{
    r4 = sfmpy(r4,r5)
    r5 = sfmpy(r7,r12)
}
{
    r7 = sfmpy(r9,r8)
    r6 = sfsub(r10,r6)
}
{
    r15 = sfsub(r11,r15)
    r3 = sfadd(r4,r3)
}
{
    r7 = sfadd(r7,r5)
}
{
    r6 = sfsub(r6,r15)
}
{
    r3 = sfsub(r3,r7)
}
{
    r4 = sfmpy(r6,r1)
    r5 = sfmpy(r6,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r3 = sfsub(r5,r3)
    memw(r17+#16) = r3.new
}
{
    r3 = sfadd(r6,r4)
    memw(r17+#20) = r3.new
}
{
    r3 = memw(r16+#24)
    r5 = memw(r16+#64)
}
{
    r13 = sfmpy(r3,r5)
    r7 = memw(r16+#40)
    r8 = memw(r16+#48)
}
{
    r10 = sfmpy(r7,r8)
    r4 = memw(r16+#28)
    r6 = memw(r16+#68)
}
{
    r9 = memw(r16+#44)
    r12 = memw(r16+#52)
}
{
    r3 = sfmpy(r3,r6)
    r6 = sfmpy(r4,r6)
}
{
    r4 = sfmpy(r4,r5)
    r11 = sfmpy(r9,r12)
}
{
    r5 = sfmpy(r7,r12)
    r7 = sfmpy(r9,r8)
}
{
    r6 = sfsub(r13,r6)
    r15 = sfsub(r10,r11)
}
{
    r3 = sfadd(r4,r3)
    r7 = sfadd(r7,r5)
}
{
    r6 = sfsub(r6,r15)
}
{
    r3 = sfsub(r3,r7)
}
{
    r4 = sfmpy(r6,r1)
    r5 = sfmpy(r6,r0)
}
{
    r6 = sfmpy(r3,r2)
    r3 = sfmpy(r3,r1)
}
{
    r3 = sfsub(r3,r5)
    memw(r17+#24) = r3.new
}
{
    r3 = sfsub(r6,r4)
    memw(r17+#28) = r3.new
}
{
    r3 = memw(r16+#0)
    r7 = memw(r16+#16)
}
{
    r8 = memw(r16+#48)
    r4 = memw(r16+#4)
}
{
    r10 = sfmpy(r7,r8)
    r5 = memw(r16+#64)
    r9 = memw(r16+#20)
}
{
    r12 = memw(r16+#52)
    r6 = memw(r16+#68)
}
{
    r13 = sfmpy(r3,r5)
}
{
    r3 = sfmpy(r3,r6)
    r6 = sfmpy(r4,r6)
}
{
    r4 = sfmpy(r4,r5)
    r11 = sfmpy(r9,r12)
}
{
    r5 = sfmpy(r7,r12)
    r7 = sfmpy(r9,r8)
}
{
    r6 = sfsub(r13,r6)
    r15 = sfsub(r10,r11)
}
{
    r3 = sfadd(r4,r3)
    r7 = sfadd(r7,r5)
}
{
    r6 = sfsub(r6,r15)
}
{
    r3 = sfsub(r3,r7)
}
{
    r4 = sfmpy(r6,r1)
    r5 = sfmpy(r6,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r3 = sfsub(r5,r3)
    memw(r17+#32) = r3.new
}
{
    r3 = sfadd(r6,r4)
    memw(r17+#36) = r3.new
}
{
    r3 = memw(r16+#0)
    r5 = memw(r16+#40)
}
{
    r13 = sfmpy(r3,r5)
    r7 = memw(r16+#16)
    r8 = memw(r16+#24)
}
{
    r10 = sfmpy(r7,r8)
    r4 = memw(r16+#4)
    r6 = memw(r16+#44)
}
{
    r9 = memw(r16+#20)
    r12 = memw(r16+#28)
}
{
    r3 = sfmpy(r3,r6)
    r6 = sfmpy(r4,r6)
}
{
    r4 = sfmpy(r4,r5)
    r11 = sfmpy(r9,r12)
}
{
    r5 = sfmpy(r7,r12)
    r7 = sfmpy(r9,r8)
}
{
    r6 = sfsub(r13,r6)
    r15 = sfsub(r10,r11)
}
{
    r3 = sfadd(r4,r3)
    r7 = sfadd(r7,r5)
}
{
    r6 = sfsub(r6,r15)
}
{
    r3 = sfsub(r3,r7)
}
{
    r4 = sfmpy(r6,r1)
    r5 = sfmpy(r6,r0)
}
{
    r6 = sfmpy(r3,r2)
    r3 = sfmpy(r3,r1)
}
{
    r3 = sfsub(r3,r5)
    memw(r17+#40) = r3.new
}
{
    r3 = sfsub(r6,r4)
    memw(r17+#44) = r3.new
}
{
    r3 = memw(r16+#24)
    r7 = memw(r16+#32)
}
{
    r4 = memw(r16+#28)
    r8 = memw(r16+#48)
}
{
    r10 = sfmpy(r7,r8)
    r5 = memw(r16+#56)
    r9 = memw(r16+#36)
}
{
    r12 = memw(r16+#52)
    r6 = memw(r16+#60)
}
{
    r13 = sfmpy(r3,r5)
}
{
    r3 = sfmpy(r3,r6)
    r6 = sfmpy(r4,r6)
}
{
    r4 = sfmpy(r4,r5)
    r11 = sfmpy(r9,r12)
}
{
    r5 = sfmpy(r7,r12)
    r7 = sfmpy(r9,r8)
}
{
    r6 = sfsub(r13,r6)
    r15 = sfsub(r10,r11)
}
{
    r3 = sfadd(r4,r3)
    r7 = sfadd(r7,r5)
}
{
    r6 = sfsub(r6,r15)
}
{
    r3 = sfsub(r3,r7)
}
{
    r4 = sfmpy(r6,r1)
    r5 = sfmpy(r6,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r3 = sfsub(r5,r3)
    memw(r17+#48) = r3.new
}
{
    r3 = sfadd(r6,r4)
    memw(r17+#52) = r3.new
}
{
    r3 = memw(r16+#0)
    r5 = memw(r16+#56)
}
{
    r13 = sfmpy(r3,r5)
    r7 = memw(r16+#8)
    r8 = memw(r16+#48)
}
{
    r10 = sfmpy(r7,r8)
    r4 = memw(r16+#4)
    r6 = memw(r16+#60)
}
{
    r9 = memw(r16+#12)
    r12 = memw(r16+#52)
}
{
    r3 = sfmpy(r3,r6)
    r6 = sfmpy(r4,r6)
}
{
    r4 = sfmpy(r4,r5)
    r11 = sfmpy(r9,r12)
}
{
    r5 = sfmpy(r7,r12)
    r7 = sfmpy(r9,r8)
}
{
    r3 = sfadd(r4,r3)
    r6 = sfsub(r13,r6)
}
{
    r4 = sfadd(r7,r5)
    r10 = sfsub(r10,r11)
}
{
    r7 = sfsub(r6,r10)
    r3 = sfsub(r3,r4)
}
{
    r2 = sfmpy(r3,r2)
    r6 = sfmpy(r7,r1)
}
{
    r5 = sfmpy(r7,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r6)
    memw(r17+#60) = r2.new
}
{
    r3 = sfsub(r3,r5)
    memw(r17+#56) = r3.new
}
{
    r2 = memw(r16+#0)
    r6 = memw(r16+#8)
}
{
    r3 = memw(r16+#32)
    r7 = memw(r16+#24)
}
{
    r14 = sfmpy(r2,r3)
    r4 = memw(r16+#36)
    r11 = memw(r16+#28)
}
{
    r15 = memw(r16+#12)
    r5 = memw(r16+#4)
}
{
    r2 = sfmpy(r2,r4)
}
{
    r4 = sfmpy(r5,r4)
    r3 = sfmpy(r5,r3)
}
{
    r5 = sfmpy(r6,r11)
    r6 = sfmpy(r6,r7)
}
{
    r7 = sfmpy(r15,r7)
    r8 = sfmpy(r15,r11)
}
{
    r2 = sfadd(r3,r2)
    r4 = sfsub(r14,r4)
}
{
    r3 = sfadd(r7,r5)
    r7 = sfsub(r6,r8)
}
{
    r2 = sfsub(r2,r3)
    r4 = sfsub(r4,r7)
}
{
    r3 = sfmpy(r4,r0)
    r6 = sfmpy(r2,r1)
}
{
    r2 = sfmpy(r2,r0)
    r4 = sfmpy(r4,r1)
    r0 = #0
}
{
    r3 = sfsub(r3,r6)
    memw(r17+#64) = r3.new
}
{
    r2 = sfadd(r2,r4)
    memw(r17+#68) = r2.new
}
    .falign
.L_SINGULAR:
{
    r17:16 = memd(r29+#32)
    r19:18 = memd(r29+#24)
}
{
    r21:20 = memd(r29+#16)
    r23:22 = memd(r29+#8)
}
{
    r25:24 = memd(r29+#0)
    r31:30 = dealloc_return(r30):raw
}
    .size   __qhblas_matrix_inverse3x3_acf, .-__qhblas_matrix_inverse3x3_acf
