/**=============================================================================
@file
    qhblas_matrix_inverse_nxn_af.c

@brief
    Calculates matrix inversion for NxN matrix of float values, where N > 4.
    LU decomposition is used for calculation.

    Function prototype
        int32_t qhblas_matrix_inverse_af(float *input_matrix, float *output_matrix, uint32_t dimension);

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdint.h>
#include <stdlib.h>
#include <qhmath.h>
#include <qhblas.h>


typedef float __attribute__ ((__aligned__(8))) float_a8_t;

int32_t LU_pivot_decompose(float *input_data, uint32_t size, float Tol, int32_t *pivot)
{
    int32_t i, j, k, q, imax;
    float maxA, temp, absA;

    for (i = 0; i <= size; i++)
        pivot[i] = i;                           // Creating pivot array

    for (i = 0; i < size; i++)
    {
        maxA = 0.0;
        imax = i;

        for (k = i; k < size; k++)
        {
            if ((absA = qhmath_abs_f(input_data[k*size + i])) > maxA)
            {
                maxA = absA;
                imax = k;
            }
        }

        if (maxA <= Tol) return -1;             //Matrix is singular or below given tolerance

        if (imax != i)
        {
                                                //pivoting pivot
            j = pivot[i];
            pivot[i] = pivot[imax];
            pivot[imax] = j;

            for (q = 0; q<size; q++)
            {
                temp = input_data[i*size + q];
                input_data[i*size + q] = input_data[imax*size + q];
                input_data[imax*size + q] = temp;
            }
        }

        for (j = i + 1; j < size; j++)
        {
            input_data[j*size + i] /= input_data[i*size + i];

            for (k = i + 1; k < size; k++)
                input_data[j*size + k] -= input_data[j*size + i] * input_data[i*size + k];
        }
    }

    return 0;
}

void LU_pivot_inverse(float *input_data, int32_t *pivot, uint32_t size, float *output_data)
{

    for (int32_t j = 0; j < size; j++)
     {
        for (int32_t i = 0; i < size; i++)
        {
            if (pivot[i] == j)
                output_data[i*size + j] = 1.0;
            else
                output_data[i*size + j] = 0.0;

            for (int32_t k = 0; k < i; k++)
                output_data[i*size + j] -= input_data[i*size + k] * output_data[k*size + j];
        }

        for (int32_t i = size - 1; i >= 0; i--)
        {
            for (int32_t k = i + 1; k < size; k++)
                output_data[i*size + j] -= input_data[i*size + k] * output_data[k*size + j];

            output_data[i*size + j] = output_data[i*size + j] / input_data[i*size + i];
        }
    }
}

int32_t qhblas_matrix_inverse_nxn_af(float *input_data, float* output_data, uint32_t dimension, float Tol)
{
    int32_t status = -1;
    int32_t i,j;
    float* matrix_copy = malloc(dimension*dimension*sizeof(float));
    int32_t* pivot = malloc((dimension)*sizeof(int32_t));

    for (i = 0; i < dimension; i++)             // Creating hard copy for input matrix.
    {
        for (j = 0; j < dimension; j++)
        {
            matrix_copy[i*dimension + j] = input_data[i*dimension + j];
        }
    }

    if (LU_pivot_decompose(matrix_copy, dimension, Tol, pivot) == 0) // Check for singular or degenerate matrix
    {
        LU_pivot_inverse(matrix_copy, pivot, dimension, output_data);
        free(matrix_copy);
        free(pivot);
        status = 0;
    }
    else
    {
        free(matrix_copy);
        free(pivot);
        status = -1;
    }

    return status;
}