/***************************************************************************
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ****************************************************************************/

#ifndef _HVX_APP_EVENT_QUEUE_H_
#define _HVX_APP_EVENT_QUEUE_H_

#include "qurt.h"
#include "adsp_hvx.h"

#define HVX_EVT_QUEUE_ERR 100
#define HVX_EVT_QUEUE_SUCCESS 0
#define QUEUE_MASK   0xf
#define QUEUE_MOD(a) ((a) & QUEUE_MASK)
#define MAX_QUEUE_NUM  (QUEUE_MASK + 1)
typedef struct hvx_evt_q_t *hvx_evt_queue_t;
typedef struct hvx_evt_queue_elem_t{
  void *data;
}hvx_evt_queue_elem_t;

typedef enum{
  QUEUE_STATE_IDLE = 0,
  QUEUE_STATE_TIMEWAIT,   // queue state during dequeue but queue is empty,
                          // and waiting for entries
  QUEUE_STATE_ABORT,      // queue is in the state of being aborted
  QUEUE_STATE_ABORTED     // queue is aborted, will not respond to dequeue/enqueue,
                            //   need reset
} hvx_evt_queue_state_t;

/* The structure defining the queue. */
typedef struct hvx_evt_q_t
{
  hvx_evt_queue_elem_t  queue_pool[MAX_QUEUE_NUM];
  uint32_t        queue_head;     // The head
  uint32_t        queue_tail;     // The tail
  int32_t         queue_cnt;      // The number of entry in the queue
  uint8_t         abort_flag;     // The flag to abort queue

  qurt_mutex_t      mutex;          // The mutex for locking use
  qurt_cond_t       get_cond;       // The condition variable signaling the
                                  // end of receipt on an entry into queue
  qurt_cond_t       abort_cond;     // The condition variable signaling the
                                  // readiness of whether any abort can
                                  // be unblocked
  hvx_evt_queue_state_t state;
}hvx_evt_q_t;

int hvx_event_queue_init(hvx_evt_queue_t *q);
int hvx_event_queue_enqueue(hvx_evt_queue_t q,
                            void **enqueue_entry,
                            uint32_t enqueue_entry_cnt);
int hvx_event_queue_dequeue(hvx_evt_queue_t q,
                            void **dequeue_entry);
void hvx_event_queue_destroy(hvx_evt_queue_t *q);
int hvx_event_queue_reset(hvx_evt_queue_t q);
int hvx_event_queue_abort(hvx_evt_queue_t *q);

#endif

