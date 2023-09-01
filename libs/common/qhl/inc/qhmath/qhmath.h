/**=============================================================================
@file
    qhmath.h

@brief
    Header file of QHMATH library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHMATH_H
#define _QHMATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef float __attribute__ ((__aligned__(8))) float_a8_t;
typedef int16_t __attribute__ ((__aligned__(8))) int16_a8_t;
typedef uint16_t __attribute__ ((__aligned__(8))) uint16_a8_t;
typedef int32_t __attribute__ ((__aligned__(8))) int32_a8_t;
typedef uint32_t __attribute__ ((__aligned__(8))) uint32_a8_t;

#define NAN       __builtin_nanf("")    /**< Not-A-Number */
#define INFINITY  __builtin_inff()      /**< Infinity */

static __inline uint32_t __FLOAT_BITS(float __f)
{
    union
    {
        float __f;
        unsigned __i;
    } __u;

    __u.__f = __f;
    return __u.__i;
}

#define signbit(x) ( \
     (int)(__FLOAT_BITS(x)>>31) )

#define isinf(x) ( \
    (__FLOAT_BITS(x) & 0x7fffffff) == 0x7f800000 )

#define isnan(x) ( \
    (__FLOAT_BITS(x) & 0x7fffffff) > 0x7f800000 )

#define isunordered(x,y) (isnan((x)) ? ((void)(y),1) : isnan((y)))

#define isequal(x,y) ((isnan((x)) && isnan((y)) ) || ( ((x)==(y)) ) || (qhmath_abs_f(((x)-(y))) < 0.00001 ) || ( (signbit((x)) == signbit((y)) ) && ((x)== 0.0f) && ((y)== 0.0f)) )


/** @defgroup qhmath_functions QHMATH functions
 *  @{
 */

/** @defgroup qhmath_16b_functions QHMATH 16-bit fixed-point functions
 *  @{
 */

/** @defgroup qhmath_16b_basic_operations QHMATH 16-bit basic operations
 *  @{
 */

/**
 * @brief       Computes the absolute value of a 16-bit fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument.
 * @return      Absolute value of x.
 */
uint16_t qhmath_abs_h(int16_t x);

/**
 * @brief       Division of 16-bit fixed-point numbers.
 *              Divides numerator by denominator.
 * @param[in]   n   Numerator.
 * @param[in]   d   Denominator.
 * @return      Result of division (n by d). Output represents Q8.8 format.
 */
int16_t qhmath_div_h(int16_t n, int16_t d);

/**
 * @} //qhmath_16b_basic_operations
 */

/** @defgroup qhmath_16b_exponential_functions QHMATH 16-bit exponential functions
 *  @{
 */

/**
 * @brief       Computes the exponential value to the given 16-bit fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument.
 * @return      Exponential value of x.
 * @note        Input argument is in Q4.11 signed fixed-point format.
 *              Max value of output is (2^32 - 1) in Q16.16 unsigned format.
 *              Min value of output is 0 in Q16.16 unsigned format.
 *              If the input value greater then 22712, output is set to max value 0xFFFFFFFF
 */
uint32_t qhmath_exp_h(int16_t x);

/**
 * @brief       Computes the 2 raised to the given argument.
 * @param[in]   x   16-bit fixed-point argument.
 * @return      Base-2 exponential of x.
 * @note        Input argument is in Q4.11 signed fixed-point format, which means that it is range [-16, 15.9995].
 *              Max value of output is (2^32 - 1) in Q16.16 unsigned format.
 *              Min value of output is 0 in Q16.16 unsigned format.
 */
uint32_t qhmath_exp2_h(int16_t x);

/**
 * @brief       Computes the 10 raised to the given argument.
 * @param[in]   x   16-bit fixed-point argument.
 * @return      Base-10 exponential of x.
 * @note        Input argument is in Q15 format, which means that it is range [-0.9999, 0.9999].
 *              Max value of output is 20480 in Q4.11 signed format.
 *              Min value of output is 204 in Q4.11 signed format.
 */
int16_t qhmath_exp10_h(int16_t x);

/**
 * @brief       Computes the natural logarithm of fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument in Q0 unsigned format.
 * @return      Natural logarithm of x in Q5.11 unsigned format.
 */
