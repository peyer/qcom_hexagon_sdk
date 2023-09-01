/*=============================================================================
 * FILE:                      sysmon_vtcm_mgr_srv.c
 *
 * DESCRIPTION:
 *    Vector TCM management
 *
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
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
#include "q6protos.h"
#include "qurt.h"
#define FARF_ERROR 1
#include "HAP_farf.h"
#include "sysmon_vtcmmgr_int.h"

/* ============================================================================
                            Variables and Defines
   ========================================================================= */

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

/* Number of static memory block identifiers (nodes) */
#define VTCMMGR_NUM_MEM_NODES           64
/* Maximum number of wait nodes to be defined statically */
#define VTCMMGR_NUM_WAIT_NODES          64

/* VTCM manager mem node descriptor structure */
typedef struct vtcmMgr_mem_node {
    struct vtcmMgr_mem_node* pNext; //Pointer to the next node
    struct vtcmMgr_mem_node* pPrev; //Pointer to the previous node
    qurt_addr_t PA;                 //Physical addr of this node
    unsigned int size;              //Allocated size
    union{
        unsigned char reserved:7;
        unsigned char bFree:1;      //Boolean for free
    } desc;
    unsigned short asid;            //Allocating PD's ASID
    qurt_addr_t VA;                 //Virtual address returned to the caller
    qurt_mem_region_t region;       //QuRT memory region identifier
} vtcmMgr_mem_node_t;

typedef struct vtcmMgr_wait_node {
    struct vtcmMgr_wait_node *pNext;
    struct vtcmMgr_wait_node *pPrev;
    int clientHandle;
    unsigned short asid;
    unsigned long long mailboxId;
    unsigned int size;
    unsigned char bSinglePage;
    unsigned char bPending;
    void *pVA;
} vtcmMgr_wait_node_t;

typedef struct vtcmMgr_wait_queue {
    vtcmMgr_wait_node_t *pBusy;
    vtcmMgr_wait_node_t *pFree;
    unsigned short mailboxCount;
} vtcmMgr_wait_queue_t;

/* VTCM manager global structure definition */
typedef struct {
    qurt_rmutex2_t mutex;
    unsigned char bInitDone;        //boolean for INITIALIZED
    qurt_mem_pool_t pool;           //QuRT memory pool identifier
    qurt_addr_t PA;                 //Physical start address for VTCM pool
    qurt_addr_t VA;                 //Virtual address returned by QuRT for v66 and above
    qurt_size_t size;               //Size of VTCM pool
    vtcmMgr_wait_queue_t waitQ;
} g_vtcmMgr_t;

/* ============================================================================
                                 Declarations
   ========================================================================= */

static void vtcmMgr_mem_init(void);
static void vtcmMgr_memNode_push(vtcmMgr_mem_node_t* pMem);
static vtcmMgr_mem_node_t* vtcmMgr_memNode_pop(void);
static unsigned int vtcmMgr_alloc_alignSize(unsigned int size,
                                            unsigned char bSinglePage);
static unsigned char vtcmMgr_alloc_checkFit(vtcmMgr_mem_node_t* pMem,
                                            unsigned int size,
                                            unsigned char bSinglePage);
void* vtcmMgr_alloc(int clientHandle,
                           unsigned short asid,
                           unsigned int size,
                           unsigned char bSinglePage);
void* vtcmMgr_alloc_async(int clientHandle,
                          unsigned short asid,
                          unsigned int size,
                          unsigned char bSinglePage,
                          unsigned long long *p_mailboxId);
int vtcmMgr_alloc_async_done(int clientHandle,
                             unsigned long long mailboxId);
int vtcmMgr_alloc_async_cancel(int clientHandle,
                               unsigned long long mailboxId,
                               void **pVA);
static void vtcmMgr_free_coalesceNode(vtcmMgr_mem_node_t* pNode);
static void vtcmMgr_free_addNode(vtcmMgr_mem_node_t* pNode);
int vtcmMgr_free(int clientHandle,
                 unsigned char asid,
                 qurt_addr_t pVA);
int vtcmMgr_query_total(unsigned int* size,
                               unsigned int* numPages);
static void vtcmMgr_query_pages(vtcmMgr_mem_node_t* pNode,
                                unsigned int* maxBlockSize,
                                unsigned int* numBlocks);
int vtcmMgr_query_avail(unsigned int* freeBlockSize,
                               unsigned int* maxPageSize,
                               unsigned int* numPages);
static void vtcmMgr_waitNode_push(vtcmMgr_wait_node_t* pMem);
static vtcmMgr_wait_node_t* vtcmMgr_waitNode_pop(void);
static void vtcmMgr_recheck_waitQ(void);

/* ============================================================================
                                 Globals
   ========================================================================= */

