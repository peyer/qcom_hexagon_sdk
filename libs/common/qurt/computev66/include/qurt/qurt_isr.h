#ifndef QURT_ISR_H
#define QURT_ISR_H

/*=====================================================================
 
  @file  qurt_isr.h

  @brief  Prototypes of Qurt ISR API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2017  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include <string.h>

/*=====================================================================
 Constants and macros
======================================================================*/

#define QURT_ISR_ATTR_FRAME_NON_SHAREABLE       0
#define QURT_ISR_ATTR_FRAME_SHAREABLE           1
#define QURT_ISR_ATTR_FRAME_SHAREABLE_DEFAULT   QURT_ISR_ATTR_FRAME_SHAREABLE
#define QURT_ISR_ATTR_TCB_PARTITION_RAM         0
#define QURT_ISR_ATTR_TCB_PARTITION_TCM         1 
#define QURT_ISR_ATTR_TCB_PARTITION_DEFAULT     QURT_ISR_ATTR_TCB_PARTITION_RAM
#define QURT_ISR_ATTR_NON_DELAYED_ACK           0
#define QURT_ISR_ATTR_DELAYED_ACK               1
#define QURT_ISR_ATTR_ACK_DEFAULT               QURT_ISR_ATTR_NON_DELAYED_ACK
#define QURT_ISR_ATTR_STID_DEFAULT              0
#define QURT_ISR_ATTR_DRV_DEFAULT               0
#define QURT_ISR_ATTR_TIMETEST_ID_DEFAULT       (-2) 
#define QURT_ISR_ATTR_PRIORITY_DEFAULT_MAGIC    0x1ff
#define QURT_ISR_ATTR_BUS_PRIO_DISABLED         0
#define QURT_ISR_ATTR_BUS_PRIO_ENABLED          1 
#define QURT_ISR_ATTR_BUS_PRIO_DEFAULT          255
#define QURT_ISR_ATTR_DEFAULT_STACK_SIZE        2048
#define QURT_ISR_ATTR_NAME_MAXLEN               16

// Qurt ISR interrupt type

/* Trigger type bit fields for a PDC interrupt:
 *  Polarity  Edge  Output
 *  0         00  Level Sensitive Active Low
 *  0         01  Rising Edge Sensitive
 *  0         10  Falling Edge Sensitive
 *  0         11  Dual Edge Sensitive
 *  1         00  Level Sensitive Active High
 *  1         01  Falling Edge Sensitive
 *  1         10  Rising Edge Sensitive
 *  1         11  Dual Edge Sensitive */

#define QURT_ISR_PDC_TRIGGER_TYPE_SET(pol, edge)   (((pol & 0x01) << 2) | (edge & 0x03))

#define QURT_ISR_ATTR_INT_TRIGGER_LEVEL_LOW     QURT_ISR_PDC_TRIGGER_TYPE_SET(0, 0x00) 
#define QURT_ISR_ATTR_INT_TRIGGER_LEVEL_HIGH    QURT_ISR_PDC_TRIGGER_TYPE_SET(1, 0x00) 
#define QURT_ISR_ATTR_INT_TRIGGER_RISING_EDGE   QURT_ISR_PDC_TRIGGER_TYPE_SET(1, 0x02)  
#define QURT_ISR_ATTR_INT_TRIGGER_FALLING_EDGE  QURT_ISR_PDC_TRIGGER_TYPE_SET(0, 0x02)  
#define QURT_ISR_ATTR_INT_TRIGGER_DUAL_EDGE     QURT_ISR_PDC_TRIGGER_TYPE_SET(0, 0x03)  
#define QURT_ISR_ATTR_INT_TRIGGER_USE_DEFAULT   0xff 


// Qurt ISR attribute
#define QURT_ISR_ATTR_NULL                      0      
#define QURT_ISR_NOBLKING_MAGIC                 1      // for legacy fastint



/*=============================================================================
                                                TYPEDEFS
=============================================================================*/

/** ISR frame attributes */
typedef struct _qurt_isr_frame_attr {
    
    unsigned int   stack_size;         /**< ISR frame stack size. */
    void          *stack_addr;         /**< Pointer to the stack address base, 
                                         the stack range is (stack_addr, stack_addr+stack_size-1) */
    unsigned char  tcb_partition;      /**< ISR TCB in RAM or TCM. */
} qurt_isr_frame_attr_t;


