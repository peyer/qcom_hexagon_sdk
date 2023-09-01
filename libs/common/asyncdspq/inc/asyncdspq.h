/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Low-level shared memory userspace queue */

#ifndef ASYNCDSPQ_H
#define ASYNCDSPQ_H

#include <stdint.h>
#include <AEEStdErr.h>
#include <AEEStdDef.h>

typedef void* asyncdspq_t; /** Queue handle */
typedef uint32_t asyncdspq_attach_handle_t; /** Queue handle used for asyncdspq_attach() */

typedef enum {
    ASYNCDSPQ_ENDPOINT_APP_CPU = 1, /** Main application CPU */
    ASYNCDSPQ_ENDPOINT_CDSP /** Compute DSP, or the ADSP used for compute workloads */
} asyncdspq_endpoint_t; /** Queue endpoint */



/**
 * Data/space available callback. The callback signals there is a new
 * message available to read or more space available in the queue
 * depending on where it is used.
 *
 * Note that a read/write operation can
 * still fail or block after the callback is received - there may not
 * be enough space for the given message or another thread may have
 * consumed the message already. Clients must be prepared to handle a
 * AEE_NOMORE response or be prepared to block. There may also be more
 * than one message available or space for multiple messages in the
 * queue. Clients are expected to drain all the messages from the queue
 * after each callback, and should not wait for a new callback until
 * the queue is completely empty or full.
 *
 * @param queue The queue this callback refers to
 * @param contex Callback context from asyncdspq_create()/asyncdspq_attach().
 */
typedef void (*asyncdspq_callback_t)(asyncdspq_t, void*);


/**
 * Error callback. Called from different thread contexts.
 *
 * @param queue The queue this callback refers to
 * @param contex Callback context from asyncdspq_create()
 * @param err Error code
 */
typedef void (*asyncdspq_error_callback_t)(asyncdspq_t, void*, AEEResult);



/**
 * Creates a new queue, allocating the queue and creating threads as necessary.
 * Queues can currently be created on the application CPU only. Valid writer/reader
 * endpoint combinations are CPU/CDSP and CDSP/CPU.
 *
 * @param queue Output: Queue handle
 * @param attach_handle Output: Queue handle used with asyncdspq_attach() at the other endpoint
 * @param writer Queue writer endpoint
 * @param reader Queue reader endpoint
 * @param error_callback Error callback. Called from different thread contexts.
 * @param message_callback Message data available callback or NULL.
 *                         Called when there is new data available in the message queue.
 *                         Called from a message handler thread context.
 * @param space_callback Queue space available callback or NULL.
 *                       Called when there is more space available in the queue to write data.
 *                       Called from a message handler thread context.
 * @param callback_context Context pointer passed to callbacks
 * @param flags Queue creation flags; 0 for default configuration
 * @param queue_length Queue length in bytes. Must be a multiple of 4 and a minimum of 256 bytes.
 *
 * @return Error code, AEE_SUCCESS (0) on success
 */
