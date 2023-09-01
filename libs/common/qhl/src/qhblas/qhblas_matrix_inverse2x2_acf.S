/**=============================================================================
@file
    qhblas_matrix_inverse2x2_acf.S

@brief
    Calculates matrix inversion for 2x2 matrix of complex float values.

    Function prototype

        int32_t __qhblas_matrix_inverse2x2_acf(complex float *input_matrix, complex float *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse2x2_acf(complex float *input_matrix, complex float *output_matrix);
        {
            complex float det = (input_matrix[0]*input_matrix[3] - input_matrix[1]*input_matrix[2]);

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
    .hidden	__qhblas_matrix_inverse2x2_acf
    .global __qhblas_matrix_inverse2x2_acf
    .type __qhblas_matrix_inverse2x2_acf, @function

/*============================================================================*/

__qhblas_matrix_inverse2x2_acf:
{
    r4 = #0
    r16 = r0
    memd(r29+#-16) = r17:16
    allocframe(#8)
}
{
    r17 = r1
    r2 = memw(r0+#0)
    r3 = memw(r0+#24)
}
{
    r9 = sfmpy(r2,r3)
    r7 = memw(r0+#8)
    r8 = memw(r0+#16)
}
{
    r5 = memw(r0+#4)
    r6 = memw(r0+#28)
}
{
    r0 = #-1
    r12 = memw(r0+#12)
    r13 = memw(r0+#20)
}
{
    r2 = sfmpy(r2,r6)
    r6 = sfmpy(r5,r6)
}
{
    r3 = sfmpy(r5,r3)
    r5 = sfmpy(r7,r8)
}
{
    r8 = sfmpy(r12,r8)
    r7 = sfmpy(r7,r13)
}
{
    r12 = sfmpy(r12,r13)
    r3 = sfadd(r3,r2)
}
{
    r7 = sfadd(r8,r7)
    r6 = sfsub(r9,r6)
}
{
    r2 = sfsub(r5,r12)
}
{
    r3 = sfsub(r3,r7)
}
{
    r2 = sfsub(r6,r2)
}
{
    p1 = sfcmp.eq(r3,r4)
}
{
    p0 = sfcmp.eq(r2,r4)
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
    r4 = togglebit(r1,#31)
    r2 = memw(r16+#24)
    r3 = memw(r16+#28)
}
{
    r6 = sfmpy(r2,r1)
    r2 = sfmpy(r2,r0)
}
{
    r5 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r2,r3)
    memw(r17+#0) = r2.new
}
{
    r2 = sfadd(r5,r6)
    memw(r17+#4) = r2.new
}
{
    r2 = memw(r16+#12)
    r3 = memw(r16+#8)
}
{
    r5 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r4)
}
{
    r2 = sfsub(r2,r6)
    memw(r17+#8) = r2.new
}
{
    r2 = sfsub(r3,r5)
    memw(r17+#12) = r2.new
}
{
    r2 = memw(r16+#16)
    r3 = memw(r16+#20)
}
{
    r4 = sfmpy(r2,r4)
    r2 = sfmpy(r2,r0)
}
{
    r6 = sfmpy(r3,r0)
    r3 = sfmpy(r3,r1)
}
{
    r2 = sfsub(r3,r2)
    memw(r17+#16) = r2.new
}
{
    r2 = sfsub(r4,r6)
    memw(r17+#20) = r2.new
}
{
    r2 = memw(r16+#0)
    r3 = memw(r16+#4)
}
{
    r7 = sfmpy(r2,r0)
    r2 = sfmpy(r2,r1)
}
{
    r6 = sfmpy(r3,r1)
    r3 = sfmpy(r3,r0)
    r0 = #0
}
{
    r4 = sfsub(r7,r6)
    r2 = sfadd(r2,r3)
    memw(r17+#24) = r4.new
}
{
    memw(r17+#28) = r2
}
    .falign
.L_SINGULAR:
{
    r17:16 = memd(r29+#0)
    dealloc_return
}
    .size   __qhblas_matrix_inverse2x2_acf, .-__qhblas_matrix_inverse2x2_acf
