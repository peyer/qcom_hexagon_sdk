/**=============================================================================
@file
    qhmath_log2_ah.S

@brief
    Computes base-2 logarithm over the elements of 16-bit input array.

    Function prototype

    int32_t qhmath_log2_ah(uint16_t *input, uint16_t *output, uint32_t size)

    Reference C code

    int32_t qhmath_log2_ah(uint16_t *input, uint16_t *output, uint32_t size)
    {
        uint32_t fract;
        int64_t y;
        int32_t v0, v1, v2;
        uint32_t fract_2;
        uint32_t x_sht_11;
        int b;

        if ((input == NULL) || (output == NULL) || (size == 0))
        {
            return -1;
        }

        for (int i = 0; i < size; i++)
        {
            x_sht_11 = (uint32_t) input[i] << 14;

            // Find base index
            b = 30 - Q6_R_normamt_R(x_sht_11);
            x_sht_11 = x_sht_11 ^ (1 << b);

            // Compute fractional portion
            fract = x_sht_11 >> (b - 14);
            fract += (1 << 14);

            // Compute fract ^ 2
            fract_2 = (((int64_t) fract * fract));

            // Polynomial approximation: argument of Polynomial is fractional portion.
            // y = -1.685874806524239 + fract * 2.028812209786359
            //     - 0.342937403262120 * fract ^ 2; in float format
            v0 = LOG2_POLY_P0_Q11;
            v1 = Q6_R_mpy_RR(fract, LOG2_POLY_P1_Q29);
            v2 = -Q6_R_mpy_RR(fract_2, LOG2_POLY_P2_Q15);

            y = v0 + v1 + v2;

            b = b - 14;

            if (input[i] > 1)
            {
                output[i] = y + (b<<11);
            }
            else
            {
                output[i] = 0;
            }
        }

        // return OK status
        return 0;
    }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/


/*============================================================================*/

#define LOG2_POLY_P0_Q11      #-3453
#define LOG2_POLY_P1_Q28      #544605131
#define LOG2_POLY_P2_Q14      #-5619

#define FRACT_BITS_VECT       #11
#define FRACTIONAL_SIZE       #11
#define FRACTIONAL_TMP_SIZE   #14
#define ONE_Q14               #16384

/*============================================================================*/
    .p2align 2
    .p2align 4,,15
    .global qhmath_log2_ah
    .type qhmath_log2_ah, @function
