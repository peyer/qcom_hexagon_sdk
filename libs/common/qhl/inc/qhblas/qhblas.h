/**=============================================================================
@file
    qhblas.h

@brief
    Header file of QHBLAS library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHBLAS_H
#define _QHBLAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "qhmath.h"
#include "qhcomplex.h"

#define ABSF(x)		qhmath_abs_f( (x) )

typedef float __attribute__ ((__aligned__(8))) float_a8_t;
typedef float complex __attribute__ ((__aligned__(8))) cfloat_a8_t;
typedef int16_t __attribute__ ((__aligned__(8))) int16_a8_t;

/** @defgroup qhblas_functions QHBLAS functions
  * @{
  */

 /** @defgroup qhblas_16b_functions QHBLAS 16-bit fixed-point functions
 *  @{
 */

/**
 * @brief       Calculates matrix inversion for NxN matrix of 16-bit values, where N is 2,3 or 4.
 * @param[in]   input_matrix    Input matrix of NxN dimension. Matrix elements are in Q15 format. Matrix is represented as one linear array.
 * @param[out]  output_matrix   Inverse matrix. Matrix elements are in Q15.16 format. Matrix is represented as one linear array.
 * @param[in]   dimension       Dimension of square input matrix.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_matrix_inverse_ah(int16_t *input_matrix, int32_t *output_matrix, uint32_t dimension);

/**
 * @brief       Calculates element-wise sum of two 16-bit input vectors and stores the result in the output vector.
 * @param[in]   input_1    Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output vector of 16-bit fixed-point values where results will be stored.
 *                         Vector has to be aligned to 8 bytes.
 * @param[in]   size       Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_add_ah(int16_a8_t *input_1, int16_a8_t *input_2, int16_a8_t *output, uint32_t size);

/**
 * @brief       Calculates dot product of two input 16-bit vectors.
 * @param[in]   input_1    Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output where result will be stored.
 * @param[in]   size       Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_dot_ah(int16_a8_t *input_1, int16_a8_t *input_2, int64_t *output, uint32_t size);

/**
 * @brief       Calculates Hadamard product, i.e element-wise multiplication, of two 16-bit input vectors
 *              and stores the result in the output vector.
 * @param[in]   input_1    Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output vector of 16-bit fixed-point values where results will be stored.
 *                         Vector has to be aligned to 8 bytes.
 * @param[in]   size       Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_hadamard_ah(int16_a8_t *input_1, int16_a8_t *input_2, int16_a8_t *output, uint32_t size);

/**
 * @brief       Calculates maximum 16-bit element of input vector.
 * @param[in]   input     Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[out]  maximum   Maximum value in input vector.
 * @param[in]   size      Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_max_ah(int16_a8_t *input, int16_t *maximum, uint32_t size);

/**
 * @brief       Calculates minimum 16-bit element of input vector.
 * @param[in]   input     Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[out]  minimum   Minimum value in input vector.
 * @param[in]   size      Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_min_ah(int16_a8_t *input, int16_t *minimum, uint32_t size);

/**
 * @brief       Calculates Euclid norm of 16-bit input vector.
 * @param[in]   input     Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[out]  output    Output where result will be stored.
 * @param[in]   size      Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_norm_ah(int16_a8_t *input, int32_t *output, uint32_t size);

/**
 * @brief       Scales input vector with scaling factor and stores the result in the output vector.
 * @param[in]   input           Input vector of 16-bit fixed-point values. Vector has to be aligned to 8 bytes.
 * @param[in]   scale_factor    Scaling factor (16-bit fixed-point value).
 * @param[out]  output          Output vector of 16-bit fixed-point values where results will be stored.
 *                              Vector has to be aligned to 8 bytes.
 * @param[in]   size            Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_scaling_ah(int16_a8_t *input, int16_t scale_factor, int16_a8_t *output, uint32_t size);

/**
 * @} //qhblas_16b_functions
 */

/** @defgroup qhblas_float_functions QHBLAS float functions
 *  @{
 */

