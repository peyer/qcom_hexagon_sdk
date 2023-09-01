#ifndef __CLADE_COMMON_H__
#define __CLADE_COMMON_H__
/*==========================================================================
 * FILE:  clade_common.h
 *
 * DESCRIPTION: DDRCLADE API
 *
 * Copyright (c) 2016 Qualcomm Technologies Incorporated.
 * All Rights Reserved. QUALCOMM Proprietary and Confidential.
 ===========================================================================*/
#include "clade_types.h"
#include "clade_export.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define CLADE_MAX_PDS 4
#define MEMBLOCK_ID_LEN 32

typedef enum {
  CLADE_OK = 0,
  CLADE_NUM_PD_ERROR,
  CLADE_WRITE_BLOCKS_ERROR,
  CLADE_ADDR_ERROR,
  CLADE_DICT_ERROR,
  CLADE_PARAMS_ERROR,
  CLADE_WRITE_REPLACEABLE_ERROR,
  CLADE_INPUT_ALIGN_ERROR,
  CLADE2_EXCEED_FREELIST_ERROR,
  CLADE2_DECOMPRESS_ERROR,
  CLADE2_OUTOFRANGE_ERROR,
  CLADE_ERROR_MAX
} clade_error_t;

static const char *clade_error_names[CLADE_ERROR_MAX] = {
  "CLADE_OK", "CLADE_NUM_PD_ERROR", "CLADE_WRITE_BLOCKS_ERROR",
  "CLADE_ADDR_ERROR", "CLADE_DICT_ERROR", "CLADE_PARAMS_ERROR",
  "CLADE_WRITE_REPLACEABLE_ERROR", "CLADE_INPUT_ALIGN_ERROR",
  "CLADE2_EXCEED_FREELIST_ERROR","CLADE2_DECOMPRESS_ERROR",
  "CLADE2_OUTOFRANGE_ERROR"
};


typedef const char* (*clade_error_as_string_t)(void* config);

// Basic block for read/write/lookup. Link into ring so we can add to them
// incrementally in successive alloc calls.

typedef struct clade_memblock {
  struct clade_memblock *prev;
  struct clade_memblock *next;
  char id[MEMBLOCK_ID_LEN]; // block ID (which part of compressed data)
  int wordsize;             // in bytes
  paddr_t addr;             // start address
  int len;                  // length in words
  u8_t *data;
} clade_memblock_t;

// Callback to allocate blocks on the client side.
// Library may call this piecemeal to satisfy a clade_read or clade_write
// incrementally, passing the pointer returned from the previous call.
// The final pointer returned by this callback is then returned to the client
// from clade_read() or clade_write().

// Each empty data block can be inserted into the client's
// memory structure before returning from this callback, or this can be done
// after the clade_{read|write} returns.

// Data pointers in the returned memblocks may be null, indicating that the
// caller
// should not write any results.

// The ring of memblocks returned by this callback must be newly allocated;
// reusing the request memblocks is not permitted.

// Error returns NULL

typedef clade_memblock_t * // Return a ring of allocated empty blocks
    (*alloc_fn_t)
                           // Addresses to allocate
    (clade_memblock_t *request,
                           // Ring returned by last call to alloc
     // for the same clade_{read|write}.
     // In the first call this should be NULL.
     // Newly-allocated blocks must be linked
                           // into this ring.
     clade_memblock_t *previous,
                           // Passed through from clade_{read|write}.
     void *mem);

// Callback to look up in client's compressed memory.
// May not need to do any copying on client side, just point to data in
// client's memory.

// *** This callback may return the request ring (i.e. only allocate and
// attach data blocks.

// Error returns NULL

typedef clade_memblock_t * // Return requested blocks from client mem
    (*lookup_fn_t)(clade_memblock_t *request, // Ring of addresses to read
                   void *mem); // Passed through from clade_{read|write}.

