/*=============================================================================
 * FILE:                      sysmon_cachelock_client.c
 *
 * DESCRIPTION:
 *    Cache locking implementation.
 *
 * Copyright (c) 2017 QUALCOMM Technologies, Incorporated.
 * All Rights Reserved.
 * QUALCOMM Proprietary.
  ===========================================================================*/

/*=============================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 * when         who          what, where, why
 * ----------   ------     ------------------------------------------------
 ============================================================================*/

/* ============================================================================
               *********** Includes ***********
   ========================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include "qurt.h"
#include "HAP_farf.h"

/* ============================================================================
                            Variables and Defines
   ========================================================================= */
/* 64KB alignment */
#define ALIGN_64KB                      64*1024

/* 64 Byte alignment */
#define ALIGN_64B                       64

/* Available L2 cache size for locking in bytes */
#define AVAIL_L2_CACHE_SIZE             384*1024

/* Maximum size for code locking in bytes */
#define MAX_CODE_LOCK_SIZE              64*1024

/* Cache locking client global structure */
typedef struct
{
    unsigned char memory_init;
    void* start_vaddr_ptr;
} cachelock_client_info_t;

/* ============================================================================
                                 Declarations
   ========================================================================= */
void sysmon_cachelock_init(void);
int cachelock_query_avail(void);
int cachelock_query_total(void);
int cachelock_get_offset(unsigned int size,
                                short asid);
int cachelock_release_offset(unsigned int addr_offset, short asid);
unsigned int cachelock_get_memsize(unsigned int addr_offset, short asid);
int cachelock_addr_lock(void* vaddr_ptr, unsigned int size, short asid);
int cachelock_addr_unlock(void* vaddr_ptr, short asid);
/* ============================================================================
                                 Globals
   ========================================================================= */
static int cachelock_qdi_handle = -1;
static cachelock_client_info_t cachelock_client_info = {0};
static unsigned int spinLock = 0;
static unsigned int spinLockInit = 0;

/* ============================================================================
                               CODE STARTS HERE
   ========================================================================= */

/******************************************************************************
 * Function: cachelock_qdi_init
 * Description: QDI init function.
 *****************************************************************************/

__attribute__ ((noinline))
int cachelock_qdi_init(void)
{
    unsigned int *spinLockPtr = &spinLockInit;
    __asm__  __volatile__ (
    "check1: r0 = memw_locked(%0)  \n"
    "    p0 = cmp.eq(r0, #0)           \n"
    "    if (!p0) jump check1      \n"
    "    memw_locked(%0, p0) = %0      \n"
    "    if !p0 jump check1        \n"
    : "+r" (spinLockPtr) : : "p0","r0");

    /* Checks if QDI init is already done or not */
    if(cachelock_qdi_handle <= 0)
    {
        sysmon_cachelock_init();
        cachelock_qdi_handle = 1;
    }

    spinLockInit = 0;
    return 0;
}


/******************************************************************************
 * Function: HAP_cache_lock
 * Description: Allocates memory and locks cache for given memory size (in Bytes)
                and returns locked virtual address. In case of failure returns 0.
                "paddr_ptr" argument is to get the locked 64bit physical address.
                If physical address is not needed then can pass NULL.
 *****************************************************************************/
