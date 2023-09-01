/*=============================================================================
 * FILE:                      sysmon_cachelock_srv.c
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
                                  Includes
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

#define CACHELOCK_MEM_BLOCKS_NUM                100

#define CACHELOCK_QDI_MAX_OBJS                  20

/* Memory block structure */
typedef struct cachelock_mem_block
{
    struct cachelock_mem_block* next;
    struct cachelock_mem_block* prev;
    unsigned int addr_offset;
    short asid;
    unsigned int size;
} cachelock_mem_block_t;

/* Cache locking global structure */
typedef struct
{
    qurt_rmutex2_t mutex;
    unsigned int avail_cache_size;
    void* codelock_addr_ptr;
    unsigned int codelock_size;
    short codelock_asid;
} cachelock_info_t;

/* ============================================================================
                                 Declarations
   ========================================================================= */
static void cachelock_memblock_push(cachelock_mem_block_t* ptr);
static cachelock_mem_block_t* cachelock_memblock_pop(void);
int cachelock_query_avail(void);
int cachelock_query_total(void);
int cachelock_get_offset(unsigned int size,
                                short asid);
int cachelock_release_offset(unsigned int addr_offset, short asid);
unsigned int cachelock_get_memsize(unsigned int addr_offset, short asid);
static void cachelock_addmemblock_freelist(cachelock_mem_block_t* ptr);
static void cachelock_mergememblock_freelist(cachelock_mem_block_t* ptr);

int cachelock_addr_lock(void* vaddr_ptr, unsigned int size, short asid);
int cachelock_addr_unlock(void* vaddr_ptr, short asid);

/* ============================================================================
                                 Globals
   ========================================================================= */
cachelock_info_t cachelock_info = {{0},0,0,0,-1};
cachelock_mem_block_t cachelock_memblocks_arr[CACHELOCK_MEM_BLOCKS_NUM];
cachelock_mem_block_t* cachelock_memblocks_list_ptr = NULL;
cachelock_mem_block_t* cachelock_mem_freelist_ptr = NULL;
cachelock_mem_block_t* cachelock_mem_busylist_ptr = NULL;

/* ============================================================================
                                CODE STARTS HERE
   ========================================================================= */

/******************************************************************************
 * Function: sysmon_cachelock_init
 * Description: Initialization function for cache locking.
 *****************************************************************************/
void sysmon_cachelock_init(void)
{
    /* Mutex initialization */
    qurt_rmutex2_init(&cachelock_info.mutex);

    cachelock_info.avail_cache_size = AVAIL_L2_CACHE_SIZE;

    for (int i = 0; i < CACHELOCK_MEM_BLOCKS_NUM; i++)
    {
        cachelock_memblock_push(&cachelock_memblocks_arr[i]);
    }

    /* Pops out a memory block node from the list */
    cachelock_mem_block_t* ptr = cachelock_memblock_pop();
    if(ptr == 0)
    {
        FARF(ALWAYS, "Memory block not available in list");
        return;
    }

    ptr->addr_offset = 0;
    ptr->size = cachelock_info.avail_cache_size;
    ptr->next = NULL;
    ptr->prev = NULL;
    ptr->asid = -1;

    /* Initializing free memory blocks list with this first node */
    cachelock_mem_freelist_ptr = ptr;

}


/******************************************************************************
 * Function: cachelock_memblock_push
 * Description: Pushes given memory block to list.
 *****************************************************************************/
static void cachelock_memblock_push(cachelock_mem_block_t* ptr)
{
    if(ptr)
    {
        ptr->next = cachelock_memblocks_list_ptr;
        cachelock_memblocks_list_ptr = ptr;
    }
}


/******************************************************************************
 * Function: cachelock_memblock_pop
 * Description: Pops out a memory block from list.
 *****************************************************************************/
static cachelock_mem_block_t* cachelock_memblock_pop()
{
    cachelock_mem_block_t* ptr = cachelock_memblocks_list_ptr;

    if(cachelock_memblocks_list_ptr)
    {
        /* Moving the list pointer to next node */
        cachelock_memblocks_list_ptr = cachelock_memblocks_list_ptr->next;
    }

    return ptr;
}


