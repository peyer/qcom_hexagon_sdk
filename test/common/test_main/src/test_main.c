/*==============================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include "dlfcn.h"

#include "test_main.h"

#include "HAP_farf.h"
#include "HAP_debug.h"

static int SigVerify_start(const char* so_name, const unsigned char* pHdrs,
                           int cbHdrs, const unsigned char* pHash, int cbHash,
                           void** ppv)
{
   return 0;
}

static int SigVerify_verifyseg(void* pv, int segnum,
                               const unsigned char* pSeg, int cbSeg)
{
  return 0;
}

static int SigVerify_stop(void* pv)
{
  return 0;
}

int main(int a, char **c)
{
  int nErr = 0;

  char *builtin[] = {(char*)"libc.so", (char*)"libstdc++.so", (char*)"libgcc.so"};
  DL_vtbl vtbl = {
    sizeof(DL_vtbl),
    HAP_debug_v2,
    SigVerify_start,
    SigVerify_verifyseg,
    SigVerify_stop
  };

  (void)dlinitex(3, builtin, &vtbl);

  FARF(HIGH, "--------------------------------------------------------------------");
  FARF(HIGH, "Calling test_main_start in target module's test file.               ");
  FARF(HIGH, "--------------------------------------------------------------------");

  nErr = test_main_start(a, c);

  FARF(HIGH, "--------------------------------------------------------------------");
  FARF(HIGH, "%s                                                             ", nErr ? "Failed " : "Success");
  FARF(HIGH, "--------------------------------------------------------------------");

  return nErr;
}


#if 1

// C externs
#ifdef __cplusplus
extern "C"
{
#endif

// TODO temporary until standalone test gets fixed

/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ Qualcomm Technologies INCORPORATED.
# All Rights Reserved.
# Modified by Qualcomm Technologies INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#include <types.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>

void hexagon_code_clean(const uint8_t* addr, size_t nbytes)
{
  /*
  * 32byte cache line size
  * == 5 bits --> 0x1f
  * 32 == 0x1 << 5
  */

  int ntimes = ((nbytes + ((uint32_t)addr & 0x1f) + 0x1f)) >> 5; /* upward align */
  uint32_t readback;

  while (ntimes--) {
    asm volatile("dccleaninva (%0)\n\t"
                 ::"r"(addr));
    asm volatile("syncht \n\t"
                 "icinva (%0)\n\t"
                 "isync\n\t"
                 ::"r"(addr));
    addr += 0x20;
  }
  addr = addr - 0x20;

  asm volatile("%[readback] = memb(%[addr])\n\t"
                 :[readback] "=r" (readback)
                   :[addr]     "r"  (addr));
  asm volatile("dccleaninva (%0)\n\t"
                   ::"r"(addr));
}

_STD_BEGIN
void* sys_mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset)
{
  void* ptr = (void*)0;

  if ((ptr = (void*)memalign(0x1000, length)) == 0) {
    printf("%s:%d: memalign of length = 0x%x failed\n", __func__, __LINE__, length);
    return (void*)0;
  }

  if (fd == -1) {
    memset(ptr, 0, length);
    return ptr;
  }

  if (lseek(fd, offset, SEEK_SET) != 0) {
    printf("%s:%d: fseek failed \n", __func__, __LINE__);
    return (void*)0;
  }
  if (read(fd, ptr, length) != length) {
    printf("%s:%d: fread failed \n", __func__, __LINE__);
    return (void*)0;
  }
  return ptr;
}
_STD_END

_STD_BEGIN
int sys_munmap(void* start, size_t length)
{
  free(start);
  return 0;
}

_STD_END

_STD_BEGIN
int sys_mprotect(void* start, size_t length, int prot)
{
  hexagon_code_clean((const uint8_t*)start, length);
  return 0;
}

_STD_END

#ifdef __cplusplus
}
#endif

#endif