/* An array of VTCM mem block identifiers/nodes */
vtcmMgr_mem_node_t vtcmMemNodeArray[VTCMMGR_NUM_MEM_NODES];
/* An array of VTCM  */
vtcmMgr_wait_node_t vtcmWaitNodeArray[VTCMMGR_NUM_WAIT_NODES];
/* A single linked list operated as LIFO for memory nodes */
vtcmMgr_mem_node_t* pVtcmMemNodeList = NULL;
/* A double linked list with free memory blocks */
vtcmMgr_mem_node_t* pVtcmMemFreeList = NULL;
/* A double linked list with occupied/allocated memory blocks */
vtcmMgr_mem_node_t* pVtcmMemBusyList = NULL;
/* VTCM manager global structure */
g_vtcmMgr_t g_vtcmMgr = {{0},0};

/* ============================================================================
                                CODE STARTS HERE
   ========================================================================= */

/******************************************************************************
 * Function: sysmon_vtcm_mgr_init
 * Description: Init function to be called at DSP init. Initializes VTCM
 *              Manager mem nodes and QDI interfaces.
 *****************************************************************************/
void sysmon_vtcmMgr_init(void)
{
    qurt_rmutex2_init(&g_vtcmMgr.mutex);
    /* Initialize VTCM manager memory interface */
    vtcmMgr_mem_init();
}

/* :: vtcmMgr_mem_init ::
 * Initializes VTCM manager mem nodes and attaches to the VTCM_PHYSPOOL from
 * QuRT
 */
static void vtcmMgr_mem_init(void)
{
    //qurt_mem_pool_attr_t poolAttr;
    vtcmMgr_mem_node_t* mem;
    int i;

    /* Push all the statically defined mem nodes to the node list */
    for (i = 0; i < VTCMMGR_NUM_MEM_NODES; i++)
    {
        vtcmMgr_memNode_push(&vtcmMemNodeArray[i]);
    }
    /* Push all wait nodes to the free list */
    for (i = 0; i < VTCMMGR_NUM_WAIT_NODES; i++)
    {
        vtcmMgr_waitNode_push(&vtcmWaitNodeArray[i]);
    }
    /* Hardcode the VTCM start for simulator where in --l2tcm_base 0xd800 */
    g_vtcmMgr.PA = 0xd8200000;
    g_vtcmMgr.size = 0x40000;

#if (__HEXAGON_ARCH__ >= 66)
    i = qurt_mem_map_static_query(&g_vtcmMgr.VA,
                                g_vtcmMgr.PA,
                                g_vtcmMgr.size,
                                QURT_MEM_CACHE_WRITEBACK,
                                (QURT_PERM_READ | QURT_PERM_WRITE) );
    if (0 != i)
    {
        FARF(ERROR, "Failed to query VTCM virtual address, result = %d, VA = 0x%x",
                i, g_vtcmMgr.VA);
        g_vtcmMgr.VA = 0;
    }
    else
    {
        FARF(LOW, "Got VTCM virtual address as 0x%x", g_vtcmMgr.VA);
    }
#endif

    if ( (0 != g_vtcmMgr.PA) && (0 != g_vtcmMgr.size) )
    {
        /* We have a valid memory spread */
        /* Initialize mem free list.
         * Pop out a memory node from the mem node list for creating a head node
         * in the Free list */
        mem = vtcmMgr_memNode_pop();
        if (!mem)
        {
            /* Failed to get a mem node from list */
            return;
        }
        /*
         * Node:
         *      PA: VTCM start physical address
         *      size: Size of VTCM pool
         *      desc.bFree : Free (1)
         */
        mem->PA = g_vtcmMgr.PA;
        mem->size = g_vtcmMgr.size;
        mem->desc.bFree = TRUE;
        mem->pNext = NULL;
        mem->pPrev = NULL;
        mem->asid = 0;
        mem->VA = 0;
        /* Push the mem node created to the Free list */
        pVtcmMemFreeList = mem;
        /* Update init done to success in the end */
        g_vtcmMgr.bInitDone = TRUE;
    }
    else
    {
        /* Unable to get proper memory spread for VTCM pool
         * mark init done as FALSE */
        g_vtcmMgr.bInitDone = FALSE;
    }
}

/* :: vtcmMgr_memNode_push ::
 *    Pushes given mem node to mem node LIFO
 */
static void vtcmMgr_memNode_push(vtcmMgr_mem_node_t* pMem)
{
    if (pMem)
    {
        /* Push given object to the front of the list */
        pMem->pNext = pVtcmMemNodeList;
        pVtcmMemNodeList = pMem;
    }
}

/* :: vtcmMgr_memNode_pop ::
 *    Pops out a mem node from the mem node LIFO
 */
static vtcmMgr_mem_node_t* vtcmMgr_memNode_pop(void)
{
    vtcmMgr_mem_node_t* retVal;
    /* Pop out a mem node from the mem node list */
    retVal = pVtcmMemNodeList;
    if (pVtcmMemNodeList)
    {
        pVtcmMemNodeList = pVtcmMemNodeList->pNext;
    }
    else
    {
        pVtcmMemNodeList = NULL;
    }
    /* return the node */
    return retVal;
}

