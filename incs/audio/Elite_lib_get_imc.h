#ifndef ELITE_LIB_GET_IMC_H
#define ELITE_LIB_GET_IMC_H

/**
  @file Elite_lib_get_imc.h

  @brief A CAPIv2 supporting library for inter-module communication with other CAPIv2 modules

  This file defines a CAPIv2 supporting library that can be used to communicate with
  other CAPIv2 modules that are available in the system.
*/

/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.(QTI)
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================
  Edit History

  $Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/incs/audio/Elite_lib_get_imc.h#1 $

  when        who      what, where, why
  --------    ---      -------------------------------------------------------
  1/21/15     sabdagir   Created file.
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*------------------------------------------------------------------------------
 * Include Files
 *----------------------------------------------------------------------------*/
#include "Elite_CAPI_V2.h"

#define ELITE_LIB_GET_IMC 0x00010E5D
/**< Unique identifier to represent this library interface.*/

/*------------------------------------------------------------------------------
 * Interface definition
 *----------------------------------------------------------------------------*/
typedef struct elite_lib_get_imc_t elite_lib_get_imc_t;
typedef struct imc_src_info_t  imc_src_info_t;
typedef struct imc_dest_info_t imc_dest_info_t;

/********************************************************************************************************************
  For an inter-module communication to establish between destination and source(s), they need to register with IMC
  database with a key. Max key length is limited to 16 bytes. Key needs to be unique for each communication instance
  and needs to be same for both destination and source(s) to communicate with each each other. First 4 bytes of the
  key corresponds to communication id specific to the type of the modules communicating and bytes after that need to
  distinguish different instances of the same communication.

  When registering for IMC, following table is recommended to determine Key

  id0 = first 4 bytes of Key, id1 = next 4 bytes of Key...

  Communication Type       | id0           | id1           | id2           | id3
  -------------------------+---------------+---------------+---------------+----------------------------------------
  o 1 destination and      | Unique Commu- | Desination    | Source        | Id to distinguish different instance of
    1 source               | -nication Id. | Module Id     | Module Id     | same comm.Service session Id can be used.
  -------------------------+---------------+---------------+---------------+----------------------------------------
  o 1 destination and      | Unique Commu- | Desination    | Session Id    | --
    multiple sources       | -nication Id. | Module Id     |               |
  -------------------------+---------------+---------------+---------------+----------------------------------------
  o multiple destinations  | Source module need to open communication with different destinations with
    and 1 source           | corresponding Keys.
*********************************************************************************************************************/

/*
 * Structure for the vtable of the imc library interface.
 *
 * Usage is as follows:
 * For Source Module:
 * -----------------------------------
 * 1. Get an object of type elite_lib_get_imc_t using the CAPI_V2_EVENT_GET_LIBRARY_INSTANCE with
 *    id =  ELITE_LIB_GET_IMC
 * 2. Register with IMC database to get source handle. Source buffer queue len(should be power of 2) need to be provided.
 * 3. Open communication with Key and get destination handle. Once registered, source can open multiple connections
 *    with different destinations.
 * 4. Allocating and buffer transfer to  destination can be done in two ways.
 *    o Allocate single buffer and push it to destination queue. When destination returns the buffer, source need
 *      to handle it(free or resend to destination with new payload).
 *    o Allocate required (less than src queue len) number of buffers with buffer size and push those to its own queue.
 *      When required, pop the buffers from its queue and send it to destination Number of buffers. If sorce failed
 *      to push the buffer to destination, it need to push the buffer to its own queue.
 * 6. Once communication is done, close the connection for each destination
 * 7. De-register from IMC database.
 * 8. Call end to return the elite_lib_get_imc_t object.
 *
 * For Destination Module:
 * -----------------------------------
 * 1. Get an object of type elite_lib_get_imc_t using the CAPI_V2_EVENT_GET_LIBRARY_INSTANCE with
 *    id =  ELITE_LIB_GET_IMC
 * 2. Register with IMC database to get destination handle.
 * 3. pop the buffers from the queue.
 * 4. process the buffer.
 * 5. return the buffer to source with the result
 * 6. De-register from IMC database.
 * 7. Call end to return the elite_lib_get_imc_t object.
 *
 */
typedef struct elite_lib_get_imc_vtable_t
{
   capi_v2_library_base_t b;
   capi_v2_err_t (*src_register)(imc_src_info_t **src_handle, uint32_t src_bufq_len);
   capi_v2_err_t (*src_open_comm)(imc_src_info_t *src_handle, imc_dest_info_t **dst_handle,const int8_t *keyptr,const uint32_t keylen);
   capi_v2_err_t (*src_allocate_push_self_buf)(imc_src_info_t *src_handle, uint32_t payload_size, uint32_t num_buffers);
   capi_v2_err_t (*src_allocate_packet)(imc_src_info_t *src_handle, void **payload_ptr, uint32_t payload_size);
   capi_v2_err_t (*src_pop_buf)(imc_src_info_t *src_handle, void **payload_ptr, capi_v2_err_t *resp_result_ptr,bool_t *is_queue_empty);
   capi_v2_err_t (*src_push_buf)(imc_src_info_t *src_handle,imc_dest_info_t *dst_handle, void *payload_ptr,uint32_t payload_size);
   capi_v2_err_t (*src_return_buf)(imc_src_info_t *src_handle, void *payload_ptr,capi_v2_err_t response_result);
   capi_v2_err_t (*src_close_connection)(imc_src_info_t *src_handle, imc_dest_info_t *dst_handle);
   capi_v2_err_t (*src_deregister)(imc_src_info_t *src_handle);
   capi_v2_err_t (*dest_register)(imc_dest_info_t **dst_handle, const int8_t *keyptr,const uint32_t keylen,uint32_t dest_dataq_len);
   capi_v2_err_t (*dest_pop_buf)(imc_dest_info_t *dst_handle, void **payload_ptr,uint32_t *payload_actual_size_ptr,uint32_t *payload_max_size_ptr,bool_t *is_queue_empty);
   capi_v2_err_t (*dest_return_buf)(imc_dest_info_t *dst_handle, void *payload_ptr,capi_v2_err_t response_result);
   capi_v2_err_t (*dest_deregister)(imc_dest_info_t *dst_handle);
}elite_lib_get_imc_vtable_t;

typedef struct elite_lib_get_imc_t
{
   const elite_lib_get_imc_vtable_t *vtable;
}elite_lib_get_imc_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* ELITE_LIB_GET_IMC_H */
