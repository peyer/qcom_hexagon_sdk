/* ======================================================================== */
/**
 @file capi_v2_sp_rx.cpp

 C source file to implement the CAPIv2 interface for Speaker Protection Rx module.
 */

/* =========================================================================
 Copyright (c) 2015 QUALCOMM Technologies Incorporated.
 All rights reserved. Qualcomm Technologies Proprietary and Confidential.
 ========================================================================= */
/*------------------------------------------------------------------------
 * Include files and Macro definitions
 * -----------------------------------------------------------------------*/

#ifndef _DEBUG  // To enable FARF messages
#define _DEBUG
#endif

#include "capi_v2_sp_rx.h"
#include "capi_v2_sp_rx_utils.h"
#include <assert.h>
#include <hexagon_protos.h>
#include "AEEstd.h"
/*------------------------------------------------------------------------
 * Explanation of the Speaker Protection Rx module
 *
 *
 * Here the call flow for this module is discussed.
 * -> The capi_v2_sp_rx_get_static_properties() is called from the service layer to
 *    obtain the memory requirement of the CAPIv2 layer.
 * -> It is then followed by the capi_v2_sp_rx_init() from service layer to initialize the
 *    memory for the module and populate CAPIv2 function table.
 * -> Now we are ready to receive library configuration parameters through set param and set properties.
 * -> Since, this module needs to support two framework extensions: Feedback and Codec interrupts. The service
 *    will query for these and it should be set from here.
 * -> Now, we have set-up the needed feedback and codec interrupt frameworks from the service layer. We will
 *    send the CAPI_V2_INPUT_MEDIA_FORMAT to trigger the start of the module processing. It means data pumping
 *    is going to start and the CAPIv2 layer needs to be ready for the data buffer processing.
 * -> Once we receive the input media format, we need to initilize the library with any params received
 *    till now and raise the KPPS and BW requirements.
 * -> Now the process function will be called when service receives new data buffers.
 * -----------------------------------------------------------------------*/


/*------------------------------------------------------------------------
 * Static declarations
 * -----------------------------------------------------------------------*/

static capi_v2_err_t capi_v2_sp_rx_process(capi_v2_t* _pif, capi_v2_stream_data_t* input[],
                                           capi_v2_stream_data_t* output[]);

static capi_v2_err_t capi_v2_sp_rx_end(capi_v2_t* _pif);

static capi_v2_err_t capi_v2_sp_rx_set_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr);

static capi_v2_err_t capi_v2_sp_rx_get_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr);

static capi_v2_err_t capi_v2_sp_rx_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr);

static capi_v2_err_t capi_v2_sp_rx_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr);

/** Virtual table for capi_v2_sp_rx
 *  */
static capi_v2_vtbl_t vtbl =
{ capi_v2_sp_rx_process, capi_v2_sp_rx_end, capi_v2_sp_rx_set_param, capi_v2_sp_rx_get_param,
      capi_v2_sp_rx_set_properties, capi_v2_sp_rx_get_properties };

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_get_static_properties
 DESCRIPTION: Function to get the static properties of SP RX  module
 -----------------------------------------------------------------------*/
capi_v2_err_t capi_v2_sp_rx_get_static_properties(capi_v2_proplist_t *init_set_properties,
                                                  capi_v2_proplist_t *static_properties)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

   if(NULL != static_properties)
   {
      capi_v2_result = capi_v2_sp_rx_process_get_properties((capi_v2_sp_rx_t*)NULL,
                                                            static_properties);
      if(CAPI_V2_FAILED(capi_v2_result))
      {
         return capi_v2_result;
      }
   }

   if(NULL != init_set_properties)
   {
      FARF(HIGH, "CAPIv2 SP_RX: get static properties ignoring init_set_properties!");
   }

   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_init
 DESCRIPTION: Initialize the CAPIv2 SP_RX module and library.
 This function can allocate memory.
 -----------------------------------------------------------------------*/
