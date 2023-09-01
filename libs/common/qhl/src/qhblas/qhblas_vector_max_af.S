/**=============================================================================
@file
    qhblas_vector_max_af.S

@brief
    Calculates maximum element of float input vector.

    Function prototype
        
        int32_t qhblas_vector_max_af(float_a8_t *input, float *maximum, uint32_t size);

    Reference C code
        
        int32_t qhblas_vector_max_af(float_a8_t *input, float *maximum, uint32_t size)
        {
            if ((input == NULL) || (maximum == NULL) || (size == 0))
            {
                return -1;
            }

            float max = input[0];
            
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
    .global qhblas_vector_max_af
    .type qhblas_vector_max_af, @function

/*============================================================================*/

#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    512                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/8                   // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_max_af:
{
    p0 = !cmp.eq(r0,#0)                               // input != NULL
    p0 = !cmp.eq(r1,#0)                               // output != NULL
    p0 = cmp.gtu(r2,#0)                               // size > 0
    if (!p0.new) jump:nt .L_ret
}
{
    r3 = memw(r0)
    r2 = lsr(r2,#1)                                   // size / 2
    p1 = tstbit(r2,#0)                                // check for odd size
    if(cmp.eq(r2.new,#0)) jump:nt .L_do_one
}
{
    r5 = add(r2,#7)                                   // (size / 2) + 7
    p2 = cmp.gtu(r2,#L2_PREFETCH_ELEMS)               // check whether we can do l2fetch
}
{
    r13:12 = combine(##L2FETCH_CONFIG,#8)             // set l2fetch config and max number of iterations for .L_loop_do_two
    r5 = lsr(r5,#3)                                   // ceil(size / 2)
}
{
    r7:6 = memd(r0+#0)
    r9:8 = memd(r0+#0)
    r14 = mux(p2,r2,#0)                               // set l2fetch counter
    loop1(.L_prefetch_loop_do_two,r5)
}
    .falign
.L_prefetch_loop_do_two:
{
    dcfetch(r0+#DC_PREFETCH_AHEAD)                    // prefetch ahead for input
    r5 = min(r12,r2)                                  // min(8, size / 2)
}
{
    loop0(.L_loop_do_two,r5)
    p2 = cmp.eq(r2,r14)                               // check whether to do l2fetch
    if (!p2.new) jump:t .L_loop_do_two
}
{
    r5 = add(r2,#-L2_PREFETCH_ELEMS)                  // number of elements left to prefetch ahead
    r15 = add(r0,#L2_PREFETCH_AHEAD)                  // input addr for l2fetch
}
{
    p2 = cmp.gtu(r5,#L2_PREFETCH_ELEMS)               // check whether we can continue to do l2fetch
    if (p2.new) r14 = add(r14,#-L2_PREFETCH_ELEMS)    // adjust l2fetch counter
    if (!p2.new) r14 = #0                             // there are no more bytes left to prefetch ahead
    l2fetch(r15,r13)
}
    .falign
.L_loop_do_two:
{
    r7:6 = memd(r0++#8)
    r8 = sfmax(r8,r6)
    r9 = sfmax(r9,r7)
    r2 = add(r2,#-1)                                  // adjust (size / 2)
}:endloop0:endloop1
{
    r8 = sfmax(r8,r6)
    r9 = sfmax(r9,r7)
}
{
    r3 = sfmax(r8,r9)
    if (!p1) jump:nt .L_ret
}
    .falign
.L_do_one:
{
    r6 = memw(r0)
}
{
    r3 = sfmax(r3,r6)
}
    .falign
.L_ret:
{
    if (p0) memw(r1) = r3
    r0 = mux(p0,#0,#-1)
    jumpr r31
}
    .size   qhblas_vector_max_af, .-qhblas_vector_max_af
