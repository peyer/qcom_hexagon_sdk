#ifndef QURT_HVX_H
#define QURT_HVX_H
/**
  @file qurt_hvx.h 
  @brief   Prototypes of Qurt HVX API.  

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2014  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/


/*=============================================================================
                        TYPEDEFS
=============================================================================*/

typedef enum {
    QURT_HVX_MODE_64B = 0,      /**< HVX mode of 64 bytes */
    QURT_HVX_MODE_128B = 1      /**< HVX mode of 128 bytes */
} qurt_hvx_mode_t;

/*=============================================================================
                        CONSTANTS AND MACROS
=============================================================================*/

#define QURT_HVX_HW_UNITS_2X128B_4X64B        0x00000204       /* bit#15-#8 are for number of 128B units   */
                                                               /* bit#7-#0 are for number of 64B units     */
#define QURT_HVX_HW_UNITS_4X128B_0X64B        0x00000400   

/* HVX locking status */

#define QURT_HVX_UNLOCKED                     (0)              /* Has not locked HVX unit */
#define QURT_HVX_LOCKED                       (1)              /* Has locked HVX unit */
#define QURT_HVX_ERROR                        (-1)             /* Error, no HVX support */

/* Input value for HVX reservation */

#define QURT_HVX_RESERVE_ALL                  (4)              /* All the HVX units in terms of 64B_MODE are requested to be reserved */
#define QURT_HVX_RESERVE_ALL_AVAILABLE        (0xff)           /* All remaining unlocked HVX units in terms of 64B_MODE are requested to be reserved */

/* Return values for HVX reservation */

#define QURT_HVX_RESERVE_NOT_SUPPORTED        (-1)             /* There is no HVX hardware, or less units in the hardware than requested */
#define QURT_HVX_RESERVE_NOT_SUCCESSFUL       (-2)             /* Some HVX units are already locked/reserved by other PD, thus not enough units left for the reservation. */
#define QURT_HVX_RESERVE_ALREADY_MADE         (-3)             /* There is already a HVX reservation made. */
#define QURT_HVX_RESERVE_CANCEL_ERR           (-4)             /* The action of cancling the reservation fails because this protection domain has no reservation made before. */

// HVX set requests

#define QURT_HVX_64B                    0  
#define QURT_HVX_128B                   1  
#define QURT_HVX_NO_USE                 2  
#define QURT_HVX_RELEASE_CONTEXT        3  
#define QURT_HVX_IMMEDIATE_USE          4  

// HVX set masks

#define QURT_HVX_64B_PREFERRED          (1<<(QURT_HVX_64B  + 8))
#define QURT_HVX_128B_PREFERRED         (1<<(QURT_HVX_128B + 8))
#define QURT_HVX_64B_ACCEPTABLE         (1<<(QURT_HVX_64B  + 12))
#define QURT_HVX_128B_ACCEPTABLE        (1<<(QURT_HVX_128B + 12))

// HVX set return "result"

#define QURT_EOK                        0  
#define QURT_HVX_SET_ERROR              0xFF  

// hvx_mode_assigned for QURT_HVX_IMMEDIATE_USE 
#define QURT_HVX_64B_ASSIGNED          (1<<(QURT_HVX_64B  + 8))
#define QURT_HVX_128B_ASSIGNED         (1<<(QURT_HVX_128B + 8))

// Sizes of HVX dump buffer

#define   QURT_HVX_V65_64B_VSIZE           2084      //  64 x 32 +  8 x 4 + 4 (version)
#define   QURT_HVX_V65_128B_VSIZE          4164      // 128 x 32 + 16 x 4 + 4 (version)
#define   QURT_HVX_V66_128B_VSIZE          4420      // 128 x (32 +2) + 16 x 4 + 4 (version)
#define   QURT_HVX_VREG_BUF_SIZE           QURT_HVX_V66_128B_VSIZE

// HVX dump versions

#define QURT_HVX_DUMP_V65_64B           1  
#define QURT_HVX_DUMP_V65_128B          2  
#define QURT_HVX_DUMP_V66_128B          3  


// Qurt data struct for hvx_set input
typedef struct qurt_hvx_set_struct_ {          
    unsigned char set_req;  // LSB
    struct {
        unsigned char preferred_mask:4;
        unsigned char acceptable_mask:4;
    };
    unsigned short resvd;   // MSB
} qurt_hvx_set_struct_t;  // 4 bytes


// Qurt data struct for hvx_set return
typedef struct qurt_hvx_set_return_str_ {          
    unsigned char result;  // LSB
    unsigned char hvx_mode_assigned;
    unsigned short resvd;   // MSB
} qurt_hvx_set_return_struct_t;  // 4 bytes



/*=============================================================================
                        FUNCTIONS
=============================================================================*/

