/**=============================================================================
@file
   qhdsp_firdecim_f.c

@brief
   C implementation of floating-point FIR decimation routines.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include <stdint.h>
#include <string.h>
#if __hexagon__
#include <qhcomplex.h>
#endif
#include <qhdsp.h>
#include <qhdsp_internal.h>

int32_t qhdsp_firdecim_f(const float *in_samples, float *delay_line, const float *coefs, uint32_t taps,
                         uint32_t length, uint32_t decimator, float *out_samples)
{
    if (in_samples == NULL || coefs == NULL || length == 0 || out_samples == NULL
        || decimator < 1 || length > MAX_FLOATS_ON_STACK)
    {
        return -1;
    }

    int32_t result;
    float_a8_t tmp_samples[length];

    result = qhdsp_fir_f(in_samples, delay_line, coefs, taps, length, tmp_samples);
    if (result != 0)
    {
        return result;
    }

    uint32_t output_length = length / decimator;

    for (uint32_t i = 0; i < output_length; i++)
    {
        out_samples[i] = tmp_samples[decimator * i];
    }

    return 0;
}
