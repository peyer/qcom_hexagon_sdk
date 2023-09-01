/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include "HAP_farf.h"
#ifdef CDSP_PRESENT_FLAG
#include "calculator_multi_legacy_w_cdsp.h"
#endif

int calculator_sum(const int* vec, int vecLen, int64* res)
{
  int ii = 0;
  *res = 0;
  for (ii = 0; ii < vecLen; ++ii) {
    *res = *res + vec[ii];
  }
  FARF(HIGH, "===============     DSP: sum result %lld ===============", *res);
  return 0;
}

int adsp_calculator_sum(const int* vec, int vecLen, int64* res)
{
  return calculator_sum(vec, vecLen, res);
}

int mdsp_calculator_sum(const int* vec, int vecLen, int64* res)
{
  return calculator_sum(vec, vecLen, res);
}

int sdsp_calculator_sum(const int* vec, int vecLen, int64* res)
{
  return calculator_sum(vec, vecLen, res);
}
#ifdef CDSP_PRESENT_FLAG
int cdsp_calculator_sum(const int* vec, int vecLen, int64* res)
{
  return calculator_sum(vec, vecLen, res);
}
#endif