/******************************************************************************
 * Function: cachelock_mergememblock_freelist
 * Description: Try to combine given memory block with adjacent memory blocks
                in free list.
 *****************************************************************************/
static void cachelock_mergememblock_freelist(cachelock_mem_block_t* ptr)
{
    cachelock_mem_block_t* freelist_ptr;

    /* Checks if current node can be merged with previous memory block */
    if( ptr->prev &&
        (ptr->prev->addr_offset + ptr->prev->size == ptr->addr_offset) )
    {
        freelist_ptr = ptr;

        ptr = ptr->prev;
        ptr->size += ptr->next->size;
        ptr->next = ptr->next->next;
        if(ptr->next)
        {
            ptr->next->prev = ptr;
        }

        /* After merge, push current node back to the list */
        cachelock_memblock_push(freelist_ptr);
    }

    /* Checks if next memory block can be merged with current node */
    if( ptr->next &&
        (ptr->addr_offset + ptr->size == ptr->next->addr_offset) )
    {
        freelist_ptr = ptr->next;
        ptr->size += ptr->next->size;
        ptr->next = ptr->next->next;
        if(ptr->next)
        {
            ptr->next->prev = ptr;
        }

        /* After merge, push next node back to the list */
        cachelock_memblock_push(freelist_ptr);
    }
}


/******************************************************************************
 * Function: cachelock_addmemblock_freelist
 * Description: Adds the given memory block to free list back and try to
                combine adjacent memory block nodes in free list.
 *****************************************************************************/
static void cachelock_addmemblock_freelist(cachelock_mem_block_t* ptr)
{
    cachelock_mem_block_t* freelist_ptr = cachelock_mem_freelist_ptr;

    /* If free list is empty then add given memory block as first node */
    if(!cachelock_mem_freelist_ptr)
    {
        ptr->prev = NULL;
        ptr->next = NULL;
        cachelock_mem_freelist_ptr = ptr;
    }
    else
    {
        while(freelist_ptr)
        {
            /* Checking for the node before which given node can be added
               based on increasing order of address offset. */
            if(ptr->addr_offset < freelist_ptr->addr_offset)
            {
                ptr->next = freelist_ptr;
                ptr->prev = freelist_ptr->prev;
                freelist_ptr->prev = ptr;
                if (!ptr->prev)
                {
                    cachelock_mem_freelist_ptr = ptr;
                }
                else
                {
                    ptr->prev->next = ptr;
                }

                /* Calls merge function to check if this newly added memory
                   block in free list can be combined with adjacent nodes */
                cachelock_mergememblock_freelist(ptr);
                break;
            }
            /* Adds given memory block as last node if reaches to list end */
            else if(!freelist_ptr->next)
            {
                freelist_ptr->next = ptr;
                ptr->next = NULL;
                ptr->prev = freelist_ptr;

                cachelock_mergememblock_freelist(ptr);
                break;
            }
            else
            {
                /* Moving free list pointer to next node */
                freelist_ptr = freelist_ptr->next;
            }
        }
    }
}


/******************************************************************************
 * Function: cachelock_get_offset
 * Description: Gets available address offset based on requested size. Returns
                available address offset else failure -> -1.
 *****************************************************************************/
