#ifndef CAPI_V2_PROFILE_H
#define CAPI_V2_PROFILE_H
/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "test_profile.h"

#define SESSION_SETPARAM	1
#define	SESSION_GETPARAM	2
#define SESSION_PROCESS		3
#define SESSION_SETPROP     4
#define	SESSION_GETPROP     5

#ifdef __PROFILE
	#define START_PROCESS_PROFILE()		start_profiling(SESSION_PROCESS);
	#define STOP_PROCESS_PROFILE()		stop_profiling(SESSION_PROCESS, 0);
	#define START_GETPARAM_PROFILE()	reset_profiling(SESSION_GETPARAM);	\
											start_profiling(SESSION_GETPARAM);
	#define STOP_GETPARAM_PROFILE()		stop_profiling(SESSION_GETPARAM, 1);
	#define START_SETPARAM_PROFILE()	reset_profiling(SESSION_SETPARAM);	\
											start_profiling(SESSION_SETPARAM);
	#define STOP_SETPARAM_PROFILE()		stop_profiling(SESSION_SETPARAM, 1);
	#define START_GETPROP_PROFILE()		reset_profiling(SESSION_GETPROP);	\
											start_profiling(SESSION_GETPROP);
	#define STOP_GETPROP_PROFILE()		stop_profiling(SESSION_GETPROP, 1);
	#define START_SETPROP_PROFILE()	reset_profiling(SESSION_SETPROP);	\
											start_profiling(SESSION_SETPROP);
	#define STOP_SETPROP_PROFILE()		stop_profiling(SESSION_SETPROP, 1);
	#define RESET_PROFILE_RESULT(session_id)	reset_profiling(session_id);
	#define PRINT_PROFILE_RESULT(session_id)	print_profile_result(session_id);
	#define GET_PROFILE_CYCLES(session_id) get_profiling_cycles(session_id);
	#define ADD_PROFILE_ATTR(session_id, name, value) add_session_attributes(session_id, name, value);
#else
	#define START_PROCESS_PROFILE()
	#define STOP_PROCESS_PROFILE()
	#define START_GETPARAM_PROFILE()
	#define STOP_GETPARAM_PROFILE()
	#define START_SETPARAM_PROFILE()
	#define STOP_SETPARAM_PROFILE()
	#define START_GETPROP_PROFILE()
	#define STOP_GETPROP_PROFILE()
	#define START_SETPROP_PROFILE()
	#define STOP_SETPROP_PROFILE()
	#define START_INIT_PROFILE()
	#define STOP_INIT_PROFILE()
	#define RESET_PROFILE_RESULT(session_id)
	#define PRINT_PROFILE_RESULT(session_id)
	#define GET_PROFILE_CYCLES(session_id)
	#define ADD_PROFILE_ATTR(session_id, name, value)
#endif

#endif // CAPI_V2_PROFILE_H

