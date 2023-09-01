/**=============================================================================
@file
    qhblas_matrix_inverse2x2_af.S

@brief
    Calculates matrix inversion for 2x2 matrix of float values.

    Function prototype

        int32_t __qhblas_matrix_inverse2x2_af(float_a8_t *input_matrix, float_a8_t *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse2x2_af(float_a8_t *input_matrix, float_a8_t *output_matrix)
        {
            float_a8_t det = (input_matrix[0]*input_matrix[3] - input_matrix[1]*input_matrix[2]);

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
    .hidden	__qhblas_matrix_inverse2x2_af
    .global __qhblas_matrix_inverse2x2_af
    .type __qhblas_matrix_inverse2x2_af, @function

/*============================================================================*/

__qhblas_matrix_inverse2x2_af:
{
    r3 = #0
    r16 = r0
    memd(r29+#-16) = r17:16
    allocframe(#16)
}
{
    r2 = memw(r0+#0)
    memd(r29+#0) = r19:18
}
{
    r17 = r1
    r18 = memw(r0+#12)
    r4 = memw(r0+#4)
}
{
    r0 = #-1
    r5 = memw(r0+#8)
    r2 = sfmpy(r2,r18)
}
{
    r2-=sfmpy(r4,r5)
}
{
    p0 = sfcmp.eq(r2,r3)
    if (p0.new) jump:nt .L_SINGULAR
}
{
    call qhmath_div_f
    r1:0 = combine(r2,##1065353216)
}
{
    r3 = sfmpy(r18,r0)
    r2 = r0
    memw(r17+#0) = r3.new
}
{
    r3 = togglebit(r0,#31)
    r0 = #0
    r4 = memw(r16+#4)
}
{
    r5 = sfmpy(r4,r3)
    memw(r17+#4) = r5.new
    r4 = memw(r16+#8)
}
{
    r3 = sfmpy(r4,r3)
    memw(r17+#8) = r3.new
    r4 = memw(r16+#0)
}
{
    r2 = sfmpy(r2,r4)
    memw(r17+#12) = r2.new
}
    .falign
.L_SINGULAR:
{
    r17:16 = memd(r29+#8)
    r19:18 = memd(r29+#0)
}
{
    dealloc_return
}
.size   __qhblas_matrix_inverse2x2_af, .-__qhblas_matrix_inverse2x2_af
