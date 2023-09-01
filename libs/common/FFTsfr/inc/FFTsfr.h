#ifndef FFTSFR_H
#define FFTSFR_H
/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

typedef __complex float cfloat;

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

void  FFTsfr( float *Input, int points, cfloat *twiddles1, cfloat *twiddles2, cfloat *Output);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // FFTSFR_H