qhmath_log2_ah:
{
    p0 = cmp.gtu(r0, #0) // Check whether the input array address different of NULL
    p0 = cmp.gtu(r1, #0) // Check whether the output array address different of NULL
    p0 = cmp.gtu(r2, #0) // Check array length
    if (!p0.new) jump:nt .L_ret_error
}
{
    allocframe(#48)
    r3 = asr(r2, #1)
    r2 = and(r2, #1) // Mask to check odd number of array elements
    p0 = cmp.eq(r2, #1) // Check if the size equal 1
}
{
     if (p0) jump:nt .L_size_one
     p1 = cmp.eq(r2, #1)
}
{
    memd(sp+#0) = r17:16
    memd(sp+#8) = r19:18
    r11:10 = combine(#44, #44)
    r15:14 = combine(#30, #30)
}
{
    memd(sp+#16) = r21:20
    memd(sp+#24) = r23:22
    r23:22 = r1:0
}
{
    memd(sp+#32) = r25:24
    memd(sp+#40) = r27:26
}
{
    r24 = #LOG2_POLY_P0_Q11   // Store P0 coefficient
    r25 = #LOG2_POLY_P1_Q28   // Store P1 coefficient
                              // LOG2_POLY_P1_Q28 constant is adapt for instruction: mpy(r3, r25):<<1:sat
                              // See reference code where stand LOG2_POLY_P1_Q29
}
{
    r1 = ONE_Q14
    p3 = sp1loop0(.L_main_loop, r3) // Setup pipelined loop
}
.L_main_loop:
{
   r16 = memuh(r22)
   r17 = memuh(r22+#2) // Read input pair to registers r17:16
}
{
    r3:2 = combine(r1, r1)     // Store vector fract part
    r4 = normamt(r16)          // Find number of leading zeros in first word member
    r5 = normamt(r17)          // Find number of leading zeros in second word member
}
{
    r22 = add(r22, #4)    // Increment input address
    r9:8 = vsubw(r11:10, r5:4) // Determine msb position for input pair
    r13:12 = vsubw(r15:14, r5:4) // Calculate base index vector
}
{
    r26 = #LOG2_POLY_P2_Q14   // Store P2 coefficient
    r19:18 = vaslw(r17:16, FRACTIONAL_TMP_SIZE) // Shift left input pair in order to get Q14 fromat
    r5:4 = vaslw(r13:12, FRACTIONAL_SIZE) // Shift of base index b in left for 11 bits
}
{
    if (p3) memh(r23) = r20
    if (p3) memh(r23+#2) = r21
    r19 = togglebit(r19, r9)   // Clear msb in high part of input pair
    r18 = togglebit(r18, r8)   // Clear msb in low part of input pair
}

{
    r18 = lsr(r18, r12) // 1st sample: [fract = x >> b - FRACTIONAL_SIZE]
    r19 = lsr(r19, r13) // 2nd sample: [fract = x >> b - FRACTIONAL_SIZE]
    if (p3) r23 = add(r23, #4) // Increment output address for 2 words
}
{
    p0 = vcmpw.gt(r17:16, #1)    // Compare whether input pair 0 or 1
    r3:2 = vaddw(r3:2, r19:18)   // Vector fract += V_x_sht_11 << b;
    r9:8 = combine(r24, r24)     // Store coefficient LOG2_POLY_P0_Q11 in r9:8
}

{
    r16 = mpyi(r2, r2)  // fract_2 1st sample
    r17 = mpyi(r3, r3)  // fract_2 2nd sample
    r19:18 = combine(r5, r4) // Store b << 11 for both sample
}
{
    r18 += mpy(r2, r25):<<1:sat   // 1st sample : fract * LOG2_POLY_P1_Q28
    r19 += mpy(r3, r25):<<1:sat   // 2nd sample : fract * LOG2_POLY_P1_Q28
}
{
    r8 += mpy(r16, r26):<<1:sat  //  1st sample : (b << 11) + fract_2 * LOG2_POLY_P2_Q14
    r9 += mpy(r17, r26):<<1:sat  //  2nd sample : (b << 11) + fract_2 * LOG2_POLY_P2_Q14
}
{
    r17:16 = vaddw(r9:8, r19:18)
    r5:4 = combine(#0, #0)
}
{
    r21:20 = vmux(p0, r17:16, r5:4)
}:endloop0
{
   memh(r23) = r20     // Store next-to-last  member of output array
   memh(r23+#2) = r21  // Store last member of output array
}
{
   if (p1) r0 = memuh(r22) // Loading of last member of input array for odd array length
}
{
   if(p1) call qhmath_log2_h // Processing of last member in input array for odd array length
}
{
   if(p1) memh(r23+#4) = r0 // Store last odd member to output array
}
{
    r17:16 = memd(sp+#0)
    r19:18 = memd(sp+#8)
}
{
    r21:20 = memd(sp+#16)
    r23:22 = memd(sp+#24)
}
{
    r25:24 = memd(sp+#32)
    r27:26 = memd(sp+#40)
}
{
    r0 = #0     // Set return status to 0 -> status is ok
    dealloc_return
}
.L_size_one: // Handle if there is one element in input array
{
    r0 = memuh(r0)
    call qhmath_log2_h
}
{
    memh(r1) = r0
}
{
    r0 = #0
    dealloc_return
}
.L_ret_error:
{
    r0 = #-1 // Set return status to -1 -> error status
    jumpr r31
}
   .size qhmath_log2_ah, .-qhmath_log2_ah