uint16_t qhmath_log_h(uint16_t x);

/**
 * @brief       Computes the base-2 logarithm of fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument in Q0 unsigned format.
 * @return      Base-2 logarithm of x in Q5.11 unsigned format.
 */
uint16_t qhmath_log2_h(uint16_t x);

/**
 * @brief       Computes the base-10 logarithm of fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument in Q0 unsigned format.
 * @return      Base-10 logarithm of x in Q5.11 unsigned format.
 */
uint16_t qhmath_log10_h(uint16_t x);

/**
 * @} //qhmath_16b_exponential_functions
 */

/** @defgroup qhmath_16b_power_functions QHMATH 16-bit power functions
 *  @{
 */

/**
 * @brief       Computes the square root of 16-bit fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument in Q0 format.
 * @return      Square root of x in Q8.8 unsigned fixed-point format.
 */
uint16_t qhmath_sqrt_h(uint16_t x);

/**
 * @brief       Computes the reciprocal of square root of 16-bit fixed-point argument.
 * @param[in]   x   16-bit fixed point argument in Q0 format.
 * @return      Reciprocal of square root of x in Q15 format.
 */
uint16_t qhmath_rsqrt_h(uint16_t x);

/**
 * @} //qhmath_16b_power_functions
 */

/** @defgroup qhmath_16b_trigonometric_functions QHMATH 16-bit trigonometric functions
 *  @{
 */

/**
 * @brief       Computes the sine of a 16-bit fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument.
 * @return      Sine of x in 16-bit fixed-point representation.
 * @note        Input range (-2^15 2^15) is scaled to (-2*pi, 2*pi).
 *              Output range (-2^15 2^15) is scaled to [-0.9999, 0.9999].
 */
int16_t qhmath_sin_h(int16_t x);

/**
 * @brief       Computes the cosine of 16-bit fixed-point argument.
 * @param[in]   x   16-bit fixed-point argument.
 * @return      Cosine of x in 16-bit fixed-point representation.
 * @note        Input range (-2^15 2^15) is scaled to (-2*pi, 2*pi).
 *              Output range (-2^15 2^15) is scaled to [-0.9999, 0.9999].
 */
int16_t qhmath_cos_h(int16_t x);

/**
 * @brief       Computes the tangent of 16-bit fixed-point argument.
 * @param[in]   x   Input data in Q3.12 fixed-point representation (angle in [0,2pi] range in fixed-point).
 * @return      Tangent of x in Q6.9 fixed-point representation.
 */
int16_t qhmath_tan_h(int16_t x);

/**
 * @brief       Computes the inverse tangent of 32-bit fixed-point argument.
 * @param[in]   x   32-bit fixed-point argument in Q15.16 (an angle expressed in radians).
 * @return      Inverse tangent of x in 16-bit fixed-point Q15 format.
 *              NOTE: Output result is y ~ atan(x)/pi
 */
int16_t qhmath_atan_h(int32_t x);

/**
 * @brief       4-quadrant arctan
 *              Returns the principal value of the arctan of y/x, expressed in radians.
 *              To compute the value, the function takes into account the sign of both
 *              arguments in order to determine the quadrant.
 * @param[in]   x   Value representing the proportion of the x-coordinate in 16-bit fixed-point Q15 format.
 * @param[in]   y   Value representing the proportion of the y-coordinate in 16-bit fixed-point Q15 format.
 * @return      Principal arctan of y/x, in the interval [-1.0,+1.0] radians.
 *              NOTE: Output result is y ~ atan(y/x)/pi
 */
int16_t qhmath_atan2_h(int16_t x, int16_t y);

/**
 * @} //qhmath_16b_trigonometric_functions
 */

/** @defgroup qhmath_16b_hyperbolic_functions QHMATH 16-bit hyperbolic functions
 *  @{
 */

/**
 * @} //qhmath_16b_hyperbolic_functions
 */

/** @defgroup qhmath_16b_manipulation_functions QHMATH 16-bit manipulation functions
 *  @{
 */

/**
 * @brief       Composes a 16-bit value with the absolute value of x and the sign of k.
 * @param[in]   x   16-bit value from which absolute value is taken.
 * @param[in]   k   16-bit value from which sign is taken.
 * @return      Composed 16-bit value.
 */