// Callback to free clade_memblocks returned by lookup.
// We don't want to free() these in the library since we don't know how they
// were allocated, and the client may reuse them.

// *** This callback must be called (by the library) BEFORE freeing the
// request memblocks that were passed to the preceding lookups because
// the lookup function might return the request ring it received. Those
// memblocks must still be valid when passed to this (free_fn_t) callback.

typedef clade_error_t (*free_fn_t)(clade_memblock_t *blocks);

typedef clade_memblock_t *(*alloc_memblock_t)(clade_memblock_t **head);

typedef clade_memblock_t *(*clade_write_t)(clade_memblock_t *writes, void *mem,
                                           alloc_fn_t client_alloc,
                                           lookup_fn_t client_lookup,
                                           free_fn_t client_free);

///// Utility

static inline clade_memblock_t *alloc_memblock(clade_memblock_t **head) {

  clade_memblock_t *tmp = (clade_memblock_t *)malloc(sizeof(clade_memblock_t));
  strncpy(tmp->id, "", 1);
  tmp->wordsize = 1;
  tmp->len = 0;
  tmp->addr = 0;
  tmp->data = NULL;
  if (*head) {
    (*head)->prev->next = tmp;
    tmp->prev = (*head)->prev;
    (*head)->prev = tmp;
    tmp->next = *head;
  } else {
    tmp->prev = tmp;
    tmp->next = tmp;
    *head = tmp;
  }
  return tmp;
}

static inline clade_error_t free_memblock_parts(clade_memblock_t *blocks,
                                                int free_memblocks,
                                                int free_data) {

  clade_memblock_t *head = blocks;
  clade_memblock_t *cur = head;
  clade_memblock_t *tmp;

  if (!cur)
    return CLADE_OK;

  do {
    tmp = cur;
    assert(NULL != tmp);
    cur = cur->next;
    if (tmp->data && free_data) {
      free(tmp->data);
      tmp->data = NULL;
    }
    if (free_memblocks) {
      free(tmp);
    }
  } while (cur != head);
  return CLADE_OK;
}

static inline clade_error_t free_memblocks(clade_memblock_t *blocks) {

  return free_memblock_parts(blocks, 1, 0);
}

static inline clade_error_t free_memblocks_data(clade_memblock_t *blocks) {

  return free_memblock_parts(blocks, 1, 1);
}

static inline clade_error_t free_data(clade_memblock_t *blocks) {

  return free_memblock_parts(blocks, 0, 1);
}

static inline clade_memblock_t *free_empty_memblocks(clade_memblock_t *blocks) {

  clade_memblock_t *head = blocks;
  clade_memblock_t *cur = head;
  clade_memblock_t *tmp;

  if (!cur)
    return NULL;
  int restart;
  do {
    restart = 0;
    tmp = cur;
    cur = cur->next;
    if (NULL == tmp->data) {
      if (tmp == tmp->next) { // only one left
        free(tmp);
        return NULL;
      }
      tmp->prev->next = tmp->next;
      tmp->next->prev = tmp->prev;
      if (head == tmp) { // we are removing the head
        head = tmp->next;
        restart = 1;
      }
      free(tmp);
    }
  } while (cur != head || restart);
  return head;
}

static inline int num_memblocks(clade_memblock_t *blocks) {

  clade_memblock_t *head = blocks;
  clade_memblock_t *cur = head;

  if (!cur)
    return 0;

  int count = 0;
  do {
    count++;
    cur = cur->next;
  } while (cur != head);
  return count;
}

static inline clade_memblock_t *get_memblock(const char *name,
                                             clade_memblock_t *blocks) {

  clade_memblock_t *head = blocks;
  clade_memblock_t *cur = head;

  if (!cur)
    return NULL;

  do {
    if (!strncmp(cur->id, name, MEMBLOCK_ID_LEN))
      return cur;
    cur = cur->next;
  } while (cur != head);
  return NULL;
}

#endif