int cachelock_get_offset(unsigned int size, short asid)
{
    qurt_rmutex2_lock(&cachelock_info.mutex);

    int result = -1;

    cachelock_mem_block_t* freelist_ptr = cachelock_mem_freelist_ptr;
    cachelock_mem_block_t* ptr = NULL;
    cachelock_mem_block_t* ptr_temp = NULL;
    cachelock_mem_block_t* busylist_ptr = NULL;

    /* Finds the memory block in free list which has memory size greater
       or equals to requested size. */
    while(freelist_ptr)
    {
        if(size <= freelist_ptr->size)
        {
            if(ptr)
            {
                if(freelist_ptr->size < ptr->size)
                {
                    ptr = freelist_ptr;
                }
            }
            else
            {
                ptr = freelist_ptr;
            }
        }

        freelist_ptr = freelist_ptr->next;
    }

    if(ptr)
    {
        /* Checks if node has available memory more than requested */
        if(size < ptr->size)
        {
            /* Pops out a node from list and update the node with available
               size and new address offset after requested allocation.
               Adds this new node in free list after current node. */
            ptr_temp = cachelock_memblock_pop();
            if(ptr_temp)
            {
                ptr_temp->addr_offset = ptr->addr_offset + size;
                ptr_temp->size = ptr->size - size;
                ptr_temp->asid = -1;

                ptr_temp->prev = ptr;
                ptr_temp->next = ptr->next;
                if(ptr_temp->next)
                {
                    ptr_temp->next->prev = ptr_temp;
                }
                ptr->next = ptr_temp;

                ptr->size = ptr->size - ptr_temp->size;
            }
            else
            {
                qurt_rmutex2_unlock(&cachelock_info.mutex);
                return result;
            }
        }


        /* Removing the selected node from the free list */
        if(ptr->prev)
        {
            ptr->prev->next = ptr->next;
        }
        else
        {
            cachelock_mem_freelist_ptr = ptr->next;
        }
        if(ptr->next)
        {
            ptr->next->prev = ptr->prev;
        }


        /* Getting address offset of selected node */
        result = ptr->addr_offset;
        ptr->asid = asid;

        /* Adds allocated memory block node to busy list. */
        /* If busy list is already empty then add the node as
           first node in the list. */
        if(cachelock_mem_busylist_ptr == NULL)
        {
            cachelock_mem_busylist_ptr = ptr;
            ptr->prev = NULL;
            ptr->next = NULL;
        }
        else
        {
            busylist_ptr = cachelock_mem_busylist_ptr;

            while(busylist_ptr)
            {
                /* Finds the node in busy list before which this allocated
                   node can be added based on increasing order of address
                   offset. */
                if(busylist_ptr->addr_offset > ptr->addr_offset)
                {
                    ptr->next = busylist_ptr;
                    ptr->prev = busylist_ptr->prev;
                    if(busylist_ptr->prev)
                    {
                        busylist_ptr->prev->next = ptr;
                    }
                    else
                    {
                        cachelock_mem_busylist_ptr = ptr;
                    }
                    busylist_ptr->prev = ptr;
                    break;
                }
                /* Adds as last node if reaches to busy list end */
                else if(!busylist_ptr->next)
                {
                    busylist_ptr->next = ptr;
                    ptr->next = NULL;
                    ptr->prev = busylist_ptr;
                    break;
                }
                else
                {
                    /* Moving to next node in busy list */
                    busylist_ptr = busylist_ptr->next;
                }
            }

        }

    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);

    return result;
}


/******************************************************************************
 * Function: cachelock_release_offset
 * Description: Move the memory block node based on given address offset from
                busy list to free list back to make it again available.
 *****************************************************************************/
int cachelock_release_offset(unsigned int addr_offset, short asid)
{
    int result = 1;

    cachelock_mem_block_t* busylist_ptr;

    qurt_rmutex2_lock(&cachelock_info.mutex);

    busylist_ptr = cachelock_mem_busylist_ptr;

    while(busylist_ptr)
    {
        /* Finds the node in busy list which has given address offset
           and asid. Then removes the node from the busy list. */
        if( (busylist_ptr->addr_offset == addr_offset) && (busylist_ptr->asid == asid) )
        {
            busylist_ptr->asid = -1;

            if(busylist_ptr->next)
            {
                busylist_ptr->next->prev = busylist_ptr->prev;
            }
            if(busylist_ptr->prev)
            {
                busylist_ptr->prev->next = busylist_ptr->next;
            }
            else
            {
                cachelock_mem_busylist_ptr = busylist_ptr->next;
            }

            /* Adds this removed node back to free list. */
            cachelock_addmemblock_freelist(busylist_ptr);
            result = 0;
            break;
        }
        else
        {
            /* Moves to next node in busy list */
            busylist_ptr = busylist_ptr->next;
        }
    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);
    return result;
}


/******************************************************************************
 * Function: cachelock_get_memsize
 * Description: Gets memory block size based on given address offset.
 *****************************************************************************/
