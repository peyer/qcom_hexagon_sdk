#define FARF_HIGH 1
#define FARF_ERROR 1

#include "rpcperf.h"
#include "verify.h"
#include "HAP_power.h"
#include "AEEstd.h"
#include "HAP_farf.h"

#include <stdlib.h>

__QAIC_IMPL_EXPORT int __QAIC_IMPL(rpcperf_init)(int attr) __QAIC_IMPL_ATTRIBUTE
{
   return 0;
}

__QAIC_IMPL_EXPORT int __QAIC_IMPL(rpcperf_noop)(void) __QAIC_IMPL_ATTRIBUTE
{
   return 0;
}

__QAIC_IMPL_EXPORT int __QAIC_IMPL(rpcperf_inbuf)(const uint8* src, int srcLen, int rw) __QAIC_IMPL_ATTRIBUTE
{
   int nErr = 0;
   if (rw) {
      int ii;
      for(ii = 0; ii < srcLen; ii += 32) {
         if(src[ii] != (uint8)ii) {
            nErr = -1;
         }
      }
   }
   return nErr;
}

__QAIC_IMPL_EXPORT int __QAIC_IMPL(rpcperf_routbuf)(uint8* src, int srcLen, int rw) __QAIC_IMPL_ATTRIBUTE
{
   if (rw) {
      int ii;
      for(ii = 0; ii < srcLen; ii += 32) {
         src[ii] = (uint8)ii;
      }
   }
   return 0;
}

__QAIC_IMPL_EXPORT int __QAIC_IMPL(rpcperf_power_boost)(uint32 on) __QAIC_IMPL_ATTRIBUTE
{
   if(on) {
      HAP_power_request(100, 100, 1);
   } else {
      HAP_power_request(0, 0, INT32_MAX);
   }
   return 0;
}
