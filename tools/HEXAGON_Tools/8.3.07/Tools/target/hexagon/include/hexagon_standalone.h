/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#ifndef _HEXAGON_STANDALONE_H_
#define _HEXAGON_STANDALONE_H_ 1

#define HEXAGON_DEFAULT_PAGE_SIZE  (1048576) /* 1MB */
#define HEXAGON_DEFAULT_PAGE_PERM  (7)       /* XWR */
#define HEXAGON_DEFAULT_PAGE_CACHE (7)       /* L1, WB, NS, L2 */
#define HEXAGON_MMAP_DEFAULT_ALIGNMENT 4096

typedef enum {
     HEXAGON_EVENT_0 = 0,
     HEXAGON_EVENT_1,
     HEXAGON_EVENT_2,
     HEXAGON_EVENT_3,
     HEXAGON_EVENT_4,
     HEXAGON_EVENT_5,
     HEXAGON_EVENT_6,
     HEXAGON_EVENT_7,
     HEXAGON_EVENT_8,
     HEXAGON_EVENT_9,
     HEXAGON_EVENT_10,
     HEXAGON_EVENT_11,
     HEXAGON_EVENT_12,
     HEXAGON_EVENT_13,
     HEXAGON_EVENT_14,
     HEXAGON_EVENT_15,
     HEXAGON_EVENT_16,
     HEXAGON_EVENT_17,
     HEXAGON_EVENT_18,
     HEXAGON_EVENT_19,
     HEXAGON_EVENT_20,
     HEXAGON_EVENT_21,
     HEXAGON_EVENT_22,
     HEXAGON_EVENT_23,
     HEXAGON_EVENT_24,
     HEXAGON_EVENT_25,
     HEXAGON_EVENT_26,
     HEXAGON_EVENT_27,
     HEXAGON_EVENT_28,
     HEXAGON_EVENT_29,
     HEXAGON_EVENT_30,
     HEXAGON_EVENT_31,
     HEXAGON_EVENT_32,
     HEXAGON_EVENT_33,
     HEXAGON_EVENT_34,
     HEXAGON_EVENT_35,
     HEXAGON_EVENT_36,
     HEXAGON_EVENT_37,
     HEXAGON_EVENT_38,
     HEXAGON_EVENT_39,
     HEXAGON_EVENT_40,
     HEXAGON_EVENT_41,
     HEXAGON_EVENT_42,
     HEXAGON_EVENT_43,
     HEXAGON_EVENT_44,
     HEXAGON_EVENT_45,
     HEXAGON_EVENT_46,
     HEXAGON_EVENT_47,
     HEXAGON_EVENT_MAX
} hexagon_event_vector_t;

#ifdef __HVX__
typedef enum {
    HEXAGON_VECTOR_WAIT    = 0,
    HEXAGON_VECTOR_NO_WAIT = 1,
    HEXAGON_VECTOR_CHECK   = 2
} hexagon_vector_wait_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Start-up configuration. */
extern void *COMPILE_FOR_RTOS __attribute__ ((weak));

extern void *I_CACHE_ENABLE __attribute__ ((weak));
extern void *I_CACHE_HW_PREFETCH __attribute__ ((weak));

extern void *D_CACHE_ENABLE __attribute__ ((weak));
extern void *D_CACHE_HW_PREFETCH __attribute__ ((weak));

extern void *EVENT_VECTOR_BASE __attribute__ ((weak));

extern void *ENABLE_TRANSLATION __attribute__ ((weak));
extern void *USE_DEFAULT_TLB_MISS_HANDLER __attribute__ ((weak));
extern void *TLB_MAP_TABLE_PTR __attribute__ ((weak));
extern void *ENABLE_64_TLBS __attribute__ ((weak));

extern void *ENABLE_PCYCLE __attribute__ ((weak));

extern void *TCM_BASE_ADDR __attribute__ ((weak));
extern void *L2_CACHE_SIZE __attribute__ ((weak));
extern void *L2_PARITY __attribute__ ((weak));
extern void *L2_WB __attribute__ ((weak));

extern void *STACK_START __attribute__ ((weak));
extern void *STACK_SIZE __attribute__ ((weak));
extern void *HEAP_START __attribute__ ((weak));
extern void *HEAP_SIZE __attribute__ ((weak));

extern void *CORE_DUMP_BASE __attribute__ ((weak));

extern void *ISDB_SECURE_FLAG __attribute__ ((weak));
extern void *ISDB_TRUSTED_FLAG __attribute__ ((weak));
extern void *ISDB_DEBUG_FLAG __attribute__ ((weak));

