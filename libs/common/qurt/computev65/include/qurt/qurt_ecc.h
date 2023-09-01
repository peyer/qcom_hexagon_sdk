#ifndef QURT_ECC_H
#define QURT_ECC_H


/*=====================================================================
 
  @file  qurt_ecc.h

  @brief  Prototypes of Qurt Memory ECC API functions      

 Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ======================================================================*/


/*=============================================================================
                        TYPEDEFS
=============================================================================*/

// ECC memory definition
typedef enum {
    QURT_ECC_MEM_L1_ICACHE = 0,
    QURT_ECC_MEM_L1_DCACHE = 1, 
    QURT_ECC_MEM_L2_CACHE  = 2, 
    QURT_ECC_MEM_VTCM      = 3 
} qurt_ecc_memory_t;


/*=============================================================================
                        CONSTANTS AND MACROS
=============================================================================*/

// ECC status type
#define   QURT_ECC_ERR_DETECTED_STATUS        0
#define   QURT_ECC_ERR_TYPE                   1
         
// ECC error count type
#define  QURT_ECC_CORRECTABLE_COUNT           (1<<0)
#define  QURT_ECC_UNCORRECTABLE_COUNT         (1<<1)
#define  QURT_ECC_REGION_LOGGING              (1<<2)

// ECC enable/disable definition
#define QURT_ECC_PROTECTION_DISABLE  (0<<0)   // bit#0
#define QURT_ECC_PROTECTION_ENABLE   (1<<0)   // bit#0

/*=============================================================================
                        FUNCTIONS
=============================================================================*/


/**@ingroup func_qurt_ecc_enable
  Enable or disable ECC protection on a specified memory
  
  @datatypes
  #qurt_ecc_memory_t
  
  @param[in]  memory, set to one of
    - QURT_ECC_MEM_L1_ICACHE
    - QURT_ECC_MEM_L1_DCACHE
    - QURT_ECC_MEM_L2_CACHE
    - QURT_ECC_MEM_VTCM   

  @param[in]  enable, set to one of
    - QURT_ECC_PROTECTION_ENABLE
    - QURT_ECC_PROTECTION_DISABLE

  @param[out] 
  None.

  @return
   QURT_EOK    ECC enabling or disabling setup is performed successfully
   Others      Failure

  @dependencies
  None.
 */
int qurt_ecc_enable( qurt_ecc_memory_t memory, unsigned int enable );


/**@ingroup func_qurt_ecc_get_error_status
  Get ECC error status for a specified memory
  
  @datatypes
  #qurt_ecc_memory_t
  
  @param[in]  memory, set to one of
    - QURT_ECC_MEM_L1_ICACHE
    - QURT_ECC_MEM_L1_DCACHE
    - QURT_ECC_MEM_L2_CACHE
    - QURT_ECC_MEM_VTCM   

  @param[in]  type, set to one of
    - QURT_ECC_ERR_DETECTED_STATUS
    - QURT_ECC_ERR_TYPE

  @param[out] 
  None.

  @return
   When type is QURT_ECC_ERR_DETECTED_STATUS, returns
        0: no error detected
        1: at least one error detected
   When type is QURT_ECC_ERR_TYPE, returns
        0-1: correctable error
        2:   uncorrectable error

  @dependencies
  None.
 */
int qurt_ecc_get_error_status( qurt_ecc_memory_t memory, unsigned int type );


/**@ingroup func_qurt_ecc_get_error_count
  Get ECC error count for a specified memory
  
  @datatypes
  #qurt_ecc_memory_t
  
  @param[in]  memory, set to one of
    - QURT_ECC_MEM_L1_ICACHE
    - QURT_ECC_MEM_L1_DCACHE
    - QURT_ECC_MEM_L2_CACHE
    - QURT_ECC_MEM_VTCM   

  @param[in]  type, set to one of
    - QURT_ECC_CORRECTABLE_COUNT 
    - QURT_ECC_UNCORRECTABLE_COUNT 

  @param[out] 
  None.

  @return
   Error count for the specified error type

  @dependencies
  None.
 */
int qurt_ecc_get_error_count( qurt_ecc_memory_t memory, unsigned int type );


/**@ingroup func_qurt_ecc_clear_error_count
  Clear ECC error count or region logging for a specified memory
  
  @datatypes
  #qurt_ecc_memory_t
  
  @param[in]  memory, set to one of
    - QURT_ECC_MEM_L1_ICACHE
    - QURT_ECC_MEM_L1_DCACHE
    - QURT_ECC_MEM_L2_CACHE
    - QURT_ECC_MEM_VTCM   

  @param[in]  type, set to one or multiple ORed of,
    - QURT_ECC_CORRECTABLE_COUNT  
    - QURT_ECC_UNCORRECTABLE_COUNT 
    - QURT_ECC_REGION_LOGGING
     
  @param[out] 
  None.

  @return
    QURT_EOK: Error count is cleared successfully
    Others:   Failure at clearing the error count

  @dependencies
  None.
 */
int qurt_ecc_clear_error_count( qurt_ecc_memory_t memory, unsigned int type );


#endif /* QURT_ECC_H */