int16_t qhmath_copysign_h(int16_t x, int16_t k);

/**
 * @} //qhmath_16b_manipulation_functions
 */

/** @defgroup qhmath_16b_other_functions QHMATH 16-bit other functions
 *  @{
 */

/**
 * @brief       Computes the Sigmoid function of 16-bit fixed-point argument.
 * @param[in]   x   Input data in Q3.12 signed fixed-point representation for which sigmoid function is calculated.
 * @return      Sigmoid of x in 16-bit fixed-point Q15 signed format.
 *              NOTE: Output result is y ~ 1/(1+e^(-x))
 */
int16_t qhmath_sigmoid_h(int16_t x);

/**
 * @} //qhmath_16b_other_functions
 */

/**
 * @} //qhmath_16b_functions
 */


/** @defgroup qhmath_sp_fp_functions QHMATH single-precision floating-point functions
 *  @{
 */

/** @defgroup qhmath_sp_fp_basic_operations QHMATH SP FP basic operations
 *  @{
 */

/**
 * @brief       Computes the absolute value of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Absolute value of x.
 */
float qhmath_abs_f(float x);

/**
 * @brief       Division of single-precision floating-point values.
 *              Divides numerator by denominator.
 * @param[in]   n   Numerator.
 * @param[in]   d   Denominator.
 * @return      Result of division (n by d).
 */
float qhmath_div_f(float n, float d);

/**
 * @} //qhmath_sp_fp_basic_operations
 */

/** @defgroup qhmath_sp_fp_exponential_functions QHMATH SP FP exponential functions
 *  @{
 */

/**
 * @brief       Computes the e (Euler's number) raised to the given argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Base-e exponential of x.
 */
float qhmath_exp_f(float x);

/**
 * @brief       Computes the 2.0 raised to the given argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Base-2 exponential of x.
 */
float qhmath_exp2_f(float x);

/**
 * @brief       Computes the 10.0 raised to the given argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Base-10 exponential of x.
 */
float qhmath_exp10_f(float x);

/**
 * @brief       Computes the natural (base-e) logarithm of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Natural logarithm of x.
 */
float qhmath_log_f(float x);

/**
 * @brief       Computes the base-2 logarithm of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Base-2 logarithm of x.
 */
float qhmath_log2_f(float x);

/**
 * @brief       Computes the base-10 logarithm of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Base-10 logarithm of x.
 */
float qhmath_log10_f(float x);

/**
 * @} //qhmath_sp_fp_exponential_functions
 */

/** @defgroup qhmath_sp_fp_power_functions QHMATH SP FP power functions
 *  @{
 */

/**
 * @brief       Computes the value of base raised to the power exponent.
 * @param[in]   x   Base as single-precision floating-point value.
 * @param[in]   y   Exponent as single-precision floating-point value.
 * @return      The value of x raised to the y.
 */
float qhmath_pow_f(float x, float y);

/**
 * @brief       Computes the square root of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Square root of x.
 */
float qhmath_sqrt_f(float x);

/**
 * @brief       Computes the reciprocal of square root of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Reciprocal of square root of x.
 */
float qhmath_rsqrt_f(float x);

/**
 * @brief       Computes the hypotenuse of right-angled triangle.
 * @param[in]   a   One cathetus.
 * @param[in]   b   Another cathetus.
 * @return      Calculated hypotenuse.
 */
float qhmath_hypot_f(float a, float b);

/**
 * @} //qhmath_sp_fp_power_functions
 */

/** @defgroup qhmath_sp_fp_trigonometric_functions QHMATH SP FP trigonometric functions
 *  @{
 */

/**
 * @brief       Computes the sine of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Sine of x.
 */
float qhmath_sin_f(float x);

/**
 * @brief       Computes the cosine of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Cosine of x.
 */
float qhmath_cos_f(float x);

/**
 * @brief       Computes the tangent of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Tangent of x.
 */
float qhmath_tan_f(float x);

/**
 * @brief       Computes the cotangent of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Cotangent of x.
 */
float qhmath_cot_f(float x);

/**
 * @brief       Computes the inverse sine of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Inverse sine of x.
 */
