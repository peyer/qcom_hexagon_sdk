/**=============================================================================
@file
    qhmath_modexp_af.S

@brief
    Fuction composes an output floating-point array from input array by
    modifying exponent of each input element to k.

    Function prototype

        typedef float __attribute__ ((__aligned__(8))) float_a8_t;
        int32_t qhmath_modexp_af(float_a8_t *input, float_a8_t *output,
                                 uint32_t size, int32_t k);

    Reference C code

        int32_t qhmath_modexp_af(float_a8_t *input, float_a8_t *output,
                                 uint32_t size, int32_t k);
        {
            if ((input == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            for (int i = 0; i < size; i++)
            {
                output[i] = qhmath_modexp_f(input[i]);
            }

            return 0;
        }

@note
    Convert k into IEEE 754 representation for exponent (add 127).

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_modexp_af
    .type qhmath_modexp_af, @function

/*============================================================================*/

#define input           r0
#define output          r1
#define size_arr        r2
#define new_exponent    r3

#define out_dw          r7:6
#define out_1           r7
#define out_0           r6

#define odd_cond        p2

#define return_reg      r0

/*============================================================================*/

qhmath_modexp_af:
{
    p0 = cmp.gtu(size_arr,#0)
    if (!p0.new) jump:nt .L_input_error
    odd_cond = tstbit(size_arr,#0)
    size_arr = lsr(size_arr,#1)
    new_exponent = add(new_exponent,#0x7f)
}
{
    p0 = cmp.gtu(input,#0)
    p0 = cmp.gtu(output,#0)
    p0 = cmp.ge(new_exponent,#0)
    p0 = !cmp.gt(new_exponent,#254)
    if (!p0.new) jump:nt .L_input_error
}
{
    p1 = cmp.gtu(size_arr,#0)
    if (!p1.new) jump:nt .L_odd_member

    p3 = sp1loop0(.L_loop,size_arr)
}
    .falign
.L_loop:
{
    if (p3) memd(output++#8) = out_dw

    out_dw = memd(input++#8)
}
{
    out_1 = insert(new_exponent,#8,#23)
    out_0 = insert(new_exponent,#8,#23)
}:endloop0
    .falign
.L_cleanup:
{
    memd(output++#8) = out_dw
}
    .falign
.L_odd_member:
{
    return_reg = #0

    if (!odd_cond) jumpr r31

    if (odd_cond) out_0 = memw(input+#0)
}
{
    out_0 = insert(new_exponent,#8,#23)
    memw(output+#0) = out_0.new

    jumpr r31
}
    .falign
.L_input_error:
{
    return_reg = #-1
    jumpr r31
}
    .size qhmath_modexp_af, .-qhmath_modexp_af
