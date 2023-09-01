#ifndef ELITE_LIB_GET_CAPI_V2_MODULE_H
#define ELITE_LIB_GET_CAPI_V2_MODULE_H

/**
  @file Elite_lib_capi_v2_module.h

  @brief A CAPIv2 supporting library for creating other CAPIv2 modules

  This file defines a CAPIv2 supporting library that can be used to create
  instances of other CAPIv2 modules that are available in the system.
*/

/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.(QTI)
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================
  Edit History

  $Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/incs/audio/Elite_lib_get_capi_v2_module.h#1 $

  when        who      what, where, why
  --------    ---      -------------------------------------------------------
  12/29/14    deepang   Created file.
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "Elite_CAPI_V2.h"

#define ELITE_LIB_GET_CAPI_V2_MODULE 0x00010914
/**< Unique identifier to represent this library interface.*/

/*------------------------------------------------------------------------------
 * Interface definition
 *----------------------------------------------------------------------------*/

typedef struct elite_lib_get_capi_v2_t elite_lib_get_capi_v2_t;
typedef struct elite_lib_capi_v2_handle elite_lib_capi_v2_handle;

/*******************************************************************************
  When working with CAPI_V2 modules use the following table to
  determine the values of id1 and id2

  Type       | id1           | id2
  -----------+---------------+---------------
  Generic    | Module id Set | Set to zero
  Encoder    | Output format | Set to zero
  Decoder    | Input format  | Set to zero
  Converter  | Input format  | Output format
  Packetizer | Input format  | Set to zero
*******************************************************************************/
#define ELITE_LIB_CAPI_V2_TYPE_GENERIC 0
#define ELITE_LIB_CAPI_V2_TYPE_DECODER 1
#define ELITE_LIB_CAPI_V2_TYPE_ENCODER 2
#define ELITE_LIB_CAPI_V2_TYPE_CONVERTER 3
#define ELITE_LIB_CAPI_V2_TYPE_PACKETIZER 4

/*
 * Structure for the vtable of the library interface.
 *
 * Usage is as follows:
 * 1. Get an object of type elite_lib_get_capi_v2_t using the CAPI_V2_EVENT_GET_LIBRARY_INSTANCE.
 * 2. Call get_handle with the appropriate values of type, id1 and id2 to get a handle that can be used to call the capi_v2 get_static_properties and init functions.
 * 3. The handle can be used to call the get_static_properties function of the module using call_get_static_properties.
 * 4. The handle can be used to call the init function of the module using call_init.
 * 5. Multiple instances of the module can be created with multiple calls to init (using call_init).
 * 6. After the required instances are created, the handle can be released using release_handle. The handle should not be used
 *    after this call. Note: The instances of the modules may continue to exist after the handle is released.
 * 7. Multiple capi_v2 handles may be queried using a single library instance. Hence, steps 2-6 should be repeated for all modules that are to be created.
 * 8. Once all the modules are created and all the handles are closed, the library instance can be
 *    destroyed by calling b.end. Note: The instances of the modules may continue to exist after the library instance is destroyed.
 */
struct elite_lib_get_capi_v2_vtable
{
   capi_v2_library_base_t b;
   capi_v2_err_t (*get_handle)(elite_lib_get_capi_v2_t *obj_ptr, uint32_t type, uint32_t id1, uint32_t id2, elite_lib_capi_v2_handle** h_ptr);
   capi_v2_err_t (*call_get_static_properties)(elite_lib_get_capi_v2_t *obj_ptr, elite_lib_capi_v2_handle* h, capi_v2_proplist_t* init_set_properties, capi_v2_proplist_t* static_properties);
   capi_v2_err_t (*call_init)(elite_lib_get_capi_v2_t *obj_ptr, elite_lib_capi_v2_handle* h, capi_v2_t* _pif, capi_v2_proplist_t* init_set_properties);
   capi_v2_err_t (*release_handle)(elite_lib_get_capi_v2_t *obj_ptr, elite_lib_capi_v2_handle *h);
};

struct elite_lib_get_capi_v2_t
{
   const elite_lib_get_capi_v2_vtable *vtable;
};
 
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef ELITE_LIB_GET_CAPI_V2_MODULE_H */
