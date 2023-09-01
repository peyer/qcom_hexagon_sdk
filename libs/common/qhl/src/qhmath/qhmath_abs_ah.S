/**=============================================================================
@file
    qhmath_abs_ah.S

@brief
    Computes absolute value of each element in 16-bit fixed-point input array
    and returns array of results.

    Function prototype

        typedef int16_t __attribute__ ((__aligned__(8))) int16_a8_t;

        int32_t qhmath_abs_ah(int16_a8_t *input, int16_a8_t *output, uint32_t size);

    Reference C code

        int32_t qhmath_abs_ah(int16_a8_t *input, int16_a8_t *__restrict output, uint32_t size)
        {
            if (!input || !output)
            {
                return -1;
            }

            if (size == 0)
            {
                return -1;
            }

            for (uint32_t i = 0; i < size; i++)
            {
                int16_t tmp = input[i];

                if (tmp < 0)
                    tmp = -tmp;

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
    .global qhmath_abs_ah
    .type qhmath_abs_ah, @function

/*============================================================================*/

#define input               r0
#define output              r1
#define size_arr            r2
//----------------------------
#define scalar_loop_cnt     r6
#define vector_loop_cnt     r7
#define loop_cnts           r7:6

#define in_w                r8
#define in_dw               r9:8

#define out_w               r10
#define out_dw              r11:10

#define in_addr             r7
//----------------------------
#define ret_val             r0

/*============================================================================*/

qhmath_abs_ah:
{
    p0 = !cmp.eq(input, #0)
    p0 = !cmp.eq(output, #0)
    p0 = cmp.gtu(size_arr, #0)
    if (!p0.new) jump:nt .L_input_error

    loop_cnts = bitsplit(size_arr, #2)         // vector_loop_cnt = size_arr / 4
                                               // scalar_loop_cnt = size_arr % 4
}
{
    p0 = cmp.gt(size_arr, #3)
    if (!p0.new) jump:nt .L_SCALAR

    vector_loop_cnt = add(vector_loop_cnt, #-1)

    p1 = cmp.gt(vector_loop_cnt, #1)           // size_arr >= 8
    p2 = cmp.gt(vector_loop_cnt, #2)           // size_arr >= 12
}
{
    if (p1) in_dw = memd(input++#8)

    p3 = sp1loop0(.L_LOOP_VECTOR, vector_loop_cnt)
}

    .falign
.L_LOOP_VECTOR:
{
    if (p3) memd(output++#8) = out_dw

    in_dw = memd(input++#8)

    out_dw = vabsh(in_dw)
}:endloop0

    .falign
.L_cleanup:
{
    if (p1) memd(output++#8) = out_dw

    out_dw = vabsh(in_dw)
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
    in_w = memh(input+#0)
    in_addr = add(input, #2)
    ret_val = #0
}
{
    p0 = cmp.gt(scalar_loop_cnt, #1)
    if (p0.new) in_w = memh(in_addr++#2)

    out_dw = vabsh(in_dw)

    p1 = cmp.gt(scalar_loop_cnt, #2)
}
{
    memh(output++#2) = out_w

    if (p1) in_w = memh(in_addr++#2)

    out_dw = vabsh(in_dw)

    if (!p0) jumpr:nt r31
}
{
    memh(output++#2) = out_w

    out_dw = vabsh(in_dw)

    if (!p1) jumpr:nt r31
}
{
    memh(output++#2) = out_w

    jumpr r31
}

    .falign
.L_input_error:
{
    ret_val = #-1
    jumpr r31
}
    .size  qhmath_abs_ah, .-qhmath_abs_ah
