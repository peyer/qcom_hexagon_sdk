/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <dlfcn.h>

#include "test_main.h"

#include "HAP_perf.h"
#include "HAP_farf.h"

#include "template_so.h"

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

#define TRY(exception, func) \
   if (TEST_SUCCESS != (exception = func)) {\
      goto exception##bail; \
   }

#define THROW(exception, errno) \
   exception = errno; \
   goto exception##bail;

#define CATCH(exception) exception##bail: if (exception != TEST_SUCCESS)


int test_main_start(int argc, char *argv[])
{
   long long int cyclesStart;
   long long int cyclesEnd;
   int nErr = TEST_SUCCESS;

   FARF(HIGH, "-- start lib test --                                                ");

   FARF(HIGH, "Calling template_so(%10d)                                       ", (int)&nErr);
   cyclesStart = HAP_perf_get_pcycles();
   nErr = template_so((int)&nErr);
   cyclesEnd = HAP_perf_get_pcycles();

   FARF(ALWAYS, "Calling template_so() took %10d cycles                          ", (int)(cyclesEnd - cyclesStart));

   if (nErr == (int)&nErr) {
     nErr = TEST_SUCCESS;
   } else {
     FARF(ERROR, "template_so returned %10d instead of %10d", nErr, (int)&nErr);
     THROW(nErr, TEST_FAILURE);
   }


   FARF(HIGH, "Test Passed                                                         ");

   CATCH(nErr){};

   FARF(HIGH, "-- end lib test test --                                             ");

   return nErr;
}