/** ISR callback attributes */
typedef struct _qurt_isr_cb_attr {
    char           name[QURT_ISR_ATTR_NAME_MAXLEN];   /**< ISR frame/group name. */
    unsigned short priority;                          /**< ISR priority. */
    unsigned char  ack_type;                          /**< ISR ack type. */
    unsigned char  interrupt_type;                    /**< ISR interrupt type. */
    unsigned int   stack_size;                        /**< ISR stack size, only useful when frame name=NULL */
} qurt_isr_cb_attr_t;


/*=====================================================================
 Functions
======================================================================*/

/**@ingroup func_qurt_isr_frame_attr_init
  Initializes the structure used to set the ISR frame thread.
  After an attribute structure is initialized, the individual attributes in the structure can be
  explicitly set using the ISR attribute APIs.

  The default attribute values set by the initialize operation are the following: \n
  - stack_size    -- 0   meaning no stack is provided by caller
  - stack_addr    -- 0
  - TCB partition -- #QURT_ISR_ATTR_TCB_PARTITION_DEFAULT

  @datatypes
  #qurt_isr_frame_attr_t
  
  @param[in,out] attr Pointer to the ISR frame attribute structure.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_isr_frame_attr_init (qurt_isr_frame_attr_t *attr)
{
    attr->stack_size     = 0; // meaning no stack is provided by caller
    attr->stack_addr     = 0;
    attr->tcb_partition  = QURT_ISR_ATTR_TCB_PARTITION_DEFAULT;
}


/**@ingroup func_qurt_isr_cb_attr_init
  Initializes the structure used to set the ISR callback attributes when a Qurt ISR callback 
  is registered.  After an attribute structure is initialized, the individual attributes in 
  the structure can be explicitly set using the ISR attribute APIs.

  The default attribute values set by the initialize operation are the following: \n
  - priority       -- #QURT_ISR_ATTR_PRIORITY_DEFAULT_MAGIC \n
  - frameshareable -- #QURT_ISR_ATTR_FRAME_SHARABLE_DEFAULT
  - stack_size     -- 0  meaning no stack is provided by caller
  - ack_type       -- QURT_ISR_ATTR_ACK_DEFAULT
  - drv_number     -- QURT_ISR_ATTR_DRV_DEFAULT 
  - interrupt_type -- QURT_ISR_ATTR_INT_TRIGGER_USE_DEFAULT
  - name           -- 0  meaning no frame name assigned

  @datatypes
  #qurt_isr_attr_t
  
  @param[in,out] attr Pointer to the ISR attribute structure.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_isr_cb_attr_init (qurt_isr_cb_attr_t *attr)
{
    attr->priority        = QURT_ISR_ATTR_PRIORITY_DEFAULT_MAGIC;
    attr->ack_type        = QURT_ISR_ATTR_ACK_DEFAULT;
    attr->name[0]         = 0;    // meaning no frame name assigned
    attr->interrupt_type  = QURT_ISR_ATTR_INT_TRIGGER_USE_DEFAULT;
    attr->stack_size      = 0;    // meaning no stack is provided by caller
}


/**@ingroup func_qurt_isr_frame_attr_set_stack_addr
  Sets the stack address attribute. \n
  Specifies the stack address of the stack memory area to be used in the ISR frame thread.

  stack_addr must contain an address value that is 8-byte aligned.

  @datatypes
  #qurt_isr_frame_attr_t
  
  @param[in,out] attr    - Pointer to the isr attribute structure.
  @param[in] stack_addr  - Pointer to the 8-byte aligned address of the stack memory.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_isr_frame_attr_set_stack_addr (qurt_isr_frame_attr_t *attr, void *stack_addr)
{
    attr->stack_addr = stack_addr;
}


/**@ingroup func_qurt_isr_frame_attr_set_stack_size
  Sets stack size attribute for ISR frame thread.\n
  Specifies the size of the stack memory area to be used for ISR callstack.

  @datatypes
  #qurt_isr_frame_attr_t

  @param[in,out] attr Pointer to the isr attribute structure.
  @param[in] stack_size Size (in bytes) of the stack.

  @return
  None.

  @dependencies
  None.
*/

static inline void qurt_isr_frame_attr_set_stack_size (qurt_isr_frame_attr_t *attr, unsigned int stack_size)
{
    attr->stack_size = stack_size;
}