capi_v2_err_t capi_v2_sp_rx_init(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

   if(NULL == _pif || NULL == init_set_properties)
   {
      FARF(ERROR, "CAPIv2 SP_RX: "
           "Init received bad pointer, 0x%lx, 0x%lx",
           (uint32_t )_pif, (uint32_t )init_set_properties);

      return CAPI_V2_EBADPARAM;
   }

   int8_t* ptr = (int8_t*)_pif;
   capi_v2_sp_rx_t *me_ptr = (capi_v2_sp_rx_t *)ptr;

   // Initialize the memory to 0
   memset((void *)me_ptr, 0, sizeof(capi_v2_sp_rx_t));

   // Disable the module by default
   me_ptr->enable_flag = 0;

   me_ptr->vtbl.vtbl_ptr = &vtbl;

   //should contain  EVENT_CALLBACK_INFO, PORT_INFO
   capi_v2_result = capi_v2_sp_rx_process_set_properties(me_ptr, init_set_properties);

   //Ignoring non-fatal error code.
   capi_v2_result ^= (capi_v2_result & CAPI_V2_EUNSUPPORTED);
   if(CAPI_V2_FAILED(capi_v2_result))
   {
      return capi_v2_result;
   }

   FARF(HIGH, "CAPIv2 SP_RX: Init done!");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_process
 DESCRIPTION: Processes an input buffer and generates an output buffer.
 -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_rx_process(capi_v2_t* _pif, capi_v2_stream_data_t* input[],
                                           capi_v2_stream_data_t* output[])
{

   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_sp_rx_t* me_ptr = (capi_v2_sp_rx_t*)(_pif);
   sp_v2_tx_fb_data_payload_t *fb_data_payload_ptr = NULL;
   int16_t *vsen_buf_ptr[MAX_SP_SPKRS] =
   { NULL };
   int16_t *isen_buf_ptr[MAX_SP_SPKRS] =
   { NULL };
   assert(me_ptr);
   assert(input);
   assert(output);

   FARF(ERROR, "capi_v2_sp_rx_process: Process");
   
   /* The processing block size should be a minimum of
    *   1. input_actual_data_length/decimation_factor
    *   2. output_max_data_length
    *
    *   Also, the input_actual_data_length and output_max_data_length
    *   is in bytes. Assuming bits per sample is 16 or 2 bytes per sample,
    *   num of samples are calculated by bitwise shifting by 1 byte to the right.
    * */
   uint32_t num_samples = Q6_R_min_RR((input[0]->buf_ptr->actual_data_len >> 1),
                                      (output[0]->buf_ptr->max_data_len >> 1));

   if(me_ptr->enable_flag)
   {
      /* Setting up the CX data */
      int8_t *fb_data_ptr[MAX_SP_SPKRS] =
      { 0 };

      void *fb_data_buf_rcvd_ptr[MAX_SP_SPKRS] =
      { 0 };

      void *fb_data_buf_client_ptr[MAX_SP_SPKRS] =
      { 0 };

      /* If client info rcvd then feedback path might be enabled, look for fb data buff from Tx path*/
      if(me_ptr->client_list_ptr_ptr)
      {
         capi_v2_sp_rx_get_fb_data(me_ptr, fb_data_ptr, fb_data_buf_rcvd_ptr,
                                   fb_data_buf_client_ptr);
      }

      sp_rx_process(&me_ptr->lib_mem, vsen_buf_ptr, isen_buf_ptr, num_samples,
                    fb_data_payload_ptr->fb_data_ptr);

      /* Loop through SP client list and return the fb data buffers */
      if(me_ptr->client_list_ptr_ptr)
      {
         capi_v2_sp_rx_return_fb_data(me_ptr, fb_data_buf_rcvd_ptr, fb_data_buf_client_ptr);
      }
   }

   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_end
 DESCRIPTION: Returns the library to the uninitialized state and frees the
 memory that was allocated by init(). This function also frees the virtual
 function table.
 -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_rx_end(capi_v2_t* _pif)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   if(NULL == _pif)
   {
      FARF(ERROR, "CAPIv2 SP_RX: End received bad pointer, 0x%lx", (uint32_t )_pif);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_sp_rx_t* me_ptr = (capi_v2_sp_rx_t*)(_pif);

   capi_v2_sp_rx_release_memory(me_ptr);

   FARF(HIGH, "CAPIv2 SP_RX: End done");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_set_param
 DESCRIPTION: Sets either a parameter value or a parameter structure containing
 multiple parameters. In the event of a failure, the appropriate error code is
 returned.
 -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_rx_set_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr)

{
   if(NULL == _pif || NULL == params_ptr)
   {
      FARF(ERROR, "CAPIv2 SP_RX: Set param received bad pointer, 0x%lx, 0x%lx", (uint32_t )_pif,
           (uint32_t )params_ptr);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_sp_rx_t* me_ptr = (capi_v2_sp_rx_t*)(_pif);

   switch(param_id)
   {
      case CAPI_V2_SP_RX_CFG_1_PARAM_ID:
      {
         if(params_ptr->actual_data_len >= sizeof(sp_rx_lib_config_1_t))
         {
            sp_rx_lib_config_1_t* cfg_ptr = (sp_rx_lib_config_1_t*)(params_ptr->data_ptr);

            std_memscpy(&me_ptr->lib_config.cfg1, sizeof(sp_rx_lib_config_1_t), cfg_ptr,
                    sizeof(sp_rx_lib_config_1_t));

            FARF(HIGH, "CAPIv2 SP_RX : CAPI_V2_SP_RX_CFG_1_PARAM_ID rcvd");
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_RX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_SP_RX_ENABLE:
      {
         if(params_ptr->actual_data_len >= sizeof(sp_rx_enable_cfg_t))
         {
            sp_rx_enable_cfg_t* cfg_ptr = (sp_rx_enable_cfg_t*)(params_ptr->data_ptr);

            me_ptr->enable_flag = cfg_ptr->enable_flag;

            FARF(HIGH, "CAPIv2 SP_RX : CAPI_V2_PARAM_ID_SP_RX_ENABLE rcvd %lu",
                 me_ptr->enable_flag);
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_RX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      /* This is a param of FWK_EXTN_FEEDBACK, it will be used to iterate through possible multiple modules in
       * Tx path that want to send feedback data to this Rx module. The client_list_ptr_ptr abstracts all the
       * information needed to perform this buffer passing from Tx to Rx and and return the data to Tx path once
       * it has been used.*/
      case CAPI_V2_PARAM_ID_CLIENT_INFO:
      {
         if(params_ptr->actual_data_len >= sizeof(void *))
         {
            /* Cache the Feedback info */
            me_ptr->client_list_ptr_ptr = (void **)params_ptr->data_ptr;

            FARF(HIGH, "CAPIv2 SP_RX : CAPI_V2_PARAM_ID_CLIENT_INFO rcvd ");
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_RX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      /* These are the params in FWK_EXTN_CDC_INTERRUPT. The service layer would use this to send codec interrupts
       * to this module as and when raised by the codec. The PCM levels would also be copied as part of these IDs.
       * */
      case CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP:
      case CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP:
      {
         if(params_ptr->actual_data_len >= sizeof(capi_v2_cdc_clip_pcm_info_t))
         {
            capi_v2_result = capi_v2_sp_v2_rx_intr_handler(me_ptr, param_id);
            capi_v2_result = capi_v2_sp_v2_rx_copy_pcmlevels_to_libbuf(me_ptr, param_id,
                                                                       params_ptr->data_ptr);
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_RX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }

         break;
      }

      /* These are the also params in FWK_EXTN_CDC_INTERRUPT. The service layer would use this to send codec interrupts
       * to this module as and when raised by the codec.
       * */
      case CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK:
      case CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE:
      {
         capi_v2_result = capi_v2_sp_v2_rx_intr_handler(me_ptr, param_id);
         break;
      }

      default:
         FARF(ERROR, "CAPIv2 SP_RX: Set unsupported param ID 0x%x", (int )param_id);
         CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EBADPARAM);
   }

   FARF(HIGH, "CAPIv2 SP_RX: Set param done");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_get_param
 DESCRIPTION: Gets either a parameter value or a parameter structure
 containing multiple parameters. In the event of a failure, the appropriate
 error code is returned.
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_rx_get_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr)
{
   if(NULL == _pif || NULL == params_ptr)
   {
      FARF(ERROR, "CAPIv2 SP_RX: Get param received bad pointer, 0x%lx, 0x%lx", (uint32_t )_pif,
           (uint32_t )params_ptr);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_sp_rx_t* me_ptr = (capi_v2_sp_rx_t*)(_pif);
   //void *param_payload_ptr  = (void *)(params_ptr->data_ptr);

   switch(param_id)
   {
      case CAPI_V2_SP_RX_CFG_1_PARAM_ID:
      {
         if(params_ptr->max_data_len >= sizeof(sp_rx_lib_config_1_t))
         {
            sp_rx_lib_config_1_t* cfg_ptr = (sp_rx_lib_config_1_t*)(params_ptr->data_ptr);

            std_memscpy(cfg_ptr, sizeof(sp_rx_lib_config_1_t), &me_ptr->lib_config.cfg1,
                    sizeof(sp_rx_lib_config_1_t));

            params_ptr->actual_data_len = sizeof(sp_rx_lib_config_1_t);
            FARF(HIGH,
                 "<<get_param>> CAPI_V2_SP_RX_CFG_1_PARAM_ID                                    ");
         }
         else
         {
            FARF(ERROR, "<<get_param>> Bad param size %lu                                    ",
                 params_ptr->max_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_SP_RX_ENABLE:
      {
         if(params_ptr->max_data_len >= sizeof(sp_rx_enable_cfg_t))
         {
            sp_rx_enable_cfg_t* cfg_ptr = (sp_rx_enable_cfg_t*)(params_ptr->data_ptr);

            cfg_ptr->enable_flag = me_ptr->enable_flag;

            params_ptr->actual_data_len = sizeof(sp_rx_enable_cfg_t);
            FARF(HIGH, "<<get_param>> Enable/Disable %lu                                    ",
                 cfg_ptr->enable_flag);
         }
         else
         {
            FARF(ERROR, "<<get_param>> Bad param size %lu                                    ",
                 params_ptr->max_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_CDC_INT_LIST:
      {
         if(params_ptr->max_data_len >= sizeof(capi_v2_cdc_int_t) * me_ptr->num_cdc_int)
         {
            // Copy the requested amount of data to the o/p buffer
            capi_v2_cdc_int_t *cfg_ptr = (capi_v2_cdc_int_t *)params_ptr->data_ptr;

            if(me_ptr->num_spkr == 2)
            {
               cfg_ptr[0].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP;
               cfg_ptr[1].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_SPKR2_AUDIO_CLIP;
               cfg_ptr[2].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK;
               cfg_ptr[3].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE;
            }
            else
            {
               cfg_ptr[0].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_SPKR_AUDIO_CLIP;
               cfg_ptr[1].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_VBAT_ATTACK;
               cfg_ptr[2].cdc_int = (uint32_t)CAPI_V2_PARAM_ID_CDC_INT_VBAT_RELEASE;
            }

            params_ptr->actual_data_len = sizeof(capi_v2_cdc_int_t) * me_ptr->num_cdc_int;

            FARF(HIGH,
                 "<<get_param>> CAPI_V2_PARAM_ID_CDC_INT_LIST                                    ");
         }
         else
         {
            FARF(ERROR, "<<get_param>> Bad param size %lu                                    ",
                 params_ptr->max_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_NUM_NEEDED_CDC_INT:
      {
         if(params_ptr->max_data_len >= sizeof(capi_v2_num_needed_cdc_int_t))
         {
            // Copy the requested amount of data to the o/p buffer
            capi_v2_num_needed_cdc_int_t *cfg_ptr =
                  (capi_v2_num_needed_cdc_int_t *)params_ptr->data_ptr;

            if(me_ptr->num_spkr == 2)
            {
               cfg_ptr->num_cdc_int = 4;
            }
            else
            {
               cfg_ptr->num_cdc_int = 3;
            }

            me_ptr->num_cdc_int = cfg_ptr->num_cdc_int;

            params_ptr->actual_data_len = sizeof(capi_v2_num_needed_cdc_int_t);
            FARF(HIGH,
                 "<<get_param>> CAPI_V2_PARAM_ID_NUM_NEEDED_CDC_INT                                    ");
         }
         else
         {
            FARF(ERROR, "<<get_param>> Bad param size %lu                                    ",
                 params_ptr->max_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      default:
      {
         FARF(ERROR, "CAPIv2 SP_RX: Get unsupported param ID 0x%x", (int )param_id);
         CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EBADPARAM);
      }
   }

   FARF(HIGH, "CAPIv2 SP_RX: Get param done");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_set_properties
 DESCRIPTION: Function to set the properties for the SP_RX module
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_rx_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   if(NULL == _pif)
   {
      FARF(ERROR, "CAPIv2 SP_RX: Set properties received bad pointer, 0x%lx", (uint32_t )_pif);
      return CAPI_V2_EBADPARAM;
   }
   capi_v2_sp_rx_t *me_ptr = (capi_v2_sp_rx_t *)_pif;
   capi_v2_result = capi_v2_sp_rx_process_set_properties(me_ptr, props_ptr);
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_rx_get_properties
 DESCRIPTION: Function to get the properties for the SP_RX module
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_rx_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   if(NULL == _pif)
   {
      FARF(ERROR, "CAPIv2 SP_RX: Get properties received bad pointer, 0x%lx", (uint32_t )_pif);
      return CAPI_V2_EBADPARAM;
   }
   capi_v2_sp_rx_t *me_ptr = (capi_v2_sp_rx_t *)_pif;
   capi_v2_result = capi_v2_sp_rx_process_get_properties(me_ptr, props_ptr);
   return capi_v2_result;
}

