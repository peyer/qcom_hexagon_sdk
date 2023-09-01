/*==============================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include "verify.h"

#include "calculator_multi_domains.h"
#include "calculator_test.h"

#ifdef ANDROID
#include "adsp_info.h"
#endif

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

   VERIFY(0 == (nErr = calculator_test(1, DOMAIN_ID_ADSP, 256)));
   nPass++;
   VERIFY(0 == (nErr = calculator_test(1, DOMAIN_ID_CDSP, 256)));
   nPass++;
   VERIFY(0 == (nErr = calculator_test(1, DOMAIN_ID_SDSP, 256)));
   nPass++;
   VERIFY(0 == (nErr = calculator_test(0, 0, 256)));
   nPass++;

bail:
   printf("############################################################\n");
   printf("Summary Report \n");
   printf("############################################################\n");
   printf("Pass: %d\n", nPass);
   printf("Undetermined: 0\n");
   printf("Fail: %d\n", 4 - nPass);
   printf("Did not run: 0\n");

   return nErr;
}