float qhmath_asin_f(float x);

/**
 * @brief       Computes the inverse cosine of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Inverse cosine of x.
 */
float qhmath_acos_f(float x);

/**
 * @brief       Computes the inverse tangent of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Inverse tangent of x.
 */
float qhmath_atan_f(float x);

/**
 * @brief       4-quadrant arctan
 *              Returns the principal value of the arctan of y/x, expressed in radians.
 *              To compute the value, the function takes into account the sign of both
 *              arguments in order to determine the quadrant.
 * @param[in]   x   Value representing the proportion of the x-coordinate.
 * @param[in]   y   Value representing the proportion of the y-coordinate.
 * @return      Principal arctan of y/x, in the interval [-pi,+pi] radians.
 */
float qhmath_atan2_f(float x, float y);

/**
 * @} //qhmath_sp_fp_trigonometric_functions
 */

/** @defgroup qhmath_sp_fp_hyperbolic_functions QHMATH SP FP hyperbolic functions
 *  @{
 */

/**
 * @brief       Computes the hyperbolic sine of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Hyperbolic sine of x.
 */
float qhmath_sinh_f(float x);

/**
 * @brief       Computes the hyperbolic cosine of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Hyperbolic cosine of x.
 */
float qhmath_cosh_f(float x);

/**
 * @brief       Computes the hyperbolic tangent of single-precision floating-point argument.
 * @param[in]   x   Single-precision floating-point argument (an angle expressed in radians).
 * @return      Hyperbolic tangent of x.
 */
float qhmath_tanh_f(float x);

/**
 * @} //qhmath_sp_fp_hyperbolic_functions
 */

/** @defgroup qhmath_sp_fp_nearest_integer_functions QHMATH SP FP nearest integer functions
 *  @{
 */

/**
 * @brief       Computes the smallest integer value not less than given argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Smallest integer value not less than x.
 */
float qhmath_ceil_f(float x);

/**
 * @brief       Computes the largest integer value not greater than given argument.
 * @param[in]   x   Single-precision floating-point argument.
 * @return      Largest integer value not greater than x.
 */
float qhmath_floor_f(float x);

/**
 * @} //qhmath_sp_fp_nearest_integer_functions
 */

/** @defgroup qhmath_sp_fp_manipulation_functions QHMATH SP FP manipulation functions
 *  @{
 */

/**
 * @brief       Modifies exponent of single-precision floating-point value.
 *              Returns a floating-point value with the same mantissa and sign as x
 *              and the exponent k.
 * @param[in]   x   Floating-point value from which sign and mantissa are taken.
 * @param[in]   k   Exponent (base 2) of modified floating-point value.
 * @return      Floating-point value with modified exponent.
 */
float qhmath_modexp_f(float x, int32_t k);

/**
 * @brief       Composes a floating-point value with the magnitude of x and the sign of y.
 * @param[in]   x   Floating-point value from which magnitude is taken.
 * @param[in]   y   Floating-point value from which sign is taken.
 * @return      Composed floating-point value.
 */
float qhmath_copysign_f(float x, float y);

/**
 * @} //qhmath_sp_fp_manipulation_functions
 */

/** @defgroup qhmath_sp_fp_other_functions QHMATH SP FP other functions
 *  @{
 */

/**
 * @brief       Sigmoid function for floating-point values.
 * @param[in]   x   Floating-point value for which sigmoid function is calculated.
 * @return      Sigmoid of x.
 */
float qhmath_sigmoid_f(float x);

/**
 * @} //qhmath_sp_fp_other_functions
 */

/**
 * @} //qhmath_sp_fp_functions
 */

/** @defgroup qhmath_16b_vector_functions QHMATH 16-bit fixed-point vector functions
 *  @{
 */

/** @defgroup qhmath_16b_vector_basic_operations QHMATH 16-bit vector basic operations
 *  @{
 */

/**
 * @brief       Computes absolute value of each element in 16-bit fixed-point input array and returns array of results.
 * @param[in]   input   Input array of 16-bit fixed-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed absolute values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_abs_ah(int16_a8_t *input, int16_a8_t *output, uint32_t size);

/**
 * @brief       Divides each element in input_num array by corresponding element in input_den array.
 * @param[in]   input_num   Input array of numerators.
 * @param[in]   input_den   Input array of denominators.
 * @param[out]  output      Output array where division results will be stored.
 * @param[in]   size        Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 * @note        Elements of output array are in Q8.8 format.
 */
