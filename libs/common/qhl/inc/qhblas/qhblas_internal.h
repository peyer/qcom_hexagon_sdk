/**=============================================================================
@file
    qhblas_internal.h

@brief
    QHBLAS helper functions.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHBLAS_INTERNAL_H
#define _QHBLAS_INTERNAL_H

int32_t __qhblas_matrix_inverse2x2_af(float_a8_t *input_matrix, float_a8_t *output_matrix);
int32_t __qhblas_matrix_inverse3x3_af(float_a8_t *input_matrix, float_a8_t *output_matrix);
int32_t __qhblas_matrix_inverse4x4_af(float_a8_t *input_matrix, float_a8_t *output_matrix);
int32_t __qhblas_matrix_inverse2x2_ah(int16_t *input_matrix, int32_t *output_matrix);
int32_t __qhblas_matrix_inverse3x3_ah(int16_t *input_matrix, int32_t *output_matrix);
int32_t __qhblas_matrix_inverse4x4_ah(int16_t *input_matrix, int32_t *output_matrix);
int32_t __qhblas_matrix_inverse2x2_acf(complex float *input_matrix, complex float *output_matrix);
int32_t __qhblas_matrix_inverse3x3_acf(complex float *input_matrix, complex float *output_matrix);
int32_t __qhblas_matrix_inverse4x4_acf(complex float *input_matrix, complex float *output_matrix);

#endif