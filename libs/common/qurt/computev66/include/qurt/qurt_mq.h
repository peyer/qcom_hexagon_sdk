#ifndef QURT_MQ_H
#define QURT_MQ_H
/**
  @file  qurt_mq.h

  @brief  Prototypes of secure message queues API functions

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2019  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
======================================================================*/
#include <qurt_types.h>
#include <qurt_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QURT_MQ_NAME_MAXLEN            16  /**< Maximum name length. */

/* this enum needs to be generated in accordance to process class class numbers
   for now it is made to match generated version, do not change this unless 
   there is a corresponding change in the process_class.py, indecies start from 0
   basically: QURT_MQ_SECURITY_SCOPE_<x> = (1 << QURTK_process_class_index_<x>)
*/
typedef enum {
    QURT_MQ_SECURITY_SCOPE_KERNEL =   ( 1 << 0 ),
    QURT_MQ_SECURITY_SCOPE_SRM =      ( 1 << 1 ),
    QURT_MQ_SECURITY_SCOPE_SECURE =   ( 1 << 2 ),
    QURT_MQ_SECURITY_SCOPE_CPZ =      ( 1 << 3 ),
    QURT_MQ_SECURITY_SCOPE_ROOT =     ( 1 << 4 ),
    QURT_MQ_SECURITY_SCOPE_SIGNED =   ( 1 << 5 ),
    QURT_MQ_SECURITY_SCOPE_UNSIGNED = ( 1 << 6 )
} qurt_mq_security_scope_t;

typedef enum {
    QURT_MQ_CARDINALITY_PTP =   (1 << 0),
    QURT_MQ_CARDINALITY_MTO =   (1 << 1)
}qurt_mq_cardinality_t;

typedef unsigned int qurt_mqd_t;

typedef union{
    struct {
        unsigned int perms:2;
        unsigned int cardinality:1;
        unsigned int blocking:1;

        qurt_mq_security_scope_t creator_scope: 8;
        qurt_mq_security_scope_t allowed_scope: 8; //can be a bitmask in case of MTO
        unsigned int queue_closed: 1;
        unsigned int reserved: 11;
    }; //try to do anonymous struct
    unsigned int raw;
} qurt_mq_flags_t;


/* permissions are from qurt_types.h , block X though */
#if 0
/** Memory access permission. */
typedef enum {
        QURT_PERM_READ=0x1, /**< */
        QURT_PERM_WRITE=0x2,  /**< */
        QURT_PERM_EXECUTE=0x4,  /**< */
        QURT_PERM_FULL=QURT_PERM_READ|QURT_PERM_WRITE|QURT_PERM_EXECUTE,  /**< */
} qurt_perm_t;
#endif

struct qurt_mq_attr {
   /* @brief Configured flags. Only meaningful with get_attr() */
   unsigned flags;  

   /* @brief Maximum number of messages. Used with create() and get_attr */
   unsigned mq_maxmsg;

   /* @brief Maximum size (bytes) of message in receiver facing queue.
             i.e from sender -> receiver */
   unsigned short mq_send_msgsize;

   /* @brief Maximum size (bytes) of message in sender facing queue.
             i.e from receiver -> sender */
   unsigned short mq_recv_msgsize;

   /* @brief Process ID of sender that is allowed to send messages on this queue
             Only meaningful with create() */
   unsigned sender_pid;

    /* @brief cardinality of message queue connection, see below */
    qurt_mq_cardinality_t    cardinality; 

    /* @brief security scope of the senders to the queue */ 
    qurt_mq_security_scope_t scope;
};

void qurt_mq_attr_init(struct qurt_mq_attr * attr);
void qurt_mq_attr_set_send_msgsize (struct qurt_mq_attr *attr, size_t len);
void qurt_mq_attr_set_recv_msgsize (struct qurt_mq_attr *attr, size_t len);
void qurt_mq_attr_set_maxmsg (struct qurt_mq_attr *attr, unsigned int depth);
void qurt_mq_attr_set_scope (struct qurt_mq_attr *attr, qurt_mq_security_scope_t scope);
void qurt_mq_attr_set_flags (struct qurt_mq_attr *attr, unsigned int flags);


int qurt_mq_open (qurt_mqd_t *mqd, const char *name, qurt_mq_flags_t flags);
int qurt_mq_create(qurt_mqd_t *mqd, const char *name, struct qurt_mq_attr * attr);
int qurt_mq_send(qurt_mqd_t mqd, const char *msg_ptr, size_t msg_len); 
int qurt_mq_recv(qurt_mqd_t mqd, unsigned char *msg_ptr, size_t *msg_len);
int qurt_mq_close(qurt_mqd_t mqd);
int qurt_mq_destroy(qurt_mqd_t mqd);
int qurt_mq_release_with_pid(unsigned int pid);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
#endif //QURT_MQ_H