int32_t qhmath_div_ah(int16_t *input_num, int16_t *input_den, int16_t *output, uint32_t size);

/**
 * @} //qhmath_16b_vector_basic_operations
 */

/** @defgroup qhmath_16b_vector_exponential_functions QHMATH 16-bit vector exponential functions
 *  @{
 */

/**
 * @brief       Computes base-e exponential over the elements of 16-bit input array.
 * @param[in]   input   16-bit fixed-point array consisting of power arguments.
 * @param[out]  output  16-bit fixed-point where will be stored result for each element of input array.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution or -1 if some error occurred.
 * @note        Input argument is in Q4.11 signed fixed-point format.
 *              Max value in output array is (2^32 - 1) in Q16.16 unsigned format.
 *              Min value in output array is 0 in Q16.16 unsigned format.
 */
int32_t qhmath_exp_ah(int16_t *input, uint32_t *output, uint32_t size);

/**
 * @brief       Computes base-2 exponential over the elements of 16-bit input array.
 * @param[in]   input   16-bit fixed-point array consisting of power arguments.
 * @param[out]  output  16-bit fixed-point where will be stored result for each element of input array.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution or -1 if some error occurred.
 * @note        Input arguments are in Q4.11 signed fixed-point format, which means that they are in range [-16, 15.9995].
 *              Max value in output array is (2^32 - 1) in Q16.16 unsigned format.
 *              Min value in output array is 0 in Q16.16 unsigned format.
 */
int32_t qhmath_exp2_ah(int16_t *input, uint32_t *output, uint32_t size);

/**
 * @brief       Computes base-10 exponential over the elements of 16-bit input array.
 * @param[in]   input   16-bit fixed-point array consisting of power arguments.
 * @param[out]  output  16-bit fixed-point where will be stored result for each element of input array.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution or -1 if some error occurred.
 * @note        Input arguments are in Q15 format, which means they are in range [-0.9999, 0.9999].
 *              Max value in output array is 20480 in Q4.11 signed format.
 *              Min value in output array is 204 in Q4.11 signed format.
 */
int32_t qhmath_exp10_ah(int16_t *input, int16_t *output, uint32_t size);

/**
 * @brief       Computes the natural (base-e) logarithm over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q0 unsigned format for which natural logarithm is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q5.11 unsigned format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_log_ah(uint16_t *input, uint16_t *output, uint32_t size);

/**
 * @brief       Computes the base-2 logarithm over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q0 unsigned format for which base-2 logarithm is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q5.11 unsigned format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_log2_ah(uint16_t *input, uint16_t *output, uint32_t size);

/**
 * @brief       Computes the base-10 logarithm over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q0 unsigned format for which base-10 logarithm is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q5.11 unsigned format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_log10_ah(uint16_t *input, uint16_t *output, uint32_t size);

/**
 * @} //qhmath_16b_vector_exponential_functions
 */

/** @defgroup qhmath_16b_vector_power_functions QHMATH 16-bit vector power functions
 *  @{
 */

/**
 * @brief       Computes the square root over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q0 unsigned format for which square root is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q8.8 unsigned format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_sqrt_ah(uint16_t *input, uint16_t *output, uint32_t size);

/**
 * @brief       Computes the inverse square root over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q0 unsigned format for which inverse square root is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q15 format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_rsqrt_ah(uint16_t *input, uint16_t *output, uint32_t size);

/**
 * @} //qhmath_16b_vector_power_functions
 */

/** @defgroup qhmath_16b_vector_trigonometric_functions QHMATH 16-bit vector trigonometric functions
 *  @{
 */

/**
 * @brief       Computes the sine over the elements of 16-bit input array.
 * @param[in]   input   Array of elements for which sine is computed.
 * @param[out]  output  Output array where computed values will be stored.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 * @note        Input range (-2^15 2^15) is scaled to (-2*pi, 2*pi).
 *              Output range (-2^15 2^15) is scaled to [-0.9999, 0.9999].
 */
