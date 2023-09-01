/*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* Example asynchronous FastCV-based image processing operation queue.
   Common apps/DSP definitions. */

#ifndef FCVQUEUE_COMMON_H
#define FCVQUEUE_COMMON_H

/* Message structure - requests

       Buffer in/out (cache ops):
           0: Request
           4: Buffer FD

       Image processing requests:
           0: Request
           4: Operation/algorithm (fcvqueue_op_t)
           8: Input Buffer FD
           12: Output Buffer FD

       Sync:
           0: Request
           4: Context low 32 bits
           8: Context up 32 bits

   Responses
       Error:
           0: Response
           4: Error code

       Sync:
           0: Response
           4: Context low 32 bits
           8: Context up 32 bits
*/

enum {
    FCVQUEUE_REQ_BUF_IN = 1,
    FCVQUEUE_REQ_BUF_OUT,
    FCVQUEUE_REQ_SYNC,
    FCVQUEUE_REQ_PROCESS
};

enum {
    FCVQUEUE_RESP_ERROR = 1,
    FCVQUEUE_RESP_SYNC
};


#endif //FCVQUEUE_COMMON_H
