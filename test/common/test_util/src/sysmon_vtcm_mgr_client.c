/*=============================================================================
 * FILE:                      sysmon_vtcm_mgr_client.c
 *
 * DESCRIPTION:
 *    Vector TCM management client
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
               *********** Includes ***********
   ========================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include "qurt.h"
#include "HAP_farf.h"
#include "sysmon_vtcmmgr_int.h"

/* ============================================================================
                            Variables and Defines
   ========================================================================= */
/* Stack size, should be multiple of 8 bytes */
#define SYSMON_VTCM_CLIENT_THREAD_STACK_SIZE            4096
#define SYSMON_VTCM_CLIENT_TIMEOUT_SIG                  2
#define SYSMON_VTCM_CLIENT_EXIT_SIG                     1
#define HAP_VTCM_ASYNC_REQUEST_MIN_TIMEOUT              100


typedef struct {
    unsigned long long mailboxId;
    qurt_anysignal_t sig;
    unsigned long long timeStamp;
    unsigned int timeout_us;
    unsigned int returnVal;
} sysmon_vtcm_th_arg_t;

/* ============================================================================
                                 Declarations
   ========================================================================= */
void* vtcmMgr_alloc(int clientHandle,
                    unsigned short asid,
                    unsigned int size,
                    unsigned char bSinglePage);
int vtcmMgr_free(int clientHandle,
                 unsigned char asid,
                 void* pVA);
int vtcmMgr_query_total(unsigned int* size,
                        unsigned int* numPages);
int vtcmMgr_query_avail(unsigned int* freeBlockSize,
                        unsigned int* maxPageSize,
                        unsigned int* numPages);
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
int HAP_query_total_VTCM(unsigned int* page_size, unsigned int* page_count);
int HAP_query_avail_VTCM(unsigned int* avail_block_size,
                          unsigned int* max_page_size,
                          unsigned int* num_pages);
/* ============================================================================
                                 Globals
   ========================================================================= */

static int vtcmMgrQdiHandle = -1;
static unsigned int spinLock = 0;

/* ============================================================================
                                CODE STARTS HERE
   ========================================================================= */

__attribute__ ((noinline))
int vtcmMgr_qdi_init(void)
{
	unsigned int *spinLockPtr = &spinLock;
	__asm__  __volatile__ (
	"check: r0 = memw_locked(%0)  \n"
	"    p0 = cmp.eq(r0, #0)           \n"
	"    if (!p0) jump check      \n"
	"    memw_locked(%0, p0) = %0      \n"
	"    if !p0 jump check        \n"
	: "+r" (spinLockPtr) : : "p0","r0");

    if (0 > vtcmMgrQdiHandle)
    {
		//FARF(HIGH, "Calling VTCM MGR init");
        sysmon_vtcmMgr_init();
        vtcmMgrQdiHandle = 1;
    }

    spinLock = 0;
    return 0;
}

static void vtcm_timeout_entry(void *arg)
{
    sysmon_vtcm_th_arg_t *inp_data = (sysmon_vtcm_th_arg_t *)arg;
    qurt_timer_t timer;
    qurt_timer_attr_t attr;
    unsigned long long currTimeStamp;
    unsigned int timeout_us;
    unsigned int sigMask;

    qurt_timer_attr_init(&attr);
    currTimeStamp = qurt_sysclock_get_hw_ticks();
    if (currTimeStamp > inp_data->timeStamp)
    {
        timeout_us = ( ( (unsigned int)(currTimeStamp - inp_data->timeStamp) )
                       * 1000 ) / 19200;
    }
    else
    {
        timeout_us = (unsigned long)(UINT64_MAX
                            - inp_data->timeStamp + currTimeStamp);
        timeout_us = (timeout_us * 1000) / 19200;
    }
    if (inp_data->timeout_us > timeout_us)
    {
        timeout_us = inp_data->timeout_us - timeout_us;
        qurt_timer_attr_set_duration(&attr, timeout_us);

        if (QURT_EOK == qurt_timer_create(&timer, &attr, &inp_data->sig,
                    SYSMON_VTCM_CLIENT_TIMEOUT_SIG) )
        {
            sigMask = qurt_anysignal_wait(&inp_data->sig,
                                           (SYSMON_VTCM_CLIENT_TIMEOUT_SIG |
                                                   SYSMON_VTCM_CLIENT_EXIT_SIG) );
            if (sigMask & SYSMON_VTCM_CLIENT_TIMEOUT_SIG)
            {
                FARF(HIGH, "HAP_request_async_VTCM: timeout triggered");
                vtcmMgr_alloc_async_cancel(0,
                                           inp_data->mailboxId,
                                           0);
                inp_data->returnVal = SYSMON_VTCM_CLIENT_TIMEOUT_SIG;
            }
            else
            {
                inp_data->returnVal = SYSMON_VTCM_CLIENT_EXIT_SIG;
            }
            qurt_timer_delete(timer);
        }
        else
        {
            FARF(ERROR, "HAP_request_async_VTCM: failed to create a timer "
                    "instance");
            /* Unable to create a timeout instance, inject a timeout
             * immediately */
            vtcmMgr_alloc_async_cancel(0,
                                       inp_data->mailboxId,
                                       0);
            inp_data->returnVal = SYSMON_VTCM_CLIENT_TIMEOUT_SIG;
        }
    }
    else
    {
        /* Given timeout elapsed, set a signal already */
        vtcmMgr_alloc_async_cancel(0,
                                   inp_data->mailboxId,
                                   0);
        inp_data->returnVal = SYSMON_VTCM_CLIENT_TIMEOUT_SIG;
    }
    return;
}