/* :: vtcmMgr_alloc_alignSize ::
 *    Aligns size provided by the user to desired VTCM mem block size based on
 *    single page requirement
 */
static unsigned int vtcmMgr_alloc_alignSize(unsigned int size,
                                            unsigned char bSinglePage)
{
    unsigned int tempSize = 0;
    if (bSinglePage)
    {
        /* User is requesting for a single page memory allocation.
         * Align size to 4k/16k/64k/256KB multiples to get a
         * single page allocation (TLB entry) by QuRT.
         */
        tempSize = 31 - Q6_R_cl0_R(size);
        if (tempSize < 12)
        {
            tempSize = 12;
        }
        tempSize = ( (tempSize >> 1) << 1);
        tempSize = 1 << tempSize;
        if (tempSize < size)
        {
            tempSize = tempSize * 4;
        }
    }
    else
    {
        /* Align to 4k multiple */
        tempSize = ( (size + 4095) & (~4095) );
    }
    return tempSize;
}

/* :: vtcmMgr_alloc_checkFit ::
 *    Checks if the provide mem node (from free list) can accommodate a user
 *    request of given size and single page requirements
 *    Caller should ensure that the size parameter fits into VTCM space
 */
static unsigned char vtcmMgr_alloc_checkFit(vtcmMgr_mem_node_t* pMem,
                                            unsigned int size,
                                            unsigned char bSinglePage)
{
    unsigned char bResult = FALSE;
    qurt_addr_t PA_aligned = pMem->PA;
    if (bSinglePage)
    {
        /* Align PA to ensure a single page */
        PA_aligned = ( ( pMem->PA + (size - 1) ) & ( ~(size - 1) ) );
    }
    if ( (PA_aligned + size) <= (pMem->PA + pMem->size) )
    {
        /* Current mem node supports the size request, return success */
        bResult = TRUE;
    }
    /* Return result */
    return bResult;
}

/* :: vtcmMgr_alloc ::
 *    Allocates a memory block in VTCM for the requested ASID based on its
 *    size and single page requirements.
 *
 *    Returns virtual address to the allocated mem block on success. Returns 0
 *    on failure.
 */