/**@ingroup func_qurt_isr_frame_attr_set_tcb_partition
  Sets the TCB partition attribute for the ISR frame.
  Specifies the memory type where a thread control block (TCB) of a ISR frame thread is allocated.
  TCBs can be allocated in RAM or TCM/LPM.

  @datatypes
  #qurt_isr_frame_attr_t

  @param[in,out] attr  Pointer to the ISR attribute structure.
  @param[in] tcb_partition TCB partition. Values:\n
                     0 - TCB resides in RAM \n
                     1 - TCB resides in TCM/LCM

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_isr_frame_attr_set_tcb_partition (qurt_isr_frame_attr_t *attr, unsigned char tcb_partition)
{
    attr->tcb_partition = tcb_partition;
}


/**@ingroup func_qurt_isr_cb_attr_set_priority
  Sets the priority to ISR callback.
  The priorities are specified as numeric values in the range 1 to 255, with 1 representing
  the highest priority.

  @datatypes
  #qurt_isr_cb_attr_t

  @param[in,out] attr  - Pointer to the ISR attribute structure.
  @param[in] priority  - Priority of the ISR callback.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_isr_cb_attr_set_priority (qurt_isr_cb_attr_t *attr, unsigned short priority)
{
    attr->priority = priority;
}


/**@ingroup func_qurt_isr_cb_attr_set_stack_size
  Sets stack size attribute for ISR callback\n
  Specifies the size of the stack memory area to be used for ISR callstack.

  @datatypes
  #qurt_isr_cb_attr_t

  @param[in,out] attr - Pointer to the isr attribute structure.
  @param[in] stack_size - Size (in bytes) of the stack.

  @return
  None.

  @dependencies
  None.
*/

static inline void qurt_isr_cb_attr_set_stack_size (qurt_isr_cb_attr_t *attr, unsigned int stack_size)
{
    attr->stack_size = stack_size;
}


/**@ingroup func_qurt_isr_cb_attr_set_group
   Sets the name of the ISR frame that this ISR callback wants to join and use the ISR frame for processing the callback. 
   The ISR frame needs to be created beforehand. 

   @datatypes
   #qurt_isr_cb_attr_t

   @param[in] isr attribute structure.

   @param[in] frame_name. Name of the ISR frame that the callback want to use/join.

   @return
   None

   @dependencies
   None.
*/
static inline void qurt_isr_cb_attr_set_group (qurt_isr_cb_attr_t *attr, char *frame_name)
{
    strlcpy (attr->name, frame_name, QURT_ISR_ATTR_NAME_MAXLEN);
    attr->name[QURT_ISR_ATTR_NAME_MAXLEN - 1] = 0;
}


/**@ingroup func_qurt_isr_cb_attr_set_interrupt_type
   Sets interrupt type on the interrupt used for the ISR callback. 
   If there is no special setting on the interrupt type, by default, Qurt ISR 
   uses the interrupt type information defined in Qurt XML file.

   @datatypes
   #qurt_isr_cb_attr_t

   @param[in] isr attribute structure.

   @param[in] interrupt_type.  Values: \n 
         - QURT_ISR_ATTR_INT_TRIGGER_USE_DEFAULT
         - QURT_ISR_ATTR_INT_TRIGGER_LEVEL_HIGH 
         - QURT_ISR_ATTR_INT_TRIGGER_LEVEL_LOW 
         - QURT_ISR_ATTR_INT_TRIGGER_RISING_EDGE 
         - QURT_ISR_ATTR_INT_TRIGGER_FALLING_EDGE

   @return
   None

   @dependencies
   None.
*/
static inline void qurt_isr_cb_attr_set_interrupt_type (qurt_isr_cb_attr_t *attr, unsigned char interrupt_type)
{
    attr->interrupt_type = interrupt_type;
}


/**@ingroup func_qurt_isr_cb_attr_set_ack
   Sets ISR attribute for interrupt acknowledgement type.
   Setting to delayed interrupt acknowledgement disables Qurt kernel from re-enabling the interrupt
   after the ISR callback returns. The client needs to explicitly re-enable the interrupt afterwards.

   @datatypes
   #qurt_isr_cb_attr_t

   @param[in] isr attribute structure.

   @param[in] ack_type.  Values: \n 
         - #QURT_ISR_ATTR_NON_DELAYED_ACK  
         - #QURT_ISR_ATTR_DELAYED_ACK        

   @return
   None

   @dependencies
   None.
*/
static inline void qurt_isr_cb_attr_set_ack (qurt_isr_cb_attr_t *attr, unsigned char ack_type)
{
    if(ack_type <= QURT_ISR_ATTR_DELAYED_ACK) {
        attr->ack_type = ack_type;
    }
}


