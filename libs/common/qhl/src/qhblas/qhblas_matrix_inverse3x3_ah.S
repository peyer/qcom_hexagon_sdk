/**=============================================================================
@file
    qhblas_matrix_inverse3x3_ah.S

@brief
    Calculates matrix inversion for 3x3 matrix of 16-bit fixed-point values.

    Function prototype

        int32_t __qhblas_matrix_inverse3x3_ah(int16_t *input_matrix, int32_t *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse3x3_ah(int16_t *input_matrix, int32_t *output_matrix)
        {
            int32_t det = A[0]*A[4]*A[8] - A[0]*A[5]*A[7] - A[1]*A[3]*A[8] + A[1]*A[5]*A[6] + A[2]*A[3]*A[7] - A[2]*A[4]*A[6];

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
    .hidden	__qhblas_matrix_inverse3x3_ah
    .global __qhblas_matrix_inverse3x3_ah
    .type __qhblas_matrix_inverse3x3_ah, @function

/*============================================================================*/

__qhblas_matrix_inverse3x3_ah:
{
    allocframe(#72)
}
{
    memd(r29+#56) = r17:16
}
{
    r8 = memh(r0+#2)
    memd(r29+#16) = r27:26
}
{
    r26 = memh(r0+#6)
    memd(r29+#40) = r21:20
}
{
    r20 = memh(r0+#10)
    memd(r29+#32) = r23:22
}
{
    r4 = mpyi(r26,r8)
    r24 = memh(r0+#0)
    memd(r29+#24) = r25:24
}
{
    r19 = mpyi(r8,r20)
    r22 = memh(r0+#8)
    memd(r29+#48) = r19:18
}
{
    r5 = mpyi(r22,r24)
    r6 = mpyi(r20,r24)
    r23 = memh(r0+#4)
    r21 = memh(r0+#12)
}
{
    r3 = mpyi(r19,r21)
    r27 = memh(r0+#16)
    r25 = memh(r0+#14)
}
{
    r17 = mpyi(r23,r26)
    r0 = #-1
}
{
    r2 = mpyi(r4,r27)
    r18 = mpyi(r23,r22)
}
{
    r3 += mpyi(r5,r27)
    r2 += mpyi(r6,r25)
}
{
    r2 += mpyi(r18,r21)
    r3 += mpyi(r17,r25)
}
{
    r2 = sub(r3,r2)
    if (cmp.eq(r2.new,#0)) jump:nt .L_SINGULAR
    r3 = #0
    r10 = #0
}
{
    p0 = cmp.gt(r2,r3)
    if (!p0.new) r10 = #1
    if (!p0.new) r2 = sub(r3,r2)
}
{
    memw(r29+#64) = r10
    r16 = r1                                   // save output pointer
    memw(r29+#12) = r4
}
{
    memw(r29+#8) = r5
    memw(r29+#4) = r6
}
{
    call qhmath_inv_h
    r0 = asr(r2,#15)
    memw(r29+#0) = r8
}
{
    r10 = memw(r29+#64)
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
    r2 = mpyi(r27,r22)
    r7 = mpyi(r26,r27)
    r3 = sub(#0,r0)
    r13 = memw(r29+#0)
}
{
    r2 -= mpyi(r25,r20)
    r8 = mpyi(r27,r24)
    r4 = sub(r19,r18)
    r6 = memw(r29+#4)
}
{
    r5 = mpyi(r13,r27)
    r7 -= mpyi(r21,r20)
    r6 = sub(r6,r17)
}
{
    r5 -= mpyi(r23,r25)
    r2 = mpyi(r0,r2)
    memw(r16+#0) = r2.new
}
{
    r8 -= mpyi(r23,r21)
    r12 = mpyi(r25,r24)
}
{
    r2 = mpyi(r5,r3)
    r12 -= mpyi(r21,r13)
    memw(r16+#4) = r2.new
}
{
    r2 = mpyi(r7,r3)
    r9 = mpyi(r26,r25)
    memw(r16+#12) = r2.new
}
{
    r2 = mpyi(r0,r8)
    r9 -= mpyi(r21,r22)
    memw(r16+#16) = r2.new
}
{
    r2 = mpyi(r6,r3)
    r4 = mpyi(r0,r4)
    memw(r16+#20) = r2.new
}
{
    r2 = mpyi(r12,r3)
    r6 = mpyi(r0,r9)
    r3 = memw(r29+#12)
    memw(r16+#28) = r2.new
}
{
    r7 = memw(r29+#8)
    memw(r16+#8) = r4
}
{
    r3 = sub(r7,r3)
    memw(r16+#24) = r6
}
{
    r2 = mpyi(r0,r3)
    r0 = #0
    memw(r16+#32) = r2.new
}
    .falign
.L_SINGULAR:
{
    r17:16 = memd(r29+#56)
    r19:18 = memd(r29+#48)
}
{
    r21:20 = memd(r29+#40)
    r23:22 = memd(r29+#32)
}
{
    r25:24 = memd(r29+#24)
    r27:26 = memd(r29+#16)
}
{
    dealloc_return
}
    .size   __qhblas_matrix_inverse3x3_ah, .-__qhblas_matrix_inverse3x3_ah
