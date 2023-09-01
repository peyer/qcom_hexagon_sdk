#ifndef QURT_TRACE_RESTRICTED_H
#define QURT_TRACE_RESTRICTED_H
/**
  @file qurt_trace.h 
  @brief  Prototypes of system call tracing helpers API  

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/


/**@ingroup func_qurt_etm_set_config
  Sets the configuration for the ETM. This operation specifies the types of program information 
  to trace, the destination of the trace information, and various filters to limit the amount 
  of trace information generated. This function must be called before enabling the ETM, 
  or after stopping the ETM.  

  @returns
    #QURT_ETM_SETUP_OK -- Success. \n
    #QURT_ETM_SETUP_ERR -- Failure.

  @dependencies
  None.
*/
unsigned int qurt_etm_set_config(unsigned int type, unsigned int route, unsigned int filter);


/**@ingroup func_qurt_etm_enable
  Enables or disables the ETM trace. ETM is disabled by default, and must be configured before it is enabled.

  The default ETM configuration after system bootup is of type #QURT_ETM_TYPE_PC_ADDR, route #QURT_ETM_ROUTE_TO_QDSS, 
  and filter #QURT_ETM_TRACE_FILTER_ALL. These values are applied to qurt_etm_enable() if qurt_etm_set_config() is 
  not called first.
  
  @param[in] enable_flag Values: \n
                         #QURT_ETM_ON \n
						 #QURT_ETM_OFF  
      
  @returns
    #QURT_ETM_SETUP_OK -- Success. \n
    #QURT_ETM_SETUP_ERR -- Failure

  @dependencies
  None.
*/
unsigned int qurt_etm_enable(unsigned int enable_flag);

/**@ingroup func_qurt_etm_testbus_set_config
  Sets configuration data for the Hexagon ETM test bus.

  @note1hang The ETM test bus design and configuration can be changed per Hexagon chipset.

  @param[in] cfg_data    Configuration data.

  @returns
  #QURT_ETM_SETUP_OK -- Success. \n
  #QURT_ETM_SETUP_ERR -- Failure.

  @dependencies
  None.
*/
unsigned int qurt_etm_testbus_set_config(unsigned int cfg_data);


/**@ingroup func_qurt_etm_set_breakpoint
  Sets a breakpoint at the specified memory access.
  When the breakpoint is triggered, the ETM unit generates an ETM interrupt. 
  The breakpoint can be configured to apply only to read, write, or read/write accesses,
  or when the access involves a specific data value. 
 
  @note1hang The ETM interrupt handler raises an NMI exception after receiving the ETM interrupt.

  @note1hang The maximum number of breakpoints supported is limited by the ETM hardware.

  @param[in] type  Memory access type. Values: \n
           #QURT_ETM_READWRITE_BRKPT  \n
           #QURT_ETM_READ_BRKPT    \n
           #QURT_ETM_WRITE_BRKPT   \n
           #QURT_ETM_BRKPT_INVALIDATE 
  @param[in] address   ETM breakpoint address
  @param[in] data      Memory compare value. If the data value stored at the specified memory 
                       address matches the compare value, the breakpoint is triggered.
                       For every bit in the mask parameter that is set to 1, the corresponding
                       bit in the compare value must be set to 0. 
  @param[in] mask      Memory compare mask. A 32-bit mask value that limits the memory compare 
                       operation to the specified bits. For every bit in the mask value that is 
					   set to 1, the corresponding bit in the compare value parameter must be set 
					   to 0, and the bit is ignored during the memory compare operation. 
                       When the mask value is set to 0xffffffff, no memory compare operation
                       is performed as part of the breakpoint test.

   @returns
   #QURT_ETM_SETUP_OK -- Success. \n
   #QURT_ETM_SETUP_ERR -- Failure

   @dependencies
   None.
*/
 
unsigned int qurt_etm_set_breakpoint(unsigned int type, unsigned int address, unsigned int data, unsigned int mask);  
 

/**@ingroup func_qurt_etm_set_breakarea
  Sets a breakpoint for the specified memory area. When the breakpoint is triggered, 
  the ETM unit generates an ETM interrupt. The breakpoint can be configured to apply 
  only to read, write, or read/write accesses, or when a specific number of accesses 
  has been performed to the memory area. 

  @note1hang The ETM interrupt handler raises an NMI exception after receiving the ETM interrupt.

  @note1hang The maximum number of area breakpoints supported is limited by the ETM hardware.

  @param[in] type  Memory access type. Values: \n
           #QURT_ETM_READWRITE_BRKPT  \n
           #QURT_ETM_READ_BRKPT  \n
           #QURT_ETM_WRITE_BRKPT \n  
           #QURT_ETM_BRKPT_INVALIDATE 
  @param[in] start_address   Start address of breakpoint memory area.
  @param[in] end_address     End address of breakpoint memory area.
  @param[in] count           Number of memory accesses performed to memory area before the breakpoint is triggered. 

  @returns
  #QURT_ETM_SETUP_OK -- Success. \n
  #QURT_ETM_SETUP_ERR -- Failure

  @dependencies
  None.
*/
 
unsigned int qurt_etm_set_breakarea(unsigned int type, unsigned int start_address, unsigned int end_address, unsigned int count);


/**@ingroup func_qurt_dtm_enable
  Enables or disables the DTM trace. 

  @param[in] enable_flag Values: \n
                         #QURT_DTM_ON \n
						 #QURT_DTM_OFF  
      
  @returns
    #QURT_EOK -- Success. \n
    Others   -- Failure

  @dependencies
  None.
*/
unsigned int qurt_dtm_enable(unsigned int enable_flag);


#endif /* QURT_TRACE_RESTRICTED_H */