void* HAP_request_async_VTCM(unsigned int size,
                             unsigned int single_page_flag,
                             unsigned int timeout_us)
{
    void* pVA = 0;
    int result = 0;
    unsigned long long mailboxId = 0;
    unsigned long long data = 0;
    unsigned long long timeStamp;
    void *stack = NULL;
    sysmon_vtcm_th_arg_t *inp_data = NULL;
    qurt_thread_attr_t attr;
    qurt_thread_t tid;

    if (0 == vtcmMgr_qdi_init())
    {
        if (timeout_us < HAP_VTCM_ASYNC_REQUEST_MIN_TIMEOUT)
        {
            pVA = vtcmMgr_alloc(0, 1, size, single_page_flag);
        }
        else
        {
            timeStamp = qurt_sysclock_get_hw_ticks();
            pVA = vtcmMgr_alloc_async(0, 1, size, single_page_flag, &mailboxId);
            if (0 == pVA)
            {
                FARF(HIGH, "Async request failed, return mailboxId: %llu",
                     mailboxId);

                if (mailboxId)
                {
                    stack = malloc(SYSMON_VTCM_CLIENT_THREAD_STACK_SIZE
                            + sizeof(sysmon_vtcm_th_arg_t) );

                    if (!stack)
                    {
                        FARF(ERROR, "HAP_request_async_VTCM: Failed to "
                                "allocate from heap for timeout");
                        vtcmMgr_alloc_async_cancel(0,
                                                   mailboxId,
                                                   &pVA);
                        goto bail;
                    }

                    /* Update thread input data structure */
                    inp_data = (sysmon_vtcm_th_arg_t *)( (unsigned int)stack
                            + SYSMON_VTCM_CLIENT_THREAD_STACK_SIZE);
                    inp_data->mailboxId = mailboxId;
                    inp_data->timeout_us = timeout_us;
                    inp_data->timeStamp = timeStamp;
                    qurt_anysignal_init(&inp_data->sig);

                    /* Start a thread */
                    qurt_thread_attr_init(&attr);
                    qurt_thread_attr_set_name(&attr, "VTCM_TIMEOUT");
                    qurt_thread_attr_set_priority(&attr,
                                 qurt_thread_get_priority(qurt_thread_get_id() ) );
                    qurt_thread_attr_set_stack_addr(&attr, stack);
                    qurt_thread_attr_set_stack_size(&attr,
                                             SYSMON_VTCM_CLIENT_THREAD_STACK_SIZE);
                    if (QURT_EOK != qurt_thread_create(&tid,
                                                       &attr,
                                                       vtcm_timeout_entry,
                                                       (void *)inp_data))
                    {
                        FARF(ERROR, "HAP_request_async_VTCM: failed to create "
                                "timer thread");
                        vtcmMgr_alloc_async_cancel(0,
                                                   mailboxId,
                                                   &pVA);
                        goto bail;
                    }
                    /* We have set up the timeout now, wait for ack from VTCM
                     * driver
                     */
                    if (QURT_EOK == (result =
                                qurt_mailbox_receive(mailboxId,
                                        QURT_MAILBOX_RECV_WAITING, &data) ) )
                    {
                        pVA = (void *) (data & 0xFFFFFFFF);
                        vtcmMgr_alloc_async_done(0,
                                                 mailboxId);
                        qurt_anysignal_set(&inp_data->sig,
                                                   SYSMON_VTCM_CLIENT_EXIT_SIG);
                        FARF(HIGH, "Received allocation success from VTCM manager,"
                             "return data: 0x%x", pVA);
                    }
                    else
                    {
                        FARF(HIGH, "wait on mailbox returned failure %d, returning "
                                "error to user", result);
                    }
                    /* Wait for the timeout thread to join */
                    qurt_thread_join(tid, NULL);
                }
                else
                {
                    FARF(ERROR, "HAP_request_async_VTCM: SRM driver didn't return "
                            "a mailboxId");
                }
            }
        }
    }
    else
    {
        FARF(ERROR, "HAP_request_async_VTCM: driver open failed");
    }
bail:
    if (stack)
        free(stack);
    return pVA;
}

