/**=============================================================================
@file
    qhmath_copysign_af.S

@brief
    Vectorized copysign function for floating-point values.
    Fuction composes an output floating-point array from input array by
    modifying sign of each input element to sign of y.

    Function prototype

        typedef float __attribute__ ((__aligned__(8))) float_a8_t;
        int32_t qhmath_copysign_af(float_a8_t *input,
                                   float_a8_t *output,
                                   uint32_t size,
                                   float y);

    Reference C code

        int32_t qhmath_copysign_af(float_a8_t *input, float_a8_t *__restrict output, uint32_t size, float y)
        {
            if ((input == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            for (uint32_t i = 0; i < size; ++i)
            {
                float abs_data = input[i] < 0 ? -input[i] : input[i];

                ok_data[i] = y < 0 ? -abs_data : abs_data;
            }

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_copysign_af
    .type qhmath_copysign_af, @function

/*============================================================================*/

#define input           r0
#define output          r1
#define size_arr        r2
#define new_sign        r3

#define in_dw           r5:4
#define in_1            r5
#define in_0            r4

#define out_dw          r7:6
#define out_1           r7
#define out_0           r6

#define odd_cond        p2

#define return_reg      r0

/*============================================================================*/

qhmath_copysign_af:
{
    p0 = cmp.gtu(size_arr,#0)
    p0 = cmp.gtu(input,#0)
    p0 = cmp.gtu(output,#0)
    if (!p0.new) jump:nt .L_input_error
}
{
    odd_cond = tstbit(size_arr, #0)
    size_arr = lsr(size_arr, #1)
}
{
    p1 = cmp.gt(size_arr, #0)
    if (!p1.new) jump:nt .L_odd_member

    out_dw = combine(new_sign,new_sign)

    p3 = sp2loop0(.L_loop,size_arr)
}
    .falign
.L_loop:
{
    if (p3) memd(output++#8) = out_dw

    in_dw = memd(input++#8)

    out_1 = insert(in_1,#0x1f,#0x0)
    out_0 = insert(in_0,#0x1f,#0x0)
}:endloop0
    .falign
.L_cleanup:
{
    p0 = cmp.eq(size_arr, #1)
    if (!p0.new) memd(output++#8) = out_dw

    out_1 = insert(in_1,#0x1f,#0x0)
    out_0 = insert(in_0,#0x1f,#0x0)
}
{
    memd(output++#8) = out_dw
}
    .falign
.L_odd_member:
{
    return_reg = #0

    if (!odd_cond) jumpr r31

    if (odd_cond) in_0 = memw(input+#0)
}
{
    out_0 = insert(in_0,#0x1f,#0x0)
    memw(output+#0) = out_0.new

    jumpr r31
}
    .falign
.L_input_error:
{
    return_reg = #-1
    jumpr r31
}
    .size qhmath_copysign_af, .-qhmath_copysign_af
