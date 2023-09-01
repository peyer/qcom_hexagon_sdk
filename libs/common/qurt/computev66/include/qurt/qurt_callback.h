#ifndef QURT_CALLBACK_H
#define QURT_CALLBACK_H

/**
  @file qurt_callback.h
  @brief  Definitions, macros, and prototypes used when using qdi callback
  mechanism

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2019  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include "qurt_qdi.h"
#include "qurt_qdi_constants.h"
#include "qurt_qdi_imacros.h"
#include "qurt_mutex.h"

/** Callback framework error codes*/
typedef enum{
  QURT_CB_ERROR=-1,               /* CB registration failed*/
  QURT_CB_OK=0,                   /* Sucess*/
  QURT_CB_MALLOC_FAILED=-2,       /* QurtOS malloc failure*/
  QURT_CB_WAIT_CANCEL=-3,         /* Process exit cancelled wait operation*/
  QURT_CB_CONFIG_NOT_FOUND=-4,    /* CB Configuration for process was not found*/
  QURT_CB_QUEUE_FULL=-5,          /* CB queue is serving at max capacity*/
}qurt_cb_result_t;

typedef struct {
  void* cb_func;             /* pointer to callback function */
  unsigned cb_arg;           /* not interpreted by the framework*/
} qurt_cb_data_t;

/*these defines are used as default, if cust_config does not specify them*/
#define CALLBACK_WORKER_STACK_SIZE 0x2000

/*Callback driver's private methods*/
#define QDI_CALLBACK_CBDINFO_GET          (QDI_PRIVATE + 1)
#define QDI_CALLBACK_CBDATA_GET           (QDI_PRIVATE + 2)
#define QDI_CALLBACK_WORKER_REGISTER      (QDI_PRIVATE + 3)

/**
  This function initializes callback data structure.
  It should be called by the entity that registers callback with root process driver.

  @param  cb_data         Pointer to callback data structure
 */
static inline void qurt_cb_data_init (qurt_cb_data_t* cb_data){
    cb_data->cb_func = NULL;
    cb_data->cb_arg = 0;
}

/**
  This function sets up the callback function.
  It should be called by the entity that registers callback with root process driver.
  This will set up the callback function that will be executed when the callback is executed.

  @param  cb_data         Pointer to callback data structure.
  @param  cb_func         Pointer to callback function.
 */
static inline void qurt_cb_data_set_cbfunc (qurt_cb_data_t* cb_data, void* cb_func){
  cb_data->cb_func = cb_func;
}

/**
  This function sets up the callback argument.
  It should be called by the entity that registers callback with root process driver.
  This will set up the argument passed to the callback function when the callback is executed.

  @param  cb_data         Pointer to callback data structure.
  @param  cb_arg          Argument for the callback function.
 */
static inline void qurt_cb_data_set_cbarg (qurt_cb_data_t* cb_data, unsigned cb_arg){
  cb_data->cb_arg = cb_arg;
}

#endif