void* vtcmMgr_alloc(int clientHandle,
                           unsigned short asid,
                           unsigned int size,
                           unsigned char bSinglePage)
{
    void* pVA = 0;
    /* Get aligned size based on user inputs */
    unsigned int size_aligned;
    vtcmMgr_mem_node_t* pFreeList;
    vtcmMgr_mem_node_t* pNode = NULL;
    vtcmMgr_mem_node_t* pBusyList = NULL;
    vtcmMgr_mem_node_t* pNodeTemp = NULL;
    qurt_addr_t PA_aligned = NULL;
    if ( (!g_vtcmMgr.bInitDone) || (size > g_vtcmMgr.size) || (size == 0) )
    {
        /* vtcm manager not initialized, return 0 VA */
        return pVA;
    }
    qurt_rmutex2_lock(&g_vtcmMgr.mutex);
    size_aligned = vtcmMgr_alloc_alignSize(size, bSinglePage);
    pFreeList = pVtcmMemFreeList;
    /* Iterate over available mem nodes to find the best fit */
    while (pFreeList)
    {
        if (pFreeList->desc.bFree &&
            vtcmMgr_alloc_checkFit(pFreeList, size_aligned, bSinglePage) )
        {
            if (pNode)
            {
                /* Find the closest block available for requested size */
                if (pFreeList->size < pNode->size)
                {
                    pNode = pFreeList;
                }
            }
            else
            {
                pNode = pFreeList;
            }
        }
        pFreeList = pFreeList->pNext;
    }
    if (pNode)
    {
        /* found a node to accommodate the request */
        PA_aligned = pNode->PA;
        if (bSinglePage)
        {
            /* Align PA to ensure a single page */
            PA_aligned = ( ( pNode->PA + (size_aligned - 1) ) & ( ~(size_aligned -1) ) );
        }
        if (pNode->PA < PA_aligned)
        {
            /* An alignment hole is introduced due to aligning PA
             * Create a free node for the size of the alignment hole and add
             * it to the free list
             */
            pNodeTemp = vtcmMgr_memNode_pop();
            if (pNodeTemp)
            {
                /* Prepend a new node:
                 *      PA: Current node PA
                 *      Size: alignment hole
                 */
                pNodeTemp->PA = pNode->PA;
                pNodeTemp->size = PA_aligned - pNode->PA;
                pNodeTemp->desc.bFree = 1;
                pNodeTemp->asid = 0;
                pNodeTemp->region = 0;
                pNodeTemp->VA = 0;
                /* Assign links */
                pNodeTemp->pPrev = pNode->pPrev;
                pNodeTemp->pNext = pNode;
                if (pNode->pPrev)
                {
                    pNode->pPrev->pNext = pNodeTemp;
                }
                else
                {
                    /* We are modifying the head node.
                     * Modify the list pointer.
                     */
                    pVtcmMemFreeList = pNodeTemp;
                }
                pNode->pPrev = pNodeTemp;
                /* Modify current node:
                 *      PA: Aligned PA
                 *      size: Remaining size
                 */
                pNode->PA = PA_aligned;
                pNode->size = pNode->size - pNodeTemp->size;
            }
            else
            {
                /* Unable to create a new node, return */
                goto bail;
            }
        }
        if ( (PA_aligned + size_aligned) < (pNode->PA + pNode->size) )
        {
            /* There's size available in the node after the allocation.
             * Create a new node for this block and append it to the free list
             */
            pNodeTemp = vtcmMgr_memNode_pop();
            if (pNodeTemp)
            {
                /* Append a new node:
                 *         PA: Aligned PA + aligned size
                 *         size: Remaining size in the node
                 */
                pNodeTemp->PA = PA_aligned + size_aligned;
                pNodeTemp->size = pNode->size - size_aligned;
                pNodeTemp->desc.bFree = 1;
                pNodeTemp->asid = 0;
                pNodeTemp->region = 0;
                pNodeTemp->VA = 0;
                /* Modify the links */
                pNodeTemp->pPrev = pNode;
                pNodeTemp->pNext = pNode->pNext;
                if (pNodeTemp->pNext)
                {
                    pNodeTemp->pNext->pPrev = pNodeTemp;
                }
                pNode->pNext = pNodeTemp;
                /* Modify current node:
                 *         size: Remaining size
                 */
                pNode->size = pNode->size - pNodeTemp->size;
            }
            else
            {
                /* Failed to allocate a mem node.
                 * Do clean up.
                 */
                vtcmMgr_free_coalesceNode(pNode);
                goto bail;
            }
        }
        /* Mark current node as busy */
        pNode->desc.bFree = 0;
        pNode->region = 0;
        /* Delete the node from free list */
        if (pNode->pPrev)
        {
            pNode->pPrev->pNext = pNode->pNext;
        }
        else
        {
            pVtcmMemFreeList = pNode->pNext;
        }
        if (pNode->pNext)
        {
            pNode->pNext->pPrev = pNode->pPrev;
        }
        /* Create direct memory mapping assuming idempotent virtual address is available for Simulator */
        if (0 == qurt_mapping_create( pNode->PA,
                                      pNode->PA,
                                      (qurt_size_t) pNode->size,
                                      QURT_MEM_CACHE_WRITEBACK,
                                      (QURT_PERM_READ | QURT_PERM_WRITE) ) )
        {
            pNode->VA = pNode->PA;
            pVA = (void *)pNode->VA; //Return value to caller
            pNode->asid = asid;
            /* Add the node to the busy list */
            if (NULL == pVtcmMemBusyList)
            {
                /* Busy list is empty, add the current node to the head */
                pVtcmMemBusyList = pNode;
                pNode->pPrev = NULL;
                pNode->pNext = NULL;
            }
            else
            {
                pBusyList = pVtcmMemBusyList;
                while (pBusyList)
                {
                    if (pBusyList->PA > pNode->PA)
                    {
                        /* Add the node at the current position */
                        pNode->pNext = pBusyList;
                        pNode->pPrev = pBusyList->pPrev;
                        if (pBusyList->pPrev)
                        {
                            pBusyList->pPrev->pNext = pNode;
                        }
                        else
                        {
                            pVtcmMemBusyList = pNode;
                        }
                        pBusyList->pPrev = pNode;
                        break;
                    }
                    else if (!pBusyList->pNext)
                    {
                        /* We are at the end of the busy list.
                         * Add the node to the end */
                        pBusyList->pNext = pNode;
                        pNode->pNext = NULL;
                        pNode->pPrev = pBusyList;
                        break;
                    }
                    else
                    {
                        pBusyList = pBusyList->pNext;
                    }
                }
            }
        }
#if (__HEXAGON_ARCH__ >= 66)
        else if (g_vtcmMgr.VA)
        {
            pNode->VA = (pNode->PA - g_vtcmMgr.PA) + g_vtcmMgr.VA;
            pVA = (void *)pNode->VA; //Return value to caller
            pNode->asid = asid;
            /* Add the node to the busy list */
            if (NULL == pVtcmMemBusyList)
            {
                /* Busy list is empty, add the current node to the head */
                pVtcmMemBusyList = pNode;
                pNode->pPrev = NULL;
                pNode->pNext = NULL;
            }
            else
            {
                pBusyList = pVtcmMemBusyList;
                while (pBusyList)
                {
                    if (pBusyList->PA > pNode->PA)
                    {
                        /* Add the node at the current position */
                        pNode->pNext = pBusyList;
                        pNode->pPrev = pBusyList->pPrev;
                        if (pBusyList->pPrev)
                        {
                            pBusyList->pPrev->pNext = pNode;
                        }
                        else
                        {
                            pVtcmMemBusyList = pNode;
                        }
                        pBusyList->pPrev = pNode;
                        break;
                    }
                    else if (!pBusyList->pNext)
                    {
                        /* We are at the end of the busy list.
                         * Add the node to the end */
                        pBusyList->pNext = pNode;
                        pNode->pNext = NULL;
                        pNode->pPrev = pBusyList;
                        break;
                    }
                    else
                    {
                        pBusyList = pBusyList->pNext;
                    }
                }
            }
        }
#endif
        else
        {
            pNode->desc.bFree = 1;
            vtcmMgr_free_addNode(pNode);
        }
    }
bail:
    /* Return allocated virtual address to the caller in success case.
     * or 0 in failure.
     */
    qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
    return pVA;
}

