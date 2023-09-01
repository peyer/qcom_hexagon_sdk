/**=============================================================================
@file
    qhmath_clipping_ah.S

@brief
    Clips 16-bit signed values from input array and store results to the
    same-size output array. If particular member is greather then high_level
    parameter, its value is set to high_level. If member's value is lower then
    low_level parameter, value of that member is set to low_value.

    Function prototype

        typedef int16_t __attribute__((__aligned__(8))) int16_a8_t;

        int32_t qhmath_clipping_ah(int16_a8_t *input,
                                   int16_a8_t *output,
                                   uint32_t size,
                                   int16_t lowest_val,
                                   int16_t highest_val);

    Reference C code

        int32_t qhmath_clipping_ah(int16_a8_t *input, int16_a8_t *output,
                                uint32_t size, int16_t lowest_val, int16_t highest_value)
        {
            if (!input || !output)
            {
                return -1;
            }

            if (size == 0)
            {
                return -1;
            }

            if (lowest_val > highest_value)
            {
                return -1;
            }

            for (uint32_t i = 0; i < size; i++)
            {
                int16_t actual_val = input[i];

                if (lowest_val > actual_val)
                    actual_val = lowest_val;
                else if (actual_val > highest_value)
                    actual_val = highest_value;

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
    .global qhmath_clipping_ah
    .type qhmath_clipping_ah, @function

/*============================================================================*/

#define input               r0
#define output              r1
#define size_arr            r2
#define lowest              r3
#define highest             r4
//----------------------------
#define lo_w                r2
#define lo_dw               r3:2                    // lowest              r3

#define hi_w                r4
#define hi_dw               r5:4                    // highest             r4

#define scalar_loop_cnt     r6
#define vector_loop_cnt     r7
#define loop_cnts           r7:6

#define in_w                r8
#define in_dw               r9:8

#define out_w               r10
#define out_dw              r11:10

#define out_dw1             r13:12
//----------------------------
#define ret_val             r0

/*============================================================================*/

qhmath_clipping_ah:
{
    p0 = !cmp.eq(input, #0)
    p0 = !cmp.eq(output, #0)
    p0 = cmp.gtu(size_arr, #0)
    p0 = cmp.gt(highest, lowest)
    if (!p0.new) jump:nt .L_input_error
}
{
    loop_cnts = bitsplit(size_arr, #2)         // vector_loop_cnt = size_arr / 4
                                               // scalar_loop_cnt = size_arr % 4

    lo_w = combine(lowest.l, lowest.l)         // create comparation vector
    hi_w = combine(highest.l, highest.l)

    p0 = cmp.gt(size_arr, #3)
    if (!p0.new) jump:nt .L_SCALAR
}
{
    p1 = cmp.gt(vector_loop_cnt, #1)           // >= 8
    p2 = cmp.gt(vector_loop_cnt, #2)           // >= 12

    vector_loop_cnt = add(vector_loop_cnt, #-1)
}
{
    if (p1) in_dw = memd(input++#8)

    lo_dw = combine(lo_w, lo_w)
    hi_dw = combine(hi_w, hi_w)

    p3 = sp2loop0(.L_LOOP_VECTOR, vector_loop_cnt)
}

    .falign
.L_LOOP_VECTOR:
{
    if (p3) memd(output++#8) = out_dw
    in_dw = memd(input++#8)
    out_dw1 = vminh(in_dw, hi_dw)
    out_dw = vmaxh(out_dw1, lo_dw)
}:endloop0

    .falign
.L_cleanup:
{
    if (p2) memd(output++#8) = out_dw

    out_dw1 = vminh(in_dw, hi_dw)
    out_dw = vmaxh(out_dw1, lo_dw)
}
{
    if (p1) memd(output++#8) = out_dw

    out_dw = vmaxh(out_dw1, lo_dw)
}
{
    memd(output++#8) = out_dw

    p0 = cmp.eq(scalar_loop_cnt, #0)           // exit
    if (p0.new) r0 = #0
    if (p0.new) jumpr:t r31
}

    .falign
.L_SCALAR:
{
    p0 = cmp.gt(scalar_loop_cnt, #1)
    p1 = cmp.gt(scalar_loop_cnt, #2)

    in_w = memh(input++#2)
}
{
    if (p0) in_w = memh(input++#2)

    out_dw1 = vminh(in_dw, hi_dw)
}
{
    if (p1) in_w = memh(input++#2)

    out_dw1 = vminh(in_dw, hi_dw)
    out_dw = vmaxh(out_dw1, lo_dw)
}
{
    memh(output++#2) = out_w

    out_dw1 = vminh(in_dw, hi_dw)
    out_dw = vmaxh(out_dw1, lo_dw)

    if (!p0) jump:nt .L_RETURN_OK              // no XTYPE slot for jumpr r31
}
{
    memh(output++#2) = out_w

    out_dw1 = vminh(in_dw, hi_dw)
    out_dw = vmaxh(out_dw1, lo_dw)

    if (!p1) jump:nt .L_RETURN_OK              // no XTYPE slot for jumpr r31
}
{
    memh(output++#2) = out_w
}

    .falign
.L_RETURN_OK:
{
    r0 = #0
    jumpr r31
}

    .falign
.L_input_error:
{
    ret_val = #-1
    jumpr r31
}
    .size  qhmath_clipping_ah, .-qhmath_clipping_ah
