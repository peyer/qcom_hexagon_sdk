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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "verify.h"

#define CALC_FILENAME "calculator.input"

#define MAX_LINE_LEN 100
int calculator_test( int num)
{
    int nErr = 0;
    int *test = 0;
    int len = 0;
    int ii;
    int64 result1 = 0;
    int64 result2 = 0;
    char line[MAX_LINE_LEN];
    FILE *fp = fopen("/system/lib/rfsa/adsp/"CALC_FILENAME, "r");
    rpcmem_init();

    len = sizeof(*test) * num;
    FARF(HIGH, "- allocate %d bytes from ION heap", len);

    int heapid = RPCMEM_HEAP_ID_SYSTEM;
    #if defined(SLPI) || defined(MDSP)
    heapid = RPCMEM_HEAP_ID_CONTIG;
    #endif
    VERIFY(0 != (test = (int *)rpcmem_alloc(heapid, RPCMEM_DEFAULT_FLAGS, len)));

    FARF(HIGH, "- creating sequence of numbers from 0 to %d", num - 1);
    for (ii = 0; ii < num; ++ii) {
        test[ii] = ii;
    }

    FARF(HIGH, "- compute sum on the aDSP");
    VERIFY(!calculator_plus_sum(test, num, &result1));

    FARF(HIGH, "- sum = %lld", result1);

    VERIFY(result1 == (num * (num - 1)) / 2);

    FARF(HIGH, "- compute sum on the aDSP using static constructor");
    VERIFY(!(nErr = calculator_plus_static_sum(test, num, &result1, &result2)));

    FARF(HIGH, "- result1 = %lld result2 = %lld", result1, result2);
    VERIFY(result1 == result2);
    VERIFY(result1 == (num * (num - 1)) / 2);
    if (fp) {
        FARF(HIGH, "- compute sum on the aDSP using iostream");
        VERIFY(!calculator_plus_iostream_sum(CALC_FILENAME, &result1));

        result2 = 0;
        if (fp) {
            while (fgets( line, MAX_LINE_LEN, fp )) {
                result2 += atoi(line);
            }
        }

        FARF(HIGH, "- local iostream result1 = %lld DSP calculated iostream result %lld", result1, result2);
        VERIFY(result1 == result2);
    } else {
        FARF(HIGH,
             "- can't compute sum on the aDSP using iostream, fopen failed for calculator.input");
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
    int num = 0;

    if (argc != 2) {
        nErr = 1;
        goto bail;
    }
    num = atoi(argv[1]);

    printf("- starting calculator test\n");

    nErr = calculator_test(num);

bail:
    if (nErr) {
        printf("usage: %s <uint32 size>\n", argv[0]);
    } else {
        printf("- success\n");
    }

    return nErr;
}
void HAP_debug(const char *msg, int level, const char *filename, int line)
{
	#ifdef __LE_FLAG
    __android_log_print(level, "adsprpc", "%s:%d: %s", filename, line, msg);
	#endif
}
