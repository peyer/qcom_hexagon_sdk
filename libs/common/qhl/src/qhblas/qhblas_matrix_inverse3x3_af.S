/**=============================================================================
@file
    qhblas_matrix_inverse3x3_af.S

@brief
    Calculates matrix inversion for 3x3 matrix of float values.

    Function prototype

        int32_t __qhblas_matrix_inverse3x3_af(float_a8_t *input_matrix, float_a8_t *output_matrix);

    Reference C code

        int32_t __qhblas_matrix_inverse3x3_af(float_a8_t *input_matrix, float_a8_t *output_matrix)
        {
            float det = A[0]*A[4]*A[8] - A[0]*A[5]*A[7] - A[1]*A[3]*A[8] + A[1]*A[5]*A[6] + A[2]*A[3]*A[7] - A[2]*A[4]*A[6];

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
    .hidden	__qhblas_matrix_inverse3x3_af
    .global __qhblas_matrix_inverse3x3_af
    .type __qhblas_matrix_inverse3x3_af, @function

/*============================================================================*/

__qhblas_matrix_inverse3x3_af:
{
  r16 = r0
  memd(r29+#-16) = r17:16
  allocframe(#24)
}
{
  r19 = memw(r0+#16)
  memd(r29+#8) = r19:18
}
{
  r2 = memw(r0+#0)
  memd(r29+#0) = r21:20
}
{
  r5 = sfmpy(r2,r19)
  r17 = r1
  r18 = memw(r0+#20)
  r3 = memw(r0+#4)
}
{
  r2 = sfmpy(r2,r18)
  r4 = memw(r0+#12)
  r21 = memw(r0+#32)
}
{
  r5 = sfmpy(r5,r21)
  r7 = sfmpy(r18,r3)
  r20 = memw(r0+#28)
  r6 = memw(r0+#8)
}
{
  r3 = sfmpy(r3,r4)
  r0 = #-1
  r8 = memw(r0+#24)
}
{
  r2 = sfmpy(r2,r20)
  r4 = sfmpy(r4,r6)
}
{
  r3 = sfmpy(r21,r3)
  r7 = sfmpy(r7,r8)
}
{
  r2 = sfsub(r5,r2)
  r6 = sfmpy(r19,r6)
}
{
  r4 = sfmpy(r20,r4)
}
{
  r2 = sfsub(r2,r3)
  r3 = sfmpy(r8,r6)
}
{
  r2 = sfadd(r7,r2)
}
{
  r2 = sfadd(r2,r4)
}
{
  r2 = sfsub(r2,r3)
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
{
  call qhmath_div_f
  r1:0 = combine(r2,##1065353216)
}
{
  r3 = sfmpy(r19,r21)
  r4 = sfmpy(r18,r20)
}
{
  r2 = togglebit(r0,#31)
}
{
  r3 = sfsub(r3,r4)
}
{
  r3 = sfmpy(r3,r0)
  memw(r17+#0) = r3.new
}
{
  r3 = memw(r16+#4)
  r5 = memw(r16+#8)
}
{
  r6 = memw(r16+#28)
  r4 = memw(r16+#32)
}
{
  r3 = sfmpy(r3,r4)
  r6 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r6)
}
{
  r3 = sfmpy(r3,r2)
  memw(r17+#4) = r3.new
}
{
  r4 = memw(r16+#20)
  r3 = memw(r16+#4)
}
{
  r5 = memw(r16+#8)
  r6 = memw(r16+#16)
}
{
  r3 = sfmpy(r3,r4)
}
{
  r7 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r7)
}
{
  r3 = sfmpy(r0,r3)
  memw(r17+#8) = r3.new
}
{
  r3 = memw(r16+#12)
  r4 = memw(r16+#32)
}
{
  r5 = memw(r16+#20)
  r6 = memw(r16+#24)
}
{
  r3 = sfmpy(r3,r4)
}
{
  r6 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r6)
}
{
  r3 = sfmpy(r3,r2)
  memw(r17+#12) = r3.new
}
{
  r3 = memw(r16+#0)
  r5 = memw(r16+#8)
}
{
  r4 = memw(r16+#32)
  r6 = memw(r16+#24)
}
{
  r3 = sfmpy(r3,r4)
  r7 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r7)
}
{
  r3 = sfmpy(r0,r3)
  memw(r17+#16) = r3.new
}
{
  r5 = memw(r16+#8)
  r3 = memw(r16+#0)
}
{
  r6 = memw(r16+#12)
  r4 = memw(r16+#20)
}
{
  r3 = sfmpy(r3,r4)
  r6 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r6)
}
{
  r3 = sfmpy(r3,r2)
  memw(r17+#20) = r3.new
}
{
  r4 = memw(r16+#28)
  r3 = memw(r16+#12)
}
{
  r5 = memw(r16+#16)
  r6 = memw(r16+#24)
}
{
  r3 = sfmpy(r3,r4)
}
{
  r7 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r7)
}
{
  r3 = sfmpy(r0,r3)
  memw(r17+#24) = r3.new
}
{
  r3 = memw(r16+#0)
  r4 = memw(r16+#28)
}
{
  r5 = memw(r16+#4)
  r6 = memw(r16+#24)
}
{
  r3 = sfmpy(r3,r4)
}
{
  r6 = sfmpy(r5,r6)
}
{
  r3 = sfsub(r3,r6)
}
{
  r2 = sfmpy(r3,r2)
  memw(r17+#28) = r2.new
}
{
  r2 = memw(r16+#0)
  r3 = memw(r16+#16)
}
{
  r7 = memw(r16+#4)
  r6 = memw(r16+#12)
}
{
  r2 = sfmpy(r2,r3)
}
{
  r3 = sfmpy(r7,r6)
}
{
  r2 = sfsub(r2,r3)
}
{
  r2 = sfmpy(r0,r2)
  r0 = #0
  memw(r17+#32) = r2.new
}
    .falign
.L_SINGULAR:
{
  r17:16 = memd(r29+#16)
  r19:18 = memd(r29+#8)
}
{
  r21:20 = memd(r29+#0)
  dealloc_return
}
.size   __qhblas_matrix_inverse3x3_af, .-__qhblas_matrix_inverse3x3_af