/**@ingroup func_qurt_hvx_lock
  Lock one HVX unit specified by the HVX mode.
  
  @note1hang Input variable can be 128B_MODE or 64B_MODE etc. If a HVX unit in this mode 
             is available, this function will lock the unit and returns right away;
             if the current HVX mode is different from the requested mode, the current 
             thread will be blocked. When all HVX units become idle, Qurt will change 
             the mode, lock the HVX unit, and return.  
  
  @datatypes
  #qurt_mode_t
  
  @param[in]  lock_mode, either QURT_HVX_MODE_64B or QURT_HVX_MODE_128B

  @param[out] 
  None.

  @return
  QURT_EOK - successful return
  Others - failure

  @dependencies
  None.
  
 */
int qurt_hvx_lock(qurt_hvx_mode_t lock_mode);

/**@ingroup func_qurt_hvx_unlock
  Unlock the HVX unit held by this software thread.
  
  @note1hang  Unlock the HVX unit hold by this software thread.  
  
  @datatypes
  None.
  
  @param[out] 
  None.

  @return
  QURT_EOK - successful return
  Others - failure

  @dependencies
  None.
  
 */
int qurt_hvx_unlock(void);

/**@ingroup func_qurt_hvx_try_lock
  Try to lock one HVX unit specified by the HVX mode.
  
  @note1hang Input variable can be 128B_MODE or 64B_MODE etc. If a HVX unit in this mode 
             is available, this function will lock the unit and returns QURT_EOK; Otherwise,
             the function will return a failure, but will not block the current software 
             thread to wait for the HVX unit.
  
  @datatypes
  #qurt_mode_t
  
  @param[out] 
  None.

  @return
  QURT_EOK - successful return
  Others - failure

  @dependencies
  None.
  
 */
int qurt_hvx_try_lock(qurt_hvx_mode_t lock_mode);

/**@ingroup func_qurt_hvx_get_mode
  Get the current HVX mode configured by Qurt.
  
  @note1hang The function returns QURT_HVX_MODE_128B or QURT_HVX_MODE_64B, based on 
             the current HVX configuration
  
  @datatypes
  None.
  
  @param[out] 
  None.

  @return
  QURT_HVX_MODE_128B 
  QURT_HVX_MODE_64B
  -1:  not available

  @dependencies
  None.
  
 */
int qurt_hvx_get_mode(void);


/**@ingroup func_qurt_hvx_get_units
  Get the HVX hardware configuration supported by the chipset.
  
  @note1hang The function returns the HVX hardware configuration supported by the chipset.
  
  @datatypes
  None.
  
  @param[out] 
  None.

  @return
     The bit-mask of the units, 1X64, 2X64, 4X64, 1X128, 2X128, etc.
      - QURT_HVX_HW_UNITS_2X126B_4X64B : for V60, V62, V65 HVX
      - QURT_HVX_HW_UNITS_4X128B_0X64B : for V66 CDSP or newer
      - 0:  not available

  @dependencies
  None.
  
 */
int qurt_hvx_get_units(void);


/**@ingroup func_qurt_hvx_reserve
  Reserve HVX units in terms of 64-byte mode for the caller's protection domain (PD).
  
  @note1hang Only one HVX reservation in the system is supported, as of now.
             If one HVX unit is already locked by the application in the same PD, the unit will be 
             added to the returned count as one reserved unit for the PD.
  
  @datatypes
  None.

  @param[in]  num_units  Number of HVX units in terms of 64B_MODE to be reserved for the PD
                         QURT_HVX_RESERVE_ALL to reserve all the HVX units
                         QURT_HVX_RESERVE_ALL_AVAILABLE to reserve all remaining unlocked units

  @param[out] 
  None.

  @return
    - number of units successfully reserved, including the units already locked in the same PD 
    - QURT_HVX_RESERVE_NOT_SUPPORTED      
    - QURT_HVX_RESERVE_NOT_SUCCESSFUL     
    - QURT_HVX_RESERVE_ALREADY_MADE    


  @dependencies
  None.
  
 */
int qurt_hvx_reserve(int num_units);


/**@ingroup func_qurt_hvx_cancel_reserve
  Cancel the HVX reservation in the caller's protection domain (PD).
  
  @note1hang Only one HVX reservation in the system is supported, as of now.
  
  @datatypes
  None.

  @param[in]
  None.

  @param[out] 
  None.

  @return
    - 0, sucess
    - QURT_HVX_RESERVE_CANCEL_ERR, failure      

  @dependencies
  None.
  
 */
int qurt_hvx_cancel_reserve(void);


/**@ingroup func_qurt_hvx_get_lock_val
  Get the HVX locking status value of the caller's thread. 
  
  @note1hang Returns status of whether the caller's thread already locks a HVX unit or not.
  
  @datatypes
  None.

  @param[in]
  None.

  @param[out] 
  None.

  @return
    - QURT_HVX_UNLOCKED   
    - QURT_HVX_LOCKED    
    - QURT_HVX_ERROR    

  @dependencies
  None.
  
 */
int qurt_hvx_get_lock_val(void);


