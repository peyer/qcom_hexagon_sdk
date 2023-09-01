#ifndef FFTSFC_H
#define FFTSFC_H
/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

typedef __complex float cfloat;

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

void FFTsfc(cfloat* Input, int points, cfloat* twiddles, cfloat* Output);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // FFTSFC_H


