#ifndef _CAPI_V2_UTILS_H
#define _CAPI_V2_UTILS_H
/*==============================================================================
    Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

/* -----------------------------------------------------------------------
** Standard include files
** ----------------------------------------------------------------------- */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if defined(__cplusplus)
  #include <new>
extern "C" {
#endif // __cplusplus
#include<assert.h>

/* -----------------------------------------------------------------------
** ADSP and QURT Elite include files
** ----------------------------------------------------------------------- */
#include "mmdefs.h"
// TODO hexagon_sim_timer.h instead?
#include "q6sim_timer.h"

#include "HAP_debug.h"
#include "HAP_mem.h"

/* -----------------------------------------------------------------------
** Standard Integer Types and Definitions
** ----------------------------------------------------------------------- */
typedef unsigned long long uint64_t;  /* Unsigned 64 bit value */
typedef unsigned long int  uint32_t;  /* Unsigned 32 bit value */
typedef unsigned short     uint16_t;  /* Unsigned 16 bit value */
typedef unsigned char      uint8_t;   /* Unsigned 8  bit value */

typedef signed long long   int64_t;   /* Signed 64 bit value */
typedef signed long int    int32_t;   /* Signed 32 bit value */
typedef signed short       int16_t;   /* Signed 16 bit value */
typedef signed char        int8_t;    /* Signed 8  bit value */

#undef TRUE
#undef FALSE

#define TRUE   (1)  /* Boolean true value. */
#define FALSE  (0)  /* Boolean false value. */

#ifndef NULL
#define NULL (0)
#endif

/**
  Error code test macro.
 */
#define ADSP_FAILED(x) (ADSP_EOK != (x))

/* ------------------------------------------------------------------------
 ** QURT ELITE Memory Definitions
 ** ------------------------------------------------------------------------ */
  typedef enum
  {
    QURT_ELITE_HEAP_DEFAULT=0,         
    QURT_ELITE_HEAP_OUT_OF_RANGE      //keep at end
  } QURT_ELITE_HEAP_ID;


#if defined(__cplusplus)
// TODO temporary C++ definitions or new/delete

#define HAP_placement_new(pObj, pMemory, typeType, ...) { \
   (pObj) = (new (pMemory) (typeType)(__VA_ARGS__)); \
}

#define HAP_placement_delete(pObject, typeType) { \
   if (pObject) { (pObject)->~typeType(); (pObject) = NULL; }\
}

#define HAP_new(pObject, typeType, heapId, ...) { \
   void *pObj = 0; \
   (void)HAP_malloc(sizeof(typeType), &pObj); \
   (pObject) = (pObj) ? (new (pObj) (typeType)(__VA_ARGS__)) : NULL; \
}

#define HAP_delete(pObject, typeType) { \
   if (pObject) { (pObject)->~typeType(); \
   (void)HAP_free(pObject); \
   (pObject) = NULL; } \
}

#endif // __cplusplus

/*
   Qurt_elite version of assert.
*/
#define QURT_ELITE_ASSERT(x) assert(x)

#define s32_min_s32_s32(var1, var2)  Q6_R_min_RR(var1,var2)

#define   LL_shr(x, n)       (Q6_P_asr_PR((x),(n)))

#define   extract_h(x)       ((int16_t)Q6_R_asrh_R( (x) ))       

#define   L_sat(x)           (Q6_R_sat_P( (x) ))

#if defined(__cplusplus)
} // extern "C"
#endif

#endif /* _CAPI_V2_UTILS_H */