int32_t qhmath_sin_ah(int16_t *input, int16_t *output, uint32_t size);

/**
 * @brief       Computes the cosine over the elements of 16-bit input array.
 * @param[in]   input   Array of elements for which cosine is computed.
 * @param[out]  output  Output array where computed values will be stored.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 * @note        Input range (-2^15 2^15) is scaled to (-2*pi, 2*pi).
 *              Output range (-2^15 2^15) is scaled to [-0.9999, 0.9999].
 */
int32_t qhmath_cos_ah(int16_t *input, int16_t *output, uint32_t size);

/**
 * @brief       Computes the tangent over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q3.12 format for which tangent is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q6.9 format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 * @note        Input range (-2^15 2^15) is scaled to (-2*pi, 2*pi).
 *              Output range (-2^15 2^15) is scaled to [-0.9999, 0.9999].
 */
int32_t qhmath_tan_ah(int16_t *input, int16_t *output, uint32_t size);

/**
 * @brief       Computes the arctan over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q15.16 format for which arctan is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q15 format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_atan_ah(int32_t *input, int16_t *output, uint32_t size);

/**
 * @brief       Computes the 4-quadrant arctan over the elements of 16-bit input array.
 * @param[in]   input_x Values representing the proportion of the x-coordinate in 16-bit fixed-point Q15 format.
 * @param[in]   input_y Values representing the proportion of the y-coordinate in 16-bit fixed-point Q15 format.
 * @param[out]  output  Output array where computed values will be stored.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_atan2_ah(int16_t *input_x, int16_t *input_y, int16_t *output, uint32_t size);

/**
 * @} //qhmath_16b_vector_trigonometric_functions
 */

/** @defgroup qhmath_16b_vector_other_functions QHMATH 16-bit vector other functions
 *  @{
 */

/**
 * @brief       Composes output array of 16-bit values with the absolute value of inputs and the sign of k.
 * @param[in]   input   Array of 16-bit values from which absolute value is taken.
 * @param[in]   k       16-bit value from which sign is taken.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q15 format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_copysign_ah(int16_t *input, int16_t k, int16_t *output, uint32_t size);

/**
 * @brief       Clips 16-bit signed values from input array and store results to the same-size output array.
 *              If particular member is greather then high_level parameter, its value is set to high_level.
 *              If member's value is lower then low_level parameter, value of that member is set to low_level.
 *
 * @param[in]   samples     Input array of 16-bit values for clipping. Array has to be aligned to 8 bytes.
 * @param[out]  clipped     Output array of clipped 16-bit values. Array has to be aligned to 8 bytes.
 * @param[in]   size        Number of elements of input and output arrays.
 * @param[in]   low_level   Minimum clipping value.
 * @param[in]   high_level  Maximum clipping value.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_clipping_ah(int16_a8_t *samples, int16_a8_t *clipped, uint32_t size, int16_t low_level, int16_t high_level);

/**
 * @brief       Computes the sigmoid function over the elements of 16-bit input array.
 * @param[in]   input   Array of elements in Q3.12 format for which sigmoid function is computed.
 * @param[out]  output  Output array where computed values will be stored. Outputs are in Q15 format.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_sigmoid_ah(int16_t *input, int16_t *output, uint32_t size);

/**
 * @} //qhmath_16b_vector_other_functions
 */

/**
 * @} //qhmath_16b_vector_functions
 */

/** @defgroup qhmath_sp_fp_vector_functions QHMATH single-precision floating-point vector functions
 *  @{
 */

/** @defgroup qhmath_sp_fp_vector_basic_operations QHMATH SP_FP vector basic operations
 *  @{
 */

/**
 * @brief       Computes absolute value of each element in floating-point input array and returns array of results.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed absolute values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_abs_af(float_a8_t *input, float_a8_t *output, uint32_t size);

/**
 * @brief       Divides each element in input_num array by corresponding element in input_den array.
 * @param[in]   input_num   Input array of numerators.
 * @param[in]   input_den   Input array of denominators.
 * @param[out]  output      Output array where division results will be stored.
 * @param[in]   size        Number of elements of input and output arrays.
 * @return      Array of results of division operation.
 */
int32_t qhmath_div_af(float_a8_t *input_num, float_a8_t *input_den,
                    float_a8_t *output, uint32_t size);

