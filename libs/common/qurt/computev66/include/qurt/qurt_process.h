#ifndef QURT_PROCESS_H
#define QURT_PROCESS_H
/**
  @file qurt_process.h
  @brief Prototypes of QuRT process control APIs

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/
#include "qurt_callback.h"

#define QURT_PROCESS_ATTR_NAME_MAXLEN QURT_MAX_NAME_LEN

#define QURT_PROCESS_NON_SYSTEM_CRITICAL        (1u << 1)     /* Bit 1 (0x00000002) */
#define QURT_PROCESS_UNTRUSTED                  (1u << 7)     /* Bit 7 (0x00000080) */
#define QURT_PROCESS_DEFAULT_CEILING_PRIO           0         /*Higest possible priority*/
#define QURT_PROCESS_DEFAULT_MAX_THREADS            -1        /*Very high/unlikely number*/

/** @addtogroup process_types
@{ */

/** QuRT process types */
typedef enum {
    QURT_PROCESS_TYPE_RESERVED,
    QURT_PROCESS_TYPE_KERNEL,
    QURT_PROCESS_TYPE_SRM,
    QURT_PROCESS_TYPE_SECURE,
    QURT_PROCESS_TYPE_ROOT,
    QURT_PROCESS_TYPE_USER,
}qurt_process_type_t;
/** QuRT process attributes. */
typedef struct _qurt_process_attr {
    /** @cond */
    char name[QURT_PROCESS_ATTR_NAME_MAXLEN]; /* Process name. */
    int flags;
    unsigned sid;
    unsigned max_threads;
    unsigned short ceiling_prio;
    qurt_process_type_t type;

    /** @endcond */
} qurt_process_attr_t; 
/** @} */ /* end_addtogroup process_types */

typedef enum {
   QURT_PROCESS_DUMP_CB_ROOT,
   QURT_PROCESS_DUMP_CB_ERROR,
   QURT_PROCESS_DUMP_CB_PRESTM,
   QURT_PROCESS_DUMP_CB_MAX     //Used for error checking
}qurt_process_dump_cb_type_t;

/*=============================================================================
FUNCTIONS
=============================================================================*/
/**@ingroup func_qurt_process_create
  Creates a process with the specified attributes, and start the process.

  The process executes the code in the specified executable ELF file.

  @datatypes
  #qurt_process_attr_t

  @param[out] attr Accepts an initialized process attribute structure, which specifies
                   the attributes of the created process.

  @return
  None.

  @dependencies
  None.
*/
int qurt_process_create (qurt_process_attr_t *attr);

/**@ingroup func_qurt_process_get_id
  Returns the process identifier for the current thread. 

  @return
  None.

  @dependencies
  Process identifier for the current thread..
*/
int qurt_process_get_id (void);

/**@ingroup func_qurt_process_get_uid
  Returns the user identifier for the current thread. 

  @return
  None.

  @dependencies
  user identifier for the current thread..
*/
int qurt_process_get_uid (void);

/**@ingroup func_qurt_process_attr_init
  Initializes the structure that is used to set the process attributes when a thread is created.

  After an attribute structure is initialized, the individual attributes in the structure can 
  be explicitly set using the process attribute operations.

  Table @xref{tbl:processAttrDefaults} lists the default attribute values set by the initialize 
  operation.

  @inputov{table_process_attribute_defaults}

  @datatypes
  #qurt_process_attr_t

  @param[out] attr Pointer to the structure to initialize.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_process_attr_init (qurt_process_attr_t *attr)
{
    attr->name[0] = 0;
    attr->flags = 0;
	 attr->sid = 0;
    attr->max_threads = QURT_PROCESS_DEFAULT_MAX_THREADS;
    attr->ceiling_prio = QURT_PROCESS_DEFAULT_CEILING_PRIO;
    attr->type = QURT_PROCESS_TYPE_RESERVED;
}

/**@ingroup func_qurt_process_attr_set_executable
  Sets the process name in the specified process attribute structure.

  Process names are used to identify process objects that are already 
  loaded in memory as part of the QuRT system.

  @note1hang Process objects are incorporated into the QuRT system at build time.

  @datatypes
  #qurt_process_attr_t

  @param[in] attr Pointer to the process attribute structure.
  @param[in] name Pointer to the process name.
 
  @return
  None.

  @dependencies
  None.
*/
void qurt_process_attr_set_executable (qurt_process_attr_t *attr, char *name);

/**@ingroup func_qurt_process_attr_set_flags
Sets the process properties in the specified process attribute structure.
Process properties are represented as defined symbols that map into bits 
0 through 31 of the 32-bit flag value. Multiple properties are specified by OR'ing 
together the individual property symbols.

@datatypes
#qurt_process_attr_t

@param[in] attr  Pointer to the process attribute structure.
@param[in] flags QURT_PROCESS_SUSPEND_ON_STARTUP suspends the process after creating it.

@return

@dependencies
None.
*/
static inline void qurt_process_attr_set_flags (qurt_process_attr_t *attr, int flags)
{
    attr->flags = flags;
}

/**@ingroup func_qurt_process_attr_set_sid
Sets the process streamID in the specified process attribute structure.


@datatypes
#qurt_process_attr_t

@param[in] attr  Pointer to the process attribute structure.
@param[in] sid   streamID that needs to be set for this process.

@return

@dependencies
None.
*/
static inline void qurt_process_attr_set_sid (qurt_process_attr_t *attr, unsigned sid)
{
    attr->sid = sid;
}


