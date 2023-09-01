/**=============================================================================
@file
    qhblas_vector_max_ah.S

@brief
    Calculates maximum element of 16-bit input vector.

    Function prototype
        
        int32_t qhblas_vector_max_ah(int16_a8_t *input, int16_t *maximum, uint32_t size);

    Reference C code
        
        int32_t qhblas_vector_max_ah(int16_a8_t *input, int16_t *maximum, uint32_t size)
        {
            if ((input == NULL) || (maximum == NULL) || (size == 0))
            {
                return -1;
            }

            int16_t max = input[0];
            
            for (uint32_t i = 0; i < size; ++i)
            {
                max = ((input[i] > max) ? input[i] : max);
            }

            *maximum = max;

            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhblas_vector_max_ah
    .type qhblas_vector_max_ah, @function

/*============================================================================*/

#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    512                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/16                  // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_max_ah:
{
    p0 = !cmp.eq(r0,#0)                           // input != NULL
    p0 = !cmp.eq(r1,#0)                           // output != NULL
    p0 = cmp.gtu(r2,#0)                           // size > 0
    if (!p0.new) jump:nt .L_ret_error
}
{
    r2 = lsr(r2,#3)                                   // size / 8
    r4 = and(r2,#7)                                   // size & 7
    allocframe(#8)
    memd(r29+#-16) = r17:16
}
{
    r3 = memh(r0)
    p0 = cmp.eq(r2,#0)
    if (p0.new) jump:nt .L_do_one
}
{
    r5 = add(r2,#3)                                   // (size / 8) + 3
    p1 = cmp.gtu(r2,#L2_PREFETCH_ELEMS)               // check whether we can do l2fetch
    r17:16 = combine(##L2FETCH_CONFIG,#4)             // set l2fetch config and max number of iterations for .L_loop_do_eight
}
{
    r5 = lsr(r5,#2)                                   // ceil(size / 8)
    r14 = mux(p1,r2,#0)                               // set l2fetch counter
    r7:6 = memd(r0+#0)
    r9:8 = memd(r0+#0)
}
{
    r11:10 = memd(r0+#0)
    r13:12 = memd(r0+#0)
    loop1(.L_prefetch_loop_do_eight,r5)
}
   .falign
.L_prefetch_loop_do_eight:
{
    dcfetch(r0+#DC_PREFETCH_AHEAD)                    // prefetch ahead for input
    r5 = min(r16,r2)                                  // min(4, size / 8)
}
{
    loop0(.L_loop_do_eight,r5)
    p1 = cmp.eq(r2,r14)                               // check whether to do l2fetch
    if (!p1.new) jump:t .L_loop_do_eight
}
{
    r5 = add(r2,#-L2_PREFETCH_ELEMS)                  // number of elements left to prefetch ahead
    r15 = add(r0,#L2_PREFETCH_AHEAD)                  // input addr for l2fetch
}
{
    p1 = cmp.gtu(r5,#L2_PREFETCH_ELEMS)               // check whether we can continue to do l2fetch
    if (p1.new) r14 = add(r14,#-L2_PREFETCH_ELEMS)    // adjust l2fetch counter
    if (!p1.new) r14 = #0                             // there are no more bytes left to prefetch ahead
    l2fetch(r15,r17)
}
    .falign
.L_loop_do_eight:
{
    r7:6 = memd(r0++#16)
    r9:8 = memd(r0+#8)
    r11:10 = vmaxh(r7:6,r11:10)
    r13:12 = vmaxh(r9:8,r13:12)
}:endloop0
{
    r2 = add(r2,#-4)                                  // adjust (size / 8)
}:endloop1
{
    r11:10 = vmaxh(r7:6,r11:10)
    r13:12 = vmaxh(r9:8,r13:12)
}
{
    r11:10 = vmaxh(r13:12,r11:10)
}
{
    r5 = sxth(r11)
    r6 = asrh(r11)
    r7 = sxth(r10)
    r8 = asrh(r10)
}
{
    r5 = max(r5,r6)
    r6 = max(r7,r8)
}
{
    r3 = max(r5,r6)
    p0 = cmp.eq(r4,#0)
    if (p0.new) jump:nt .L_ret
}
    .falign
.L_do_one:
{
    loop0(.L_loop_do_one,r4)
    r6 = memh(r0)
}
    .falign
.L_loop_do_one:
{
    r6 = memh(r0++#2)
    r3 = max(r6,r3)
}:endloop0
{
    r3 = max(r6,r3)
}
    .falign
.L_ret:
{
    r0 = #0
    memh(r1) = r3
}
{
    r17:16 = memd(r29+#0)
    dealloc_return
}
    .falign
.L_ret_error:
{
    r0 = #-1
    jumpr r31
}
    .size   qhblas_vector_max_ah, .-qhblas_vector_max_ah
