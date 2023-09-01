/**=============================================================================
@file
    qhmath_macros.h

@brief
    Internal macros used in QHMATH implementation.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#define __GET_NTH(_1,_2,N,...) N

#define __CHECK_1_INPUT(INPUT) (INPUT == NULL)
#define __CHECK_2_INPUTS(INPUT_1, INPUT_2) ((INPUT_1 == NULL) || (INPUT_2 == NULL))
#define __CHECK_INPUTS(...) __GET_NTH(__VA_ARGS__, __CHECK_2_INPUTS, __CHECK_1_INPUT)(__VA_ARGS__)

#define __GET_1_INPUT(COUNTER, INPUT) INPUT[COUNTER]
#define __GET_2_INPUTS(COUNTER, INPUT_1, INPUT_2) INPUT_1[COUNTER], INPUT_2[COUNTER]
#define __GET_INPUTS(COUNTER,...) __GET_NTH(__VA_ARGS__, __GET_2_INPUTS, __GET_1_INPUT)(COUNTER, __VA_ARGS__)

#define QHMATH_VECTORIZE_FUN(FNAME, OUTPUT_PTR, SIZE, ...)                  \
    if ((OUTPUT_PTR == NULL) || (SIZE == 0) || __CHECK_INPUTS(__VA_ARGS__)) \
    {                                                                       \
        return -1;                                                          \
    }                                                                       \
                                                                            \
    for (uint32_t i = 0; i < SIZE; i++)                                     \
    {                                                                       \
        OUTPUT_PTR[i] = FNAME(__GET_INPUTS(i,__VA_ARGS__));                 \
    }                                                                       \
                                                                            \
    return 0;