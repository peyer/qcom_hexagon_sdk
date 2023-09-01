/**=============================================================================
@file
    qhblas_matrix_inverse_acf.c

@brief
    Calculates matrix inversion for NxN matrix of complex float values, where N is 2,3 or 4.

    Function prototype
        int32_t qhblas_matrix_inverse_acf(complex float *input_matrix, complex float *output_matrix, uint32_t dimension);

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdint.h>
#include <stdlib.h>
#include "qhblas.h"
#include "qhblas_internal.h"

int32_t qhblas_matrix_inverse_acf(complex float *input_matrix, complex float *output_matrix, uint32_t dimension)
{
    if ((input_matrix == NULL) || (output_matrix == NULL) || (dimension == 0) || (dimension > 4))
    {
        return (-1);
    }
    
    int32_t status = 0;

    switch(dimension)
    {
        case 1:
            if (input_matrix[0] != 0)
            {
                output_matrix[0] = 1/input_matrix[0];
            }
            else
            {
                status = -1;
            }
            break;
        case 2:
            status = __qhblas_matrix_inverse2x2_acf(input_matrix, output_matrix);
            break;
        case 3:
            status = __qhblas_matrix_inverse3x3_acf(input_matrix, output_matrix);
            break;
        case 4:
            status = __qhblas_matrix_inverse4x4_acf(input_matrix, output_matrix);
            break;
        default:
            status = -1;
            break;
    }

    return status;
}