void* vtcmMgr_alloc_async(int clientHandle,
                          unsigned short asid,
                          unsigned int size,
                          unsigned char bSinglePage,
                          unsigned long long *p_mailboxId)
{
    void *pVA = NULL;
    char mailboxName[9];
    vtcmMgr_wait_node_t *waitNode;
    unsigned long long mailboxId;
    /* Input validation */
    if ( (!g_vtcmMgr.bInitDone) || 
         (0 == size) ||
         ( size > g_vtcmMgr.size) )
    {
        return pVA;
    }
    qurt_rmutex2_lock(&g_vtcmMgr.mutex);
    pVA = vtcmMgr_alloc(clientHandle,
                        asid,
                        size,
                        bSinglePage);
    if (pVA)
    {
        goto err;
    }
    if (p_mailboxId && !pVA)
    {
        /* Failed to allocate user request, create a mailbox
         * Pop a wait node from wait queue */
        waitNode = vtcmMgr_waitNode_pop();
        if (waitNode)
        {
            /* Create a clientHandle + mailboxcount string for
             * unique identifier
             */
            snprintf(mailboxName,
                     9,
                     "vtcm%x",
                     g_vtcmMgr.waitQ.mailboxCount);
            /* Create a mailbox with receiver as userPD. This
             * works when receiver is GuestOS, userPD, unsigned
             * and CPZ processes as well
             */
            mailboxId = qurt_mailbox_create(mailboxName,
                                      QURT_MAILBOX_AT_USERPD);
            if (mailboxId == QURT_MAILBOX_ID_NULL)
            {
                /* Failed to create a mailbox ID for the waiting
                 * thread, return error. Push the wait node
                 * back to the list
                 */
                vtcmMgr_waitNode_push(waitNode);
                goto err;
            }
            else
            {
                /* Pass the mailbox Id to the user */
                *p_mailboxId = mailboxId;
                /* Update wait node */
                waitNode->clientHandle = clientHandle;
                waitNode->asid = asid;
                waitNode->mailboxId = mailboxId;
                waitNode->size = size;
                waitNode->bSinglePage = bSinglePage;
                waitNode->bPending = TRUE;
                waitNode->pVA = 0;
                /* Push it to the tail of the queue */
                waitNode->pNext = g_vtcmMgr.waitQ.pBusy;
                waitNode->pPrev = NULL;
                if (g_vtcmMgr.waitQ.pBusy)
                    g_vtcmMgr.waitQ.pBusy->pPrev = waitNode;
                g_vtcmMgr.waitQ.pBusy = waitNode;
                /* Increment mailbox count */
                g_vtcmMgr.waitQ.mailboxCount++;
            }
        }
    }
err: 
    qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
    return pVA;
}

int vtcmMgr_alloc_async_done(int clientHandle,
                             unsigned long long mailboxId)
{
    int result = 0;
    vtcmMgr_wait_node_t *waitNode;

    qurt_rmutex2_lock(&g_vtcmMgr.mutex);
    waitNode = g_vtcmMgr.waitQ.pBusy;
    if (!mailboxId)
    {
        result = HAP_VTCM_ALLOC_ASYNC_INVALID_ARGS;
        goto err;
    }
    while (waitNode)
    {
        if ( (waitNode->clientHandle == clientHandle) &&
             (waitNode->mailboxId == mailboxId ) )
        {
            if (QURT_EOK == qurt_mailbox_delete(waitNode->mailboxId) )
            {
                g_vtcmMgr.waitQ.mailboxCount--;
            }
            /* Pop the node from busy list */
            if (waitNode->pPrev)
            {
                waitNode->pPrev->pNext = waitNode->pNext;
            }
            else
            {
                g_vtcmMgr.waitQ.pBusy = waitNode->pNext;
            }
            if (waitNode->pNext)
            {
                waitNode->pNext->pPrev = waitNode->pPrev;
            }
            /* Push it to the free list */
            vtcmMgr_waitNode_push(waitNode);
            break;
        }
        waitNode = waitNode->pNext;
    }
err:
    qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
    return result;
}

