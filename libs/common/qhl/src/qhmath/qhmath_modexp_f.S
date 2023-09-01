/**=============================================================================
@file
    qhmath_modexp_f.S

@brief
    Modifies exponent of single-precision floating-point value.
    Returns a floating-point value with the same mantissa and sign as x
    and the exponent k.

    Function prototype

        float qhmath_modexp_f(float x, int32_t k);

    Reference C code

        typedef union
        {
            float value;
            struct
            {
                uint32_t mantissa : 23;
                uint32_t exponent : 8;
                uint32_t sign : 1;
            };
        } fp_iee754_t;

        float qhmath_modexp_f(float x, int32_t new_exponent)
        {
            ((fp_iee754_t *)&x)->exponent = new_exponent + 0x7F;
            return ((fp_iee754_t *)&x)->value;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_modexp_f
    .type qhmath_modexp_f, @function

/*============================================================================*/

qhmath_modexp_f:
{
    r2 = add(r1, #0x7F)
}
{
    r0 = insert(r2, #8, #23)
    jumpr r31
}
    .size qhmath_modexp_f, .-qhmath_modexp_f
