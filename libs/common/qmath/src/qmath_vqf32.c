/**=============================================================================

@file
   qmath_vqf32.c

@brief
   HVX Implementations of 32-bit vector qfloat (vqf32) math operations.

Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
// includes
#include "qmath.h"

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
QMATH_API qm_vqf32_t
qm_vqf32_from_int(int* src)
{
    return qm_vqf32_from_int_inl(src);
}

QMATH_API qm_vqf32_t
qm_vqf32_from_float(float* src)
{
    return qm_vqf32_from_float_inl(src);
}

QMATH_API void
qm_vqf32_to_float(qm_vqf32_t src, float* dst)
{
    return qm_vqf32_to_float_inl(src, dst);
}

QMATH_API qm_vqf32_t
qm_vqf32_from_double(double* src)
{
    return qm_vqf32_from_double_inl(src);
}

QMATH_API void
qm_vqf32_to_double(qm_vqf32_t src, double* dst)
{
    return qm_vqf32_to_double_inl(src, dst);
}

QMATH_API qm_vqf32_t
qm_vqf32_mpy(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_mpy_inl(a, b);
}

QMATH_API qm_vqf32_t
qm_vqf32_add(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_add_inl(a, b);
}

QMATH_API qm_vqf32_t
qm_vqf32_sub(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_sub_inl(a, b);
}

QMATH_API qm_vqf32_t
qm_vqf32_mpyadd(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_mpyadd_inl(acc, a, b);
}

QMATH_API qm_vqf32_t
qm_vqf32_mpysub(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_mpysub_inl(acc, a, b);
}

QMATH_API qm_vqf32_t
qm_vqf32_negate(qm_vqf32_t a)
{
    return qm_vqf32_negate_inl(a);
}

QMATH_API qm_vqf32_t
qm_vqf32_abs(qm_vqf32_t a)
{
    return qm_vqf32_abs_inl(a);
}

QMATH_API qm_vqf32_t
qm_vqf32_min(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_min_inl(a, b);
}

QMATH_API qm_vqf32_t
qm_vqf32_max(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_max_inl(a, b);
}

QMATH_API HVX_VectorPred
qm_vqf32_lessthan(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_lessthan_inl(a, b);
}

QMATH_API HVX_VectorPred
qm_vqf32_greaterthan(qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_greaterthan_inl(a, b);
}

QMATH_API qm_vqf32_t     
qm_vqf32_inverse(qm_vqf32_t a)
{
    return qm_vqf32_inverse_inl(a);
}

QMATH_API qm_vqf32_t     
qm_vqf32_invsqrt(qm_vqf32_t a)
{
    return qm_vqf32_invsqrt_inl(a);
}

QMATH_API qm_vqf32_t     
qm_vqf32_sqrt(qm_vqf32_t a)
{
    return qm_vqf32_sqrt_inl(a);
}
