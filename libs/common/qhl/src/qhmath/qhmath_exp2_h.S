/**=============================================================================
@file
    qhmath_exp2_h.S

@brief
    Computes the 2 raised to the given argument in fixed-point.
    Input argument is in Q4.11 signed fixed-point.
    Output is in Q16.16 unsigned fixed-point format.

    Function prototype

        uint32_t qhmath_exp2_h(int16_t x)

   Reference C code

        uint32_t qhmath_exp2_h(int16_t x)
        {
            uint64_t y;
            int32_t shift_result;
            int32_t x_1 = x << 5; // Adapt for Q16 format
            uint64_t k = (((uint32_t)x_1 & FractionMask_1) << 14);

            // Compute polynomial approximation
            y = ((uint64_t) ((uint64_t) C5_Q30 * k) >> 30);
            y  = ((uint64_t) (((uint64_t) k) * ((uint64_t) y + C4_Q30))>>30);
            y = ((uint64_t)((uint64_t) k * ((uint64_t) y + C3_Q30))>>30);
            y = ((uint64_t)((uint64_t)  k * (y + C2_Q30))>> 30);
            y = ((uint64_t)((uint64_t)  k * (y + C1_Q30))>> 30);
            y = (y + C0_Q30);

            uint32_t int_part = x >> Q11_IN_FMT;

            shift_result = 14 - int_part;

            y = ((shift_result >= 0) ? (y >> shift_result):(y << (-shift_result)));

            return (uint32_t) y;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_exp2_h
    .type qhmath_exp2_h, @function

/*============================================================================*/

#define Q30_SHT_CONST  #14
#define Q11_IN_FMT     #11
#define C5_Q31         4035806
#define C4_Q30         9654007
#define C3_Q30         59934847
#define C2_Q30         257869054
#define C1_Q30         744266012
#define C0_Q30         1073741824

/*============================================================================*/

qhmath_exp2_h:
{
    r8 = r0
    r3:2 = combine(#C5_Q31, Q30_SHT_CONST) // Move C5_Q30 for one position left in order to
                                            // store result of mpy(r8,r3):<<1:sat in upper 32-bit
}
{
    r2 -= asr(r0, Q11_IN_FMT)    // [shift_result = 14 - (x >> Q11_IN_FMT)]
    r8 = and(##1073217536,asl(r8,#19)) // [k = ((x << 5) & 0xFFFF)) << 14]
}
{
    r6 = #C3_Q30 // [r6 = C3_Q30]
    r4 = #C4_Q30
}
{
    r4 += mpy(r8,r3):<<1:sat // [y = ((C5_Q30 * k) >> 30) + C4_Q30]
    r0 = #C2_Q30
    r10 = asl(r8, #1)     // [r10 = k << 1]
                          // NOTE: In order to store result of mpy instruction in upper 32-bit
}
{
    r6 += mpy(r4, r10):<<1:sat // [y = (k * (y + C4_Q30)>> 30) + C3_Q30;]
}
{
    r0 += mpy(r6, r10):<<1:sat // [y = (k * (y + C3_Q30)>> 30) + C2_Q30;]
    r6 = #C1_Q30
}
{
    r4 = #C0_Q30
    r6 += mpy(r0, r10):<<1:sat // [y = (k * (y + C2_Q30)>> 30) + C1_Q30;]
}
{
    r4 += mpy(r6, r10):<<1:sat // [y = (k * (y + C1_Q30)>> 30) + C0_Q30;]
}
{
    r0 = lsr(r4, r2) // [y = y >> shift_result]
    jumpr r31
}
    .size    qhmath_exp2_h, .-qhmath_exp2_h
