/***************************************************************************
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 ****************************************************************************/

#include "dsp_streamer_common.h"

#ifndef _DSP_STREAMER_IF_APP_H_
#define _DSP_STREAMER_IF_APP_H_

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// config_from_client_t
///
/// @brief  Configuration data set by the client (consumed by the App).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct config_from_client_t{
    hvx_IFE_mode_t              ife_mode;       /**< Single VFE if equals to HVX_VFE0 or HVX_VFE1, dual VFE otherwise */
    int                         id;             /**< If DUAL IFE mode, identifies the streamer that this App instance is
                                                associated with.Can take values in [0;MAX_NUM_STREAMER]*/
    tapping_point_select_t      tapping_point;  /**< Tapping point within the IFE pipeline */
    ife_frame_info_t            frame_info;     /**< Input/output info. */
    unsigned long               ife_clk_freq;   /**< IFE clock frequency. */
    unsigned int                frm_rate;       /**< Streamed frame rate. */
    unsigned int                num_frms_streamed;  /* Total number of frames streamed so far */
    unsigned int                num_lines_processed;/* Total number of lines processed so far */
    unsigned int                sof_with_rup;       /* If =1, the SOF is accompanied by a register update (RUP). */
    void                        *metadata_p;        /* Metadata buffer containing data sent by the HLOS client. */
    unsigned long long          time_at_last_sof;   /**< System clk (in us) polled just after SOF. */
}config_from_client_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// config_from_app_t
///
/// @brief  Configuration data set by the App (consumed by the client).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct config_from_app_t{
    streamer_request_config_t   streamer_config;
    dsp_power_voting_t          power_votes;
}config_from_app_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// config_data_t
///
/// @brief  Structure containing pointers to configuration data associated with a given app instance.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    void                        *app_ctx_p;
    config_from_client_t        *client_cfg_p; /* Contents are read-only after static configuration stage */
    config_from_app_t           *app_cfg_p;     /* Contents are read-only after static configuration stage */
}config_data_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// process_lines_params_t
///
/// @brief  Structure Provided at each process() call.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    config_from_client_t        *client_cfg_p;          /* Contents are read-only after static configuration stage */
    config_from_app_t           *app_cfg_p;             /* Contents are read-only after static configuration stage */
}process_lines_params_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// fp_sync_flag_get
///
/// @brief  Poll sync_flag.
///
/// @param client_p: client handle.
///
/// @return: Value of sync_flag.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef uint32 (*fp_sync_flag_get)(void *client_p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// fp_buf_get_empty
///
/// @brief  Obtain an empty buffer.
///
/// @param client_p: client handle.
///
/// @param buf_p: pointer to an empty buffer. The App can write data to this buffer.
///
/// @param blocking: if =1, this call will block until a buffer is available.
///
/// @return: evt_return_type_t.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int (*fp_buf_get_empty)(void *client_p, void* buf_p, uint32 blocking);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// fp_buf_send_full
///
/// @brief  Send a full buffer to the client.
///
/// @param client_p: client handle.
///
/// @param buf_p: pointer to a buffer which has been filled by the App.
///
/// @param blocking: if =1, this call will block until the buffer is sent.
///
/// @return: evt_return_type_t.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int (*fp_buf_send_full)(void *client_p, void* buf_p, uint32 blocking);