void* HAP_request_VTCM(unsigned int size, unsigned int single_page_flag)
{
    void* pVA = 0;
    //FARF(ALWAYS, "VTCM Request for size: %d, single page flag: %d", size, single_page_flag);
    if (0 == vtcmMgr_qdi_init())
    {
        //unsigned int page_size, page_count, avail_block_size;
        //int result = 0;
        //result = HAP_query_total_VTCM(&page_size, &page_count);
        //FARF(ALWAYS, "HAP Query total VTCM page size: %d, page_count: %d, result = %d", page_size, page_count, result);
        //result = HAP_query_avail_VTCM(&avail_block_size, &page_size, &page_count);
        //FARF(ALWAYS, "HAP query avail VTCM available block size: %d, max page size: %d, max page size count: %d, result = %d", avail_block_size, page_size, page_count, result);
        pVA = vtcmMgr_alloc(0, 1, size, (unsigned char) single_page_flag);
        //result = HAP_query_avail_VTCM(&avail_block_size, &page_size, &page_count);
        //FARF(ALWAYS, "HAP query avail VTCM available block size: %d, max page size: %d, max page size count: %d, result = %d", avail_block_size, page_size, page_count, result);
    }
    //FARF(ALWAYS, "VTCM Request done, pVA: 0x%x", (unsigned int)pVA);
    return pVA;
}

int HAP_release_VTCM(void* pVA)
{
    int result = 0;
    //unsigned int page_size, page_count, avail_block_size;
    //int local_result = 0;
    //FARF(ALWAYS, "VTCM Release request, pVA: 0x%x", (unsigned int)pVA);
    if (0 != vtcmMgr_qdi_init())
    {
        result = HAP_VTCM_RELEASE_FAIL_QDI;
    }
    else
    {
        result = vtcmMgr_free(0, 1, pVA);
        //local_result = HAP_query_avail_VTCM(&avail_block_size, &page_size, &page_count);
        //FARF(ALWAYS, "HAP query avail VTCM available block size: %d, max page size: %d, max page size count: %d, result = %d", avail_block_size, page_size, page_count, local_result);
    }
    //FARF(ALWAYS, "VTCM Release done, result: %d", result);
    return result;
}

int HAP_query_total_VTCM(unsigned int* page_size, unsigned int* page_count)
{
    int result = 0;
    if (0 != vtcmMgr_qdi_init())
    {
        FARF(ALWAYS, "QDI open failed");
        if (page_size)
        {
            *page_size = 0;
        }
        if (page_count)
        {
            *page_count = 0;
        }
        result = HAP_VTCM_QUERY_FAIL_QDI;
    }
    else
    {
        result = vtcmMgr_query_total(page_size, page_count);
        if (0 != result)
        {
            FARF(ALWAYS, "QDI method VTCMMGR_QUERY_TOTAL failed %d", result);
            if (page_size)
            {
                *page_size = 0;
            }
            if (page_count)
            {
                *page_count = 0;
            }
            result = HAP_VTCM_QUERY_FAIL_QDI_INVOKE;
        }
    }
    return result;
}

int HAP_query_avail_VTCM(unsigned int* avail_block_size,
                          unsigned int* max_page_size,
                          unsigned int* num_pages)
{
    int result = 0;
    if (0 != vtcmMgr_qdi_init())
    {
        if (avail_block_size)
        {
            *avail_block_size = 0;
        }
        if (max_page_size)
        {
            *max_page_size = 0;
        }
        if (num_pages)
        {
            *num_pages = 0;
        }
        result = HAP_VTCM_QUERY_FAIL_QDI;
    }
    else
    {
        result = vtcmMgr_query_avail(avail_block_size,
                                     max_page_size,
                                     num_pages);
        if (0 != result)
        {
            if (avail_block_size)
            {
                *avail_block_size = 0;
            }
            if (max_page_size)
            {
                *max_page_size = 0;
            }
            if (num_pages)
            {
                *num_pages = 0;
            }
            result = HAP_VTCM_QUERY_FAIL_QDI_INVOKE;
        }
    }
    return result;
}
