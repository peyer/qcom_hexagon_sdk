/**=============================================================================
@file
    qhblas_vector_dot_ah.S

@brief
    Calculates dot product of two input 16-bit vectors.

    Function prototype
        
        int32_t qhblas_vector_dot_ah(int16_a8_t *input_1, int16_a8_t *input_2, int64_t *output, uint32_t size);

    Reference C code
        
        int32_t qhblas_vector_dot_ah(int16_a8_t *input_1, int16_a8_t *input_2, int64_t *output, uint32_t size)
        {
            if ((input_1 == NULL) || (input_2 == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            int64_t dot = 0;
            for (uint32_t i = 0; i < size; ++i)
            {
                dot += input_1[i] * input_2[i];
            }

            *output = dot;
            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhblas_vector_dot_ah
    .type qhblas_vector_dot_ah, @function

/*============================================================================*/

#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    256                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/8                   // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_dot_ah:
{
    p0 = !cmp.eq(r0,#0)                           // input_1 != NULL
    p0 = !cmp.eq(r1,#0)                           // input_2 != NULL
    p0 = !cmp.eq(r2,#0)                           // output != NULL
    p0 = cmp.gtu(r3,#0)                           // size > 0
    if (!p0.new) jump:nt .L_ret
}
{
    r11:10 = #0
    r3 = lsr(r3,#2)                               // size / 4
    r4 = and(r3,#3)                               // size & 3
    if(cmp.eq(r3.new,#0)) jump:nt .L_do_one
}
{
    r7:6 = #0
    r9:8 = #0
    r5 = add(r3,#7)                               // (size / 4) + 7
    p1 = cmp.gtu(r3,#L2_PREFETCH_ELEMS)           // check whether we can do l2fetch
}
{
    r5 = lsr(r5,#3)                               // ceil(size / 4)
    r14 = mux(p1,r3,#0)                           // set l2fetch counter
}
{
    r13:12 = combine(##L2FETCH_CONFIG,#8)         // set l2fetch config and max number of iterations for .L_loop_do_four
    loop1(.L_prefetch_loop_do_four,r5)
}
    .falign
.L_prefetch_loop_do_four:
{
    dcfetch(r0+#DC_PREFETCH_AHEAD)                // prefetch ahead for input_1
    r5 = min(r12,r3)                              // min(8, size / 4)
}
{
    dcfetch(r1+#DC_PREFETCH_AHEAD)                // prefetch ahead for input_2
    loop0(.L_loop_do_four,r5)
    p1 = cmp.eq(r3,r14)                           // check whether to do l2fetch
    if (!p1.new) jump:t .L_loop_do_four
}
{
    r5 = add(r3,#-L2_PREFETCH_ELEMS)              // number of elements left to prefetch ahead
    r15 = add(r0,#L2_PREFETCH_AHEAD)              // input_1 addr for l2fetch
}
{
    p1 = cmp.gtu(r5,#L2_PREFETCH_ELEMS)           // check whether we can continue to do l2fetch
    r15 = add(r1,#L2_PREFETCH_AHEAD)              // input_2 addr for l2fetch
    l2fetch(r15,r13)
}
{
    if (p1) r14 = add(r14,#-L2_PREFETCH_ELEMS)    // adjust l2fetch counter
    if (!p1) r14 = #0                             // there are no more bytes left to prefetch ahead
    l2fetch(r15,r13)
}
    .falign
.L_loop_do_four:
{
    r7:6 = memd(r0++#8)
    r9:8 = memd(r1++#8)
    r11:10 += vrmpyh(r7:6,r9:8)
    r3 = add(r3,#-1)                              // adjust (size / 4)
}:endloop0:endloop1
{
    r11:10 += vrmpyh(r7:6,r9:8)
    p1 = cmp.eq(r4,#0)
    if (p1.new) jump:nt .L_ret
}
    .falign
.L_do_one:
{
    loop0(.L_loop_do_one,r4)
}
    .falign
.L_loop_do_one:
{
    r6 = memh(r0++#2)
    r7 = memh(r1++#2)
}
{
    r11:10 += mpy(r6,r7)
}:endloop0
    .falign
.L_ret:
{
    r0 = mux(p0,#0,#-1)
    r11:10 = asl(r11:10,#1)
}
{
    if (p0) memd(r2) = r11:10
    jumpr r31
}
    .size   qhblas_vector_dot_ah, .-qhblas_vector_dot_ah
