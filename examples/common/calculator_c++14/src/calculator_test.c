/*==============================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/
#define VERIFY_PRINT_ERROR 1
#define FARF_ERROR 1
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "calculator.h"
#include "rpcmem.h"
#include "verify.h"
#include "HAP_farf.h"

#define MAX_LINE_LEN 100

#ifndef CALCULATOR_STATICLIB_TEST
typedef int (*calculator_sum_fn)(int*, int, int64*);
typedef int (*calculator_static_sum_fn)(int*, int, int64*, int64*);
typedef int (*calculator_iostream_sum_fn)(char *, int64*);
typedef int (*calculator_uppercase_fn)(char *, int64*);

static void* calculator_plus_skel_open(char *name) 
{
    int nErr = 0;
    void *handle;

    VERIFY(name);
    handle = dlopen(name, RTLD_NOW);
    if (!handle) {
       FARF(ERROR, "failed to load module %s", name);
       nErr = -1;
       goto bail;
    }
    return handle;
bail:
    return NULL;
}
#endif

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
#ifndef CALCULATOR_STATICLIB_TEST
    void *handle = NULL;
#else
    int result3 = 0;
#endif

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

#ifndef CALCULATOR_STATICLIB_TEST
    calculator_sum_fn sum_fn;
    calculator_static_sum_fn static_sum_fn;
    calculator_iostream_sum_fn iostream_sum_fn;

    // Test shared object
    FARF(ALWAYS, "Testing shared object");

    //Open calculator skel so
    handle = calculator_plus_skel_open("libcalculator_plus_skel.so");
    VERIFY(handle);

    sum_fn = (calculator_sum_fn)dlsym(handle, "calculator_plus_sum");
    VERIFY(sum_fn);
    VERIFY(!sum_fn(test, num, &result1));
    VERIFY(result1 == (num * (num - 1)) / 2);

    static_sum_fn = (calculator_static_sum_fn)dlsym(handle, "calculator_plus_static_sum");
    VERIFY(static_sum_fn);
    VERIFY(!static_sum_fn(test, num, &result1, &result2));
    VERIFY(result1 == result2);
    VERIFY(result1 == (num * (num - 1)) / 2);

    VERIFY (fp);
    iostream_sum_fn = (calculator_iostream_sum_fn)dlsym(handle, "calculator_plus_iostream_sum");
    VERIFY(iostream_sum_fn);
    VERIFY(!iostream_sum_fn("calculator.input", &result1));

    result2 = 0;
    while (fgets( line, MAX_LINE_LEN, fp )) {
        result2 += atoi(line);
    }
    VERIFY(result1 == result2);

    {
      calculator_uppercase_fn uppercase_fn;
      uppercase_fn = (calculator_uppercase_fn)dlsym(handle, "calculator_plus_uppercase_count");
      VERIFY(uppercase_fn);
      VERIFY(!uppercase_fn("Hello World C++14!", &result1));
      VERIFY(result1 == 3);
    }
#else
    // Test static library
    FARF(ALWAYS, "Testing static linking");

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

    VERIFY(!calculator_plus_uppercase_count("Hello World C++14!", &result3));
    VERIFY(result3 == 3);
#endif

bail:
//    if (handle) 
//        dlclose(handle);
    if (test)
        rpcmem_free(test);
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    rpcmem_deinit();
    return nErr;
}