int vtcmMgr_alloc_async_cancel(int clientHandle,
                               unsigned long long mailboxId,
                               void **pVA)
{
    int result = 0;
    vtcmMgr_wait_node_t *waitNode;

    if (!mailboxId)
    {
        result = HAP_VTCM_ALLOC_ASYNC_INVALID_ARGS;
        return result;
    }

    qurt_rmutex2_lock(&g_vtcmMgr.mutex);
    waitNode = g_vtcmMgr.waitQ.pBusy;

    while (waitNode)
    {
        /* Search the node in wait queue */
        if ( (waitNode->clientHandle == clientHandle) &&
             (waitNode->mailboxId == mailboxId ) &&
             ( (waitNode->bPending == TRUE) || pVA) )
        {
            if ( (waitNode->bPending == FALSE) && (waitNode->pVA) )
            {
                /* Driver already allocated the requested chunk
                 * to the caller. Return user the allocated pointer
                 */
                *pVA = waitNode->pVA;
            }
            /* Found a wait node in pending state matching the
             * client information. Delete the node.
             */
            if (QURT_EOK == qurt_mailbox_delete(waitNode->mailboxId) )
            {
                g_vtcmMgr.waitQ.mailboxCount--;
            }
            /* Pop the node from busy list */
            if (waitNode->pPrev)
            {
                waitNode->pPrev->pNext = waitNode->pNext;
            }
            else
            {
                g_vtcmMgr.waitQ.pBusy = waitNode->pNext;
            }
            if (waitNode->pNext)
            {
                waitNode->pNext->pPrev = waitNode->pPrev;
            }
            /* Push it to the free list */
            vtcmMgr_waitNode_push(waitNode);
            break;
        }
        waitNode = waitNode->pNext;
    }

    qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
    return result;
}

/* :: vtcmMgr_free_coalesceNode ::
 *    Combines given node with adjacent nodes (if possible )in the free list
 */
static void vtcmMgr_free_coalesceNode(vtcmMgr_mem_node_t* pNode)
{
    vtcmMgr_mem_node_t* pFree;
    /* If a previous node exists, check if it can be coalesced
     * to the current one
     */
    if (pNode->pPrev &&
        (pNode->pPrev->PA + pNode->pPrev->size == pNode->PA) )
    {
        /* Mark current node as free.
         * To be pushed to the mem node list.
         */
        pFree = pNode;
        /* Move pNode to the previous node */
        pNode = pNode->pPrev;
        pNode->size += pNode->pNext->size;
        pNode->pNext = pNode->pNext->pNext;
        if (pNode->pNext)
        {
            pNode->pNext->pPrev = pNode;
        }
        /* Now the node is free and can be pushed back to the list */
        vtcmMgr_memNode_push(pFree);

    }
    if (pNode->pNext &&
        (pNode->PA + pNode->size == pNode->pNext->PA) )
    {
        /* Mark next node in the list as free.
         * To be pushed to the mem node list.
         */
        pFree = pNode->pNext;
        pNode->size += pNode->pNext->size;
        pNode->pNext = pNode->pNext->pNext;
        if (pNode->pNext)
        {
            pNode->pNext->pPrev = pNode;
        }
        /* Now the node is free and can be pushed back to the list */
        vtcmMgr_memNode_push(pFree);
    }
}

/* :: vtcmMgr_free_addNode ::
 *    Adds the provided done into the free list and combines the adjacent
 *    nodes if possible
 */
static void vtcmMgr_free_addNode(vtcmMgr_mem_node_t* pNode)
{
    vtcmMgr_mem_node_t* pFreeList = pVtcmMemFreeList;
    pNode->desc.bFree = 1; //Set free bit
    if (!pVtcmMemFreeList)
    {
        /* Free list is empty. Add the node to the head of the list */
        pNode->pPrev = NULL;
        pNode->pNext = NULL;
        pVtcmMemFreeList = pNode;
    }
    else
    {
        while (pFreeList)
        {
            if (pNode->PA < pFreeList->PA)
            {
                /* Attach given node to the current position in the free list */
                pNode->pNext = pFreeList;
                pNode->pPrev = pFreeList->pPrev;
                pFreeList->pPrev = pNode;
                if (!pNode->pPrev)
                {
                    /* We are adding a head node, modify the free list */
                    pVtcmMemFreeList = pNode;
                }
                else
                {
                    pNode->pPrev->pNext = pNode;
                }
                /* Combine adjacent nodes if possible */
                vtcmMgr_free_coalesceNode(pNode);
                break;
            }
            else if (pNode->PA == pFreeList->PA)
            {
                /* The node is already present in the free list.
                 * Try combining the node with adjacent nodes if possible.
                 */
                vtcmMgr_free_coalesceNode(pFreeList);
                if (pNode != pFreeList)
                {
                    /* Duplicate node found, push it to the mem nodes */
                    vtcmMgr_memNode_push(pNode);
                }
                break;
            }
            else if (!pFreeList->pNext)
            {
                /* Reached end of the list */
                pFreeList->pNext = pNode;
                pNode->pNext = NULL;
                pNode->pPrev = pFreeList;
                /* Combine adjacent nodes if possible */
                vtcmMgr_free_coalesceNode(pNode);
                break;
            }
            else
            {
                pFreeList = pFreeList->pNext;
            }
        }
    }
}

