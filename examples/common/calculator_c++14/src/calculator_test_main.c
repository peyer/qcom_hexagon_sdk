/*==============================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "verify.h"
#include "HAP_farf.h"

#include "calculator.h"
#include "calculator_test.h"

#ifndef CALC_EXPORT
#define CALC_EXPORT
#endif /*CALC_EXPORT*/

#if defined(_WIN32) && !defined(_ARM_)
#include "ptl_remote_invoke.h"
CALC_EXPORT int init(pfn_getSymbol GetSymbol)
{
   return remote_invoke_stub_init(GetSymbol);
}
#endif

CALC_EXPORT int main(void)
{
   int nErr  =  0;
   int nPass =  0;
   DL_vtbl vtbl;
   char *builtin[] = { (char*)"libc.so", (char*)"libgcc.so"};

   memset(&vtbl, 0, sizeof(DL_vtbl));
   vtbl.size = sizeof(DL_vtbl);
   vtbl.msg = HAP_debug_v2;
   (void)dlinitex(2, builtin, &vtbl);

   VERIFY(0 == (nErr = calculator_test(1, 256)));
   nPass++;

bail:
   printf("############################################################\n");
   printf("Summary Report \n");
   printf("############################################################\n");
   printf("Pass: %d\n", nPass);
   printf("Undetermined: 0\n");
   printf("Fail: %d\n", 1 - nPass);
   printf("Did not run: 0\n");

   return nErr;
}
