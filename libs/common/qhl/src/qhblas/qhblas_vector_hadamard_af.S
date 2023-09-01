/**=============================================================================
@file
    qhblas_vector_hadamard_af.S

@brief
    Calculates Hadamard product, i.e element-wise multiplication, of two float
    input vectors and stores the result in the output vector.

    Function prototype
        
        int32_t qhblas_vector_hadamard_af(float_a8_t *input_1, float_a8_t *input_2,
                                          float_a8_t *output, uint32_t size);

    Reference C code
        
        int32_t qhblas_vector_hadamard_af(float_a8_t *input_1, float_a8_t *input_2,
                                          float_a8_t *output, uint32_t size)
        {
            if ((input_1 == NULL) || (input_2 == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            for (uint32_t i = 0; i < size; ++i)
            {
                output[i] = input_1[i] * input_2[i];
            }

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhblas_vector_hadamard_af
    .type qhblas_vector_hadamard_af, @function

/*============================================================================*/

#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    256                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/16                  // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_hadamard_af:
{
    p0 = !cmp.eq(r0,#0)                           // input_1 != NULL
    p0 = !cmp.eq(r1,#0)                           // input_2 != NULL
    p0 = !cmp.eq(r2,#0)                           // output != NULL
    p0 = cmp.gtu(r3,#0)                           // size > 0
    if (!p0.new) jump:nt .L_ret_error
}
{
    r3 = lsr(r3,#2)                               // size / 4
    r4 = and(r3,#3)                               // size & 3
    allocframe(#24)
    memd(r29+#-16) = r17:16
}
{
    p0 = cmp.eq(r3,#0)
    if (p0.new) jump:nt .L_do_one
    memd(r29+#8) = r19:18
    memd(r29+#0) = r21:20
}
{
    r5 = add(r3,#3)                               // (size / 4) + 3
    p1 = cmp.gtu(r3,#L2_PREFETCH_ELEMS)           // check whether we can do l2fetch
}
{
    r5 = lsr(r5,#2)                               // ceil(size / 4)
    r20 = mux(p1,r3,#0)                           // set l2fetch counter
}
{
    r19:18 = combine(##L2FETCH_CONFIG,#4)         // set l2fetch config and max number of iterations for .L_loop_do_four
    loop1(.L_prefetch_loop_do_four,r5)
}
    .falign
.L_prefetch_loop_do_four:
{
    dcfetch(r0+#DC_PREFETCH_AHEAD)                // prefetch ahead for input_1
    r5 = min(r18,r3)                              // min(4, size / 4)
}
{
    dcfetch(r1+#DC_PREFETCH_AHEAD)                // prefetch ahead for input_2
    p3 = sp1loop0(.L_loop_do_four,r5)
    p1 = cmp.eq(r3,r20)                           // check whether to do l2fetch
    if (!p1.new) jump:t .L_loop_do_four
}
{
    r5 = add(r3,#-L2_PREFETCH_ELEMS)              // number of elements left to prefetch ahead
    r21 = add(r0,#L2_PREFETCH_AHEAD)              // input_1 addr for l2fetch
}
{
    p1 = cmp.gtu(r5,#L2_PREFETCH_ELEMS)           // check whether we can continue to do l2fetch
    r21 = add(r1,#L2_PREFETCH_AHEAD)              // input_2 addr for l2fetch
    l2fetch(r21,r19)
}
{
    if (p1) r20 = add(r20,#-L2_PREFETCH_ELEMS)    // adjust l2fetch counter
    if (!p1) r20 = #0                             // there are no more bytes left to prefetch ahead
    l2fetch(r21,r19)
}
    .falign
.L_loop_do_four:
{
    r7:6 = memd(r0++#8)
    r9:8 = memd(r1++#8)
    r11 = sfmpy(r7,r9)
    r10 = sfmpy(r6,r8)
}
{
    r13:12 = memd(r0++#8)
    r15:14 = memd(r1++#8)
    r17 = sfmpy(r13,r15)
    r16 = sfmpy(r12,r14)
}
{
    if (p3) memd(r2++#16) = r11:10
    if (p3) memd(r2+#8) = r17:16
}:endloop0
{
    r11 = sfmpy(r7,r9)
    r10 = sfmpy(r6,r8)
}
{
    r17 = sfmpy(r13,r15)
    r16 = sfmpy(r12,r14)
}
{
    r3 = add(r3,#-4)                                 // adjust (size / 4)
    memd(r2++#16) = r11:10
    memd(r2+#8) = r17:16
}:endloop1
{
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
    r6 = memw(r0++#4)
    r7 = memw(r1++#4)
}
{
    r8 = sfmpy(r6,r7)
    memw(r2++#4) = r8.new
}:endloop0
    .falign
.L_ret:
{
    r17:16 = memd(r29+#16)
    r19:18 = memd(r29+#8)
}
{
    r0 = #0
    r21:20 = memd(r29+#0)
    dealloc_return
}
    .falign
.L_ret_error:
{
    r0 = #-1
    jumpr r31
}
    .size   qhblas_vector_hadamard_af, .-qhblas_vector_hadamard_af