typedef int (*fp_prof_generation_init    )(void);
typedef int (*fp_prof_generation_deinit  )(void);
typedef int (*fp_prof_generation_reset   )(const char* func_name);
typedef int (*fp_prof_generation_dump    )(void);
typedef int (*fp_prof_collection_init    )(dsp_streamer_buf_desc_t *buf_p);
typedef int (*fp_prof_collection_deinit  )(void);
typedef int (*fp_prof_collection_load_evt)(unsigned int event_num);
typedef int (*fp_prof_collection_save_evt)(unsigned int last_frm);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// framework_callback_functns_t
///
/// @brief  Framework function pointers.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct framework_callbac_functns {
    fp_sync_flag_get            sync_flag_get_func_ptr; /*The App will call this to obtain value of sync flag.*/
    fp_buf_get_empty            buf_get_empty_func_ptr; /*The App will call this to receive an empty buffer.*/
    fp_buf_send_full            buf_send_full_func_ptr; /*The App will call this to send a filled buffer to the consumer.*/
    /* Diagnostic function callbacks */
    fp_prof_generation_init     prof_generation_init_func_ptr;
    fp_prof_generation_deinit   prof_generation_deinit_func_ptr;
    fp_prof_generation_reset    prof_generation_reset_func_ptr;
    fp_prof_generation_dump     prof_generation_dump_func_ptr;
    fp_prof_collection_init     prof_collection_init_func_ptr;
    fp_prof_collection_deinit   prof_collection_deinit_func_ptr;
    fp_prof_collection_load_evt prof_collection_load_evt_func_ptr;
    fp_prof_collection_save_evt prof_collection_save_evt_func_ptr;
}framework_callback_functns_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// get_config
///
/// @brief  Function to be called by framework to obtain configuration data from the user App.
///
/// @param app_context_p: pointer to the app context (originally allocated by the app at entry point).
///
/// @param config_in_p: pointer to the (input) configuration data set by the client.
///
/// @param config_out_p: pointer to the (output) configuration data set by the app and returned to the client.
///
/// @return None
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int (*fp_get_config_t)(
                void* app_context_p,
                config_from_client_t* config_in_p,
                config_from_app_t* config_out_p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// process_lines
///
/// @brief  Process a number of lines.
///
/// @param app_context_p: pointer to the app context (originally allocated by the app at entry point).
///
/// @param dst_p: pointer to destination buffer (
///
/// @param
///
/// @param
///
///
/// @return None.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*fp_process_lines_t)(
                            void* app_context_p,
                            void* dst_p,
                            void* src_p,
                            process_lines_params_t* params_p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// work_loop
///
/// @brief  One instance of this work look is executed by the DSP framework on a separate thread. One instance per IFE.
///
/// @param  p: pointer to payload.
///
/// @return None.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*fp_work_loop_t)(void *p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// fp_app_exit
///
/// @brief  Exit point. The DSP framework call this when closing the session. The app should deallocate the context.
///
/// @param app_context_p: pointer to the app context (originally allocated by the app at entry point).
///
/// @return None.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*fp_app_exit_t)(void *app_context_p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// app_callbac_functns_t
///
/// @brief  Structure containing pointers to callback functions implemented by the app.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct app_callbac_functns {
    fp_get_config_t         get_config_func_ptr;        /* Framework will call this to obtain configuration data. */
    fp_process_lines_t      process_lines_func_ptr;     /* Framework will call this to process a given number of lines. */
    fp_work_loop_t          work_loop_func_ptr;         /* Framework will call this to start a custom workloop in a separate thread. */
    fp_app_exit_t           exit_func_ptr;              /* Framework will call this at termination stage. */
}app_callbac_functns_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// fp_app_entry
///
/// @brief  Initialization function. The DSP framework call this to obtain pointers to callback functions implemented by the App.
///
/// @param  app_callbacks_p: pointer to structure containing pointers to custom callback functions. Updated by the App.
///
/// @param app_context_pp: dbl-pointer to the app context (which is allocated within this call). The framework will return the
///                         pointer to this same app context in all subsequent calls.
/// @param client_p: pointer to the caller. Must be used when calling framework callback functions.
///
/// @param framework_callbacks_p: pointer to structure containing client framework callback functions.
///
/// @return None.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*fp_app_entry_t)(
                app_callbac_functns_t *app_callbacks_p,
                void **app_context_pp,
                void *client_p,
                framework_callback_functns_t *framework_callbacks_p);

#endif //_DSP_STREAMER_IF_APP_H_
