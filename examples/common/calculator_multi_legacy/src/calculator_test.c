/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "calculator_multi_legacy.h"
#ifdef CDSP_PRESENT_FLAG
#include "calculator_multi_legacy_w_cdsp.h"
#endif
#include "calculator_test.h"
#include "rpcmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int local_calculator_sum(const int* vec, int vecLen, int64* res)
{
  int ii = 0;
  *res = 0;

  for (ii = 0; ii < vecLen; ++ii) {
    *res = *res + vec[ii];
  }
  printf( "===============     DSP: sum result %lld ===============\n", *res);
  return 0;
}

int calculator_test(int runLocal, int num)
{
  int nErr = 0, nErr1 = 0, nErr2 = 0, nErr3 = 0, nErr4 = 0;
  int* test = NULL;
  int* test_contig = NULL;
  int len = 0;
  int ii;
  int64 result = 0;

  rpcmem_init();

  len = sizeof(*test) * num;
  printf("- allocate %d bytes from ION heap from system memory\n", len);
  if (0 == (test = (int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, len))) {
    printf("Error: alloc failed\n");
    nErr = 1;
    goto bail;
  }

  #if defined(SLPI_PRESENT_FLAG) || defined(MDSP_PRESENT_FLAG)
  printf("- allocate %d bytes from ION heap from physically contiguous memory\n", len);
  if (0 == (test_contig = (int*)rpcmem_alloc(RPCMEM_HEAP_ID_CONTIG, RPCMEM_DEFAULT_FLAGS, len))) {
    printf("Error: alloc failed\n");
    nErr = 1;
    goto bail;
  }
  #endif

  printf("- creating sequence of numbers from 0 to %d\n", num - 1);
  for (ii = 0; ii < num; ++ii) {
    test[ii] = ii;
	#if defined(SLPI_PRESENT_FLAG) || defined(MDSP_PRESENT_FLAG)
	test_contig[ii] = ii;
	#endif
  }

  if (runLocal) {
    printf("\n- compute sum locally\n");
    if (0 != local_calculator_sum(test, num, &result)) {
      printf("Error: local compute failed\n");
      nErr = 1;
      goto bail;
    }
  } else {
    printf("\n- compute sum on the aDSP\n");
    if (0 != adsp_calculator_sum(test, num, &result)) {
      printf("Error: compute on aDSP failed\n");
      nErr1 = 1;
    } else {
      printf("- sum = %lld\n", result);
    }
	#ifdef CDSP_PRESENT_FLAG
    printf("- compute sum on the cDSP\n");
    if (0 != cdsp_calculator_sum(test, num, &result)) {
      printf("Error: compute on cDSP failed\n");
      nErr4 = 1;
    } else {
      printf("- sum = %lld\n", result);
    }
	#else
	nErr4 = 0;
	#endif
    
	#ifdef MDSP_PRESENT_FLAG
    printf("\n- compute sum on the mDSP\n");
    if (0 != mdsp_calculator_sum(test_contig, num, &result)) {
      printf("Error: compute on mDSP failed\n");
      nErr2 = 1;
    } else {
      printf("- sum = %lld\n", result);
    }
	#endif
	
	#ifdef SLPI_PRESENT_FLAG
    printf("\n- compute sum on the sDSP\n");
    if (0 != sdsp_calculator_sum(test_contig, num, &result)) {
      printf("Error: compute on sDSP failed\n");
      nErr3 = 1;
    } else {
      printf("- sum = %lld\n", result);
    }
	#endif

    nErr = nErr1 | nErr2 | nErr3 | nErr4;
  }

bail:
  if (test)
    rpcmem_free(test);
  if (test_contig)
    rpcmem_free(test_contig);
  rpcmem_deinit();
  return nErr;
}
