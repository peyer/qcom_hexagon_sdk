/**=============================================================================
@file
    qhmath_div_af.S

@brief
    
    Divides each element in input_num array by corresponding element in input_den array.

    Function prototype
        typedef float __attribute__ ((__aligned__(8))) float_a8_t;
        int32_t qhmath_div_af(float_a8_t *input_num, float_a8_t *input_den,
                              float_a8_t *output, uint32_t size);

    Reference C code

       int32_t qhmath_div_af(float_a8_t *input_num, float_a8_t *input_den,
                             float_a8_t *output, uint32_t size);
       {
           if ((input_num == NULL) || (input_den == NULL) || (output == NULL) || (size == 0))
           {
               return -1;
           }

           for (int i = 0; i < size; ++i)
           {
               output[i] = qhmath_div_f(input_num[i], input_den[i]);
           }

           return 0;
       }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_div_af
    .type qhmath_div_af, @function

/*============================================================================*/

#define input_num        r0
#define input_den        r1
#define output           r2
#define size_arr         r3
#define addresses_in     r17:16
#define addr_in_num      r16
#define addr_in_den      r17
#define addr_out         r20
#define loop_cnt         r18
#define loop_length      r19
#define loop_registers   r19:18

/*============================================================================*/

qhmath_div_af:
{
    p0 = cmp.gtu(input_num,#0)
    p0 = cmp.gtu(input_den,#0)
    p0 = cmp.gtu(output,#0)
    if (!p0.new) jump:nt .L_input_error
    allocframe(#24)
}
{
    p0 = cmp.gtu(size_arr, #0)
    if (!p0.new) jump:nt .L_input_error
    memw(sp+#16) = r20
    addr_out = r2
}
{
    addresses_in = r1:0
    memd(sp+#0) = r17:16
    memd(sp+#8) = r19:18
}
{
    loop_registers = combine(size_arr, #0)
}
    .falign
.L_loop:
{
    loop_cnt = add(loop_cnt,#1)
    r0 = memw(addr_in_num++#4)
    r1 = memw(addr_in_den++#4)
    call qhmath_div_f
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
    r20 = memw(sp+#16)
    dealloc_return
}
.L_input_error:
{
    r20 = memw(sp+#16)
    r0 = #-1
    dealloc_return
}
    .size qhmath_div_af, .-qhmath_div_af
