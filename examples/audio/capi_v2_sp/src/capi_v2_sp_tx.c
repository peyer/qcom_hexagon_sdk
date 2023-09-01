/* ======================================================================== */
/**
 @file capi_v2_sp_tx.cpp

 C source file to implement the CAPIv2 interface for Speaker Protection Tx module.

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
#include "capi_v2_sp_tx.h"
#include "capi_v2_sp_tx_utils.h"
#include <assert.h>
#include <hexagon_protos.h>
#include "AEEstd.h"

/*------------------------------------------------------------------------
 * Explanation of the Speaker Protection Tx module
 *
 *
 * Here the call flow for this module is discussed.
 * -> The capi_v2_sp_Tx_get_static_properties() is called from the service layer to
 *    obtain the memory requirement of the CAPIv2 layer.
 * -> It is then followed by the capi_v2_sp_tx_init() from service layer to initialize the
 *    memory for the module and populate CAPIv2 function table.
 * -> Now we are ready to receive library configuration parameters through set param and set properties.
 * -> Since, this module needs to support one framework extensions: Feedback. The service
 *    will query for these and it should be set from here.
 * -> Now, we have set-up the needed feedback from the service layer. We will
 *    send the CAPI_V2_INPUT_MEDIA_FORMAT to trigger the start of the module processing. It means data pumping
 *    is going to start and the CAPIv2 layer needs to be ready for the data buffer processing.
 * -> Once we receive the input media format, we need to initilize the library with any params received
 *    till now, initialize the feedback path and raise the KPPS and BW requirements.
 * -> Now the process function will be called when service receives new data buffers.
 * -----------------------------------------------------------------------*/




/*------------------------------------------------------------------------
 * Static declarations
 * -----------------------------------------------------------------------*/

static capi_v2_err_t capi_v2_sp_tx_process(capi_v2_t* _pif, capi_v2_stream_data_t* input[],
                                           capi_v2_stream_data_t* output[]);

static capi_v2_err_t capi_v2_sp_tx_end(capi_v2_t* _pif);

static capi_v2_err_t capi_v2_sp_tx_set_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr);

static capi_v2_err_t capi_v2_sp_tx_get_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr);

static capi_v2_err_t capi_v2_sp_tx_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr);

static capi_v2_err_t capi_v2_sp_tx_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr);

/** Virtual table for capi_v2_sp_tx
 */
static capi_v2_vtbl_t vtbl =
{
      capi_v2_sp_tx_process,
      capi_v2_sp_tx_end,
      capi_v2_sp_tx_set_param,
      capi_v2_sp_tx_get_param,
      capi_v2_sp_tx_set_properties,
      capi_v2_sp_tx_get_properties
};

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_get_static_properties
 DESCRIPTION: Function to get the static properties of SP Tx module
 -----------------------------------------------------------------------*/
capi_v2_err_t capi_v2_sp_tx_get_static_properties(capi_v2_proplist_t *init_set_properties,
                                                  capi_v2_proplist_t *static_properties)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

   if(NULL != static_properties)
   {
      capi_v2_result = capi_v2_sp_tx_process_get_properties((capi_v2_sp_tx_t*)NULL,
                                                            static_properties);
      if(CAPI_V2_FAILED(capi_v2_result))
      {
         return capi_v2_result;
      }
   }

   if(NULL != init_set_properties)
   {
      FARF(HIGH, "CAPIv2 SP_TX: get static properties ignoring init_set_properties!");
   }

   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_init
 DESCRIPTION: Initialize the CAPIv2 SP_TX module and library.
 This function can allocate memory.
 -----------------------------------------------------------------------*/
