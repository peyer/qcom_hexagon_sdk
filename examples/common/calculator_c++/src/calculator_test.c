/*==============================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>

#include "calculator.h"
#include "rpcmem.h"
#include "verify.h"

#define MAX_LINE_LEN 100

int calculator_test(void)
{
    int nErr = 0;
    int *test = 0;
    int len = 0;
    int ii;
    int num = 10000;
    int64 result1 = 0;
    int64 result2 = 0;
    char line[MAX_LINE_LEN];
    FILE *fp = fopen("calculator.input", "r");

    rpcmem_init();

    len = sizeof(*test) * num;

	int heapid = RPCMEM_HEAP_ID_SYSTEM;
	#if defined(SLPI) || defined(MDSP)
	heapid = RPCMEM_HEAP_ID_CONTIG;
	#endif 

	VERIFY(test = (int *)rpcmem_alloc(heapid, RPCMEM_DEFAULT_FLAGS, len));
	
    for (ii = 0; ii < num; ++ii) {
        test[ii] = ii;
    }

    VERIFY(!calculator_plus_sum(test, num, &result1));
    VERIFY(result1 == (num * (num - 1)) / 2);

    VERIFY(!calculator_plus_static_sum(test, num, &result1, &result2));
    VERIFY(result1 == result2);
    VERIFY(result1 == (num * (num - 1)) / 2);

    VERIFY (fp);
    VERIFY(!calculator_plus_iostream_sum("calculator.input", &result1));

    result2 = 0;
    while (fgets( line, MAX_LINE_LEN, fp )) {
        result2 += atoi(line);
    }

    VERIFY(result1 == result2);

bail:
    if (test) {
        rpcmem_free(test);
    }
    if (fp) {
        fclose(fp);
        fp = NULL;
    }

    rpcmem_deinit();
    return nErr;
}