/* :: vtcmMgr_free ::
 *    Deletes memory region created for the given ASID at given virtual address (pVA)
 *    and adds the node back to the free list.
 */
int vtcmMgr_free(int clientHandle,
                 unsigned char asid,
                 qurt_addr_t pVA)
{
    unsigned int result = HAP_VTCM_RELEASE_FAIL_INVALID_ARGS;
    //void *userBuf = NULL;
    /* Find the node in the busy list */
    vtcmMgr_mem_node_t* pBusyList;
    if ( (0 == pVA) || (0 == g_vtcmMgr.bInitDone) )
    {
        /* If invalid pVA or if vtcm manager init is not done,
         * return error
         */
        return result;
    }
    qurt_rmutex2_lock(&g_vtcmMgr.mutex);
    pBusyList = pVtcmMemBusyList;
    while (pBusyList)
    {
        if ( (pBusyList->VA == pVA) &&
             (pBusyList->asid == asid) &&
             (pBusyList->desc.bFree == 0) )
        {
            //userBuf = pVA;
            /* Clear the buffer contents */
            //memset(userBuf, 0, pBusyList->size);
            /* Proceed to delete mapping */
            result = qurt_mapping_remove( (qurt_addr_t)pBusyList->VA,
                                          (qurt_paddr_t)pBusyList->PA,
                                          (qurt_size_t)pBusyList->size);

#if (__HEXAGON_ARCH__ < 66)
            /*
             * No need to check for result in case of v66 and beyond. 
             * We are relying on QuRT static mapping in that case.
             */
            if (0 != result) 
            {
                /* Failed to delete memory region */
                result = HAP_VTCM_RELEASE_FAIL_MEMREGION_DEL;
                qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
                return result;
            }
#endif

            pBusyList->region = 0;
            pBusyList->desc.bFree = 1; //Mark the node free
            /*
             * Remove from busy list and insert the node into the free list.
             */
            if (pBusyList->pNext)
            {
                pBusyList->pNext->pPrev = pBusyList->pPrev;
            }
            if (pBusyList->pPrev)
            {
                pBusyList->pPrev->pNext = pBusyList->pNext;
            }
            else
            {
                /* Change the head node */
                pVtcmMemBusyList = pBusyList->pNext;
            }
            /* Add the node to the free list */
            vtcmMgr_free_addNode(pBusyList);
            /* Return success */
            result = 0;
            break;
        }
        else
        {
            pBusyList = pBusyList->pNext;
        }
    }
    /* Check if we can allocate for any of the waiting threads */
    vtcmMgr_recheck_waitQ();
    qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
    /* Return result = success 0 /failure 1 */
    return result;
}

/* :: vtcmMgr_query_total ::
 *    Returns total available page size and number of such pages in VTCM
 */
int vtcmMgr_query_total(unsigned int* size, unsigned int* numPages)
{
    unsigned int temp = 0;
    int result = 0;
    if (g_vtcmMgr.bInitDone)
    {
        temp = 31 - Q6_R_cl0_R(g_vtcmMgr.size);
        temp = ( (temp >> 1) << 1);
        if (numPages)
            *numPages = g_vtcmMgr.size >> temp;
        if (size)
            *size = (1 << temp);
    }
    else
    {
        /* VTCM manager not initialized yet, return 0 for size and page count */
        result = HAP_VTCM_QUERY_FAIL_INIT_NOTDONE;
        if (size)
            *size = 0;
        if (numPages)
            *numPages = 0;
    }
    return result;
}

/* :: vtcmMgr_query_pages ::
 *    Queries max page that can be created in the given node and number of such pages possible
 */
