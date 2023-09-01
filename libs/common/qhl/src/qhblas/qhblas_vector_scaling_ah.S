/**=============================================================================
@file
    qhblas_vector_scaling_ah.S

@brief
    Scales input vector with scaling factor and stores the result in the output vector.

    Function prototype
        
        int32_t qhblas_vector_scaling_ah(int16_a8_t *input,int16_t scale_factor
                                         int16_a8_t *output, uint32_t size);

    Reference C code
        
        int32_t qhblas_vector_scaling_ah(int16_a8_t *input,int16_t scale_factor
                                         int16_a8_t *output, uint32_t size)
        {
            if ((input == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            for (uint32_t i = 0; i < size; ++i)
            {
                output[i] = input[i] * scale_factor;
            }

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhblas_vector_scaling_ah
    .type qhblas_vector_scaling_ah, @function

/*============================================================================*/

#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    256                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/8                   // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_scaling_ah:
{
    p0 = !cmp.eq(r0,#0)
    p0 = !cmp.eq(r2,#0)
    p0 = cmp.gtu(r3,#0)
    if (!p0.new) jump:nt .L_ret
}
{
    r3 = lsr(r3,#2)                                   // size / 4
    r4 = and(r3,#3)                                   // size & 3
    if(cmp.eq(r3.new,#0)) jump:nt .L_do_one
}
{
    r10 = combine(r1.l,r1.l)
    r5 = add(r3,#7)                                   // (size / 4) + 7
    p1 = cmp.gtu(r3,#L2_PREFETCH_ELEMS)               // check whether we can do l2fetch
}
{
    r13:12 = combine(##L2FETCH_CONFIG,#8)             // set l2fetch config and max number of iterations for .L_loop_do_four
    r5 = lsr(r5,#3)                                   // ceil(size / 4)
}
{
    r14 = mux(p1,r3,#0)                               // set l2fetch counter
    loop1(.L_prefetch_loop_do_four,r5)
}
    .falign
.L_prefetch_loop_do_four:
{
    dcfetch(r0+#DC_PREFETCH_AHEAD)                    // prefetch ahead for input
    r5 = min(r12,r3)                                  // min(8, size / 4)
}
{
    p3 = sp2loop0(.L_loop_do_four,r5)
    p1 = cmp.eq(r3,r14)                               // check whether to do l2fetch
    if (!p1.new) jump:t .L_loop_do_four
}
{
    r5 = add(r3,#-L2_PREFETCH_ELEMS)                  // number of elements left to prefetch ahead
    r15 = add(r0,#L2_PREFETCH_AHEAD)                  // input addr for l2fetch
}
{
    p1 = cmp.gtu(r5,#L2_PREFETCH_ELEMS)               // check whether we can continue to do l2fetch
    if (p1.new) r14 = add(r14,#-L2_PREFETCH_ELEMS)    // adjust l2fetch counter
    if (!p1.new) r14 = #0                             // there are no more bytes left to prefetch ahead
    l2fetch(r15,r13)
}
    .falign
.L_loop_do_four:
{
    r7:6 = memd(r0++#8)
    r9 = vmpyh(r7,r10):<<1:rnd:sat
    r8 = vmpyh(r6,r10):<<1:rnd:sat
    if (p3) memd(r2++#8) = r9:8
}:endloop0
{
    p1 = cmp.eq(r3,#1)
    if (!p1.new) memd(r2++#8) = r9:8
    r9 = vmpyh(r7,r10):<<1:rnd:sat
    r8 = vmpyh(r6,r10):<<1:rnd:sat
}
{
    r3 = add(r3,#-8)                                  // adjust (size / 4)
    memd(r2++#8) = r9:8
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
    r6 = memh(r0++#2)
}
{
    r7 = mpy(r1,r6.l):<<1:rnd:sat
    memh(r2++#2) = r7.new
}:endloop0
    .falign
.L_ret:
{
    r0 = mux(p0,#0,#-1)
    jumpr r31
}
    .size   qhblas_vector_scaling_ah, .-qhblas_vector_scaling_ah
