/*==============================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/
#define FARF_HIGH 1
#include "HAP_farf.h"
#include "calculator.h"
#include "rpcmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "verify.h"

#define MAX_LINE_LEN (100)

int calculator_test(int argc, char *argv[])
{
    int nErr = 0;
    int *test = NULL;
    int result = 0;
    int ii, len = 0;
    int num;
    int64 num64;
    FILE *fp = NULL;
    int64 result1 = 0;
    int64 result2 = 0;

    VERIFY(argc >= 2);
    rpcmem_init();

    if (!strcmp(argv[1], "uppercase_count")) {
       VERIFY(argc > 2);
       FARF(LOW, "- run uppercase_count: %s", argv[2]);
       VERIFY(!(nErr = calculator_plus_uppercase_count(argv[2], &result)));
       printf("\n%d upper case letters found in %s\n", result, argv[2]);
    } else if (!strcmp(argv[1], "test_tls")) {
       VERIFY(argc > 1);
       printf("- run test_tls\n");
       VERIFY(!(nErr = calculator_plus_test_tls(&result1)));
       VERIFY(!result1);
    } else if (!strcmp(argv[1], "sum")) {
       VERIFY(argc > 2);
       VERIFY(argv[2]);
       num = atoi(argv[2]);
       VERIFY(num > 0);
       num64 = num;

       printf("- compute sum on the aDSP\n");

       len = sizeof(*test) * num;
       FARF(HIGH, "- allocate %d bytes from ION heap", len);

	   int heapid = RPCMEM_HEAP_ID_SYSTEM;
       #if defined(SLPI) || defined(MDSP)
       heapid = RPCMEM_HEAP_ID_CONTIG;
       #endif
	   VERIFY(0 != (test = (int *)rpcmem_alloc(heapid,
                                            RPCMEM_DEFAULT_FLAGS, len)));

       FARF(HIGH, "- creating sequence of numbers from 0 to %d", num - 1);
       for (ii = 0; ii < num; ++ii) {
           test[ii] = ii;
       }

       VERIFY(!calculator_plus_sum(test, num, &result1));
       VERIFY(result1 == (num64 * (num64 - 1)) / 2);
       printf("- sum = %lld\n", result1);
    } else if (!strcmp(argv[1], "static_sum")) {
       VERIFY(argc > 2);
       VERIFY(argv[2]);
       num = atoi(argv[2]);
       VERIFY(num > 0);
       num64 = num;

       printf("- compute sum on the aDSP\n");

       len = sizeof(*test) * num;
       FARF(HIGH, "- allocate %d bytes from ION heap", len);
       VERIFY(0 != (test = (int *)rpcmem_alloc(RPCMEM_DEFAULT_HEAP,
                                            RPCMEM_DEFAULT_FLAGS, len)));
       FARF(HIGH, "- creating sequence of numbers from 0 to %d", num - 1);
       for (ii = 0; ii < num; ++ii) {
           test[ii] = ii;
       }

       FARF(HIGH, "- compute sum on the aDSP using static constructor");
       VERIFY(!(nErr = calculator_plus_static_sum(test, num, &result1, &result2)));
       VERIFY(result2 == (num64 * (num64 - 1)) / 2);
       printf("- sum = %lld\n", result2);
    } else if (!strcmp(argv[1], "iostream_sum")) {
       char line[MAX_LINE_LEN];
       VERIFY(argc > 2);
       VERIFY(argv[2]);

       printf("- compute sum on the aDSP using iostream, file %s\n", argv[2]);
       fp = fopen(argv[2], "r");
       VERIFY (fp);
       VERIFY(!calculator_plus_iostream_sum(argv[2], &result1));

       result2 = 0;
       while (fgets( line, MAX_LINE_LEN, fp )) {
          result2 += atoi(line);
       }
       VERIFY(result1 == result2);
       printf("- sum = %lld\n", result1);
    } else {
       printf("unsupported method - %s\n", argv[1]);
       nErr = -1;
    }
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

int main(int argc, char *argv[])
{
    int nErr = 0;

    if(argc < 2) {
        printf("usage: %s <method> <arguments>\n", argv[0]);
        printf("usage: supported methods: sum, static_sum and uppercase_count\n");
	nErr = -1;
        goto bail;
    }

    nErr = calculator_test(argc, argv);

bail:
    if (nErr) {
        printf("- failed\n");
    } else {
        printf("- success\n");
    }

    return nErr;
}
void HAP_debug(const char *msg, int level, const char *filename, int line)
{
	#ifdef __LA_FLAG
    __android_log_print(level, "adsprpc", "%s:%d: %s", filename, line, msg);
	#endif
}
