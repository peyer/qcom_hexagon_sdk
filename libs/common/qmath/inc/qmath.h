#ifndef QMATH_H
#define QMATH_H

/**=============================================================================

 @file
 qmath.h

 @brief
 Collection of math related utilities for HVX.

 Copyright (c) 2017 Qualcomm Technologies Incorporated.
 All Rights Reserved Qualcomm Proprietary

 Export of this technology or software is regulated by the U.S.
 Government. Diversion contrary to U.S. law prohibited.

 All ideas, data and information contained in or disclosed by
 this document are confidential and proprietary information of
 Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
 By accepting this material the recipient agrees that this material
 and the information contained therein are held in confidence and in
 trust and will not be used, copied, reproduced in whole or in part,
 nor its contents revealed in any manner to others without the express
 written permission of Qualcomm Technologies Incorporated.

 =============================================================================**/
//==============================================================================
// DEFINES
//==============================================================================
#ifdef BUILDING_SO
/// MACRO enables function to be visible in shared-library case.
#define QMATH_API __attribute__ ((visibility ("default")))
#else
/// MACRO empty for non-shared-library case.
#define QMATH_API
#endif

/// This library only supports 128B HVX mode.
#define QMATH_VLEN    128         

/// Using HIGH_QUALITY_INVERSE uses achieves a higher accuracy for 1/x inverse
/// (max error ~= 1e-9 vs. ~= 1e-7), but costs about 10% more cycles.
#define HIGH_QUALITY_INVERSE


#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Include Files
//==============================================================================
#include "hexagon_protos.h"
#include "hexagon_types.h"

/*===========================================================================
 TYPEDEF
 ===========================================================================*/
/// Upper vector contains 32 2's complement 32-bit exponents, lower contains 32 S1.30 format mantissas.
typedef HVX_VectorPair qm_vqf32_t;

//==============================================================================
// Declarations
//==============================================================================

/// 32-bit vector qfloat (vqf32) API's

//---------------------------------------------------------------------------
/// @brief
///   Convert array of 32 ints into a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param src
///   pointer to the array of ints.
///
/// @return
///   Converted vqf32 vector.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_from_int(int* src);
static inline qm_vqf32_t qm_vqf32_from_int_inl(int* src);

//---------------------------------------------------------------------------
/// @brief
///   Convert array of 32 floats into a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param src
///   pointer to the array of floats.
///
/// @return
///   Converted vqf32 vector.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_from_float(float* src);
static inline qm_vqf32_t qm_vqf32_from_float_inl(float* src);

//---------------------------------------------------------------------------
/// @brief
///   Convert a vqf32 vector into an array of 32 floats.
///
/// @detailed
///    TBD.
///
/// @param src
///   vqf32 vector to convert.
///
/// @param dst
///   pointer to the array of floats.
///
/// @return
///   none.
//---------------------------------------------------------------------------
QMATH_API void     qm_vqf32_to_float(qm_vqf32_t src, float* dst);
static inline void qm_vqf32_to_float_inl(qm_vqf32_t src, float* dst);

//---------------------------------------------------------------------------
/// @brief
///   Convert array of 32 doubles into a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param src
///   pointer to the array of doubles.
///
/// @return
///   Converted vqf32 vector.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_from_double(double* src);
static inline qm_vqf32_t qm_vqf32_from_double_inl(double* src);

//---------------------------------------------------------------------------
/// @brief
///   Convert a vqf32 vector into an array of 32 doubles.
///
/// @detailed
///    TBD.
///
/// @param src
///   vqf32 vector to convert.
///
/// @param dst
///   pointer to the array of doubles.
///
/// @return
///   none.
//---------------------------------------------------------------------------
QMATH_API void     qm_vqf32_to_double(qm_vqf32_t src, double* dst);
static inline void qm_vqf32_to_double_inl(qm_vqf32_t src, double* dst);

