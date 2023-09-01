/**=============================================================================
@file
   qhdsp_iirdecim_h.c

@brief
   C implementation of 16-bit fixed-point IIR decimation routines.

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

int32_t qhdsp_iirdecim_h(int16_t *in_samples, int16_t *coefs, int64_t *states, uint32_t num_biquads,
                         uint32_t length, uint32_t decimator, int16_t *out_samples)
{
    if (in_samples == NULL || coefs == NULL || length == 0 || out_samples == NULL
        || decimator < 1 || length > MAX_FXPs_ON_STACK)
    {
        return -1;
    }

    int32_t result;
    int16_t tmp_samples[length];

    result = qhdsp_iir_h(in_samples, coefs, states, num_biquads, length, tmp_samples);
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
