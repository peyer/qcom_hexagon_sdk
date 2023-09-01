/**=============================================================================
@file
    qhmath_exp_h.S

@brief
    Calculating of exponential value to the given 16-bit fixed-point argument
    Input argument is Q4.11 signed fixed-point.
    Output is Q16.16 unsigned fixed-point format.

    Function prototype

        uint32_t qhmath_exp_h(int16_t x)

   Reference C code

        uint32_t qhmath_exp_h(int16_t x)
        {
            int32_t x_32b = x;
            // In order to set multiplication of x * R_LN2_Q30 to upper 32-bit
            x_32 = Q6_R_asl_RI(x_32, 2);
            x_32 = Q6_P_mpy_RR(x_int32b, R_LN2_Q30);

            // Call exp2 function
            y = qhmath_exp2_h((int16_t) x_32);

            return (uint32_t) y;
        }
   NOTE: Calculating of exp(x) is performed using exp2(x): exp(x) = exp2(x/ln(2))

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_exp_h
    .type qhmath_exp_h, @function

/*============================================================================*/

#define R_LN2_Q30      1549082005 // 1/ln(2) in Q30 format

/*============================================================================*/

qhmath_exp_h:
{
    r1 = ##R_LN2_Q30
    r0 = asl(r0, #2)
    allocframe(#8)
}
{
    memw(sp+#0) = r16
    r16 = r0
    r0 = mpy(r1, r0) // [x = (int16_t)((uint64)(x * R_LN2_Q30) >> 32);]
    call qhmath_exp2_h
}
{
    p0 = cmp.gt(r16, #39683) // Check whether the input greater than 4.8441870 in
                             // floating-point format
    r16 = memw(sp+#0)
    if (p0.new) r0 = #-1     // If the input overflows, set saturation to output
}
{
    dealloc_return
}
    .size    qhmath_exp_h, .-qhmath_exp_h
