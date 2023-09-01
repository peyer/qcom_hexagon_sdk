/**=============================================================================
@file
    qhblas_vector_norm_ah.S

@brief
    Calculates Euclid norm of 16-bit input vector.

    Function prototype

        int32_t qhblas_vector_norm_ah(int16_a8_t *input, int32_t *output, uint32_t size);

    Reference C code

        int32_t qhblas_vector_norm_ah(int16_a8_t *input, int32_t *output, uint32_t size)
        {
            if ((input == NULL) || (output == NULL) || (size == 0))
            {
                return -1;
            }

            float norm = 0;
            
            for (uint32_t i = 0; i < size; ++i)
            {
                norm += input[i] * input[i];
            }

            norm = sqrtf(norm);
            *output = norm;
            
            return 0;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhblas_vector_norm_ah
    .type qhblas_vector_norm_ah, @function

/*============================================================================*/

#define shift_by_two         0x4f000000
#define DC_PREFETCH_AHEAD    64                                    // number of bytes for DCFETCH
#define L2_PREFETCH_AHEAD    512                                   // number of bytes for L2FETCH
#define L2FETCH_CONFIG       0x0100FF00+(L2_PREFETCH_AHEAD/256)    // [stride = 256 : width = 255 : height = bytes/256]
#define L2_PREFETCH_ELEMS    L2_PREFETCH_AHEAD/16                  // number of elements to prefetch with L2FETCH

/*============================================================================*/

qhblas_vector_norm_ah:
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
    r11:10 = #0
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
    r7:6 = #0
    r9:8 = #0
}
{
    r13:12 = #0
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
    r11:10 += vrmpyh(r7:6,r7:6)
    r13:12 += vrmpyh(r9:8,r9:8)
}:endloop0
{
    r2 = add(r2,#-4)                                  // adjust (size / 8)
}:endloop1
{
    r11:10 += vrmpyh(r7:6,r7:6)
    r13:12 += vrmpyh(r9:8,r9:8)
}
{
    r11:10 = add(r11:10,r13:12)
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
    r11:10 += mpy(r6.l,r6.l)
}:endloop0

    .falign
.L_ret:
{
    r16 = r1                                          // prevent output pointer from being clobbered
    r1:0 = asl(r11:10,#1)
}
{
    r0 = convert_d2sf(r1:0)
    r1 = ##shift_by_two
    call qhmath_div_f
}
{
    call qhmath_sqrt_f
    r17 = ##1199570944
}
{
    call roundf
    r0 = sfmpy(r0,r17)
}
{
    r0 = convert_sf2w(r0):chop
    r17:16 = memd(r29+#0)
    memw(r16) = r0.new                                // write result to output
}
{
    r0 = #0
    dealloc_return
}
    .falign
.L_ret_error:
{
    r0 = #-1
    jumpr r31
}
    .size   qhblas_vector_dot_ah, .-qhblas_vector_dot_ah
