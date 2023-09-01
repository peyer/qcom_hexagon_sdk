/*==============================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <windows.h>
#include <stdlib.h>
#include "rpcmem.h"

void rpcmem_init(void) {
}

void rpcmem_deinit(void) {
}

void* rpcmem_alloc(int heapid, uint32 flags, int size) {

   UNREFERENCED_PARAMETER(heapid);
   UNREFERENCED_PARAMETER(flags);

   return malloc(size);
}

void rpcmem_free(void* po) {
   if (po) {
      free(po);
   }
}

int rpcmem_to_fd(void* po) {
   UNREFERENCED_PARAMETER(po);
   return -1;
}
