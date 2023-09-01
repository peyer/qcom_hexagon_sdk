/*-----------------------------------------------------------------------
   Copyright (c) 2019 QUALCOMM Technologies, Incorporated.
   All Rights Reserved.
   QUALCOMM Proprietary.
-----------------------------------------------------------------------*/

#ifndef HAP_USER_PMU_H_
#define HAP_USER_PMU_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @file HAP_user_pmu.h
 *  @brief Header file with APIs to read user pmu values
 */

#define HAP_USER_PMU_READ_NOT_SUPPORTED                           0x80000FFF
#define HAP_USER_PMU_READ_FAILED                                  0xDEADDEAD

/**
 * @struct HAP_pmu_group_config_t
 * @brief Input parameter type used when group of PMU events have to be read
 */
typedef struct {
    int contextId;
    /**< Return value after registering PMU group using HAP_register_pmu_group */

    unsigned int num_events;
    /**< Input param to specify the number of PMU events to be registered*/

    unsigned short pmu_events[4];
    /**< Input param to specify the list of PMU events to be registered*/

    unsigned int pmu_value[4];
    /**< Output param containing values of PMU events registered*/
} HAP_pmu_group_config_t;


//----------------------Prototypes----------------------------------

int __attribute__((weak)) __HAP_register_pmu_group(HAP_pmu_group_config_t* pmu_config);
int __attribute__((weak)) __HAP_deregister_pmu_group(int contextId);
int __attribute__((weak)) __HAP_read_pmu_group(HAP_pmu_group_config_t* pmu_config);
int __attribute__((weak)) __HAP_register_pmu_event(unsigned short pmu_event);
int __attribute__((weak)) __HAP_deregister_pmu_event(unsigned short pmu_event);
unsigned int __attribute__((weak)) __HAP_read_pmu_event(unsigned short pmu_event);

/**************************************************************************//**
 * @fn HAP_register_pmu_group
 * @brief Register for the required set of PMU events
 * @param ptr to HAP_pmu_group_config_t struct holding the number of events and
          list of events
 * @return returns 0 on success
           contextId is updated in the input struct
 *****************************************************************************/
int HAP_register_pmu_group(HAP_pmu_group_config_t* pmu_config) {
    if(__HAP_register_pmu_group)
        return __HAP_register_pmu_group(pmu_config);

    return HAP_USER_PMU_READ_NOT_SUPPORTED;
}

/**************************************************************************//**
 * @fn HAP_deregister_pmu_group
 * @brief Deregister the registered set of PMU events using contextId
 * @param Pass the same ptr to HAP_pmu_group_config_t struct that is passed
          during registration
 * @return returns 0 on successful deregistration
 *****************************************************************************/
int HAP_deregister_pmu_group(HAP_pmu_group_config_t* pmu_config) {
    if(__HAP_deregister_pmu_group)
        return __HAP_deregister_pmu_group(pmu_config->contextId);

    return HAP_USER_PMU_READ_NOT_SUPPORTED;
}

/**************************************************************************//**
 * @fn HAP_read_pmu_group
 * @brief Read the PMU values of registered PMU events
 * @param Pass the same ptr to HAP_pmu_group_config_t struct that is passed
          during registration
 * @return returns 0 on success
           the input struct contains the PMU values in the same order as events
 *****************************************************************************/
int HAP_read_pmu_group(HAP_pmu_group_config_t* pmu_config) {
    if(__HAP_read_pmu_group)
        return __HAP_read_pmu_group(pmu_config);

    return HAP_USER_PMU_READ_NOT_SUPPORTED;
}

/**************************************************************************//**
 * @fn HAP_register_pmu_event
 * @brief Register to read the given PMU event
 * @param Pass the hex code of the PMU event
 * @return returns 0 on successful registration
 *****************************************************************************/
int HAP_register_pmu_event(unsigned short pmu_event) {
    if(__HAP_register_pmu_event)
        return __HAP_register_pmu_event(pmu_event);

    return HAP_USER_PMU_READ_NOT_SUPPORTED;
}

/**************************************************************************//**
 * @fn HAP_deregister_pmu_event
 * @brief Deregister the PMU event registered
 * @param Pass the hex code of the PMU event
 * @return returns 0 on successful deregistration
 *****************************************************************************/
int HAP_deregister_pmu_event(unsigned short pmu_event) {
    if(__HAP_deregister_pmu_event)
        return __HAP_deregister_pmu_event(pmu_event);

    return HAP_USER_PMU_READ_NOT_SUPPORTED;
}

/**************************************************************************//**
 * @fn HAP_read_pmu_event
 * @brief Read the value of PMU event
 * @param Pass the hex code of the PMU event
 * @return returns the value of the PMU event passed as input param
 *****************************************************************************/
unsigned int HAP_read_pmu_event(unsigned short pmu_event) {
    if(__HAP_read_pmu_event)
        return __HAP_read_pmu_event(pmu_event);

    return HAP_USER_PMU_READ_NOT_SUPPORTED;
}

#ifdef __cplusplus
}
#endif
#endif /*HAP_USER_PMU_H_*/
