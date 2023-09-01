/**=============================================================================
@file
    qhmath_abs_af.S

@brief
    Computes absolute value of each element in floating-point input array and
    returns array of results.

    Function prototype

        typedef float __attribute__ ((__aligned__(8))) float_a8_t;
        int32_t qhmath_abs_af(float_a8_t *input, float_a8_t *output, uint32_t size);

    Reference C code

        int32_t qhmath_abs_af(float_a8_t *input, float_a8_t *__restrict output, uint32_t size)
        {
            int i;

            if (size <= 0 || input ==  NULL || output == NULL)
            {
                return -1;
            }

            for (i = 0; i < size; i++)
            {
                float tmp = input[i];
                tmp = tmp < 0 ? -tmp : tmp; // fabsf(tmp);
                output[i] = tmp;
            }

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_abs_af
    .type qhmath_abs_af, @function

/*============================================================================*/

#define input           r0
#define output          r1
#define size_arr        r2
#define size_arr_sht    r9

#define data_dw         r5:4
#define data_1          r5
#define data_0          r4

#define out_dw          r7:6
#define out_1           r7
#define out_0           r6

#define odd_cond        p2

#define return_reg      r0

/*============================================================================*/

qhmath_abs_af:
{
    p0 = cmp.gtu(size_arr,#0)
    p0 = cmp.gtu(input,#0)
    p0 = cmp.gtu(output,#0)
    if (!p0.new) jump:nt .L_input_error
    size_arr_sht = lsr(size_arr, #1)
}
{
    odd_cond = tstbit(size_arr, #0)
    p1 = cmp.gtu(size_arr_sht, #0)
    if (!p1.new) jump:nt .L_odd_member
    p3 = sp2loop0(.L_loop,size_arr_sht)
}
    .falign
.L_loop:
{
    if (p3) memd(output++#8) = out_dw

    data_dw = memd(input++#8)

    out_1 = clrbit(data_1,#31)
    out_0 = clrbit(data_0,#31)
}:endloop0
    .falign
.L_cleanup:
{
    p0 = cmp.eq(size_arr_sht, #1)
    if (!p0.new) memd(output++#8) = out_dw

    out_1 = clrbit(data_1,#31)
    out_0 = clrbit(data_0,#31)
}
{
    memd(output++#8) = out_dw
}
    .falign
.L_odd_member:
{
    return_reg = #0

    if (!odd_cond) jumpr r31

    if (odd_cond) data_0 = memw(input+#0)
}
{
    out_0 = clrbit(data_0, #31)
    memw(output+#0) = out_0.new

    jumpr r31
}
    .falign
.L_input_error:
{
    return_reg = #-1
    jumpr r31
}
    .size qhmath_abs_af, .-qhmath_abs_af
