#ifndef _GRAPHITE_API_H_
#define _GRAPHITE_API_H_
/*==============================================================================
  @file graphite_api.h
  @brief this file contains Graphite API Definitions

  Copyright (c) 2016 Qualcomm Technologies, Inc.(QTI)
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================
                       EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header:

  when       who        what, where, why
  --------   --- ------------------------------------------------------
  04/19/16 likhengp     Created file.
==============================================================================*/

/*------------------------------------------------------------------------------
 *  Header Includes
 *----------------------------------------------------------------------------*/
#include "mmdefs.h"

/*------------------------------------------------------------------------------
 *  Command Definitions
 *----------------------------------------------------------------------------*/
/*
  Graphite basic response results is used to convey the basic status of a
  response. This is a mandatory response message that all command senders should
  handle.
*/
#define GRAPHITE_BASIC_RSP_RESULT 0x0002

/*
  This is the payload of the GRAPHITE_BASIC_RSP_RESULT command response message.
*/
typedef struct graphite_basic_rsp_result_t graphite_basic_rsp_result_t;

#include "graphite_begin_pack.h"
struct graphite_basic_rsp_result_t
{
  uint32_t status;
  /**<  Valid Graphite error code.
        Completion status (see graphite_error_codes.h) */
}
#include "graphite_end_pack.h"
;

/*
  GRAPHITE_CMD_OPEN:
    - Description:
        Each graph is set up using the GRAPHITE_CMD_OPEN command.
        This command can be used to set up either a complete graph or a portion
        of the graph.
        Each GRAPHITE_CMD_OPEN contains a list of modules and their calibration.
        The module list is sent with a calibration header just like other
        calibration parameters.

        The payload can contain multiple calibrations concatenated in the
        payload memory. All calibrations need to start with a calibration
        header. The command handler will parse and interpret the sub-payload
        differently depending on the type of calibration specified in the
        calibration header param_id field.

        It is important to note that the command payload will be parsed and
        processed from start to end. This implies that the calibration
        (including connection information) for a module, if any, should follow
        the module definition in the module list. If a module is not present
        when calibration is received for that module then it is treated as
        an error.

        It should be noted that if there is an error in either the module
        creation or calibration then all the modules that have been instantiated
        as part of that command will be destroyed and its resources freed up and
        a suitable error code returned to the client.

    - Payloads:
      - Module list:
        graphite_calibration_header_t (
          param_id = GRAPHITE_PARAM_ID_MODULE_LIST)
        graphite_module_t
        [repeat for (graphite_calibration_header_t.size / sizeof(
          graphite_module_t)) times]

      - Connections:
        graphite_calibration_header_t (param_id = GRAPHITE_PARAM_ID_CONNECT/
          GRAPHITE_PARAM_ID_DISCONNECT)
        graphite_edge_src_module_count_t
        graphite_edge_src_module_t
          graphite_edge_dest_module_t
            graphite_edge_t
            [repeat for (graphite_edge_dest_module_t.num_edges) times]
          [repeat for (graphite_edge_src_module_t.num_destination_modules)
            times]
        [repeat for (graphite_edge_src_module_count_t.num_source_modules)
          times]

      - Indirect calibration data (data sent through GRAPHITE_CMD_LOAD_DATA):
        graphite_calibration_header_t (
          param_id = GRAPHITE_PARAM_ID_INDIRECT_CALIB_DATA)
        graphite_indirect_calib_data_t

      - Other calibrations:
        graphite_calibration_header_t (param_id = <some PID>)
        some_calibration_payload_struct

    - Response: GRAPHITE_BASIC_RSP_RESULT error code
*/
#define GRAPHITE_CMD_OPEN 0x0003

/*
  GRAPHITE_CMD_CLOSE:
    - Description:
        Every complete graph or a part of it can be torn down using the
        GRAPHITE_CMD_CLOSE command.  The close command consists of a list of
        modules (specified by a MID-IID pair) to be closed. Any connections
        coming into the module and going out of the module are implicitly
        removed as part of the operation. Also all calibration associated with
        the module is lost. Modules shall be closed in the order specified in
        the payload beginning with the first entry.

    - Payloads:
      - Module list:
        graphite_calibration_header_t (param_id = GRAPHITE_PARAM_ID_MODULE_LIST)
        graphite_module_t
        [repeat for (graphite_calibration_header_t.size / sizeof(
          graphite_module_t)) times]
    - Response: GRAPHITE_BASIC_RSP_RESULT error code
*/
#define GRAPHITE_CMD_CLOSE 0x0004