extern void *ANGEL_SUPPORT __attribute__ ((weak));

extern void (*PRE_INIT) (void) __attribute__ ((weak));
extern void (*POST_EXIT) (void) __attribute__ ((weak));

/* Stand-alone API. */
void add_translation (void *va, void *pa, int cacheability);
void add_translation_fixed (int index, void *va, void *pa, int cacheability,
                            int permissions);
int add_translation_extended (int index, void *va, uint64_t pa,
                              unsigned int page_size, unsigned int xwru,
                              unsigned int cccc, unsigned int asid,
                              unsigned int aa, unsigned int vg);

void register_interrupt (int intno, void (*handler) (int intno));

void lockMutex (int *mutex);
int trylockMutex (int *mutex);
void unlockMutex (int *mutex);

void thread_create (void (*pc) (void *), void *sp, int threadno, void *param);
#if __HEXAGON_ARCH__ >= 61
int thread_create_extended (void (*pc) (void *), void *sp, int threadno,
                            unsigned framekey, unsigned stack_size, void *param);
#endif
void thread_stop (void);
void thread_join (int mask);
void thread_stack_size (int threadno, int stack_size);
int
set_event_handler(hexagon_event_vector_t entry, void (*target)(void));
uint32_t get_event_handler(hexagon_event_vector_t entry);
#ifdef  __HVX__
int acquire_vector_unit(hexagon_vector_wait_t wait);
void release_vector_unit();
void set_double_vector_mode();
void clear_double_vector_mode();

extern void power_vector_unit (uint32_t volatile *clockbase,
                               uint32_t volatile *resetbase,
                               uint32_t volatile *powerbase,
                               int delay, int state);


// Simulated HVX requires that we obtain the HVX resource
// prior to using it. This macro is intended for simulated testing only.
#define SIM_ACQUIRE_HVX \
  if (!acquire_vector_unit(HEXAGON_VECTOR_WAIT)) { \
    printf ("ERROR: Failed to acquire HVX unit.\n"); \
    assert (0 && "ERROR: Failed to acquire HVX unit.\n"); \
  }
// This macro is for simulated testing only.
#define SIM_RELEASE_HVX release_vector_unit()
// To enable double vector mode.
// This macro is intended for simulated testing only.
#define SIM_SET_HVX_DOUBLE_MODE set_double_vector_mode()
// To disable double vector mode.
// This macro is intended for simulated testing only.
#define SIM_CLEAR_HVX_DOUBLE_MODE clear_double_vector_mode()
#else
#define SIM_ACQUIRE_HVX _Pragma ("GCC error \"SIM_ACQUIRE_HVX macro used without hvx compiler option, see -mhvx\"")
#define SIM_RELEASE_HVX _Pragma ("GCC error \"SIM_RELEASE_HVX macro used without hvx compiler option, see -mhvx\"")
#define SIM_SET_HVX_DOUBLE_MODE _Pragma ("GCC error \"SIM_SET_HVX_DOUBLE_MODE macro used without hvx compiler option, see -mhvx\"")
#define SIM_CLEAR_HVX_DOUBLE_MODE _Pragma ("GCC error \"SIM_CLEAR_HVX_DOUBLE_MODE macro used without hvx compiler option, see -mhvx\"")
#endif

static uint32_t __rdcfg(uint32_t cfgbase, uint32_t offset) {
  asm volatile ("%[cfgbase]=asl(%[cfgbase], #5)\n\t"
                "%[offset]=memw_phys(%[offset], %[cfgbase])"
    :[cfgbase] "+r" (cfgbase), [offset] "+r" (offset)
    :
    :);
  return offset;
}
enum {
  __tlb_count=0x02c,
  __coproc_type=0x030,
  __coproc_ctx=0x034,
  __tcm_size=0x03c,
  __l2_tag_size=0x040,
  __l2_array_size=0x044,
  __thread_mask=0x048
};
static inline uint32_t get_thread_count() {
#if __HEXAGON_ARCH__ > 65
  uint32_t cfgbase;
  asm volatile ("%0=cfgbase;"
    :"=r"(cfgbase));
  return __builtin_popcount(__rdcfg(cfgbase, __thread_mask));
#else
  return 0;
#endif
}

#ifdef __cplusplus
}
#endif

  /* Use faster inline version instead. */
  #define thread_get_tnum() \
            ({int t; \
              __asm__ __volatile__ \
                ("%0 = htid": \
                 "=&r" (t)); \
              t;})

#endif /* _HEXAGON_STANDALONE_H_ */