/**@ingroup func_qurt_hvx_set
  Set the HVX configuration for caller's software thread. 
  
  @datatypes
  None.

  @param[in] input_arg is composed by,
      set_request | hvx_preferred_mode_mask | hvx_acceptable_mode_mask

   where "set_request" can be set to,
      QURT_HVX_64B           
      QURT_HVX_128B         
      QURT_HVX_NO_USE      
      QURT_HVX_RELEASE_CONTEXT 
      QURT_HVX_IMMEDIATE_USE  

    In case "set_request" is QURT_HVX_IMMEDIATE_USE,  
    "hvx_preferred_mode_mask" can be set to,
      QURT_HVX_64B_PREFERRED     
      QURT_HVX_128B_PREFERRED   
      
    In case "set_request" is QURT_HVX_IMMEDIATE_USE,  
    "hvx_acceptable_mode_mask" can be set to,
      QURT_HVX_64B_ACCEPTABLE  
      QURT_HVX_128B_ACCEPTABLE

  @param[out] 
  None.

  @return 
     returned "result" of the hvx setting is in the LSB 8 bits of the returned data,
       QURT_EOK               0  
       QURT_HVX_SET_ERROR     0xFF  
     
     In the case of QURT_HVX_IMMEDIATE_USE with "result" of QURT_EOK, 
     The bit#8 to bit#15 of the returned data contains "hvx_mode_assigned"
       QURT_HVX_64B_ASSIGNED      
       QURT_HVX_128B_ASSIGNED   

  @dependencies
  None.
  
 */
unsigned int qurt_hvx_set(unsigned int input_arg);


/**@ingroup func_qurt_system_hvx_regs_get_maxsize
  This function returns the maximum buffer size for saving HVX registers
  
  @datatypes
  None.

  @param[in]
  None.

  @param[out] 
  None.

  @return
    0                         - No HVX supported in the target
    QURT_HVX_VREG_BUF_SIZE    - Maximum buffer size for saving HVX registers

  @dependencies
  None.
  
 */
unsigned int qurt_system_hvx_regs_get_maxsize(void);


/**@ingroup func_qurt_system_hvx_regs_get_size
  This function returns the buffer size for saving HVX registers for a specified thread
  
  @datatypes
  None.

  @param[in]
    thread_id    Thread ID of the target thread

  @param[out] 
  None.

  @return
    0                   - No HVX assgined to the thread
    size                - Size of the buffer in bytes for saving HVX registers for the specified thread.
                          It is one of the followings, 
                             QURT_HVX_V65_64B_VSIZE        //  64 x 32 +  8 x 4 + 4 (version)
                             QURT_HVX_V65_128B_VSIZE       // 128 x 32 + 16 x 4 + 4 (version)
                             QURT_HVX_V66_128B_VSIZE       // 128 x (32 +2) + 16 x 4 + 4 (version) 

  @dependencies
  None.
  
 */
unsigned int qurt_system_hvx_regs_get_size(unsigned int thread_id);



/**@ingroup func_qurt_system_hvx_regs_get
  This function saves the HVX registers into the given buffer.
  and returns the size of the data saved into the buffer.
  After calling this function for the first time on a specified thread_id, the Qurt kernel removes the internal HVX saving buffer 
  from the specified thread. Thus if calling the function on the same thread_id for the second time, this function returns 0.
  
  @param[in]
    thread_id    Thread ID of the target thread.
    pBuf         Pointer to the buffer for HVX register saving.
                 The first 4 bytes of the buffer are for saving the HVX version, and from the 5th byte of the buffer,
                 HVX regsters are saved. The address of the 5th byte should be 256 bytes aligned. 
                 For example, a buffer can be declared at first as,
                    unsigned char vbuf[QURT_HVX_VREG_BUF_SIZE+256];
                    unsigned char *pBuf;
                 and then align the buffer pointer to,
                    pBuf = vbuf;
                    pBuf += (256 - 4 - (unsigned)pBuf%256);
    size         Size of the buffer provided, which is pointed by *pBuf. The buffer size should not be smaller than that 
                 returned from qurt_system_hvx_regs_get_size(), and the pBuf should be aligned in the way described on the above.


  @param[out] 
    pBuf         The buffer returned with HVX version and HVX registers saved, in the following format,
                    unsigned int   hvx_version;    The first 4 bytes contains one of the HVX dump version,
                                                   -QURT_HVX_DUMP_V65_64B    
                                                   -QURT_HVX_DUMP_V65_128B    
                                                   -QURT_HVX_DUMP_V66_128B    
                    unsigned char  hvx_regs[];     From the 5th byte of the buffer, HVX registers are saved.

  @return
    The total bytes of the data saved in the provided buffer.
         - 0                             // No HVX assigned to the thread
         - QURT_HVX_V65_64B_VSIZE        //  64 x 32 +  8 x 4 + 4 (version)
         - QURT_HVX_V65_128B_VSIZE       // 128 x 32 + 16 x 4 + 4 (version)
         - QURT_HVX_V66_128B_VSIZE       // 128 x (32 +2) + 16 x 4 + 4 (version) 

  @dependencies
  None.
  
 */
unsigned int qurt_system_hvx_regs_get(unsigned int thread_id, void *pBuf, size_t size);


#endif /* QURT_HVX_H */

