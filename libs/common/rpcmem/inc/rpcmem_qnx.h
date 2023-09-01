/**
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc
 */


#ifndef RPCMEM_H
#define RPCMEM_H

#include "AEEStdDef.h"

/**
 * RPCMEM_DEFAULT_HEAP
 * Dynamicaly select the heap to use.  This should be ok for most usecases.
 */
#define RPCMEM_DEFAULT_HEAP -1


/**
 * RPCMEM_DEFAULT_FLAGS should allocate memory with the same properties
 * as the ION_FLAG_CACHED flag
 */
#ifdef ION_FLAG_CACHED
#define RPCMEM_DEFAULT_FLAGS ION_FLAG_CACHED
#else
#define RPCMEM_DEFAULT_FLAGS 1
#endif

/**
 * RPCMEM HEAP IDs
 * SYSTEM HEAP:
 *   - non-contiguous physical memory
 *   - for sub-systems with SMMU
 *   - recommended for HVX/CDSPs
 * CONTIG HEAP:
 *   - Contiguous physical memory
 *   - limited memory
 *   - for sub-systems without SMMU (ex. sDSP and mDSP)
 */
#define RPCMEM_HEAP_ID_SYSTEM         (25)
#define RPCMEM_HEAP_ID_CONTIG         (22)
#define RPCMEM_HEAP_ID_SECURE         (9)
#define RPCMEM_HEAP_ID_SYSTEM_CONTIG  (21)

/**
 * RPCMEM_FLAG_UNCACHED
 * ION_FLAG_CACHED should be defined as 1
 */
#define RPCMEM_FLAG_UNCACHED 0
#define RPCMEM_FLAG_CACHED RPCMEM_DEFAULT_FLAGS

/**
 * examples:
 *
 * heap 22, uncached, 1kb
 *    rpcmem_alloc(22, 0, 1024);
 *    rpcmem_alloc(22, RPCMEM_FLAG_UNCACHED, 1024);
 *
 * heap 21, cached, 2kb
 *    rpcmem_alloc(21, RPCMEM_FLAG_CACHED, 2048);
 *    #include <ion.h>
 *    rpcmem_alloc(21, ION_FLAG_CACHED, 2048);
 *
 * just give me the defaults, 2kb
 *    rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, 2048);
 *    rpcmem_alloc_def(2048);
 *
 * give me the default flags, but from heap 18, 4kb
 *    rpcmem_alloc(18, RPCMEM_DEFAULT_FLAGS, 4096);
 *
 */
#define ION_SECURE_FLAGS    ((1 << 31) | (1 << 19))
#ifdef __cplusplus
extern "C" {
#endif

/**
 * call once to initialize the library
 * Note: should not call this if rpcmem is linked from libadsprpc.so
 * /libcdsprpc.so/libmdsprpc.so/libsdsprpc.so
 */
extern void rpcmem_init(void);

/**
 * call once for cleanup
 * Note: should not call this if rpcmem is linked from libadsprpc.so
 * /libcdsprpc.so/libmdsprpc.so/libsdsprpc.so
 */
extern void rpcmem_deinit(void);

/**
 * Allocate via ION a buffer of size
 * @heapid, the heap id to use
 * @flags, ion flags to use to when allocating
 * @size, the buffer size to allocate
 * @retval, 0 on failure, pointer to buffer on success
 *
 * For example:
 *    buf = rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, size);
 */

extern void* rpcmem_alloc(int heapid, uint32 flags, int size);

/**
 * allocate with default settings
 */
 #if !defined(WINNT) && !defined (_WIN32_WINNT)
__attribute__((unused))
#endif
static __inline void* rpcmem_alloc_def(int size) {
   return rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, size);
}

/**
 * free buffer, ignores invalid buffers
 */
extern void rpcmem_free(void* po);

/**
 * returns associated fd
 */
extern int rpcmem_to_fd(void* po);

int is_kernel_alloc_supported(int dev, int domain);

/*
 * API to allocate ION memory for internal purposes
 * Returns NULL if allocation fails
 *
 */
void* rpcmem_alloc_internal(int heapid, uint32 flags, int size);

/*
 * API to free internally allocated ION memory
 *
 */
void rpcmem_free_internal(void* po);

/*
 * API to get fd of internally allocated ION buffer
 * Returns valid fd on success and -1 on failure
 *
 */
int rpcmem_to_fd_internal(void *po);

#ifdef __cplusplus
}
#endif

#define RPCMEM_HEAP_DEFAULT     0x80000000
#define RPCMEM_HEAP_NOREG       0x40000000
#define RPCMEM_HEAP_UNCACHED    0x20000000
#define RPCMEM_HEAP_NOVA        0x10000000
#define RPCMEM_HEAP_NONCOHERENT 0x08000000

#endif /* RPCMEM_H */

