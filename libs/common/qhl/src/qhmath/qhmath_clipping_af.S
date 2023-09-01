/**=============================================================================
@file
    qhmath_clipping_af.S

@brief
    Clips floating-point values from input array and store results to the same-lenght output array.
    If particular member is greather then high_level parameter, its value is set to high_level.
    If member's value is lower then low_level parameter, value of that member is set to low_value.

    Function prototype

        typedef float __attribute__ ((__aligned__(8))) float_a8_t;
        int32_t qhmath_clipping_af(float_a8_t *input,
                                   float_a8_t *output,
                                   uint32_t size,
                                   float low_level,
                                   float high_level);

    Reference C code

        int32_t qhmath_clipping_af(float_a8_t *input,
                            float_a8_t *output,
                            uint32_t size,
                            float low_level,
                            float high_level)
        {
            if (low_level >= high_level)
            {
                return -1;
            }

            if (size <= 0)
            {
                return -1;
            }

            for (uint32_t i = 0; i < size; i++)
            {
                float actual_val = input[i];

                if (low_level > actual_val)
                    actual_val = low_level;
                else if (actual_val > high_level)
                    actual_val = high_level;

                output[i] = actual_val;
            }

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_clipping_af
    .type qhmath_clipping_af, @function

/*============================================================================*/

#define input           r0
#define output          r1
#define size_arr        r2
#define size_arr_sht    r8
#define lowest_val      r3
#define highest_val     r4

#define actual_dw       r7:6
#define actual_w1       r7
#define actual_w0       r6

#define odd_cond        p2

#define return_reg      r0

/*============================================================================*/

qhmath_clipping_af:
{
    p0 = cmp.gtu(size_arr,#0)
    p0 = cmp.gtu(input, #0)
    p0 = cmp.gtu(output, #0)
    p0 = sfcmp.gt(highest_val, lowest_val)
    if (!p0.new) jump:nt .L_input_error
}
{
    size_arr_sht = lsr(size_arr, #1)
}
{
    odd_cond = tstbit(size_arr, #0)
    p1 = cmp.gtu(size_arr_sht,#0)
    if (!p1.new) jump:nt .L_odd_member

    p3 = sp1loop0(.L_clip,size_arr_sht)
}
    .falign
.L_clip:
{
    if (p3) memd(output++#8) = actual_dw
    actual_dw = memd(input++#8)
}
{
    p0 = sfcmp.gt(lowest_val,actual_w1)
    if (p0.new) actual_w1 = add(lowest_val,#0)
    p1 = sfcmp.gt(actual_w1,highest_val)
    if (p1.new) actual_w1 = add(highest_val,#0)
}
{
    p0 = sfcmp.gt(lowest_val,actual_w0)
    if (p0.new) actual_w0 = add(lowest_val,#0)
    p1 = sfcmp.gt(actual_w0,highest_val)
    if (p1.new) actual_w0 = add(highest_val,#0)
}:endloop0
    .falign
.L_cleanup:
{
    memd(output++#8) = actual_dw
}
    .falign
.L_odd_member:
{
    return_reg = #0

    if (!odd_cond) jumpr r31

    if (odd_cond) actual_w1 = memw(input+#0)
}
{
    p0 = sfcmp.gt(lowest_val,actual_w1)
    if (p0.new) actual_w1 = add(lowest_val,#0)
    p1 = sfcmp.gt(actual_w1,highest_val)
    if (p1.new) actual_w1 = add(highest_val,#0)
}
{
    memw(output+#0) = actual_w1
    jumpr r31
}
    .falign
.L_input_error:
{
    return_reg = #-1
    jumpr r31
}
    .size qhmath_clipping_af, .-qhmath_clipping_af
