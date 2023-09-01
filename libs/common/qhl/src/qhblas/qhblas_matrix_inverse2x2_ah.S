/**=============================================================================
@file
    qhblas_matrix_inverse2x2_ah.S

@brief
    Calculates matrix inversion for 2x2 matrix of 16-bit fixed-point values.

    Function prototype

        int32_t __qhblas_matrix_inverse2x2_ah(int16_a8_t *input_matrix, int32_a8_t *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse2x2_ah(int16_a8_t *input_matrix, int32_a8_t *output_matrix)
        {
            int32_t det = (input_matrix[0]*input_matrix[3] - input_matrix[1]*input_matrix[2]);

            if ( det == 0)
            {
                return -1;
            }

            det = 1/det;

            output_matrix[0] =  det * input_matrix[3];
            output_matrix[1] = -det * input_matrix[1];
            output_matrix[2] = -det * input_matrix[2];
            output_matrix[3] =  det * input_matrix[0];

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .hidden	__qhblas_matrix_inverse2x2_ah
    .global __qhblas_matrix_inverse2x2_ah
    .type __qhblas_matrix_inverse2x2_ah, @function

/*============================================================================*/

__qhblas_matrix_inverse2x2_ah:
{
    memd(r29+#-16) = r17:16
    allocframe(#24)
}
{
    r17 = memh(r0+#0)
    memd(r29+#0) = r21:20
}
{
    r20 = memh(r0+#6)
    memd(r29+#8) = r19:18
}
{
    r16 = r1
    r18 = memh(r0+#2)
    r19 = memh(r0+#4)
}
{
    r2 = mpyi(r20,r17)
    r21 = #0
    r3 = #0
}
{
    r2 -= mpyi(r19,r18)
    if (cmp.eq(r2.new,#0)) jump:nt .L_SINGULAR
}
{
    p0 = cmp.gt(r2,r3)
    if (!p0.new) r21 = #1
    if (!p0.new) r2 = sub(r3,r2)
}
{
    call qhmath_inv_h
    r0 = asr(r2,#15)
}
{
    r1 = add(r1,#-15)
    r4 = #0
}
{
    r0 = asr(r0,r1)
}
{
    p0 = cmp.eq(r21,#1)
    if(p0.new) r0 = sub(r4,r0)
}
{
    r2 = mpyi(r0,r20)
    memw(r16+#0) = r2.new
    r4 = sub(#0,r0)
}
{
    r3 = mpyi(r4,r19)
    r2 = mpyi(r4,r18)
    memw(r16+#8) = r3.new
}
{
    r2 = mpyi(r0,r17)
    memw(r16+#4) = r2
}
{
    r0 = #0 ; jump .L_RETURN
    memw(r16+#12) = r2
}
    .falign
.L_SINGULAR:
{
    r0 = #-1
}
    .falign
.L_RETURN:
{
    r17:16 = memd(r29+#16)
    r19:18 = memd(r29+#8)
}
{
    r21:20 = memd(r29+#0)
    dealloc_return
}
    .size   __qhblas_matrix_inverse2x2_ah, .-__qhblas_matrix_inverse2x2_ah
