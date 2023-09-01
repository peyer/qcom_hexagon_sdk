/*-----------------------------------------------------------------------------
   Copyright (c) 2019 QUALCOMM Technologies, Incorporated.
   All Rights Reserved.
   QUALCOMM Proprietary.
-----------------------------------------------------------------------------*/

#ifndef HAP_SYSMON_RM_QOS_
#define HAP_SYSMON_RM_QOS_

#ifdef __cplusplus
extern "C" {
#endif

#define HAP_SYSMON_RM_QOS_DURATION_DEFAULT                          10
#define HAP_SYSMON_RM_SET_QOS_NOT_SUPPORTED                         0x80000404

int __attribute__((weak)) sysmon_rm_set_qos_duration(unsigned int duration);
int __attribute__((weak)) sysmon_rm_set_qos(void);

/**************************************************************************//**
 * @fn HAP_sysmon_rm_set_qos
 * @brief Sets CPU QoS to disallow CPU low power modes (power collapse) for
          a duration (set by either HAP_sysmon_rm_set_qos_duration or the default
          HAP_SYSMON_RM_QOS_DURATION_DEFAULT milli-seconds). This can be used
          effectively to wake up CPU in preparation of a return from DSP.
          User needs to call this API before 2 - 3 milli-seconds from the actual
          return to CPU - this is to accommodate the worst case CPU wakeup latency
          if CPU was in low power mode by the time user invokes this API.
 * @return int 0 for success and non-zero for failure.
 *****************************************************************************/
static inline int HAP_sysmon_rm_set_qos(){
    if (sysmon_rm_set_qos)
        return sysmon_rm_set_qos();

    return HAP_SYSMON_RM_SET_QOS_NOT_SUPPORTED;
}

/**************************************************************************//**
 * @fn HAP_sysmon_rm_set_qos_duration
 * @brief Sets the QoS active duration field for the HAP_sysmon_rm_set_qos
          calls originating from this user process. CPU QoS will be set for this
          duration once a HAP_sysmon_rm_set_qos call is made. The duration
          parameters accepts values in milli-seconds.
          The default duration field is HAP_SYSMON_RM_QOS_DURATION_DEFAULT
          milli-seconds.
 * @param[in] duration QoS window duration in milli-seconds.
 * @return int 0 for success and non-zero for failure.
 *****************************************************************************/
static inline int HAP_sysmon_rm_set_qos_duration(unsigned int duration){
    if (sysmon_rm_set_qos_duration)
        return sysmon_rm_set_qos_duration(duration);

    return HAP_SYSMON_RM_SET_QOS_NOT_SUPPORTED;
}

#ifdef __cplusplus
}
#endif

#endif /*HAP_SYSMON_RM_QOS_*/