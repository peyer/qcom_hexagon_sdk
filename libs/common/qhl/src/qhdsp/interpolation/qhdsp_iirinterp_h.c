/**=============================================================================
@file
   qhdsp_iirinterp_h.c

@brief
   C implementation of IIR interpolation routines for 16-bit fixed-point.

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

int32_t qhdsp_iirinterp_h(int16_t *in_samples, int16_t *coefs, int64_t *states, uint32_t num_biquads,
                          uint32_t length, uint32_t r, int16_t *out_samples)
{
    if (in_samples == NULL || coefs == NULL || length == 0 || out_samples == NULL || r < 1)
    {
        return -1;
    }

    uint32_t output_length = length * r;

    if (output_length > MAX_FXPs_ON_STACK)
    {
        return -1;
    }

    int32_t result;
    int16_t tmp_samples[output_length];
    int16_t *tmp_samples_ptr = tmp_samples;

    for (uint32_t i = 0; i < length; i++)
    {
        *tmp_samples_ptr++ = in_samples[i];

        for (uint32_t r_idx = 1; r_idx < r; r_idx++)
        {
            *tmp_samples_ptr++ = 0;
        }
    }

    result = qhdsp_iir_h(tmp_samples, coefs, states, num_biquads, output_length, out_samples);

    return result;
}
