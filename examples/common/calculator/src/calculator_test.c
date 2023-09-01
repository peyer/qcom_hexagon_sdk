/*==============================================================================
  Copyright (c) 2012-2014,2017 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "AEEStdErr.h"
#include "calculator.h"
#include "calculator_test.h"
#include "rpcmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>

int nErr = 0; 

int local_calculator_sum(const int* vec, int vecLen, int64* res)
{
  int ii = 0;
  *res = 0;

  for (ii = 0; ii < vecLen; ++ii) {
    *res = *res + vec[ii];
  }
  return 0;
}

int calculator_test(int runLocal, int num)
{
  int* test = 0;
  int len = 0;
  int ii;
  int64 result = 0;

  rpcmem_init();

  len = sizeof(*test) * num;
  printf("\n---Allocate %d bytes from ION heap\n", len);

  int heapid = RPCMEM_HEAP_ID_SYSTEM;
  #if defined(SLPI) || defined(MDSP)
  heapid = RPCMEM_HEAP_ID_CONTIG;
  #endif 

  if (0 == (test = (int*)rpcmem_alloc(heapid, RPCMEM_DEFAULT_FLAGS, len))) {
    printf("---Error: alloc failed\n");
	nErr = -1;
    goto bail;
  }
  
  printf("---Creating sequence of numbers from 0 to %d\n", num - 1);
  for (ii = 0; ii < num; ++ii) {
    test[ii] = ii;
  }

  if (runLocal) {
    printf("\n---Compute sum locally\n");
    if (0 != local_calculator_sum(test, num, &result)) {
      printf("Error: local compute failed\n");
	  nErr = -1;
      goto bail;
    }
  } else {
#ifdef __hexagon__
    printf("\n---Compute sum on the DSP\n");
    if (0 != calculator_sum(test, num, &result)) {
      printf("Error: compute on DSP failed\n");
	  nErr = -1;
      goto bail;
    }
#else 
  void* H = 0;
  int (*func_ptr)(int* test, int len, int64* result);
  int retry_count = 0;

retry:  
   H = dlopen("libcalculator.so", RTLD_NOW);
   if (!H) {
      printf("---ERROR, Failed to load libcalculator.so\n");
	  nErr = -1;
	  goto bail;
   }

   func_ptr = (int (*)(int*, int, int64*))dlsym(H, "calculator_sum");
   if (!func_ptr) {
      printf("---ERROR, calculator_sum not found\n");
	  dlclose(H);
	  nErr = -1;
      goto bail;
   }

   printf("\n---Compute sum on the DSP\n");
   if (0 != (nErr = (*func_ptr)(test, num, &result))) {
      printf("---Error: compute on DSP failed, nErr = %d\n", nErr);
	  dlclose(H);
      if (nErr == AEE_ECONNRESET) {
        printf("---SSR happened. Retrying.... nErr = %d\n", nErr);
        sleep(5);
        retry_count++;
        if (retry_count < 10)
            goto retry; 
        else
            printf("---Retry attempt unsuccessful. Timing out....\n---DSP is not up after Sub-system restart...\n");
       }
	   goto bail;
   }  
   dlclose(H);
#endif
}
   printf("---Sum = %lld\n", result);
bail:
  if (test)
    rpcmem_free(test);
  rpcmem_deinit();
  return nErr;
}