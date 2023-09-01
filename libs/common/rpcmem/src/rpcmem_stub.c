/*==============================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "rpcmem.h"

#include <stdlib.h>
#include <stdio.h>

#if defined(WINNT) || defined (_WIN32_WINNT)
#include <malloc.h>
#endif

void rpcmem_init()
{
}

void rpcmem_deinit()
{
}

int rpcmem_to_fd(void* po) {
   return (int)(uintptr_t)po;
}

void* rpcmem_alloc(int heapid, uint32 flags, int size)
{
   (void)heapid;
   (void)flags;

   #if defined(WINNT) || defined (_WIN32_WINNT)
   return _aligned_malloc(size, 4096);
   #else
   return memalign(4096, size);
   #endif
}

void rpcmem_free(void* po)
{
   #if defined(WINNT) || defined (_WIN32_WINNT)
   _aligned_free(po);
   #else
   free(po);
   #endif
   
}