/**
 * @brief       Calculates matrix inversion for NxN matrix of float values, where N is 2,3 or 4.
 * @param[in]   input_matrix    Input matrix of float values. Matrix is represented as one linear array.
 * @param[out]  output_matrix   Output matrix of float values where inverse matrix will be stored.
 *                              Matrix is represented as one linear array.
 * @param[in]   dimension       Dimension of square input matrix.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_matrix_inverse_af(float *input_matrix, float *output_matrix, uint32_t dimension);

/**
 * @brief       Calculates element-wise sum of two float input vectors and stores the result in the output vector.
 * @param[in]   input_1    Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output vector of float values where results will be stored.
 *                         Vector has to be aligned to 8 bytes.
 * @param[in]   size       Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_add_af(float_a8_t *input_1, float_a8_t *input_2, float_a8_t *output, uint32_t size);

/**
 * @brief       Calculates dot product of two input float vectors.
 * @param[in]   input_1    Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output where result will be stored.
 * @param[in]   size       Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_dot_af(float_a8_t *input_1, float_a8_t *input_2, float *output, uint32_t size);

/**
 * @brief       Calculates Hadamard product, i.e element-wise multiplication, of two float input vectors
 *              and stores the result in the output vector.
 * @param[in]   input_1    Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output vector of float values where results will be stored.
 *                         Vector has to be aligned to 8 bytes.
 * @param[in]   size       Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_hadamard_af(float_a8_t *input_1, float_a8_t *input_2, float_a8_t *output, uint32_t size);

/**
 * @brief       Calculates maximum element of float input vector.
 * @param[in]   input     Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[out]  maximum   Maximum value in input vector.
 * @param[in]   size      Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_max_af(float_a8_t *input, float *maximum, uint32_t size);

/**
 * @brief       Calculates minimum element of float input vector.
 * @param[in]   input     Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[out]  minimum   Minimum value in input vector.
 * @param[in]   size      Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_min_af(float_a8_t *input, float *minimum, uint32_t size);

/**
 * @brief       Calculates Euclid norm of float input vector.
 * @param[in]   input     Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[out]  output    Output where result will be stored.
 * @param[in]   size      Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_norm_af(float_a8_t *input, float *output, uint32_t size);

/**
 * @brief       Scales input vector with scaling factor and stores the result in the output vector.
 * @param[in]   input           Input vector of float values. Vector has to be aligned to 8 bytes.
 * @param[in]   scale_factor    Scaling factor (float value).
 * @param[out]  output          Output vector of float values where results will be stored.
 *                              Vector has to be aligned to 8 bytes.
 * @param[in]   size            Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_scaling_af(float_a8_t *input, float scale_factor, float_a8_t *output, uint32_t size);

/**
 * @} //qhblas_float_functions
 */

/** @defgroup qhblas_complex_float_functions QHBLAS complex float functions
 *  @{
 */

/**
 * @brief       Calculates matrix inversion for NxN matrix of complex float values, where N is 2,3 or 4.
 * @param[in]   input_matrix    Input matrix of complex float values.
 * @param[out]  output_matrix   Output matrix of complex float values where inverse matrix will be stored.
 * @param[in]   dimension       Dimension of square input matrix.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_matrix_inverse_acf(complex float *input_matrix, complex float *output_matrix, uint32_t dimension);

/**
 * @brief       Calculates dot product of two input complex float vectors.
 * @param[in]   input_1    Input vector of complex float values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of complex float values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output where result will be stored.
 * @param[in]   size       Number of elements in input vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_dot_acf(cfloat_a8_t *input_1, cfloat_a8_t *input_2, float complex *output, uint32_t size);

/**
 * @brief       Calculates Hadamard product, i.e element-wise multiplication, of two complex float input
 *              vectors and stores the result in the output vector.
 * @param[in]   input_1    Input vector of complex float values. Vector has to be aligned to 8 bytes.
 * @param[in]   input_2    Input vector of complex float values. Vector has to be aligned to 8 bytes.
 * @param[out]  output     Output vector of complex float values where results will be stored.
 *                         Vector has to be aligned to 8 bytes.
 * @param[in]   size       Number of elements in input/output vector.
 * @return      Function status: returns 0 on successful execution, otherwise -1.
 */
int32_t qhblas_vector_hadamard_acf(cfloat_a8_t *input_1, cfloat_a8_t *input_2, cfloat_a8_t *output, uint32_t size);

/**
 * @} //qhblas_complex_float_functions
 */

/**
 * @} //qhblas_functions
 */

#ifdef __cplusplus
}
#endif

#endif /* _QHBLAS_H */