/*
  GRAPHITE_CMD_SET_CONFIG:
    - Description:
        It is possible to apply calibration parameters to the graph once it is
        set up using the GRAPHITE_CMD_SET_CONFIG command. The payload for the
        GRAPHITE_CMD_SET_CONFIG is similar to that of the GRAPHITE_CMD_OPEN
        command, except that it does not have a list of module to be created.
        Connections and disconnections can also be sent as part of
        GRAPHITE_CMD_SET_CONFIG using their respective param IDs.

    - Payloads:
      - Connections/Disconnections:
        graphite_calibration_header_t (param_id = GRAPHITE_PARAM_ID_CONNECT/
          GRAPHITE_PARAM_ID_DISCONNECT)
        graphite_edge_src_module_count_t
        graphite_edge_src_module_t
          graphite_edge_dest_module_t
            graphite_edge_t
            [repeat for (graphite_edge_dest_module_t.num_edges) times]
          [repeat for (graphite_edge_src_module_t.num_destination_modules)
            times]
        [repeat for (graphite_edge_src_module_count_t.num_source_modules)
          times]

      - Other calibrations:
        graphite_calibration_header_t (param_id = <some PID>)
        <some_calibration_payload_struct>

    - Response: GRAPHITE_BASIC_RSP_RESULT error code
*/
#define GRAPHITE_CMD_SET_CONFIG 0x0005

/*
  GRAPHITE_CMD_GET_CONFIG:
    - Description:
        The graph calibration can be queried using the GRAPHITE_CMD_GET_CONFIG
        command. This command can be used to query calibration parameters for
        any existing module(s) including the connection information.
        The GRAPHITE_CMD_GET_CONFIG payload consists of a set of MID-IID-PID
        triplets for which the calibration information is required along with
        the expected size of the calibration.

    - Payloads:
      - Calibrations to query for (MID-IID-PID combination):
        graphite_calibration_header_t (some MID-IID-PID combination)
        [repeat for cmi_v2_header_t.payload_size /
          sizeof(graphite_calibration_header_t) time]

      - Note: If the client does not know the calibration payload size,
              graphite_calibration_header_t.size for that calibration should be
              set to 0xFFFFFFFF.

*/
#define GRAPHITE_CMD_GET_CONFIG 0x0006

/* Macro to define the size value for a GET_CONFIG calibration with unknown
 * size.
 * This tells Graphite that it needs to Query the module for the calibration
 * payload size.
 */
#define GRAPHITE_GET_CONFIG_UNKNOWN_SIZE (0xffffffff)

/*
  GRAPHITE_CMDRSP_GET_CONFIG:
    - Description:
        As a response to the GRAPHITE_CMD_GET_CONFIG the WDSP responds with the
        GRAPHITE_CMDRSP_GET_CONFIG message which contains the actual values for
        the calibration parameters which were queried using the
        GRAPHITE_CMD_GET_CONFIG command.

    - Payloads:
        graphite_basic_rsp_result_t (indicates if query was successful)
        graphite_calibration_header_t
          <some_calibration_payload_struct>
        [repeats for number of calibrations queried]
*/
#define GRAPHITE_CMDRSP_GET_CONFIG 0x0007

/*
  GRAPHITE_EVENT:
    - Description:
        This is the Opcode used for events raised from the Graphite server to
        a Graphite client.

        Module can raise event defined by an event id. If a Graphite client has
        registered for a particular event id from a module id, instance id entry
        – then that event is packed in a generic GRAPHITE_EVENT message as shown
        below and delivered to the Graphite client

        Each GRAPHITE_EVENT contains a list of events that were raised. Event
        header is the same as calibration header with param_id (PID) signifying
        the event parameter being raised.

        Use the following calibration configuration to register/deregister for
        generic Graphite events.
          module_id = GRAPHITE_MODULE_FWK
          instance_id = 0
          param_id = GRAPHITE_PARAM_ID_EVENT_REGISTER/
                      GRAPHITE_PARAM_ID_EVENT_DEREGISTER

    - Payloads:
      - graphite_calibration_header_t
        - param_id field will denote the event_id which the client registered
          for.
*/
#define GRAPHITE_EVENT 0x000B

/*
  The Graphite calibration header is used in all Graphite commands to tell
  the Graphite command handler how to parse the command payload.
  The calibration header will be in the CMI packet payload, and there can be
  multiple calibration headers per payload.
 */
typedef struct graphite_calibration_header_t graphite_calibration_header_t;

#include "graphite_begin_pack.h"
struct graphite_calibration_header_t
{
  uint32_t module_id;
  /**<  Any non-zero value.
        A unique identifier for a particular module */

  uint16_t instance_id;
  /**<  Valid 16-bit value.
        A unique identifier for a particular instance of a module. The
        combination of module ID and instance ID needs to be unique across all
        modules in the WDSP.
        instance_id is not applicable to framework modules, and will be
        ignored.*/

  uint16_t reserved;
  /**<  Reserved field, must be set to 0. */

  uint32_t param_id;
  /**<  Any non-zero value.
        Indicates a particular calibration parameter that needs to be applied to
        the module/instance ID listed above.

        For Events types, this represents the particular event that is raised
        by Graphite.*/

