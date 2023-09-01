/**=============================================================================
@file
    qhmath_sigmoid_f.S

@brief
    Sigmoid function for floating-point values.

    Function prototype

        float qhmath_sigmoid_f(float x);

    Reference C code

        float qhmath_sigmoid_f(float x)
        {
            return 1/(expf(-x)+1);
        }

@note
    Sigmoid function is calculated as following:
    sigmoid(x) =  1.0/(e^(-x)+1.0)

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_sigmoid_f
    .type qhmath_sigmoid_f, @function

/*============================================================================*/

qhmath_sigmoid_f:
{
    allocframe(#8)
    r0 = togglebit(r0,#31)             // Create -x by toggling sign bit
    call qhmath_exp_f
}
{
    r2 = sfmake(#384):pos
}
{
    call qhmath_div_f
    r1 = sfadd(r0,r2)
    r0 = r2
}
{
    dealloc_return
}
    .size qhmath_sigmoid_f, .-qhmath_sigmoid_f