static void vtcmMgr_query_pages(vtcmMgr_mem_node_t* pNode, unsigned int* maxBlockSize, unsigned int* numBlocks)
{
    unsigned int blockSize = 0, blocks = 0, blockSizeBitPos = 0, alignedPA = 0, alignedSize = 0;
    if (pNode && pNode->desc.bFree && pNode->size)
    {
        blockSizeBitPos = 31 - Q6_R_cl0_R(pNode->size);
        blockSizeBitPos = ( (blockSizeBitPos >> 1) << 1);
        while (!blocks)
        {
            blockSize = 1 << blockSizeBitPos;
            alignedPA = ( ( (pNode->PA) + blockSize - 1) & (~(blockSize - 1) ) ) ;
            alignedSize = pNode->size - (alignedPA - (pNode->PA) );
            blocks = alignedSize >> blockSizeBitPos;
            if ( (4 * 1024) >= blockSize)
            {
                break;
            }
            if (!blocks)
            {
                blockSizeBitPos = blockSizeBitPos - 2;
            }
        }
    }
    if (maxBlockSize)
        *maxBlockSize = blockSize;
    if (numBlocks)
        *numBlocks = blocks;
}

/* :: vtcmMgr_query_avail ::
 *    Returns max contiguous VTCM block size avaiable, maximum possible single page allocation
 *    and number of such pages
 */
int vtcmMgr_query_avail(unsigned int* freeBlockSize, unsigned int* maxPageSize, unsigned int* numPages)
{
    unsigned int maxBlock = 0, maxPage = 0, maxNumPages = 0, tempMaxPage = 0, tempMaxNumPages = 0;
    int result = 0;
    vtcmMgr_mem_node_t* pNode;
    qurt_rmutex2_lock(&g_vtcmMgr.mutex);
    pNode = pVtcmMemFreeList;
    if (g_vtcmMgr.bInitDone)
    {
        while (pNode)
        {
            maxBlock = (maxBlock < pNode->size) ? pNode->size : maxBlock;
            vtcmMgr_query_pages(pNode, &tempMaxPage, &tempMaxNumPages);
            if (tempMaxPage > maxPage)
            {
                maxPage = tempMaxPage;
                maxNumPages = tempMaxNumPages;
            }
            pNode = pNode->pNext;
        }
        if (freeBlockSize)
            *freeBlockSize = maxBlock;
        if (maxPageSize)
            *maxPageSize = maxPage;
        if (numPages)
            *numPages = maxNumPages;
    }
    else
    {
        /* VTCM manager not initialized yet, return failure */
        result = HAP_VTCM_QUERY_FAIL_INIT_NOTDONE;
        if (freeBlockSize)
            *freeBlockSize = 0;
        if (maxPageSize)
            *maxPageSize = 0;
        if (numPages)
            *numPages = 0;
    }
    qurt_rmutex2_unlock(&g_vtcmMgr.mutex);
    return result;
}

/* :: vtcmMgr_recheck_waitQ ::
 *    If free list is not empty, traverse the wait queue
 *    and check for possible allocations
 */
static void vtcmMgr_recheck_waitQ()
{
    vtcmMgr_wait_node_t *waitNode;
    void *pVA = NULL;

    /* Check if there is a waiting thread */
    if (pVtcmMemFreeList && g_vtcmMgr.waitQ.pBusy)
    {
        waitNode = g_vtcmMgr.waitQ.pBusy;
        while (waitNode->pNext != NULL)
            waitNode = waitNode->pNext;
        while (waitNode && pVtcmMemFreeList)
        {
            if (waitNode->bPending &&
                waitNode->mailboxId &&
                waitNode->size &&
                (waitNode->size <= g_vtcmMgr.size) )
            {
                pVA = vtcmMgr_alloc(waitNode->clientHandle,
                                    waitNode->asid,
                                    waitNode->size,
                                    waitNode->bSinglePage);
                if (pVA)
                {
                    qurt_mailbox_send(waitNode->mailboxId,
                                      QURT_MAILBOX_SEND_OVERWRITE,
                                      (unsigned long long) pVA);
                    waitNode->bPending = FALSE;
                    waitNode->pVA = pVA;
                }
            }
            waitNode = waitNode->pPrev;
        }
    }
}

/* :: vtcmMgr_waitNode_push ::
 *    Pushes given wait node to waitQ free LIFO
 */
static void vtcmMgr_waitNode_push(vtcmMgr_wait_node_t* pMem)
{
    if (pMem)
    {
        /* Push given object to the front of the list */
        pMem->pNext = g_vtcmMgr.waitQ.pFree;
        g_vtcmMgr.waitQ.pFree = pMem;
    }
}

/* :: vtcmMgr_waitNode_pop ::
 *    Pops out a wait node from the waitQ free LIFO
 */
static vtcmMgr_wait_node_t* vtcmMgr_waitNode_pop()
{
    vtcmMgr_wait_node_t* retVal;
    /* Pop out a mem node from the mem node list */
    retVal = g_vtcmMgr.waitQ.pFree;
    if (g_vtcmMgr.waitQ.pFree)
    {
        g_vtcmMgr.waitQ.pFree = g_vtcmMgr.waitQ.pFree->pNext;
    }
    else
    {
        g_vtcmMgr.waitQ.pFree = NULL;
    }
    /* return the node */
    return retVal;
}