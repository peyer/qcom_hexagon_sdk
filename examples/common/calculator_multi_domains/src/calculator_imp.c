/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "HAP_farf.h"
#include "calculator_multi_domains.h"

int calculator_multi_domains_open(const char*uri, remote_handle64* handle) {
   void *tptr = NULL;
  /* can be any value or ignored, rpc layer doesn't care
   * also ok
   * *handle = 0;
   * *handle = 0xdeadc0de;
   */
   tptr = (void *)malloc(1);
   *handle = (remote_handle64)tptr;
   assert(*handle);
   return 0;
}

/**
 * @param handle, the value returned by open
 * @retval, 0 for success, should always succeed
 */
int calculator_multi_domains_close(remote_handle64 handle) {
   if (handle)
      free((void*)handle);
   return 0;
}

int calculator_multi_domains_sum(remote_handle64 h, const int* vec, int vecLen, int64* res)
{
  int ii = 0
;
  *res = 0;
  for (ii = 0; ii < vecLen; ++ii)
    *res = *res + vec[ii];

  FARF(HIGH, "===============     DSP: sum result %lld ===============", *res);
  return 0;
}

int calculator_multi_domains_max(remote_handle64 h, const int* vec, int vecLen, int* res) {
  int ii = 0;
  int max = 0;

  for (ii = 0; ii < vecLen; ++ii)
    max = (vec[ii] > max) ? vec[ii] : max;

  *res = max;
  FARF(HIGH, "===============     DSP: maximum result %d ==============", *res);
  return 0;
}
