#ifndef QURT_SCLK_RESTRICTED_H
#define QURT_SCLK_RESTRICTED_H
/**
  @file qurt_sclk.h 
  @brief  The header file that describes the APIs supported by QURT system sclk
   feature.

EXTERNAL FUNCTIONS
   None.

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2018  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/

/**@ingroup func_qurt_sysclock_register
  @xreflabel{sec:qurt_sysclock_register}
  Register a signal object to receive an event on a system clock alarm or timer.

  The return value is a client identifier value, which is used to associate a registered signal
  with a system clock alarm or timer object.

  @datatypes
  #qurt_anysignal_t

  @param[in]   signal       Signal object set when the system clock event occurs.
  @param[in]   signal_mask  Signal mask, which specifies the signal to set in the signal object when
                            the clock event occurs. 
 
  @return 
  Integer --  System clock client identifier. \n
  #QURT_EFATAL -- Not enough memory to create timer.
    
  @dependencies
  None.
 */

extern int qurt_sysclock_register (qurt_anysignal_t *signal, unsigned int signal_mask);

/**@ingroup func_qurt_sysclock_alarm_create
  Creates a system clock alarm with the specified time value, and starts the alarm.

  @param[in]   id       System clock client ID; indirectly indicating the signal that the 
                        alarm-expired event sets. The signal must already be 
                        registered to receive a system clock event 
                        (Section @xref{sec:qurt_sysclock_register}) -- it is 
                        specified here by the client identifier that is returned by the 
                        register operation.
  @param[in]  ref_count   System clock count when the match value was calculated, which specifies the 
                        system clock time when the match_value parameter is calculated. 
                        This value is obtained using the get hardware ticks operation 
                        (Section @xref{sec:qurt_sysclock_get_hw_ticks}).
  @param[in] match_value  Match value to be programmed in system clock hardware; indicates 
                        the system clock time (in system clock ticks) when the alarm
                        should expire.
 
  @return  
  Integer -- Match value programmed.

  @dependencies
  None.             
 */

extern unsigned long long qurt_sysclock_alarm_create (int id, unsigned long long ref_count, unsigned long long match_value);

/**@ingroup func_qurt_sysclock_timer_create
  Creates a system clock timer with the specified duration, and starts the timer.
    
  @param[in]  id          System clock client ID. Indirectly specifies the signal that the timer-expired 
                          event sets. The signal must already be registered to receive a system clock event 
                          (Section @xref{sec:qurt_sysclock_register}) -- the client 
                          identifier returned by the register operation specifies the signal.
  @param[in]   duration   Timer duration (in system clock ticks). Specifies the interval between the 
                          creation of the system clock timer object and the generation of the corresponding 
                          timer event.
                           
  @return  
  #QURT_EOK -- Timer successfully created.
 
  @dependencies
  None.
 */

extern int qurt_sysclock_timer_create (int id, unsigned long long duration);

/**@ingroup func_qurt_sysclock_get_expiry
  Gets the duration until next timer event.\n
  Returns the number of system ticks that elapse before the next timer event occurs on
  any active timer in the QuRT application system.

  A system tick is defined as one cycle of the Hexagon processor's 19.2 MHz Qtimer clock.
 
  @return 
  Integer -- Number of system ticks until next timer event.

  @dependencies
  None.
 */

extern unsigned long long qurt_sysclock_get_expiry (void);

#endif /* QURT_SCLK_RESTRICTED_H */
