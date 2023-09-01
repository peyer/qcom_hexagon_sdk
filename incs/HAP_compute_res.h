/*-----------------------------------------------------------------------------
   Copyright (c) 2019 QUALCOMM Technologies, Incorporated.
   All Rights Reserved.
   QUALCOMM Proprietary.
-----------------------------------------------------------------------------*/

#ifndef HAP_COMPUTE_RES_H_
#define HAP_COMPUTE_RES_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HAP_COMPUTE_RES_NOT_SUPPORTED                           0x80000404

/**
 *  @file HAP_compute_res.h
 *  @brief Header file with APIs to allocate compute resources.
 */

/*
 * @struct compute_res_attr_t
 * @brief compute resource attribute structure.
 */
typedef struct {
    unsigned long long attributes[8];
} compute_res_attr_t;

int __attribute__((weak)) compute_resource_attr_init(
                                        compute_res_attr_t* res_info);

int __attribute__((weak)) compute_resource_attr_set_serialize(
                                        compute_res_attr_t* res_info,
                                        unsigned char b_serialize);

int __attribute__((weak)) compute_resource_attr_set_vtcm_param(
                                        compute_res_attr_t* res_info,
                                        unsigned int vtcm_size,
                                        unsigned char b_vtcmSinglePage);

unsigned int __attribute__((weak)) compute_resource_acquire(
                                        compute_res_attr_t* res_info,
                                        unsigned int timeout_us);

void* __attribute__((weak)) compute_resource_attr_get_vtcm_ptr(
                                        compute_res_attr_t* res_info);

int __attribute__((weak)) compute_resource_release(
                                        unsigned int context_id);

/******************************************************************************
 * @fn HAP_compute_res_attr_init
 * @brief Initializes the compute resource attribute structure. To be called
 *        before using the attribute instance in an acquire call.
 * @param[in] res_info pointer to compute resource attribute structure
 *                     (compute_res_attr_t *)
 * @return int 0 for success and non-zero for failure.
 *****************************************************************************/
static inline int HAP_compute_res_attr_init(compute_res_attr_t* res_info)
{
    if (compute_resource_attr_init)
        return compute_resource_attr_init(res_info);

    return HAP_COMPUTE_RES_NOT_SUPPORTED;
}

/******************************************************************************
 * @fn HAP_compute_res_attr_set_serialize
 * @brief sets/resets the serialize option in the attribute structure based on
 *        input. To be called post HAP_compute_res_attr_init. Serialization,
 *        if enabled, will queue any further serialization enabled client till
 *        the current request is released.
 *        NOTE: A non-participating i.e. non-serialized client's request can
 *        still be served while a serialized client request is active.
 *        To be called post HAP_compute_res_attr_init only.
 * @param[in] res_info pointer to compute resource attribute structure
 *                     (compute_res_attr_t *)
 * @param[in] b_serialize 0 - not to be serialized (resets serialize option)
 *                        1 - to be serialized (sets serialize option)
 * @return int 0 for success and non-zero for failure.
 *****************************************************************************/
static inline int HAP_compute_res_attr_set_serialize(
                                                compute_res_attr_t* res_info,
                                                unsigned char b_serialize)
{
    if (compute_resource_attr_set_serialize)
    {
        return compute_resource_attr_set_serialize(res_info,
                                                   b_serialize);
    }

    return HAP_COMPUTE_RES_NOT_SUPPORTED;
}

/******************************************************************************
 * @fn HAP_compute_res_attr_set_vtcm_param
 * @brief Allows clients to request for VTCM memory by providing size and
 *        single page requirements.
 *        To be called post HAP_compute_res_attr_init only.
 * @param[in] res_info pointer to compute resource attribute structure
 *                     (compute_res_attr_t *)
 * @param[in] vtcm_size size of the VTCM request. 0 if VTCM allocation is NOT
 *                      needed.
 *            b_vtcmSinglePage 1 - requested VTCM size to be allocated in a
 *                                 single page
 *                             0 - no page requirement (allocation can spread
 *                                 across multiple pages. VTCM manager will
 *                                 always try to get the best fit)
 * @return int 0 for success and non-zero for failure.
 *****************************************************************************/
static inline int HAP_compute_res_attr_set_vtcm_param(
                                             compute_res_attr_t* res_info,
                                             unsigned int vtcm_size,
                                             unsigned char b_vtcmSinglePage)
{
    if (compute_resource_attr_set_vtcm_param)
    {
        return compute_resource_attr_set_vtcm_param(res_info,
                                                    vtcm_size,
                                                    b_vtcmSinglePage);
    }

    return HAP_COMPUTE_RES_NOT_SUPPORTED;
}

/******************************************************************************
 * @fn HAP_compute_res_acquire
 * @brief Accepts a prepared attribute structure and assigns a context for a
 *        successful request with in the provided timeout (micro-seconds).
 * @param[in] res_info pointer to compute resource attribute structure
 *                     (compute_res_attr_t *)
 * @param[in] timeout_us timeout in micro-seconds, should be greater than 100.
 * @return non-zero context Id in case of success
 *         0 in case of a failure (i.e unable to acquire requested resource
 *              in given timeout duration)
 *****************************************************************************/
static inline unsigned int HAP_compute_res_acquire(
                                              compute_res_attr_t* res_info,
                                              unsigned int timeout_us)
{
    if (compute_resource_acquire)
    {
        return compute_resource_acquire(res_info, timeout_us);
    }

    return 0;
}

/******************************************************************************
 * @fn HAP_compute_res_attr_get_vtcm_ptr
 * @brief Reads the VTCM pointer from the given attribute structure. Returns
 *        a non-zero pointer post a successful HAP_compute_res_acquire call
 *        done with given vtcm size and single page requirement.
 * @param[in] res_info pointer to compute resource attribute structure
 *                     (compute_res_attr_t *)
 * @return void* pointer to the allocated VTCM section.
 *****************************************************************************/
static inline void* HAP_compute_res_attr_get_vtcm_ptr(compute_res_attr_t* res_info)
{
    if (compute_resource_attr_get_vtcm_ptr)
    {
        return compute_resource_attr_get_vtcm_ptr(res_info);
    }

    return 0;
}

/******************************************************************************
 * @fn HAP_compute_res_release
 * @brief Releases all the resources linked to the given context id.
 *        To be called with the context_id returned by a successful
 *        HAP_compute_res_acquire call.
 * @param[in] context_id context id returned by HAP_compute_res_acquire call.
 * @return 0 for success, non-zero for failure.
 *****************************************************************************/
static inline int HAP_compute_res_release(unsigned int context_id)
{
    if (compute_resource_release)
    {
        return compute_resource_release(context_id);
    }

    return HAP_COMPUTE_RES_NOT_SUPPORTED;
}

#ifdef __cplusplus
}
#endif

#endif //HAP_COMPUTE_RES_H_