/**@ingroup func_qurt_isr_set_hw_config_callback
  Set callback function for the configuration related to interrupt hardware.
  In a process, the callback function can only be set once.

  @param[in]   cb_addr      address of the callback function.
   
  @return 
  #QURT_EOK     -- the callback function is set succssfully. \n
  #QURT_EFAILED -- Failure. The callback function has been set before. 

  @dependencies
  None.
 */
int qurt_isr_set_hw_config_callback(unsigned int cb_addr);


/**@ingroup func_qurt_isr_set_hw_enable_callback
  Set callback function for enabling the configuration related to interrupt hardware.
  In a process, the callback function can only be set once.

  @param[in]   cb_addr      address of the callback function.
   
  @return 
  #QURT_EOK     -- the callback function is set succssfully. \n
  #QURT_EFAILED -- Failure. The callback function has been set before. 

  @dependencies
  None.
 */
int qurt_isr_set_hw_enable_callback(unsigned int cb_addr);


/**@ingroup func_qurt_isr_set_hw_disable_callback
  Set callback function for disabling the configuration related to interrupt hardware.
  In a process, the callback function can only be set once.

  @param[in]   cb_addr      address of the callback function.
   
  @return 
  #QURT_EOK     -- the callback function is set succssfully. \n
  #QURT_EFAILED -- Failure. The callback function has been set before. 

  @dependencies
  None.
 */
int qurt_isr_set_hw_disable_callback(unsigned int cb_addr);


/**@ingroup func_qurt_isr_frame_create
  Creates a Qurt ISR frame with the specified frame attributes and the frame name.

  @datatypes
   qurt_isr_frame_attr_t
  
  @param[in]   attr           Pointer to the initialized ISR frame attribute structure
  @param[in]   frame_name     name of the ISR frame to be created
   
  @return 
   QURT_EOK     -- Qurt ISR frame created successfully. \n
   QURT_EFAILED -- Failure. 

  @dependencies
   None.
 */

int qurt_isr_frame_create(qurt_isr_frame_attr_t *pAttr, char *frame_name);


/**@ingroup func_qurt_isr_frame_delete
  Deletes a Qurt ISR frame with its frame name 
  The ISR frame thread is deleted when function returns success.

  @param[in]   frame_name     name of the ISR frame to be deleted
   
  @return 
  #QURT_EOK     -- Qurt ISR frame deleted successfully. \n
  #QURT_EFAILED -- Failure. 

  @dependencies
  None.
 */

int qurt_isr_frame_delete(char *frame_name);


/**@ingroup func_qurt_isr_callback_register
  Registers a Qurt ISR callback with the specified attributes.
  The interrupt is enabled when this function returns success.

  @datatypes
   qurt_isr_cb_attr_t
  
  @param[in]   attr         Pointer to the initialized ISR callback attribute structure. 
                            The pointer can be set as zero (NULL) when the client wants to rely on Qurt to use 
                            the default ISR setting to create a non-shareable ISR frame for the ISR callback.
  @param[in]   int_num      ISR interrupt number.
  @param[in]   callback     ISR callback function. 
                            There are 2 arguments passed in to the callback function when it is called. The first 
                            one is the *arg, and the second one is the int_num. 
  @param[in]   arg          Pointer to a ISR-specific argument structure.
   
  @return 
   QURT_EOK     -- Qurt ISR callback registered successfully. \n
   QURT_EFAILED -- Failure. 

  @dependencies
   None.
 */

int qurt_isr_callback_register(qurt_isr_cb_attr_t *pAttr, int int_num, void (*callback) (void *, int), void *arg);


/**@ingroup func_qurt_isr_callback_deregister
  De-registers the ISR callback registered for the specified interrupt.
  The interrupt is disabled when this function returns success.

  @param[in]   int_num      interrupt number for the ISR callback.
   
  @return 
  #QURT_EOK     -- ISR callback deregistered successfully. \n
  #QURT_EFAILED -- Failure. 

  @dependencies
  None.
 */

int qurt_isr_callback_deregister(int int_num);


#endif /* QURT_ISR_H */


