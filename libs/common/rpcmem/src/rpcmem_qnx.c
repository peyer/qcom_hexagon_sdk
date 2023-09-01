/*
 * Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc
 */
#ifndef VERIFY_PRINT_ERROR
#define VERIFY_PRINT_ERROR
#endif //VERIFY_PRINT_ERROR

/* Includes ------------------------------------------------------------------*/
#include "verify.h"
#include "rpcmem_qnx.h"
#include "AEEQList.h"
#include "AEEstd.h"
#include "AEEStdErr.h"
#include "remote.h"
#include "pmem.h"
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
//TODO:sunny
//#include <linux/types.h>


/* Define / Macros -----------------------------------------------------------*/
//#define FASTRPC_PMEM_ID

#define LOG_FASTRPC(_fmt, ...)                                                  \
    fprintf(stderr, "%s:%d " _fmt "\n", __func__, __LINE__ , ##__VA_ARGS__)

#ifdef FASTRPC_PMEM_ID
#define PMEM_HEAP_ID_SECURE         (9)
#define PMEM_HEAP_ID_SYSTEM_CONTIG  (21)

#define PMEM_HEAP_ID_ADSP           (22)

#define PMEM_HEAP_ID_PIL1           (23)
#define PMEM_HEAP_ID_SYSTEM         (25)

#define HEAP_ID_TO_MASK(bit)        (1 << (bit))
#endif /* FASTRPC_PMEM_ID */

#define FASTRPC_DFLT_PMEM_ID        (PMEM_MDP_ID)

#define ION_FLAG_CACHED             (1)
#define SET_FLAG(__cache, __flag)   ((__cache) | (__flag))

/* Structure Definitions -----------------------------------------------------*/
struct ion_allocation_data {
    uint64_t len;
    uint32_t heap_id_mask;
    uint32_t flags;
    uint32_t fd;
    uint32_t unused;
};

struct pmem_fd_data {
    pmem_handle_t handle;
    int fd;
};

struct mmap_info {
    QNode qn;
    void *paddr;
    int bufsize;
    uint32 flags;
    struct pmem_fd_data data;
};

/* PRIVATE - Variables -------------------------------------------------------*/
/**
 * this heap id is used when -1 is passed to alloc.
 * users should change this to whatever heap id they expect.
 */
static QList lst;
static pthread_mutex_t mt;
static int rpcmem_fd;

/* EXTERNAL - Referenced Functions -------------------------------------------*/
extern int is_smmu_enabled(void);
extern void remote_register_buf(void* buf, int size, int fd);
#pragma weak  remote_register_buf
extern void remote_register_buf_attr(void* buf, int size, int fd, int attrs);
#pragma weak  remote_register_buf_attr
extern void *remote_register_fd(int fd, int size);
#pragma weak  remote_register_fd

/* PRIVATE - Function Prototypes ---------------------------------------------*/
static int rpcmem_contig_alloc(struct mmap_info *h, uint32 heap_mask, uint32 memflags, uint32 pmem_flags, int size, void **ppo);
static int rpcmem_contig_free(struct mmap_info *h, void *po);
static void register_buf(void* buf, int size, int fd, int attr);

/* GLOBAL - Function Definitions ---------------------------------------------*/
void rpcmem_init(void)
{
    QList_Ctor(&lst);
    pthread_mutex_init(&mt, 0);

    if(0 != pmem_init()) {
        LOG_FASTRPC("pmem_init() failed...");
    }
}

void rpcmem_deinit(void)
{
    pthread_mutex_destroy(&mt);
    pmem_deinit();
}

void* rpcmem_alloc(int heapid, uint32 flags, int size)
{
    return rpcmem_alloc_internal(heapid, flags, size);
}

void* rpcmem_alloc_internal(int heapid, uint32 flags, int size)
{
    int nErr = AEE_SUCCESS;
    struct mmap_info *m = 0;
    void *po = 0;
    uint32 heap_mask = FASTRPC_DFLT_PMEM_ID; // TODO: Need to initialize this
    uint32 pmem_flags;
    uint32 rpc_flags = flags;

    VERIFYC(NULL != (m = malloc(sizeof(*m))), AEE_ENOMEMORY);
    QNode_CtorZ(&m->qn);

    //! default flags should be the same as ion cached
//TODO:sunny - update RPCMEM_DEFAULT_FLAGS to map with pmem FLAGS
//    C_ASSERT(ION_FLAG_CACHED == RPCMEM_DEFAULT_FLAGS);
//    C_ASSERT(PMEM_FLAGS_CACHE_WB_WA == RPCMEM_DEFAULT_FLAGS);

    pmem_flags = rpc_flags & ~0xff000000;
    //! convert from deprecated flags
    if(rpc_flags & RPCMEM_HEAP_DEFAULT) {
       heapid = RPCMEM_DEFAULT_HEAP;
       if(!(rpc_flags & RPCMEM_HEAP_UNCACHED)) {
          pmem_flags = SET_FLAG(pmem_flags, PMEM_FLAGS_CACHE_WB_WA);
       }
    }

    if(!(rpc_flags & RPCMEM_HEAP_DEFAULT)) {
       VERIFY(!(rpc_flags & RPCMEM_HEAP_UNCACHED));
    }

    if(heapid == RPCMEM_DEFAULT_HEAP) {
       if(!po) {
#ifdef FASTRPC_PMEM_ID
          heap_mask = HEAP_ID_TO_MASK(PMEM_HEAP_ID_SYSTEM);
#endif /* FASTRPC_PMEM_ID */
          VERIFY(AEE_SUCCESS == (nErr = rpcmem_contig_alloc(m, heap_mask, rpc_flags, pmem_flags, size, &po)));
       }
    } else {
#ifdef FASTRPC_PMEM_ID
       heap_mask = HEAP_ID_TO_MASK(heapid);
       if(heap_mask & HEAP_ID_TO_MASK(PMEM_HEAP_ID_SECURE)) {
          rpc_flags = rpc_flags | RPCMEM_HEAP_NOVA;
       }
#endif /* FASTRPC_PMEM_ID */

       VERIFY(AEE_SUCCESS == (nErr = rpcmem_contig_alloc(m, heap_mask, rpc_flags, pmem_flags, size, &po)));
    }

    pthread_mutex_lock(&mt);
    QList_AppendNode(&lst, &m->qn);
    pthread_mutex_unlock(&mt);

    m->flags = rpc_flags;
    if (!(rpc_flags & RPCMEM_HEAP_NOREG) && !(rpc_flags & RPCMEM_HEAP_NOVA)) {
       int attrs = 0;

       LOG_FASTRPC("rpcmem register buf: heap mask: 0x%x\n", heap_mask);
       if (rpc_flags & RPCMEM_HEAP_NONCOHERENT)
           attrs = FASTRPC_ATTR_NON_COHERENT;

       register_buf(m->paddr, m->bufsize, m->data.fd, attrs);
    }

bail:
    if(nErr != AEE_SUCCESS) {
       LOG_FASTRPC("Error 0x%x: ION mem alloc failed for size 0x%x, heapid %d, flags 0x%x\n", nErr, size, heapid, flags);
    }
    if(nErr && m) {
       free(m);
       m = NULL;
       po = 0;
    }

    return po;
}

void rpcmem_free(void* po)
{
        rpcmem_free_internal(po);
}

void rpcmem_free_internal(void* po)
{
    int nErr = 0;
    struct mmap_info *m, *mfree = 0;
    QNode* pn, *pnn;

    pthread_mutex_lock(&mt);
    QLIST_NEXTSAFE_FOR_ALL(&lst, pn, pnn) {
       m = STD_RECOVER_REC(struct mmap_info, qn, pn);
       if(m->paddr == po) {
          mfree = m;
          QNode_Dequeue(&m->qn);
          break;
       }
    }
    pthread_mutex_unlock(&mt);

    VERIFY(mfree);
    if(!(mfree->flags & RPCMEM_HEAP_NOREG)) {
       register_buf(mfree->paddr, mfree->bufsize, -1, 0);
    }
    rpcmem_contig_free(mfree, po);
    free(mfree);
    mfree = NULL;

bail:
    return;
}

int rpcmem_to_fd(void* po)
{
        return rpcmem_to_fd_internal(po);
}

int rpcmem_to_fd_internal(void *po)
{
    struct mmap_info *m;
    int fd = -1;
    QNode* pn;

    pthread_mutex_lock(&mt);
    QLIST_FOR_ALL(&lst, pn) {
       m = STD_RECOVER_REC(struct mmap_info, qn, pn);
       if(STD_BETWEEN(po, m->paddr, (uintptr_t)m->paddr + m->bufsize)) {
          fd = m->data.fd;
          break;
       }
    }
    pthread_mutex_unlock(&mt);

    return fd;
}

/* PRIVATE - Function Definitions --------------------------------------------*/
static int rpcmem_contig_free(struct mmap_info *h, void *po)
{
    struct mmap_info *m = (struct mmap_info *)h;
    int nErr = AEE_SUCCESS;
    (void)po;
    int size = (m->bufsize + 4095) & (~4095);

    if(m->paddr) {
        if(m->flags & RPCMEM_HEAP_NOVA) {
            VERIFY((uintptr_t)remote_register_buf);
            remote_register_buf(m->paddr, size, -1);
         } else {
            if(pmem_free(m->paddr)) {
                LOG_FASTRPC("pmem_free(0x%p) failed", m->paddr);
                goto bail;
            }
        }

        m->paddr = 0;
    }

bail:
    return AEE_SUCCESS;
}

static int rpcmem_contig_alloc(struct mmap_info *h, uint32 heap_mask, uint32 rpcflags, uint32 pmem_flags, int size, void **ppo)
{
    struct mmap_info *m = (struct mmap_info *)h;
    int nErr = AEE_SUCCESS;
    //TODO: Will align remove from allocation request results any performance impact?
    int len = (m->bufsize + 4095) & (~4095);
    off64_t offset;

    m->paddr = 0;
    m->data.handle = 0;
    m->data.fd = 0;
    m->bufsize = size;
    
//    VERIFYC(0 < rpcmem_fd, AEE_ENOTINITIALIZED);

    LOG_FASTRPC("alloc data 0x%p, heap_mask=0x%x, rpcflags=0x%x, pmem_flags=0x%x, size=0x%x\n", 
                    &m->data, heap_mask, rpcflags, pmem_flags, size);
                    // (struct pmem_fd_data)

    m->paddr = pmem_malloc_ext_v2(size, FASTRPC_DFLT_PMEM_ID,
            PMEM_FLAGS_SHMEM | PMEM_FLAGS_PHYS_CONTIG,
            PMEM_ALIGNMENT_4K, 0, &(m->data.handle), NULL);
    if ( m->paddr == NULL ) {
        LOG_FASTRPC("pmem_malloc_ext_v2() failed...");
        goto bail;
    }

    if(pmem_get_fd(m->paddr, FASTRPC_DFLT_PMEM_ID, &rpcmem_fd , &offset)) {
        LOG_FASTRPC("pmem_get_fd() failed...");
        goto bail;
    }

    m->data.fd = rpcmem_fd;
    LOG_FASTRPC("mmap data 0x%p", &m->data);

    if(rpcflags & RPCMEM_HEAP_NOVA) {
       VERIFY((uintptr_t)remote_register_fd);
       m->paddr = remote_register_fd(m->data.fd, len);
       VERIFY(0 != m->paddr);
       VERIFY((void*)-1 != m->paddr);
    } else {
#if 0
       VERIFYC(MAP_FAILED != (m->paddr = (void *)mmap(NULL, len,
                            PROT_READ|PROT_WRITE, MAP_SHARED, m->data.fd, 0)), AEE_EMMAP);
//                            PROT_READ|PROT_WRITE, MAP_SHARED, m->data.fd, offset)), AEE_EMMAP);
#endif
    }

    *ppo = m->paddr;

bail:
    if (nErr != AEE_SUCCESS) {
       LOG_FASTRPC("Error 0x%x (errno 0x%x): rpc contig allocation failed (size 0x%x, heap_mask 0x%x, rpcflags 0x%x)\n", nErr, errno, size, heap_mask, rpcflags);
       rpcmem_contig_free((struct mmap_info *)m, 0);
    }

    return nErr;
}

static void register_buf(void* buf, int size, int fd, int attr)
{
    if(remote_register_buf_attr) {
       remote_register_buf_attr(buf, size, fd, attr);
    } else if(remote_register_buf) {
       remote_register_buf(buf, size, fd);
    }
}

