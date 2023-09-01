/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#ifndef __TEST_PROFILE_H__
#define __TEST_PROFILE_H__

#include <stdlib.h>
#include <string.h>

#define MAX_HW_THREADS	(4)

typedef struct ATTR {
	char *name;
	char *value;
	struct ATTR *next;
} attr_t;

typedef void* (*malloc_t)(size_t);
typedef void* (*realloc_t)(void *ptr, size_t size);
typedef void* (*calloc_t)(size_t size, size_t num);
typedef void (*free_t)(void *ptr);

void *tp_malloc(size_t);
void *tp_realloc(void*, size_t);
void *tp_calloc(size_t, size_t);
void tp_free(void*);

typedef struct {
	malloc_t malloc;
	realloc_t realloc;
	calloc_t calloc;
	free_t free;
} heap_wrappers_t;

void init_profile_ext(void);
void deinit_profile_ext(void);
unsigned long long get_cycle(void);
void get_tcycle(unsigned long long *);
void profile_result(int sessionid, unsigned long long total_cycles, unsigned long long total_time, int peak_heap, int total_heap, int num_leaks, int leak_size, unsigned long long total_pkts, unsigned long long axi, unsigned long long slot0, unsigned long long slot1, unsigned long long slot2, unsigned long long slot3, struct ATTR *attr);
unsigned int atomic_CompareAndExchange(unsigned int * volatile puDest, unsigned int uExchange, unsigned int uCompare);
void pmu_reset(void);
void pmu_config( int tmask, char event0, char event1, char event2, char event3 );
void pmu_config1( int tmask, char event0, char event1, char event2, char event3 );
void pmu_enable(void);
unsigned int get_pmu_counter( int event );

#endif /* __TEST_PROFILE_H__ */
