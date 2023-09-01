/**=============================================================================
@file
    qhmath_copysign_f.S

@brief
    Composes a floating-point value with the magnitude of x and the sign of y.

    Function prototype
        
        float qhmath_copysign_f(float x, float y);

    Reference C code

        float qhmath_copysign_f(float x, float y)
        {
            typedef union
            {
                float f;
                struct
                {
                    unsigned int : 31;
                    unsigned int s : 1;
                };
                unsigned int u;
            } fu_t;

            ((fu_t *)&x)->s = ((fu_t *)&y)->s;

            return (x);
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_copysign_f
    .type qhmath_copysign_f, @function

/*============================================================================*/

qhmath_copysign_f:
{
    r1 = insert(r0,#0x1f,#0x0)
}
{
    r0 = r1;
    jumpr r31
}
    .size qhmath_copysign_f, .-qhmath_copysign_f