//---------------------------------------------------------------------------
/// @brief
///   Add a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of sums of a + b.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_add(qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_add_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Subtract a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of differences of a - b.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_sub(qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_sub_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Multiply a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of products of a * b.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_mpy(qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_mpy_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Multiply with accumulation on a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param acc
///   The accumulator vqf32 vector.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of acc + (a * b).
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_mpyadd(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_mpyadd_inl(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Multiply with subtraction on a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param acc
///   The accumulator vqf32 vector.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of acc - (a * b).
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_mpysub(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_mpysub_inl(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Negate a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector to negate.
///
/// @return
///   The vqf32 vector of -a.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_negate(qm_vqf32_t a);
static inline qm_vqf32_t qm_vqf32_negate_inl(qm_vqf32_t a);

//---------------------------------------------------------------------------
/// @brief
///   Absolute value of a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector to take absolute value of.
///
/// @return
///   The vqf32 vector of |a|.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_abs(qm_vqf32_t a);
static inline qm_vqf32_t qm_vqf32_abs_inl(qm_vqf32_t a);

//---------------------------------------------------------------------------
/// @brief
///   Return the minimum of a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of MIN(a, b).
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_min(qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_min_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Return the maximum of a pair of vqf32 vectors.
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vqf32 vector of MAX(a, b).
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_max(qm_vqf32_t a, qm_vqf32_t b);
static inline qm_vqf32_t qm_vqf32_max_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Return the vector predicate result of (a < b).
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vector predicate result of (a < b).
//---------------------------------------------------------------------------
QMATH_API HVX_VectorPred     qm_vqf32_lessthan(qm_vqf32_t a, qm_vqf32_t b);
static inline HVX_VectorPred qm_vqf32_lessthan_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Return the vector predicate result of (a > b).
///
/// @detailed
///    TBD.
///
/// @param a
///   The first vqf32 vector.
///
/// @param b
///   The second vqf32 vector.
///
/// @return
///   The vector predicate result of (a > b).
//---------------------------------------------------------------------------
QMATH_API HVX_VectorPred     qm_vqf32_greaterthan(qm_vqf32_t a, qm_vqf32_t b);
static inline HVX_VectorPred qm_vqf32_greaterthan_inl(qm_vqf32_t a, qm_vqf32_t b);

//---------------------------------------------------------------------------
/// @brief
///   Inverse (1/x) of a vqf32 vector.
///   \n\b WARNING: A zero input results in an infinity output, and vice-versa.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector to invert.
///
/// @return
///   The vqf32 vector of 1/a.
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_inverse(qm_vqf32_t a);
static inline qm_vqf32_t qm_vqf32_inverse_inl(qm_vqf32_t a);

//---------------------------------------------------------------------------
/// @brief
///   Inverse square root (1/sqrt(x)) of a vqf32 vector.
///   \n\b WARNING: Each element must be in the range 0 < a < inf.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector to invert & square root.
///
/// @return
///   The vqf32 vector of 1/sqrt(a).
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_invsqrt(qm_vqf32_t a);
static inline qm_vqf32_t qm_vqf32_invsqrt_inl(qm_vqf32_t a);

//---------------------------------------------------------------------------
/// @brief
///   Square root of a vqf32 vector.
///   \n\b WARNING: Each element must be in the range 0 <= a < inf.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector to square root.
///
/// @return
///   The vqf32 vector of sqrt(a).
//---------------------------------------------------------------------------
QMATH_API qm_vqf32_t     qm_vqf32_sqrt(qm_vqf32_t a);
static inline qm_vqf32_t qm_vqf32_sqrt_inl(qm_vqf32_t a);

//---------------------------------------------------------------------------
/// @brief
///   Extract the vector of mantissas from a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector.
///
/// @return
///   The HVX vector of a's mantissas.
//---------------------------------------------------------------------------
static inline HVX_Vector qm_vqf32_getmant(qm_vqf32_t a);

//---------------------------------------------------------------------------
/// @brief
///   Extract the vector of exponents from a vqf32 vector.
///
/// @detailed
///    TBD.
///
/// @param a
///   The vqf32 vector.
///
/// @return
///   The HVX vector of a's exponents.
//---------------------------------------------------------------------------
static inline HVX_Vector qm_vqf32_getexp(qm_vqf32_t a);

// pull in inline implementations
#include "qmath.inl"

#ifdef __cplusplus
}
#endif

#endif  // #ifndef QMATH_H