/**
 * @} //qhmath_sp_fp_vector_basic_operations
 */

/** @defgroup qhmath_sp_fp_vector_exponential_functions QHMATH SP_FP vector exponential functions
 *  @{
 */

/**
 * @brief       Computes the e (Euler's number) raised to the given power arguments.
 * @param[in]   input   Floating-point array consisting of power arguments.
 * @param[out]  output  Floating-point array where will be stored result for each element of input array.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution or -1 if some error occurred.
 */
int32_t qhmath_exp_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the 2 raised to the given power arguments.
 * @param[in]   input   Floating-point array consisting of power arguments.
 * @param[out]  output  Floating-point array where will be stored result for each element of input array.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution or -1 if some error occurred.
 */
int32_t qhmath_exp2_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the 10 raised to the given power arguments.
 * @param[in]   input   Floating-point array consisting of power arguments.
 * @param[out]  output  Floating-point array where will be stored result for each element of input array.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution or -1 if some error occurred.
 */
int32_t qhmath_exp10_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the natural (base-e) logarithm over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_log_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the base-2 logarithm over the elements of input array and store results to the output array.
 * @param[in]   input    Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output   Output array where computed values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size     Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_log2_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the base-10 logarithm over the elements of input array and store results to the output array.
 * @param[in]   input    Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output   Output array where computed values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size     Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_log10_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @} //qhmath_sp_fp_vector_exponential_functions
 */

/** @defgroup qhmath_sp_fp_vector_power_functions QHMATH SP FP vector power functions
 *  @{
 */

/**
 * @brief       Computes the value of raised to the power exponent over the elements of
 *              input array and store results to the output array.
 * @param[in]   input   Array of bases as single-precision floating-point array.
 * @param[in]   exponent   Exponent as single-precision floating-point value.
 * param[out]   output: Array to store result of raised to the power exponent.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_pow_af(float_a8_t *input, float exponent, float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the square root over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed square root values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_sqrt_af(float_a8_t *input, float_a8_t *output, uint32_t size);

/**
 * @brief       Computes reciprocal of square root over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed reciprocal square root values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_rsqrt_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @} //qhmath_sp_fp_vector_power_functions
 */

/** @defgroup qhmath_sp_fp_vector_trigonometric_functions QHMATH SP_FP vector trigonometric functions
 *  @{
 */

/**
 * @brief       Computes the sine over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed sine values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_sin_af(float_a8_t *input, float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the cosine over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed cosine values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_cos_af(float_a8_t *input, float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the tangent over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed tangent values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_tan_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the cotangent over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed cotangent values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_cot_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the inverse sine over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed inverse sine values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_asin_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the inverse cosine over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed inverse cosine values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_acos_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the inverse tangent over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed inverse tangent values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_atan_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the atan2 over the elements of input array and store results to the output array.
 * @param[in]   input_x     Input array of x-axis values.
 * @param[in]   input_y     Input array of y-axis values.
 * @param[out]  output      Output array where will be store results.
 * @param[in]   size        Number of input/output elements.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_atan2_af(float_a8_t *input_x, float_a8_t *input_y,
                        float_a8_t *output, uint32_t size);

/**
 * @} //qhmath_sp_fp_vector_trigonometric_functions
 */

/** @defgroup qhmath_sp_fp_vector_hyperbolic_functions QHMATH SP_FP vector hyperbolic functions
 *  @{
 */

/**
 * @brief       Computes the hyperbolic sine over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed hyperbolic sine values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_sinh_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the hyperbolic cosine over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed hyperbolic cosine values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_cosh_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the hyperbolic tangent over the elements of input array and store results to the output array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed hyperbolic tangent values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_tanh_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @} //qhmath_sp_fp_vector_hyperbolic_functions
 */

/** @defgroup qhmath_sp_fp_vector_nearest_integer_functions QHMATH SP_FP vector nearest integer functions
 *  @{
 */

/**
 * @brief       Computes the smallest integer value not less than given argument over the elements of input array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_ceil_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @brief       Computes the largest integer value not greater than given argument over the elements of input array.
 * @param[in]   input   Input array of floating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_floor_af(float_a8_t *input,float_a8_t *output, uint32_t size);

/**
 * @} //qhmath_sp_fp_vector_nearest_integer_functions
 */