#ifdef __cplusplus
extern "C" {
#endif
AEEResult asyncdspq_create(asyncdspq_t *queue, asyncdspq_attach_handle_t *attach_handle,
                          asyncdspq_endpoint_t writer, asyncdspq_endpoint_t reader,
                          unsigned queue_length, asyncdspq_error_callback_t error_callback,
                          asyncdspq_callback_t message_callback, asyncdspq_callback_t space_callback,
                          void *callback_context,
                          uint32_t flags);
/**
 * Destroys a queue. Deallocates queue memory and terminates threads. Must be called
 * from the same processor where the queue was created. The other endpoint must detach
 * from the queue before it is destroyed.
 *
 * There must be no outstanding blocking read or write requests in progress when this
 * function is called. Clients can use asyncdspq_cancel() to cancel requests before
 * destroying the queue.
 *
 * @param queue Queue handle from asyncdspq_create()
 *
 * @return Error code, AEE_SUCCESS(0) on success
 */
AEEResult asyncdspq_destroy(asyncdspq_t queue);


/**
 * Attach to a queue created at the other endpoint.
 *
 * @param queue Output: Queue handle
 * @param attach_handle Queue handle from asyncdspq_create() at the other endpoint
 * @param error_callback Error callback. Called from different thread contexts.
 * @param message_callback Message data available callback or NULL.
 *                         Called when there is new data available in the message queue.
 *                         Called from a message handler thread context.
 * @param space_callback Queue space available callback or NULL.
 *                       Called when there is more space available in the queue to write data.
 *                       Called from a message handler thread context.
 * @param callback_context Context pointer passed to callbacks
 *
 * @return Error code, AEE_SUCCESS (0) on success
 */
AEEResult asyncdspq_attach(asyncdspq_t *queue, asyncdspq_attach_handle_t attach_handle,
                          asyncdspq_error_callback_t error_callback,
                          asyncdspq_callback_t message_callback, asyncdspq_callback_t space_callback,
                          void *callback_context);


/**
 * Detach from a queue; the reverse of asyncdspq_attach(). Must be called before the
 * other endpoint destroys the queue.
 *
 * There must be no outstanding blocking read or write requests in progress when this
 * function is called. Clients can use asyncdspq_cancel() to cancel requests before
 * detaching from the queue.
 *
 * @param queue Queue handle from asyncdspq_attach().
 *
 * @return Error code, AEE_SUCCESS (0) on success
 */
AEEResult asyncdspq_detach(asyncdspq_t queue);


/**
 * Writes a message to the queue. Blocks if there is no space in the queue.
 * This function can only be called at the writer endpoint of the queue.
 * This function must not be called from a asyncdspq callback, and cannot be used
 * if a space available callback has been set.
 *
 * @param queue Queue handle from asyncdspq_create() or dpqueue_attach()
 * @param msg Pointer to the message. Must be 32-bit aligned.
 *            This function will copy the data to the queue and the buffer can be rewritten
 *            once this function returns.
 * @param length Message length in bytes. Must be a multiple of 4.
 *
 * @return Error code, AEE_SUCCESS(0) on success.
 *         AEE_EEXPIRED if the request was cancelled.
 *         AEE_EBUFFERTOOSMALL if the queue is too small to fit the message (even if empty).
 *         AEE_EUNSUPPORTED if the queue has a space available callback set.
 */
AEEResult asyncdspq_write(asyncdspq_t queue, const uint8_t *msg, unsigned length);


/**
 * Writes a message to the queue. Non-blocking.
 * This function can only be called at the writer endpoint of the queue.
 *
 * @param queue Queue handle from asyncdspq_create() or dpqueue_attach()
 * @param msg Pointer to the message. Must be 32-bit aligned.
 *            This function will copy the data to the queue and the buffer can be rewritten
 *            once this function returns.
 * @param length Message length in bytes. Must be a multiple of 4.
 *
 * @return Error code, AEE_SUCCESS(0) on success.
 *         AEE_ENOMORE if there isn't enough space in the queue.
 *         AEE_EBUFFERTOOSMALL if the queue is too small to fit the message (even if empty).
 */
AEEResult asyncdspq_write_noblock(asyncdspq_t queue, const uint8_t *msg, unsigned length);


/**
 * Reads a message from the queue. Blocks if there is no message available.
 * This function can only be called from the reader endpoint of the queue.
 * This function must not be called from a asyncdspq callback, and cannot be used
 * if a message callback has been set.
 *
 * @param queue Queue handle from asyncdspq_create() or dpqueue_attach()
 * @param buf Pointer to a message buffer. Must be 32-bit aligned.
 * @param buf_length Buffer length in bytes. Must be a multiple of 4.
 * @param msg_length Output: Message length in bytes
 *
 * @return Error code, AEE_SUCCESS(0) on success.
 *         AEE_ENOMEMORY if the buffer is too small for the next message.
 *         AEE_EEXPIRED if the request was cancelled.
 *         AEE_EUNSUPPORTED if the queue has a message callback set.
 */
AEEResult asyncdspq_read(asyncdspq_t queue, uint8_t *buf, unsigned buf_length, unsigned *msg_length);


/**
 * Reads a message from the queue. Non-blocking.
 * This function can only be called from the reader endpoint of the queue.
 *
 * @param queue Queue handle from asyncdspq_create() or dpqueue_attach()
 * @param buf Pointer to a message buffer. Must be 32-bit aligned.
 * @param buf_length Buffer length in bytes. Must be a multiple of 4 and a minimum of 256.
 * @param msg_length Output: Message length in bytes
 *
 * @return Error code, AEE_SUCCESS(0) on success.
 *         AEE_ENOMORE if there is no message currently in the queue.
 *         AEE_ENOMEMORY if the buffer is too small for the next message.
 */
AEEResult asyncdspq_read_noblock(asyncdspq_t queue, uint8_t *buf, unsigned buf_length, unsigned *msg_length);


/**
 * Starts canceling any outstanding synchronous read or write requests at this queue endpoint.
 * This function can be used to cancel requests before destroying the queue or detaching
 * from it. This function will only cancel requests from the current endpoint -
 * calling it on the application CPU will only cancel application CPU requests.
 *
 * Note that this function will not wait for all requests to be canceled, but
 * will rather return immediately after the cancel request has been sent. The client must
 * wait for requests to complete, typically by joining any read/write threads related
 * to the queue.
 *
 * Also note that detaching the queue on the DSP side will effectively cancel any
 * outstanding read/write requests on the application CPU.
 *
 * @param queue Queue handle from asyncdspq_create() or asyncdspq_attach().
 *
 * @return Error code, AEE_SUCCESS(0) on success
 */
AEEResult asyncdspq_cancel(asyncdspq_t queue);

#ifdef __cplusplus
}
#endif

#endif /* ASYNCDSPQ_H */
