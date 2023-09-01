#ifndef __CLADE_API_H__
#define __CLADE_API_H__
/*==========================================================================
 * FILE:  clade_api.h
 *
 * DESCRIPTION: DDRCLADE API
 *
 * Copyright (c) 2016 Qualcomm Technologies Incorporated.
 * All Rights Reserved. QUALCOMM Proprietary and Confidential.
 ===========================================================================*/

#include "clade_types.h"
#include "clade_export.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CLADE_PREFIX 0x
#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A##B
// XX.YY.ZZ in hex, e.g. 0x01010C is v1.1.12
#define CLADE_LIB_VERSION  M_CONC(CLADE_PREFIX,M_CONC(CLADE_VERSION_MAJOR,M_CONC(CLADE_VERSION_MINOR,CLADE_VERSION_SEQ)))


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clade_version.h"
#include "clade_common.h"
#include "clade_types.h"

///// API types

typedef enum {
	LOOKUP_HIGH_MAIN = 0x01,
	LOOKUP_LOW_MAIN  = 0x02,
	LOOKUP_HIGH_EXC  = 0x03,
	LOOKUP_LOW_EXC   = 0x04,
} clade_lookup_t;


// Compressed area for each PD. For optional fields client sets a field to 0 to
// indicate that library should fill the value on clade_init() or clade_write().

// All clients must set at least .comp.

// Library must allow fields to be filled after clade_init() to accommodate
// run-time clients.

typedef struct clade_pd_params {
	// filled by client
	paddr_t comp;                // start of compressed area

	// optionally filled by client
	paddr_t exc_hi;              // high-priority exception area
	paddr_t exc_lo_small;        // low-priority (small) exception area
	paddr_t exc_lo;              // low-priority exception area
	paddr_t region_hi;           // start of high-priority compressible range
	int     region_hi_len;       // length of high-priority compressible range
	paddr_t region_lo;           // start of low-priority compressible range
	int     region_lo_len;       // length of low-priority compressible range
} clade_pd_params_t;


// CLADE library configuration. For optional fields client sets a field to 0 to
// indicate that library should fill the value on clade_init() or clade_write().

// Library must allow certain fields to be filled after clade_init() to
// accommodate run-time clients.
// If params filled by the client before clade_init() change, clade_init()
// must be called again

typedef struct clade_config {
	// filled by client before or after clade_init()
	paddr_t region;              // start of compressible range

	// optionally filled by client before clade_init()
	unsigned int num_pds;

	// filled by library on clade_init()
	unsigned int lib_version;
	unsigned int num_dicts;
	unsigned int dict_len;       // in bytes

	// allocated by client before or after clade_init()
	clade_pd_params_t *pd_params;// array of size num_pds

	// optionally allocated by client after clade_init()
	// array of dictionary pointers, size num_dicts
	// NULL signals library to create dictionaries
	u32_t **dicts;

	// instructions/data like brkpt allowed to replace others via clade_write()
	// must be set before calling clade_write() that creates the dictionaries
	// note that passing in dictionaries makes these have no effect since
	// passed in dictionaries are used verbatim and hence not updated
	// with replaceable_words listed here
	unsigned int num_replaceable_words;
	u32_t *replaceable_words;

	// filled by library on error
	clade_error_t error;
	unsigned int build_id;
} clade_config_t;

typedef clade_error_t (*clade_init_with_data_t)(clade_config_t* config, const char*, int len);
typedef clade_error_t (*clade_init_t)(clade_config_t *config);
///// API functions

// Get parameters from library
//  1. clade_init() sets num_pds to the max number of supported pds.
//  2. Prior to calling any functions other than clade_init(),
//     the number of pd_params blocks allocated by the client and
//     num_pds must agree.
DDRCLADE_API_EXPORT
clade_error_t clade_init(clade_config_t *config);

// Get string for error
static inline const char* clade_error_as_string(clade_error_t error)
{
	return clade_error_names[error];
}



DDRCLADE_API_EXPORT
const char* clade_get_error_string(void* config);

// Write: Compress a list of writes
//

// Restriction of the current library implementation:
// ---
// writes must be either list of 2 or 1 memblocks
// When 2 memblocks are passed in, the first coresponds to the
// high priority memory and the second the low priority
// When 1 memblock is passed in, compressed data must already
// exists, this allows update compressed memory
// len * wordsize must be 4 in this case
// ---
//
// Calls lookup as needed to read compressed memory
// Calls alloc to get blocks to store compressed bytes
// Calls a free_fn_t to release the list returned by lookup
// Error returns NULL
DDRCLADE_API_EXPORT
clade_memblock_t *clade_write  // Return ring of updates to compressed mem
(clade_memblock_t *writes,     // Uncompressed addr/data to write
 void *mem,                    // Client's mem, pass through to callbacks
 alloc_fn_t client_alloc, lookup_fn_t client_lookup, free_fn_t client_free);

// Read: Uncompress a list of reads
// Calls lookup to read compressed memory
// Calls alloc to get blocks to store uncompressed bytes
// Calls free to release the list returned by lookup
// Error returns NULL
DDRCLADE_API_EXPORT
clade_memblock_t *clade_read   // Return a ring of uncompressed blocks
(clade_memblock_t *reads,      // Uncompressed addresses to read
 void *mem,                    // Client's mem, pass through to callbacks
 alloc_fn_t client_alloc, lookup_fn_t client_lookup, free_fn_t client_free);

#ifdef __cplusplus
}
#endif

#endif

