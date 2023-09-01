/**=============================================================================
@file
    qhblas_matrix_inverse_af.c

@brief
    Calculates matrix inversion for NxN matrix of float values, where N is 2,3 or 4.

    Function prototype
        int32_t qhblas_matrix_inverse_af(float *input_matrix, float *output_matrix, uint32_t dimension);

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdint.h>
#include <stdlib.h>
#include "qhmath.h"
#include "qhblas.h"
#include "qhblas_internal.h"


typedef float __attribute__ ((__aligned__(8))) float_a8_t;

int32_t qhblas_matrix_inverse_af(float *input_data, float* output_data, uint32_t dimension)
{
    if ((input_data == NULL) || (output_data == NULL) || (dimension == 0) || (dimension > 4) )
    {
        return (-1);
    }
    
    int32_t status = 0;

    switch(dimension)
    {
        case 1:
            if (input_data[0] != 0)
            {
                output_data[0] = 1/input_data[0];
            }
            else
            {
                status = -1;
            }
            break;
        case 2:
            status = __qhblas_matrix_inverse2x2_af(input_data, output_data);
            break;
        case 3:
            status = __qhblas_matrix_inverse3x3_af(input_data, output_data);
            break;
        case 4:
            status = __qhblas_matrix_inverse4x4_af(input_data, output_data);
            break;
        default:
            status = -1;
            break;
    }

    return status;
}