/**@ingroup qurt_process_attr_set_max_threads
Sets the maximum number of threads allowed in the specified process attribute structure.


@datatypes
#qurt_process_attr_t

@param[in] attr          Pointer to the process attribute structure.
@param[in] max_threads   maximum number of threads allowed for this process

@return

@dependencies
None.
*/
static inline void qurt_process_attr_set_max_threads (qurt_process_attr_t *attr, unsigned max_threads)
{
    attr->max_threads = max_threads;
}

/**@ingroup qurt_process_attr_set_ceiling_prio
Sets the highest thread priority allowed in the specified process attribute structure.


@datatypes
#qurt_process_attr_t

@param[in] attr          Pointer to the process attribute structure.
@param[in] max_threads   maximum number of threads allowed for this process

@return

@dependencies
None.
*/
static inline void qurt_process_attr_set_ceiling_prio (qurt_process_attr_t *attr, unsigned short prio)
{
    attr->ceiling_prio = prio;
}


/**@ingroup func_qurt_process_cmdline_get
Gets the command line string associated with the current process.
The Hexagon simulator command line arguments are retrieved using 
this function as long as the call is made
in the process of the QuRT installation, and with the 
requirement that the program is running in a simulation environment.

If the function modifies the provided buffer, it zero-terminates
the string. It is possible that the function will not modify the
provided buffer, so the caller must set buf[0] to a NULL
byte before making the call. If the command line is longer than the provided
buffer, a truncated command line is returned.

@param[in] buf     Pointer to a character buffer that must be filled in.
@param[in] buf_siz  Size (in bytes) of the buffer pointed to by buf.

@return
None.

@dependencies
None.
*/
void qurt_process_cmdline_get(char *buf, unsigned buf_siz);

/**@ingroup func_qurt_process_get_thread_count
Gets number of threads present in the process indicated by PID
@datatypes
#unsigned int

@param[in] unsigned int pid of the process for which the information is required

@return
Number of threads in the process indicated by PID

@dependencies

*/
int qurt_process_get_thread_count(unsigned int pid);

/**@ingroup func_qurt_process_get_thread_ids
Gets all the thread ID's for a process indicated by PID
@datatypes
#unsigned int

@param[in] unsigned int pid of the process for which the information is required
@param[in] ptr      Pointer to a user passed buffer that must be filled in with thread id's
@return
None

@dependencies
 */
int qurt_process_get_thread_ids(unsigned int pid, unsigned int *ptr, unsigned thread_num);

/**@ingroup func_qurt_process_dump_get_mem_mappings_count
Gets number of mappings present in the process indicated by PID
@datatypes
#unsigned int

@param[in] unsigned int pid of the process for which the information is required

@return
Number of mappings for the process indicated by PID

@dependencies

*/
int qurt_process_dump_get_mem_mappings_count(unsigned int pid);

/**@ingroup func_qurt_process_dump_get_mappings
Gets mappings for the given pid. device-type mappings are skipped.
@datatypes
#unsigned int

@param[in] unsigned int pid of the process for which the information is required
@param[in] ptr      Pointer to a buffer that must be filled in with mappings
@param[in] count    count of mappings requested

@return
Number of mappings filled in the buffer passed by the user

@dependencies
*/
int qurt_process_dump_get_mappings(unsigned int pid, unsigned int *ptr, unsigned count);

/**@ingroup func_qurt_process_attr_get
Get the attributes of the process with which it was created
@datatypes
#unsigned int

@param[in] unsigned int pid of the process for which the information is required
@param[in/out] pointer to the user allocated attribute structure

@return
None

@dependencies
None
*/
int qurt_process_attr_get(unsigned int pid, qurt_process_attr_t *attr);

/**@ingroup func_qurt_process_pd_dump_cb_register
Register PD dump callback
@datatypes
#unsigned int

@param[in] pointer to the callback information
@param[in] callback type
   QURT_PROCESS_DUMP_CB_PRESTM   --> These callbacks get called in userPD's context before threads of the exiting process are frozen.
   QURT_PROCESS_DUMP_CB_ERROR    --> These callbacks get called in userPD's context after threads are frozen and captured. 
   QURT_PROCESS_DUMP_CB_ROOT     --> These callbacks get called in rootPD's context after threads are frozen and captured and after CB_ERROR type of callbacks
									 are called.
@return
None
@dependencies
None
*/
int qurt_process_dump_register_cb(qurt_cb_data_t *cb_data, qurt_process_dump_cb_type_t type, unsigned short priority);

/**@ingroup func_qurt_process_pd_dump_cb_deregister
De-register PD dump callback
@datatypes
#unsigned int

@param[in] callback information to deregister
@param[in] callback  type

@return
None
@dependencies
None
*/
int qurt_process_dump_deregister_cb(qurt_cb_data_t *cb_data,qurt_process_dump_cb_type_t type);

/**@ingroup func_qurt_process_set_rtld_debug
set rtld_debug for a process
@datatypes
#unsigned int

@param[in] pid  of the process for which rtld_debug needs to be set
@param[in] rtld_debug address

@return
None
@dependencies
None
*/
int qurt_process_set_rtld_debug(unsigned int pid,unsigned int address);

/**@ingroup func_qurt_process_get_rtld_debug
gets rtld_debug for a process
@datatypes
#unsigned int

@param[in] pid  of the process for which rtld_debug needs to be set
@param[in/out] user passed address in which the rtld_debug address needs to be returned

@return
None
@dependencies
None
*/
int qurt_process_get_rtld_debug(unsigned int pid,unsigned int *address);
#endif