void* HAP_cache_lock(unsigned int size, unsigned long long *paddr_ptr)
{
    /* Checks if given memory size is valid or not */
    if( (size == 0) || (size > AVAIL_L2_CACHE_SIZE) )
    {
        FARF(ALWAYS, "Invalid input memory size %u for cache locking request", size);
        return 0;
    }

    int result;
    unsigned int addr_offset;

    /* Calling QDI init */
    result = cachelock_qdi_init();
    if(result != 0)
    {
        FARF(ALWAYS, "QDI open failed in cache locking request, "
                                         "return value: %d", result);
        return 0;
    }

    unsigned int *spinLockPtr = &spinLock;
    __asm__  __volatile__ (
    "check2: r0 = memw_locked(%0)  \n"
    "    p0 = cmp.eq(r0, #0)           \n"
    "    if (!p0) jump check2      \n"
    "    memw_locked(%0, p0) = %0      \n"
    "    if !p0 jump check2        \n"
    : "+r" (spinLockPtr) : : "p0","r0");

    /* Checks if memory pool is allocated for user PD */
    if(!cachelock_client_info.memory_init)
    {
        /* Allocates memory pool */
        cachelock_client_info.start_vaddr_ptr = malloc(AVAIL_L2_CACHE_SIZE
                                                       + (ALIGN_64KB - 1));
        if(cachelock_client_info.start_vaddr_ptr == NULL)
        {
            FARF(ALWAYS,"Failed to allocate memory pool for cache locking");
            return 0;
        }

        /* To align the allocated memory pool */
        cachelock_client_info.start_vaddr_ptr = (void*)(((unsigned int)cachelock_client_info.start_vaddr_ptr
                                                                 + (ALIGN_64KB-1))&(-ALIGN_64KB));
        cachelock_client_info.memory_init = 1;
    }

    spinLock = 0;

    /*Align given size to 64 Byte */
    size = (size + (ALIGN_64B-1)) & (-ALIGN_64B);

    /* Address offset request for the given size, returns
       available offset. */
    result = cachelock_get_offset(size, 1);
    if (0 > result)
    {
        FARF(ALWAYS, "Failed to get address offset for cache locking, "
                        "return value: %d, input size: %u", result, size);
        return 0;
    }
    addr_offset = (unsigned int)result;

    /* Add offset to get the required virtual address for locking */
    void* vaddr_ptr = (void*)((unsigned int)cachelock_client_info.start_vaddr_ptr
                                                         + addr_offset);

    /* Locks cache based on virtual address and size. */
    result = qurt_mem_l2cache_line_lock((qurt_addr_t)vaddr_ptr,
                                        (qurt_size_t)size);
    if(result != 0)
    {
        FARF(ALWAYS, "Failed to lock cache, return value: %d, input size: %u", result, size);

        /* Calling to release address offset as cache lock failed */
        result = cachelock_release_offset(addr_offset, 1);
        if(result !=0)
        {
            FARF(ALWAYS, "Failed to release address offset, return "
                            "value: %d, input size: %u", result, size);
        }

        return 0;
    }

    if(paddr_ptr != NULL)
    {
        /* To get 64-bit physical address the virtual address is mapped to. */
        qurt_paddr_64_t paddr = qurt_lookup_physaddr_64((qurt_addr_t)vaddr_ptr);
        if(paddr == 0)
        {
            FARF(ALWAYS, "Failed to get physical address for locked "
                                                         "virtual address");
        }
        *paddr_ptr = paddr;
    }

    return vaddr_ptr;
}


 /******************************************************************************
 * Function: HAP_cache_unlock
 * Description: Unlocks cache and deallocates memory based on given virtual
                address. Returns success -> 0 else failure -> non-zero value.
 *****************************************************************************/
int HAP_cache_unlock(void* vaddr_ptr)
{
    int result;
    unsigned int addr_offset;

    /* Checks given virtual address */
    if(vaddr_ptr == 0)
    {
        FARF(ALWAYS, "Invalid virtual address for cache unlock request, "
                                    "input virtual address: %p", vaddr_ptr);
        return 1;
    }

    /* Checks if memory pool is allocated for user PD */
    if(cachelock_client_info.memory_init != 1)
    {
        FARF(ALWAYS, "Memory pool has not been allocated");
        return 1;
    }

    unsigned int size;

    /* Calling QDI init */
    result = cachelock_qdi_init();
    if(result != 0)
    {
        FARF(ALWAYS, "QDI open failed in cache unlock request, "
                                         "return value: %d", result);
    }
    else
    {
        addr_offset = (unsigned int)vaddr_ptr - (unsigned int)cachelock_client_info.start_vaddr_ptr;
        if(addr_offset >= AVAIL_L2_CACHE_SIZE)
        {
            FARF(ALWAYS, "Address offset %u is out of range, input "
                            "virtual address: %p", addr_offset, vaddr_ptr);
            result = 1;
        }
        else
        {
            /* Request to get the size of memory block for given virtual
               address which is to be unlocked */
            size = cachelock_get_memsize(addr_offset, 1);
            if (0 == size)
                result = -1;
            if(result != 0)
            {
                FARF(ALWAYS, "Failed to get memory block size for cache unlocking, "
                              "return value: %d, input virtual address: %p", result, vaddr_ptr);
            }
            else
            {
                /* Unlocks cache for given address and size. */
                result = qurt_mem_l2cache_line_unlock((qurt_addr_t)vaddr_ptr,
                                                           (qurt_size_t)size);
                if(result != 0)
                {
                    FARF(ALWAYS, "Failed to unlock cache, return value: %d, input "
                                            "virtual address: %p", result, vaddr_ptr);
                }
                else
                {
                    /* Calling to release address offset */
                    result = cachelock_release_offset(addr_offset, 1);
                    if(result !=0)
                    {
                        FARF(ALWAYS, "Failed to release address offset, return "
                                      "value: %d, input virtual address: %p", result, vaddr_ptr);
                    }
                }
            }
        }
    }

    return result;
}


