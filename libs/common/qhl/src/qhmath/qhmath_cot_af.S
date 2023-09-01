/**=============================================================================
@file
    qhmath_cot_af.S

@brief
    Computes the cotangent over the elements of input array and store results
    to the output array.

    Function prototype

        typedef float __attribute__ ((__aligned__(8))) float_a8_t;
        int32_t qhmath_cot_af(float_a8_t *input,float_a8_t *output, uint32_t size);

    Reference C code

        int32_t qhmath_cot_af(float_a8_t *input,float_a8_t *output, uint32_t size)
        {
            if ((input == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            for (int i = 0; i < size; i++)
            {
                output[i] = qhmath_cot_f(input[i]);
            }

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_cot_af
    .type qhmath_cot_af, @function

/*============================================================================*/

#define input            r0
#define output           r1
#define size_arr         r2
#define addresses        r17:16
#define addr_in          r16
#define addr_out         r17
#define loop_cnt         r18
#define loop_length      r19
#define loop_registers   r19:18

/*============================================================================*/

qhmath_cot_af:
{
    p0 = cmp.gtu(size_arr,#0)
    p0 = cmp.gtu(output,#0)
    p0 = cmp.gtu(input,#0)
    if (!p0.new) jump:nt .L_input_error
    allocframe(#16)
}
{
    memd(sp+#0) = addresses
    memd(sp+#8) = loop_registers
    addresses = r1:0
}
{
    loop_registers = combine(size_arr, #0)
}
    .falign
.L_loop:
{
    loop_cnt = add(loop_cnt,#1)
    r0 = memw(addr_in++#4)
    call qhmath_cot_f
}
{
    memw(addr_out++#4) = r0
    p0 = cmp.lt(loop_cnt, loop_length)
    if (p0.new) jump:t .L_loop
}
{
    r0 = #0
    r17:16 = memd(sp+#0)
    r19:18 = memd(sp+#8)
}
{
    dealloc_return
}
.L_input_error:
{
    r0 = #-1
    dealloc_return
}
    .size   qhmath_cot_af, .-qhmath_cot_af
