/**=============================================================================
@file
    qhblas_vector_hadamard_acf.S

@brief
    Calculates Hadamard product, i.e element-wise multiplication, of two complex float input
    vectors and stores the result in the output vector.

    Function prototype
        
        int32_t qhblas_vector_hadamard_acf(cfloat_a8_t *input_1, cfloat_a8_t *input_2,
                                           cfloat_a8_t *output, uint32_t size);

    Reference C code
        
        int32_t qhblas_vector_hadamard_acf(cfloat_a8_t *input_1, cfloat_a8_t *input_2,
                                           cfloat_a8_t *output, uint32_t size)
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
    .global qhblas_vector_hadamard_acf
    .type qhblas_vector_hadamard_acf, @function

/*============================================================================*/

#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    256                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/8                   // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_hadamard_acf:
{
    p0 = !cmp.eq(r0,#0)
    p0 = !cmp.eq(r1,#0)
    p0 = !cmp.eq(r2,#0)
    p0 = cmp.gtu(r3,#0)
    if (!p0.new) jump:nt .L_ret
}
{
    r5 = add(r3,#7)                               // size + 7
    p1 = cmp.gtu(r3,#L2_PREFETCH_ELEMS)           // check whether we can do l2fetch
}
{
    r5 = lsr(r5,#3)                               // ceil(size)
    r14 = mux(p1,r3,#0)                           // set l2fetch counter
}
{
    r13:12 = combine(##L2FETCH_CONFIG,#8)         // set l2fetch config and max number of iterations for .L_loop_do_one
    loop1(.L_prefetch_loop_do_one,r5)
}
    .falign
.L_prefetch_loop_do_one:
{
    dcfetch(r0+#DC_PREFETCH_AHEAD)                // prefetch ahead for input_1
    r5 = min(r12,r3)                              // min(8, size)
}
{
    dcfetch(r1+#DC_PREFETCH_AHEAD)                // prefetch ahead for input_2
    p3 = sp1loop0(.L_loop_do_one,r5)
    p1 = cmp.eq(r3,r14)                           // check whether to do l2fetch
    if (!p1.new) jump:t .L_loop_do_one
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
.L_loop_do_one:
{
    r7:6 = memd(r0++#8)
    r9:8 = memd(r1++#8)
    r10 -= sfmpy(r7,r9)
    r11 += sfmpy(r7,r8)
}
{
    r10 = sfmpy(r6,r8)
    r11 = sfmpy(r6,r9)
    if (p3) memd(r2++#8) = r11:10
}:endloop0
{
    r10 -= sfmpy(r7,r9)
    r11 += sfmpy(r7,r8)
}
{
    r3 = add(r3,#-8)                              // adjust size
    memd(r2++#8) = r11:10
}:endloop1
    .falign
.L_ret:
{
    r0 = mux(p0,#0,#-1)
    jumpr r31
}
    .size   qhblas_vector_hadamard_acf, .-qhblas_vector_hadamard_acf
