////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////
#ifndef _HVX_APP_ADD_CONSTANT_H_
#define _HVX_APP_ADD_CONSTANT_H_

/* Internal context */
typedef struct {
    unsigned int num_lines_processed;
    unsigned int num_lines_total;
    void *client_p;
    framework_callback_functns_t *callbacks_p;
}hvx_add_constant_context_t;

void hvx_add_constant_asm(void* dst,
                            void* src,
                            unsigned int linesize,
                            unsigned int constant,
                            int shift);

void hvx_add_constant_process_lines(
        void *app_context_p,
        void* dst_p,
        void* src_p,
        process_lines_params_t* params_p);

int hvx_add_constant_get_config(void *app_context_p,
        config_from_client_t* config_in_p,
        config_from_app_t* config_out_p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Termination function.
///
/// @param app_context_p: pointer to the app context (originally allocated by the app at entry point).
///
/// @return If success, return HVX_SUCCESS.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hvx_add_constant_terminate(void *app_context_p);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// app_entry
///
/// @brief  Function to be called by framework to obtain callback function pointers from the user App.
///
/// @param app_callbacks_p: pointer to structure containing callback function pointers. Pointers to the callback functions must
///                     be updated by the user App.
///
/// @param client_p: pointer to the caller. Must be used when calling framework callback functions.
///
/// @param client_callbacks_p: pointer to structure containing framework callback functions.
///
/// @return None
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void app_entry(
                app_callbac_functns_t *app_callbacks_p,
                void** app_context_pp,
                void *client_p,
                framework_callback_functns_t *client_callbacks_p);
#endif