/******************************************************************************
 * Function: HAP_query_avail_cachelock
 * Description: Query API to get the size of max contiguous memory block
                available for cache locking. In case of failure returns -1
                else available size (in Bytes).
 *****************************************************************************/
int HAP_query_avail_cachelock()
{
    int result, avail_size;

    /* Calling QDI init */
    result = cachelock_qdi_init();

    if(result != 0)
    {
        FARF(ALWAYS, "QDI open failed in available cachelock query, "
                                            "return value: %d", result);
        return -1;
    }

    unsigned int *spinLockPtr = &spinLock;
    __asm__  __volatile__ (
    "check3: r0 = memw_locked(%0)  \n"
    "    p0 = cmp.eq(r0, #0)           \n"
    "    if (!p0) jump check3      \n"
    "    memw_locked(%0, p0) = %0      \n"
    "    if !p0 jump check3        \n"
    : "+r" (spinLockPtr) : : "p0","r0");

    /* Checks if memory pool is allocated for user PD */
    if(!cachelock_client_info.memory_init)
    {
        /* Allocates memory pool */
        cachelock_client_info.start_vaddr_ptr = malloc(AVAIL_L2_CACHE_SIZE
                                                       + (ALIGN_64KB - 1));
        if(cachelock_client_info.start_vaddr_ptr == NULL)
        {
            FARF(ALWAYS,"Failed to allocate memory pool for cache locking");
        }
        else
        {
            /* To align the allocated memory pool */
            cachelock_client_info.start_vaddr_ptr = (void*)(((unsigned int)cachelock_client_info.start_vaddr_ptr
                                                                    + (ALIGN_64KB-1))&(-ALIGN_64KB));
            cachelock_client_info.memory_init = 1;
        }
    }

    spinLock = 0;

    /* To get the size of max contiguous memory block available
       for cache locking.  */
    avail_size = cachelock_query_avail();
    return avail_size;
}


/******************************************************************************
 * Function: HAP_query_total_cachelock
 * Description: Query API to get the total locked cache size. In case of
                failure returns -1 else total locked cache size (in Bytes).
 *****************************************************************************/
int HAP_query_total_cachelock()
{
    int result, total_size;

    /* Calling QDI init */
    result = cachelock_qdi_init();
    if(result != 0)
    {
        FARF(ALWAYS, "QDI open failed in total cachelock query, "
                                         "return value: %d", result);
        return -1;
    }

    /* To get the total locked cache size */
    total_size = cachelock_query_total();

    return total_size;
}


/******************************************************************************
 * Function: HAP_cache_lock_addr
 * Description: Locks cache for given virtual address and memory size (in Bytes).
                Returns success -> 0 else failure -> non-zero value. Only one
                request will be honored at a time and there is maximum size limit
                of 64KB. This API is exclusively for code locking purpose.
 *****************************************************************************/
int HAP_cache_lock_addr(void* vaddr_ptr, unsigned int size)
{
    /* Checks if given size is valid or not */
    if( (size == 0) || (size > MAX_CODE_LOCK_SIZE) )
    {
        FARF(ALWAYS, "Invalid input size %u for cache locking request", size);
        return 1;
    }

    /* Calling QDI init */
    int result = cachelock_qdi_init();
    if(result != 0)
    {
        FARF(ALWAYS, "QDI open failed in cache locking request, "
                                         "return value: %d", result);
    }
    else
    {
        /* Cache lock request for given virtual address and memory size */
        result = cachelock_addr_lock(vaddr_ptr, size, 1);
        if(result != 0)
        {
            FARF(ALWAYS, "Cache lock failed, return value: %d, input virtual "
                             "address: %p and size: %u", result, vaddr_ptr, size);
        }
    }

    return result;
}


/******************************************************************************
 * Function: HAP_cache_unlock_addr
 * Description: Unlocks cache for given virtual address. Returns success -> 0
                else failure -> non-zero value. This API should be used along
                with "HAP_cache_lock_addr" API.
 *****************************************************************************/
int HAP_cache_unlock_addr(void* vaddr_ptr)
{
    /* Calling QDI init */
    int result = cachelock_qdi_init();
    if(result != 0)
    {
        FARF(ALWAYS, "QDI open failed in cache unlock request, "
                                         "return value: %d", result);
    }
    else
    {
        /* Cache unlock request for given virtual address */
        result = cachelock_addr_unlock(vaddr_ptr, 1);
        if(result != 0)
        {
            FARF(ALWAYS, "Cache unlock failed, return value: %d, input "
                                "virtual address: %p", result, vaddr_ptr);
        }
    }

    return result;
}
