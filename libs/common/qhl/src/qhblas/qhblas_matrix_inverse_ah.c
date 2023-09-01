/**=============================================================================
@file
    qhblas_matrix_inverse_ah.c

@brief
    Calculates matrix inversion for NxN matrix of 16-bit values, where N is 2,3 or 4.
    Elements of input matrix are in Q15 format.
    Elements of output matrix are in Q15.16 format.

    Function prototype
        int32_t qhblas_matrix_inverse_ah(int16_t *input_matrix, int32_t *output_matrix, uint32_t dimension);

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdint.h>
#include <stdlib.h>
#include "qhblas.h"
#include "qhblas_internal.h"

int32_t qhblas_matrix_inverse_ah(int16_t *input_matrix, int32_t *output_matrix, uint32_t dimension)
{
    if ((input_matrix == NULL) || (output_matrix == NULL) || (dimension == 0) || (dimension > 4))
    {
        return (-1);
    }
    
    int32_t status = 0;

    switch(dimension)
    {
        case 2:
            status = __qhblas_matrix_inverse2x2_ah(input_matrix, output_matrix);
            break;
        case 3:
            status = __qhblas_matrix_inverse3x3_ah(input_matrix, output_matrix);
            break;
        case 4:
            status = __qhblas_matrix_inverse4x4_ah(input_matrix, output_matrix);
            break;
        default:
            status = -1;
    }

    return status;
}