unsigned int cachelock_get_memsize(unsigned int addr_offset, short asid)
{
    unsigned int size = 0;

    cachelock_mem_block_t* busylist_ptr;

    qurt_rmutex2_lock(&cachelock_info.mutex);

	busylist_ptr = cachelock_mem_busylist_ptr;

    while(busylist_ptr)
    {
        /* Finds memory block based on given address offset and ASID */
        if( (busylist_ptr->addr_offset == addr_offset) &&
                            (busylist_ptr->asid == asid) )
        {
            size = busylist_ptr->size;
            break;
        }

        busylist_ptr = busylist_ptr->next;
    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);

    return size;
}


/******************************************************************************
 * Function: cachelock_query_avail
 * Description: Returns max contiguous memory block size available for cache
                locking.
 *****************************************************************************/
int cachelock_query_avail()
{
    int max_avail_size = 0;

    cachelock_mem_block_t* freelist_ptr;

    qurt_rmutex2_lock(&cachelock_info.mutex);

	freelist_ptr = cachelock_mem_freelist_ptr;

    while(freelist_ptr)
    {
        /* Checks if node has greater memory size and then updates
           max size accordingly. */
        if(freelist_ptr->size > max_avail_size)
        {
            max_avail_size = freelist_ptr->size;
        }

        freelist_ptr = freelist_ptr->next;
    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);

    return max_avail_size;
}


/******************************************************************************
 * Function: cachelock_query_total
 * Description: Returns total locked cache size.
 *****************************************************************************/
int cachelock_query_total()
{
    int total_locked_size = 0;

    cachelock_mem_block_t* busylist_ptr;

    qurt_rmutex2_lock(&cachelock_info.mutex);

	busylist_ptr = cachelock_mem_busylist_ptr;

    while(busylist_ptr)
    {
        total_locked_size += busylist_ptr->size;
        busylist_ptr = busylist_ptr->next;
    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);

    return total_locked_size;
}


/******************************************************************************
 * Function: cachelock_addr_lock
 * Description: Locks cache for given virtual address and memory size. Returns
                success -> 0 else failure -> non-zero value.
 *****************************************************************************/
int cachelock_addr_lock(void* vaddr_ptr, unsigned int size, short asid)
{
    qurt_rmutex2_lock(&cachelock_info.mutex);

    /* Checks if there is already existing cache lock request */
    if(cachelock_info.codelock_addr_ptr != 0)
    {
        FARF(ALWAYS, "Request can't be honored, there is already"
                                             "existing cache lock request");
        qurt_rmutex2_unlock(&cachelock_info.mutex);
        return 1;
    }

    /* Locks cache based on virtual address and size. */
    int result = qurt_mem_l2cache_line_lock((qurt_addr_t)vaddr_ptr,
                                                 (qurt_size_t)size);
    if(result != 0)
    {
        FARF(ALWAYS, "Failed to lock cache, return value: %d", result);
    }
    else
    {
        /* Updates the global structure with locked virtual address,
           memory size and ASID */
        cachelock_info.codelock_addr_ptr = vaddr_ptr;
        cachelock_info.codelock_size = size;
        cachelock_info.codelock_asid = asid;
    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);

    return result;
}


/******************************************************************************
 * Function: cachelock_addr_unlock
 * Description: Unlocks cache for given virtual address. Returns success -> 0
                else failure -> non-zero value.
 *****************************************************************************/
int cachelock_addr_unlock(void* vaddr_ptr, short asid)
{
    qurt_rmutex2_lock(&cachelock_info.mutex);

    /* Checks if there is existing cache lock request from the
       same user for given address */
    if( (cachelock_info.codelock_addr_ptr != vaddr_ptr)
            || (cachelock_info.codelock_asid != asid) )
    {
        FARF(ALWAYS, "There is no existing cache lock request from the user");

        qurt_rmutex2_unlock(&cachelock_info.mutex);

        return 1;
    }

    /* Unlocks cache */
    int result = qurt_mem_l2cache_line_unlock((qurt_addr_t)vaddr_ptr,
                                              (qurt_size_t)cachelock_info.codelock_size);
    if(result != 0)
    {
        FARF(ALWAYS, "Failed to unlock cache, return value: %d", result);
    }
    else
    {
        /* Reset the global structure */
        cachelock_info.codelock_addr_ptr = 0;
        cachelock_info.codelock_size = 0;
        cachelock_info.codelock_asid = -1;
    }

    qurt_rmutex2_unlock(&cachelock_info.mutex);

    return result;
}