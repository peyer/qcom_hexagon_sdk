#ifndef __CLADE2_API_H__
#define __CLADE2_API_H__
/*==========================================================================
 * FILE:  clade2_api.h
 *
 * DESCRIPTION: DDRCLADE2 API
 *
 * Copyright (c) 2016 Qualcomm Technologies Incorporated.
 * All Rights Reserved. QUALCOMM Proprietary and Confidential.
 ===========================================================================*/

#include "clade_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clade_types.h"
#include "clade_common.h"
#include "clade2_version.h"

#define CLADE2_PREFIX 0x
#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A##B
  // XX.YY.ZZ in hex, e.g. 0x01010C is v1.1.12
#define CLADE2_LIB_VERSION  M_CONC(CLADE2_PREFIX,M_CONC(CLADE2_VERSION_MAJOR,M_CONC(CLADE2_VERSION_MINOR,CLADE2_VERSION_SEQ)))

///// API types

// Compressed area for each PD. For optional fields client sets a field to 0 to
// indicate that library should fill the value on clade_init() or clade_write().

// All clients must set at least .comp.

// Library must allow fields to be filled after clade_init() to accommodate
// run-time clients.

typedef struct clade2_pd_params {
  // filled by client
  u32_t meta_base_addr;
  //Number of cache lines addressed
  u32_t meta_len;
} clade2_pd_params_t;

// CLADE library configuration. For optional fields client sets a field to 0 to
// indicate that library should fill the value on clade_init() or clade_write().

// Library must allow certain fields to be filled after clade_init() to
// accommodate run-time clients.
// If params filled by the client before clade_init() change, clade_init()
// must be called again

typedef struct clade2_config {
    //following registers to be configured by client if not filling the 8 entries
  //above
  //register to enable/disable clade, send clean request and configure
  //interrupts for the engine
  u32_t control; //Specifies state of the engine
  u32_t status;
  u32_t a_base_addr; //Aperture Base Address
  u32_t c_base_addr;//compressed Base Address
  //If c_data_len is 0, assume client is the linker(and hence doing initial compression)
  u32_t c_data_len;//Compressed Data Length
  

  clade2_pd_params_t pd_params[CLADE_MAX_PDS];

  //Free list base address for 16 byte blocks
  //Free list end address for 16 byte blocks
  //Free list offset address for 16 byte blocks
  u32_t free_base_addr_16;
  u32_t free_end_addr_16;
  u32_t free_ptr_offset_addr_16;

  u32_t free_base_addr_32;
  u32_t free_end_addr_32;
  u32_t free_ptr_offset_addr_32;

  u32_t free_base_addr_48;
  u32_t free_end_addr_48;
  u32_t free_ptr_offset_addr_48;

  u32_t free_base_addr_64;
  u32_t free_end_addr_64;
  u32_t free_ptr_offset_addr_64;

  //PMU Selection Register
  u32_t pmu_selection;

  //PMU Event Counters
  u32_t pmu_ctr_0;
  u32_t pmu_ctr_1;

  //4 32 bit anchors that the client can specify as initial anchors for
  //each cache line compression
  u32_t anchor_0;
  u32_t anchor_1;
  u32_t anchor_2;
  u32_t anchor_3;

  u32_t max_fetches;

  //type of error recevied
  u32_t syndrome_0;
  //Error related information
  u32_t syndrome_1;

  clade_error_t error;

  // filled by library on clade_init()
  unsigned int lib_version;
  unsigned int build_id;
  //Parameters below only initialized by the linker, ignored by archsim/q6sim
  //all 8 values below needs to filled by client
  //First 4 entries determine the size of the compressed area
  //Second 4 entries determine number of entries in each of the free lists
  unsigned int num_16B_entries;
  unsigned int num_32B_entries;
  unsigned int num_48B_entries;
  unsigned int num_64B_entries;
  unsigned int num_16B_freelist_entries;
  unsigned int num_32B_freelist_entries;
  unsigned int num_48B_freelist_entries;
  unsigned int num_64B_freelist_entries;

  u32_t enable_pmu_updates; //0=disable updates 1=enable updates
 
  signed int clade2_reg_offset; // register offset being written (check clade2_hw_regs.h)

  u32_t priority; //Bus QoS Priority


} clade2_config_t;

typedef clade_error_t (*clade2_init_t)(clade2_config_t *config);
typedef clade_memblock_t *(*clade2_write_t)(
    clade_memblock_t *writes, // Uncompressed addr/data to write
    void *mem,                // Client's mem, pass through to callbacks
    alloc_fn_t client_alloc, lookup_fn_t client_lookup, free_fn_t client_free);
///// API functions

// Get parameters from library
//  1. clade_init() sets num_pds to the max number of supported pds.
//  2. Prior to calling any functions other than clade_init(),
//     the number of pd_params blocks allocated by the client and
//     num_pds must agree.
DDRCLADE_API_EXPORT
clade_error_t clade2_init(clade2_config_t *config);

// Get string for error
DDRCLADE_API_EXPORT
const char *clade2_get_error_string(void *config);

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
clade_memblock_t *clade2_write // Return ring of updates to compressed mem
    (clade_memblock_t *writes, // Uncompressed addr/data to write
     void *mem,                // Client's mem, pass through to callbacks
     alloc_fn_t client_alloc, lookup_fn_t client_lookup, free_fn_t client_free);

// Read: Uncompress a list of reads
// Calls lookup to read compressed memory
// Calls alloc to get blocks to store uncompressed bytes
// Calls free to release the list returned by lookup
// Error returns NULL
DDRCLADE_API_EXPORT
clade_memblock_t *clade2_read // Return a ring of uncompressed blocks
    (clade_memblock_t *reads, // Uncompressed addresses to read
     void *mem,               // Client's mem, pass through to callbacks
     alloc_fn_t client_alloc, lookup_fn_t client_lookup, free_fn_t client_free);

#ifdef __cplusplus
}
#endif

#endif