capi_v2_err_t capi_v2_sp_tx_init(capi_v2_t* _pif, capi_v2_proplist_t *init_set_properties)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;

   if(NULL == _pif || NULL == init_set_properties)
   {
      FARF(ERROR, "CAPIv2 SP_TX: "
           "Init received bad pointer, 0x%lx, 0x%lx",
           (uint32_t )_pif, (uint32_t )init_set_properties);

      return CAPI_V2_EBADPARAM;
   }

   int8_t* ptr = (int8_t*)_pif;
   capi_v2_sp_tx_t *me_ptr = (capi_v2_sp_tx_t *)ptr;

   // Initialize the memory to 0
   memset((void *)me_ptr, 0, sizeof(capi_v2_sp_tx_t));

   // Disable the module by default
   me_ptr->enable_flag = 0;

   me_ptr->vtbl.vtbl_ptr = &vtbl;

   //should contain  EVENT_CALLBACK_INFO, PORT_INFO
   capi_v2_result = capi_v2_sp_tx_process_set_properties(me_ptr, init_set_properties);

   //Ignoring non-fatal error code.
   capi_v2_result ^= (capi_v2_result & CAPI_V2_EUNSUPPORTED);
   if(CAPI_V2_FAILED(capi_v2_result))
   {
      return capi_v2_result;
   }

   FARF(HIGH, "CAPIv2 SP_TX: Init done!");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_process
 DESCRIPTION: Processes an input buffer and generates an output buffer.
 -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_tx_process(capi_v2_t* _pif, capi_v2_stream_data_t* input[],
                                           capi_v2_stream_data_t* output[])
{

   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_sp_tx_t* me_ptr = (capi_v2_sp_tx_t*)(_pif);
   void *fb_buff_ptr = NULL;
   sp_v2_tx_fb_data_payload_t *fb_data_payload_ptr = NULL;
   int16_t *vsen_buf_ptr[MAX_SP_SPKRS] =
   { NULL };
   int16_t *isen_buf_ptr[MAX_SP_SPKRS] =
   { NULL };
   int8_t i = 0;
   assert(me_ptr);
   assert(input);
   assert(output);
   FARF(ERROR, "capi_v2_sp_tx_process: Process");
   
   /* Get the number of samples contained in the buffer */
   uint32_t num_samples = Q6_R_min_RR((input[0]->buf_ptr->actual_data_len >> 1),
                                      (output[0]->buf_ptr->max_data_len >> 1));

   if(me_ptr->enable_flag)
   {
      /* Setting up the incoming buffer ptrs based on the feedback mapping sent during
       * configuration CAPI_V2_PARAM_ID_FB_PATH_INFO */
      for(i = 0; i < me_ptr->feedback_path_info.feedback_cfg.num_channels; i++)
      {
         switch(me_ptr->feedback_path_info.feedback_cfg.chan_info[i])
         {
            case AFE_FBSP_VSENS_LEFT_CHAN:
            {
               vsen_buf_ptr[0] = (int16_t *)input[0]->buf_ptr[i].data_ptr;
               break;
            }
            case AFE_FBSP_ISENS_LEFT_CHAN:
            {
               isen_buf_ptr[0] = (int16_t *)input[0]->buf_ptr[i].data_ptr;
               break;
            }
            case AFE_FBSP_VSENS_RIGHT_CHAN:
            {
               vsen_buf_ptr[1] = (int16_t *)input[0]->buf_ptr[i].data_ptr;
               break;
            }
            case AFE_FBSP_ISENS_RIGHT_CHAN:
            {
               isen_buf_ptr[1] = (int16_t *)input[0]->buf_ptr[i].data_ptr;
               break;
            }
         }
      }

      /*If spkr cfg is SPKR_R, then we need to switch the buffs */
      if(SPKR_R == me_ptr->spkr_cfg)
      {
         vsen_buf_ptr[0] = vsen_buf_ptr[1];
         isen_buf_ptr[0] = isen_buf_ptr[1];
         vsen_buf_ptr[1] = NULL;
         isen_buf_ptr[1] = NULL;
      }

      capi_v2_sp_tx_pop_fb_data_from_buffer_q(me_ptr, &fb_buff_ptr);

      /* Only need to process if FB buff is available, else no need */
      if(fb_buff_ptr)
      {
         FARF(HIGH, "CAPIv2 SP_TX: Got FB buffer from bufQ!");

         fb_data_payload_ptr = (sp_v2_tx_fb_data_payload_t *)fb_buff_ptr;

         sp_tx_process(&me_ptr->lib_mem, vsen_buf_ptr, isen_buf_ptr, num_samples,
                       fb_data_payload_ptr->fb_data_ptr);

         capi_v2_sp_tx_push_fb_data_to_data_q(me_ptr, fb_buff_ptr);
      }
   }

   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_end
 DESCRIPTION: Returns the library to the uninitialized state and frees the
 memory that was allocated by init(). This function also frees the virtual
 function table.
 -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_tx_end(capi_v2_t* _pif)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   if(NULL == _pif)
   {
      FARF(ERROR, "CAPIv2 SP_TX: End received bad pointer, 0x%lx", (uint32_t )_pif);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_sp_tx_t* me_ptr = (capi_v2_sp_tx_t*)(_pif);

   capi_v2_sp_tx_release_memory(me_ptr);

   FARF(HIGH, "CAPIv2 SP_TX: End done");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_set_param
 DESCRIPTION: Sets either a parameter value or a parameter structure containing
 multiple parameters. In the event of a failure, the appropriate error code is
 returned.
 -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_tx_set_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr)

{
   if(NULL == _pif || NULL == params_ptr)
   {
      FARF(ERROR, "CAPIv2 SP_TX: Set param received bad pointer, 0x%lx, 0x%lx", (uint32_t )_pif,
           (uint32_t )params_ptr);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_sp_tx_t* me_ptr = (capi_v2_sp_tx_t*)(_pif);

   switch(param_id)
   {
      case CAPI_V2_SP_TX_CFG_1_PARAM_ID:
      {
         if(params_ptr->actual_data_len >= sizeof(sp_tx_lib_config_1_t))
         {
            sp_tx_lib_config_1_t* cfg_ptr = (sp_tx_lib_config_1_t*)(params_ptr->data_ptr);

            std_memscpy(&me_ptr->lib_config.cfg1, sizeof(sp_tx_lib_config_1_t), cfg_ptr,
                    sizeof(sp_tx_lib_config_1_t));

            FARF(HIGH, "CAPIv2 SP_TX : CAPI_V2_SP_TX_CFG_1_PARAM_ID rcvd");
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_TX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_SP_TX_ENABLE:
      {
         if(params_ptr->actual_data_len >= sizeof(sp_tx_enable_cfg_t))
         {
            sp_tx_enable_cfg_t* cfg_ptr = (sp_tx_enable_cfg_t*)(params_ptr->data_ptr);

            me_ptr->enable_flag = cfg_ptr->enable_flag;

            FARF(HIGH, "CAPIv2 SP_TX : CAPI_V2_PARAM_ID_SP_TX_ENABLE rcvd %lu",
                 me_ptr->enable_flag);
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_TX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_FB_PATH_INFO:
      {
         if(params_ptr->actual_data_len >= sizeof(feedback_info_t))
         {
            std_memscpy(&me_ptr->feedback_path_info, sizeof(feedback_info_t), params_ptr->data_ptr,
                    params_ptr->actual_data_len);

            FARF(HIGH, "CAPIv2 SP_TX : CAPI_V2_PARAM_ID_FB_PATH_INFO rcvd ");
         }
         else
         {
            FARF(ERROR, "CAPIv2 SP_TX : <<set_param>> Bad param size %lu  ",
                 params_ptr->actual_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      default:
         FARF(ERROR, "CAPIv2 SP_TX: Set unsupported param ID 0x%x", (int )param_id);
         CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EBADPARAM);
   }

   FARF(HIGH, "CAPIv2 SP_TX: Set param done");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_get_param
 DESCRIPTION: Gets either a parameter value or a parameter structure
 containing multiple parameters. In the event of a failure, the appropriate
 error code is returned.
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_tx_get_param(capi_v2_t* _pif, uint32_t param_id,
                                             const capi_v2_port_info_t *port_info_ptr,
                                             capi_v2_buf_t *params_ptr)
{
   if(NULL == _pif || NULL == params_ptr)
   {
      FARF(ERROR, "CAPIv2 SP_TX: Get param received bad pointer, 0x%lx, 0x%lx", (uint32_t )_pif,
           (uint32_t )params_ptr);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   capi_v2_sp_tx_t* me_ptr = (capi_v2_sp_tx_t*)(_pif);
   //void *param_payload_ptr  = (void *)(params_ptr->data_ptr);

   switch(param_id)
   {
      case CAPI_V2_SP_TX_CFG_1_PARAM_ID:
      {
         if(params_ptr->max_data_len >= sizeof(sp_tx_lib_config_1_t))
         {
            sp_tx_lib_config_1_t* cfg_ptr = (sp_tx_lib_config_1_t*)(params_ptr->data_ptr);

            std_memscpy(cfg_ptr, sizeof(sp_tx_lib_config_1_t), &me_ptr->lib_config.cfg1,
                    sizeof(sp_tx_lib_config_1_t));

            params_ptr->actual_data_len = sizeof(sp_tx_lib_config_1_t);
            FARF(HIGH,
                 "<<get_param>> CAPI_V2_SP_TX_CFG_1_PARAM_ID                                    ");
         }
         else
         {
            FARF(ERROR, "<<get_param>> Bad param size %lu                                    ",
                 params_ptr->max_data_len);
            return CAPI_V2_ENEEDMORE;
         }
         break;
      }

      case CAPI_V2_PARAM_ID_SP_TX_ENABLE:
      {
         if(params_ptr->max_data_len >= sizeof(sp_tx_enable_cfg_t))
         {
            sp_tx_enable_cfg_t* cfg_ptr = (sp_tx_enable_cfg_t*)(params_ptr->data_ptr);

            cfg_ptr->enable_flag = me_ptr->enable_flag;

            params_ptr->actual_data_len = sizeof(sp_tx_enable_cfg_t);
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

      default:
      {
         FARF(ERROR, "CAPIv2 SP_TX: Get unsupported param ID 0x%x", (int )param_id);
         CAPI_V2_SET_ERROR(capi_v2_result, CAPI_V2_EBADPARAM);
      }
   }

   FARF(HIGH, "CAPIv2 SP_TX: Get param done");
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_set_properties
 DESCRIPTION: Function to set the properties for the SP_TX module
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_tx_set_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   if(NULL == _pif)
   {
      FARF(ERROR, "CAPIv2 SP_TX: Set properties received bad pointer, 0x%lx", (uint32_t )_pif);
      return CAPI_V2_EBADPARAM;
   }

   capi_v2_sp_tx_t *me_ptr = (capi_v2_sp_tx_t *)_pif;
   capi_v2_result = capi_v2_sp_tx_process_set_properties(me_ptr, props_ptr);
   return capi_v2_result;
}

/*------------------------------------------------------------------------
 Function name: capi_v2_sp_tx_get_properties
 DESCRIPTION: Function to get the properties for the SP_TX module
 * -----------------------------------------------------------------------*/
static capi_v2_err_t capi_v2_sp_tx_get_properties(capi_v2_t* _pif, capi_v2_proplist_t *props_ptr)
{
   capi_v2_err_t capi_v2_result = CAPI_V2_EOK;
   if(NULL == _pif)
   {
      FARF(ERROR, "CAPIv2 SP_TX: Get properties received bad pointer, 0x%lx", (uint32_t )_pif);
      return CAPI_V2_EBADPARAM;
   }
   capi_v2_sp_tx_t *me_ptr = (capi_v2_sp_tx_t *)_pif;
   capi_v2_result = capi_v2_sp_tx_process_get_properties(me_ptr, props_ptr);
   return capi_v2_result;
}

