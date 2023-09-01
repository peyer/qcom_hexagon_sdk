/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "calculator_multi_domains.h"
#include "calculator_test.h"
#include "rpcmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "remote.h"

int local_calculator_sum(const int* vec, int vecLen, int64* res)
{
  int ii = 0;
  *res = 0;

  for (ii = 0; ii < vecLen; ++ii)
    *res = *res + vec[ii];

  printf( "===============     DSP: sum result %lld ===============\n", *res);
  return 0;
}

int local_calculator_max(const int* vec, int vecLen, int *res)
{
  int ii;
  int max = 0;

  for (ii = 0; ii < vecLen; ii++)
     max = vec[ii] > max ? vec[ii] : max;

  *res = max;
  return 0;
}

int calculator_test(int runLocal, int domain, int num)
{
  int nErr = 0;
  int* test = NULL;
  int len = 0;
  int ii;
  int64 result = 0;
  int resultMax = 0;

  rpcmem_init();

  len = sizeof(*test) * num;
  printf("- allocate %d bytes from ION heap\n", len);

  if (domain == DOMAIN_ID_SDSP || domain == DOMAIN_ID_MDSP)
    test = (int*)rpcmem_alloc(RPCMEM_HEAP_ID_CONTIG, RPCMEM_DEFAULT_FLAGS, len);
  else
    test = (int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, len);

  if (0 == test) {
    printf("ERROR: alloc failed\n");
    nErr = 1;
    goto bail;
  }

  printf("- creating sequence of numbers from 0 to %d\n", num - 1);
  for (ii = 0; ii < num; ++ii)
    test[ii] = ii;

  if (runLocal) {
    printf("- compute sum locally\n");
    if (0 != local_calculator_sum(test, num, &result)) {
      printf("ERROR: local compute failed\n");
      nErr = 1;
      goto bail;
    }
  } else {
    remote_handle64 handle1, handle2;
    char *uri;

    printf("- compute sum on domain %d\n", domain);
    if (domain == DOMAIN_ID_ADSP)
      uri = calculator_multi_domains_URI "&_dom=adsp";
    else if (domain == DOMAIN_ID_CDSP)
      uri = calculator_multi_domains_URI "&_dom=cdsp";
    else if (domain == DOMAIN_ID_MDSP)
      uri = calculator_multi_domains_URI "&_dom=mdsp";
    else if (domain == DOMAIN_ID_SDSP) {
#ifndef __hexagon__
      // on sdsp, by default runs on sensor user process domain(PD)
      // This line is for running in separate user PD on sensor DSP
      remote_handle64 fd;
      remote_handle64_open(ITRANSPORT_PREFIX "attachuserpd&_dom=sdsp", &fd);
#endif
      uri = calculator_multi_domains_URI "&_dom=sdsp";
    } else {
      printf("ERROR: unsupported domain %d", domain);
      nErr = -1;
      goto bail;
    }

    nErr = calculator_multi_domains_open(uri, &handle1);
    if (nErr) {
      printf("ERROR: Failed to open handle for domain %d\n", domain);
      goto bail;
    }
    nErr = calculator_multi_domains_sum(handle1, test, num, &result);
    if (nErr) {
      printf("ERROR: Failed to compute sum on domain %d\n", domain);
      goto bail;
    }else {
      printf("- sum = %lld\n", result);
    }
    nErr = calculator_multi_domains_open(uri, &handle2);
    if (nErr) {
      printf("ERROR: Failed to open handle2 for domain %d\n", domain);
      goto bail;
    }
    nErr = calculator_multi_domains_max(handle2, test, num, &resultMax);
    if (nErr) {
      printf("ERROR: Failed to compute sum on domain %d\n", domain);
      goto bail;
    }else {
      printf("- max value = %d\n", resultMax);
    }
    nErr = calculator_multi_domains_close(handle1);
    if (nErr) {
      printf("ERROR: Failed to close handle1\n");
    }
    nErr = calculator_multi_domains_close(handle2);
    if (nErr) {
      printf("ERROR: Failed to close handle2\n");
    }
  }

bail:
  if (test) {
    rpcmem_free(test);
  }
  rpcmem_deinit();
  return nErr;
}
