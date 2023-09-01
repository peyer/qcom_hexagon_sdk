#ifndef __CLADE2_TRACE_H__
#define __CLADE2_TRACE_H__
/*==========================================================================
 * FILE:         clade2_trace.h
 *
 * DESCRIPTION:  Tracing facility for the CLADE library
 *
 * Copyright (c) 2016 Qualcomm Technologies Incorporated.
 * All Rights Reserved. QUALCOMM Proprietary and Confidential.
 ===========================================================================*/

#include <stdio.h>
#include <stdarg.h>

#include "clade_export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum clade2_trace_enum {
	CLADE2_TRACE_ERROR     = 0x01,
	CLADE2_TRACE_WARN      = 0x02,
	CLADE2_TRACE_INFO      = 0x04,
	CLADE2_TRACE_DATA      = 0x08,
	CLADE2_TRACE_API       = 0x10,
	CLADE2_TRACE_INTERNALS = 0x20,
	CLADE2_TRACE_DETAILED  = 0x40
} clade2_trace_t;

typedef enum clade2_trace_status {
	CLADE2_STATUS_OK,
	CLADE2_STATUS_NOT_FOUND,
} clade2_trace_status_t;

// Enables tracing levels (default is all disabled)
DDRCLADE_API_EXPORT
void clade2_set_trace(int level);

// Enables tracing levels with a string, traces correspond to all
// substrings info, error, data, api, etc (case insensitive)
// matches from str, e.g. "error info"
// (full list of substring matched in clade2_set_trace_str definition)
DDRCLADE_API_EXPORT
clade2_trace_status_t clade2_set_trace_str(const char* str);

DDRCLADE_API_EXPORT
int clade2_get_trace_level(void);

// Sends tracing setting to FILE* instead of stdout
DDRCLADE_API_EXPORT
void clade2_set_trace_file(FILE* file);

//Creates trace file with fname and sends the
//trace setting to this file instead of stdout
//returns CLADE_STATUS_OK on successful creation of file
DDRCLADE_API_EXPORT
clade2_trace_status_t clade2_create_trace_file(const char* fname, const char* fmode);

//allows user to flush contents to trace file
DDRCLADE_API_EXPORT
void clade2_flush_trace_file();

void clade2_trace_type(clade2_trace_t type, const char* fmt, ...);

void clade2_trace(const char* fmt, ...);

void clade2_vtrace(const char* fmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif
