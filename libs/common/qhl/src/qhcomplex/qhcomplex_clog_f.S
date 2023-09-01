/**=============================================================================
@file
    qhcomplex_clog_f.S

@brief
    Computes the complex natural (base-e) logarithm of z.

    Function prototype

        float complex qhcomplex_clog_f(float complex x)

    Reference C code

        float complex clogf(float complex z)
        {
            float r, phi;

            r = cabsf(z);
            phi = cargf(z);
            return (complex float){logf(r), phi};
        }

@note
    Reference C code obtained from musl library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

.p2align 2
.p2align 4,,15
.global qhcomplex_clog_f
.type qhcomplex_clog_f, @function

/*============================================================================*/

#define Im          r17
#define Re          r16
#define Z           r17:16
#define temp        r19:18
#define temp_reg_1  r18
#define temp_reg_2  r19

qhcomplex_clog_f:
{
    Z = combine(r1,r0)
    memd(r29+#-16) = Z
    allocframe(#16)
}
{
    call qhcomplex_cabs_f
    memd(r29+#0) = temp
}
{
    call qhmath_log_f
}
{
    call qhcomplex_carg_f
    temp_reg_1 = r0
    r1:0 = combine(Im,Re)
}
{
    temp_reg_2 = r0
    Z = memd(r29+#8)
}
{
    r1:0 = combine(temp_reg_2,temp_reg_1)
    temp = memd(r29+#0)
    dealloc_return
}
    .size qhcomplex_clog_f, .-qhcomplex_clog_f