/** @defgroup qhmath_sp_fp_vector_manipulation_functions QHMATH SP_FP vector manipulation functions
 *  @{
 */

/**
 * @brief       Vectorized modexp function for floating-point values.
 *              Fuction composes an output floating-point array from input array by modifying exponent of each input element to k.
 * @param[in]   input   Input array of floating-point values from which sign and mantissa are taken. Array has to be aligned to 8 bytes.
 * @param[in]   output  Output array where modified floating-point values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @param[in]   k       Exponent (base 2) which apply to all output members.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_modexp_af(float_a8_t *input, float_a8_t *output, uint32_t size, int32_t k);

/**
 * @brief       Vectorized copysign function for floating-point values.
 *              Fuction composes an output floating-point array from input array by modifying sign of each input element to sign of y.
 * @param[in]   input   Input array of floating-point values from which magnitude is taken.
 * @param[in]   output  Output array where modified floating-point values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @param[in]   y       Floating-point value from which sign is taken.
 * @return      Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_copysign_af(float_a8_t *input, float_a8_t *output, uint32_t size, float y);

/**
 * @} //qhmath_sp_fp_vector_manipulation_functions
 */

/** @defgroup qhmath_sp_fp_vector_other_functions QHMATH SP FP vector other functions
 *  @{
 */

/**
 * @brief       Computes sigmoid over the elements of input array.
 * @param[in]   input   Input array of foating-point values. Array has to be aligned to 8 bytes.
 * @param[out]  output  Output array where computed sigmoid values will be stored. Array has to be aligned to 8 bytes.
 * @param[in]   size    Number of elements of input and output arrays.
 * @return      Function status: 0 on successful execution, -1 on error.
 */
int32_t qhmath_sigmoid_af(float_a8_t *input, float_a8_t *output, uint32_t size);

/**
 * @brief       Clips floating-point values from input array and store results to the same-size output array.
 *              If particular member is greather then high_level parameter, its value is set to high_level.
 *              If member's value is lower then low_level parameter, value of that member is set to low_level.
 * @param[in]   input       Input array of floating-point values for clipping. Array has to be aligned to 8 bytes.
 * @param[out]  output      Output array of clipped floating-point values. Array has to be aligned to 8 bytes.
 * @param[in]   size        Number of elements of input and output arrays.
 * @param[in]   low_level   Minimum clipping value.
 * @param[in]   high_level  Maximum clipping value.
 * @return    Function status: 0 on successful execution, -1 on invalid input data.
 */
int32_t qhmath_clipping_af(float_a8_t *input, float_a8_t *output, uint32_t size, float low_level, float high_level);

/**
 * @} //qhmath_sp_fp_vector_other_functions
 */

/**
 * @} //qhmath_sp_fp_vector_functions
 */

/**
 * @} //qhmath_functions
 */

#define M_E             2.7182818284590452354   /**< The base of natural logarithms (Euler's number) */
#define M_LOG2E         1.4426950408889634074   /**< The logarithm to base 2 of M_E */
#define M_LOG10E        0.43429448190325182765  /**< The logarithm to base 10 of M_E */
#define M_LN2           0.69314718055994530942  /**< The natural logarithm of 2 */
#define M_LN10          2.30258509299404568402  /**< The natural logarithm of 10 */
#define M_PI            3.14159265358979323846  /**< Pi, the ratio of a circle’s circumference to its diameter */
#define M_PI_2          1.57079632679489661923  /**< M_PI/2 */
#define M_PI_4          0.78539816339744830962  /**< M_PI/4 */
#define M_1_PI          0.31830988618379067154  /**< The reciprocal of pi */
#define M_2_PI          0.63661977236758134308  /**< Two times the reciprocal of pi */
#define M_2_SQRTPI      1.12837916709551257390  /**< Two times the reciprocal of the square root of pi */
#define M_SQRT2         1.41421356237309504880  /**< The square root of two */
#define M_SQRT1_2       0.70710678118654752440  /**< The reciprocal of the square root of two (also the square root of 1/2) */


#ifdef __cplusplus
}
#endif

#endif /* _QHMATH_H */
