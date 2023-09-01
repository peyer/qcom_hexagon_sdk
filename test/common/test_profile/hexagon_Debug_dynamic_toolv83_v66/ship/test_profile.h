/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdlib.h>
#include <string.h>

#define __PROFILE	1

#ifndef __PROFILE_H__
#define __PROFILE_H__

#define MAX_HW_THREADS	(4)

#define SESSION_ALL			-1
#define SESSION_OTHER		-2

#ifdef __cplusplus
extern "C" {
#endif
	void add_session_attributes(int session_id, const char *name, const char *value);
	void reset_profiling(int session_id);
	void init_profiling(void);
	void start_profiling(int session_id);
	void * __wrap_malloc(size_t);
	void * __wrap_realloc(void *, size_t);
	void * __wrap_calloc(size_t, size_t);
	void __wrap_free(void *);
	void stop_profiling(int session_id, int printFlag);
	unsigned long long get_profiling_cycles(int sessionid);
	void print_profile_result(int session_id);
	void deinit_profiling(void);
#ifdef __cplusplus
}
#endif

#endif /* __PROFILE_H__ */