  uint32_t size;
  /**<  Valid 32-bit value.
        Size of the payload for this calibration parameter.

        For GRAPHITE_CMD_GET_CONFIG, set size to 0xFFFFFFFF to indicate that the
        size of the calibration parameter is unknown to the client, and needs
        to be calculated by Graphite. */
}
#include "graphite_end_pack.h"
;

/*
  GRAPHITE_CMD_LOAD_DATA:
    - Description:
        This command is used by the Graphite client to send a blob of data
        to the Graphite server, which will be retained for future use.

    - Payloads:
      - Variable sized payload.
      - This payload is NOT processed when it is received at the Graphite
        server. It will only be retained in memory for future use when the
        client sends another command with the address provided in the
        GRAPHITE_CMDRSP_LOAD_DATA response.
*/
#define GRAPHITE_CMD_LOAD_DATA 0x0008

/*
  GRAPHITE_CMDRSP_LOAD_DATA:
    - Description:
        This is the command response to #GRAPHITE_CMD_LOAD_DATA

    - Payloads:
      - graphite_load_data_rsp_t
*/
#define GRAPHITE_CMDRSP_LOAD_DATA 0x0009

typedef struct graphite_load_data_rsp_t graphite_load_data_rsp_t;

#include "graphite_begin_pack.h"
struct graphite_load_data_rsp_t
{
  uint32_t addr;
  /**< Valid 32-bit value.

       If addr is NULL, then #GRAPHITE_CMD_LOAD_DATA command failed.

       This is the address at which the GRAPHITE_CMD_LOAD_DATA payload was
       deposited at. The client should pass this address back to the Graphite
       server in the future when it wants this payload to be processed.*/
}
#include "graphite_end_pack.h"
;

/*
  GRAPHITE_CMD_UNLOAD_DATA:
    - Description:
        This is the command used to free a payload that was previously sent
        by the #GRAPHITE_CMD_LOAD_DATA command.

    - Payloads:
      - graphite_unload_data_t
*/
#define GRAPHITE_CMD_UNLOAD_DATA 0x000A

typedef struct graphite_unload_data_t graphite_unload_data_t;

#include "graphite_begin_pack.h"
struct graphite_unload_data_t
{
  uint32_t addr;
  /**< Valid 32-bit value.
       This is the address of the data blob that should be freed.
       This address is where GRAPHITE_CMD_LOAD_DATA payload was previously
       deposited at, and returned to the client via the
       #GRAPHITE_CMDRSP_LOAD_DATA command response. */
}
#include "graphite_end_pack.h"
;

/*
  GRAPHITE_CMD_DATA:
    - Description:
        This command is used by Graphite client APPs to send data command to module in WDSP.

    - Payloads:
      - graphite_cmd_data_t
*/
#define GRAPHITE_CMD_DATA 0x000C

typedef struct graphite_cmd_data_t graphite_cmd_data_t;

#include "graphite_begin_pack.h"
struct graphite_cmd_data_t
{
  uint32_t module_id;
  /**< Valid 32-bit value.
        Module ID is a unique identifier for a particular module. */

  uint16_t instance_id;
  /**< Instance ID identifies a particular instance of a module. 
       The combination of module ID and instance ID needs to be unique
       across all modules in the WDSP.. */

  uint16_t reserved;
  /**<  Reserved field, must be set to 0. */

  uint32_t cmd_id;
  /**< ID of the module's command */

  uint32_t size_in_bytes;
  /**< Valid 32-bit value.
       Number of bytes in the payload. */

  uint32_t token;
  /**<  Token to synchronize the data command and it response. */
}
#include "graphite_end_pack.h"
;

/*
  GRAPHITE_CMDRSP_DATA:
    - Description:
        This command is used by Graphite in response to GRAPHITE_CMD_DATA.
        CMI header will be followed by the Payload of this in the response packet.
        Mapping of read command and read response happens through the token of the command.

    - Payloads:
      - graphite_cmdrsp_data_t
*/
#define GRAPHITE_CMDRSP_DATA 0x000D

typedef struct graphite_cmdrsp_data_t graphite_cmdrsp_data_t;

#include "graphite_begin_pack.h"
struct graphite_cmdrsp_data_t
{
  uint32_t module_id;
  /**< Valid 32-bit value.
        Module ID is a unique identifier for a particular module. */

  uint16_t instance_id;
  /**< Instance ID identifies a particular instance of a module. 
       The combination of module ID and instance ID needs to be unique
       across all modules in the WDSP.. */

  uint16_t reserved;
  /**<  Reserved field, must be set to 0. */

  uint32_t cmd_id;
  /**< ID of the module's command */

  uint32_t size_in_bytes;
  /**< Valid 32-bit value.
       Number of bytes for read. */

  uint32_t token;
  /**<  Token to synchronize the data command and it response. */

}
#include "graphite_end_pack.h"
;


#endif /* _GRAPHITE_API_H_ */
