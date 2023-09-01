/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <profile.h>
#include <HAP_farf.h>
#include <test_profile.h>

typedef struct THEAP {
	void *ptr;
	int size;
	struct THEAP *next;
} heap_t;

typedef struct TPROFILE {
	unsigned int sessionid;
	unsigned int peak_heap;
	unsigned int total_heap;
	unsigned long long total_pkts;
	unsigned long long axi;
	unsigned long long slot0_pkts;
	unsigned long long slot1_pkts;
	unsigned long long slot2_pkts;
	unsigned long long slot3_pkts;
	unsigned long long profile_cycles;
	unsigned long long thread_start_cycles[MAX_HW_THREADS];
	unsigned long long thread_cycles[MAX_HW_THREADS];
	int state;
	struct ATTR 	*attributes;
	struct THEAP 	*heap_ptr;
	struct TPROFILE *next;
} profile_t;

heap_wrappers_t _wrappers = { tp_malloc, tp_realloc, tp_calloc, tp_free };
static profile_t _profile_head = { 0 };
static int sessionid = SESSION_OTHER;

static inline int get_session_id(void) {
	return sessionid;
}

static inline void add_heap_stats(profile_t* thread_ptr, int thread_id, void *ptr, size_t size) {
	heap_t *new_heap_ptr = NULL;
	heap_t *temp_ptr = NULL;
	int peak_heap = 0;

	temp_ptr = thread_ptr->heap_ptr;
	new_heap_ptr = (heap_t*) _wrappers.malloc(sizeof(heap_t));
	if(new_heap_ptr == NULL) {
		return;
	}
	new_heap_ptr->ptr = ptr;
	new_heap_ptr->size = size;
	new_heap_ptr->next = NULL;
	if(temp_ptr == NULL) {
		thread_ptr->heap_ptr = new_heap_ptr;
	} else {
		while(temp_ptr->next != NULL) {
			peak_heap += temp_ptr->size;
			temp_ptr = temp_ptr->next;
		}
		peak_heap += temp_ptr->size;
		temp_ptr->next = new_heap_ptr;
	}
	peak_heap += size;
	if(peak_heap > thread_ptr->peak_heap) {
		thread_ptr->peak_heap = peak_heap;
	}
	thread_ptr->total_heap += size;
}

static inline void remove_heap_stats(profile_t *thread_ptr, int thread_id, void *ptr) {
	heap_t *temp_ptr = NULL;

	temp_ptr = thread_ptr->heap_ptr;
	if(temp_ptr == NULL) {
		return;
	} else {
		if(temp_ptr->ptr == ptr) {
			thread_ptr->heap_ptr = temp_ptr->next;
			_wrappers.free(temp_ptr);
		}
		while(temp_ptr->next != NULL && temp_ptr->next->ptr != ptr) {
			temp_ptr = temp_ptr->next;
		}
		heap_t *free_ptr = temp_ptr->next;
		if(free_ptr != NULL && free_ptr->ptr == ptr) {
			temp_ptr->next = free_ptr->next;
			_wrappers.free(free_ptr);
		}
	}
}

static inline profile_t* add_profile_thread(int session_id) {
	profile_t *new_thread_ptr = NULL;

	new_thread_ptr = (profile_t*) _wrappers.malloc(sizeof(profile_t));
	memset(new_thread_ptr, 0, sizeof(profile_t));
	new_thread_ptr->next = NULL;
#ifdef __MULTI_HREADED__
	unsigned int current = 0;
	unsigned int previous = 0;
	do {
		profile_t *temp_ptr = &_profile_head;
		while(temp_ptr->next != NULL) {
			temp_ptr = temp_ptr->next;
		}
		current = (unsigned int)temp_ptr->next;
		previous = atomic_CompareAndExchange((unsigned int * volatile)&(temp_ptr->next), (unsigned int)new_thread_ptr, current);
	} while(previous != current);
#else
	profile_t *temp_ptr = &_profile_head;
	while(temp_ptr->next != NULL) {
		temp_ptr = temp_ptr->next;
	}
	temp_ptr->next = new_thread_ptr;
#endif
	new_thread_ptr->sessionid = session_id;
	return new_thread_ptr;
}

static inline void remove_profile_thread(profile_t *thread_ptr) {
	if(thread_ptr == NULL) {
		return;
	}
	unsigned int current = 1;
	unsigned int previous = 0;
#ifdef __MULTI_THREADED__
	do {
		profile_t *temp_ptr = &_profile_head;
		while(temp_ptr->next != NULL && (temp_ptr->next->sessionid != thread_ptr->sessionid)) {
			temp_ptr = temp_ptr->next;
		}
		if(temp_ptr->next != NULL && (temp_ptr->next->sessionid == thread_ptr->sessionid)) {
			current = (unsigned int)temp_ptr->next;
			previous = atomic_CompareAndExchange((unsigned int * volatile)&(temp_ptr->next), (unsigned int)(((profile_t *)current)->next), current);
		}
	} while(previous != current);
#else
	profile_t *temp_ptr = &_profile_head;
	while(temp_ptr->next != NULL && (temp_ptr->next->sessionid != thread_ptr->sessionid)) {
		temp_ptr = temp_ptr->next;
	}
	if(temp_ptr->next != NULL && (temp_ptr->next->sessionid == thread_ptr->sessionid)) {
		current = previous = (unsigned int)temp_ptr->next;
		temp_ptr->next = ((profile_t *)current)->next;
	}
#endif
	if(previous == current) {
		_wrappers.free((void*)current);
	}
}

static inline profile_t* get_profile_session(int session_id) {
	profile_t *temp_ptr = _profile_head.next;
	while(temp_ptr != NULL) {
		if(temp_ptr->sessionid == session_id) {
			return temp_ptr;
		}
		temp_ptr = temp_ptr->next;
	}
	return add_profile_thread(session_id);
}

void print_profile_result(int session_id) {
	unsigned long long total_cycles = 0;
	int hw_thread_index = 0;
	int total_leaks = 0, leak_size = 0;

	if(session_id == SESSION_ALL) {
		profile_t *thread_ptr = _profile_head.next;
		int total_heap = 0, peak_heap = 0;
		long long total_pkts = 0, axi = 0, slot0_pkts = 0, slot1_pkts = 0, slot2_pkts = 0, slot3_pkts = 0;
		while(thread_ptr != NULL) {
			for(hw_thread_index = 0; hw_thread_index < MAX_HW_THREADS; hw_thread_index++) {
				total_cycles += thread_ptr->thread_cycles[hw_thread_index];
			}
			total_heap += thread_ptr->total_heap;
			if(thread_ptr->peak_heap > peak_heap) {
				peak_heap = thread_ptr->peak_heap;
			}
			heap_t *heap_ptr = thread_ptr->heap_ptr;
			while(heap_ptr != NULL) {
				total_leaks++;
				leak_size += heap_ptr->size;
				heap_ptr = heap_ptr->next;
			}
			total_cycles = total_cycles - thread_ptr->profile_cycles;	// doing this because the total_cycle was calculated before this print_profiling_call.
			total_pkts += thread_ptr->total_pkts;
			axi += thread_ptr->axi;
			slot0_pkts += thread_ptr->slot0_pkts;
			slot1_pkts += thread_ptr->slot1_pkts;
			slot2_pkts += thread_ptr->slot2_pkts;
			slot3_pkts += thread_ptr->slot3_pkts;
			thread_ptr = thread_ptr->next;
		}
		profile_result(SESSION_ALL, total_cycles, 0, peak_heap, total_heap, total_leaks, leak_size,
				total_pkts, axi, slot0_pkts, slot1_pkts, slot2_pkts, slot3_pkts, NULL);
	} else {
		profile_t *thread_ptr = get_profile_session(session_id);
		if(thread_ptr == NULL) {
			return;
		}
		for(hw_thread_index = 0; hw_thread_index < MAX_HW_THREADS; hw_thread_index++) {
			total_cycles += thread_ptr->thread_cycles[hw_thread_index];
		}
		heap_t *heap_ptr = thread_ptr->heap_ptr;
		while(heap_ptr != NULL) {
			total_leaks++;
			leak_size += heap_ptr->size;
			heap_ptr = heap_ptr->next;
		}
		total_cycles = total_cycles - thread_ptr->profile_cycles;	// doing this because the total_cycle was calculated before this print_profiling_call.
		profile_result(thread_ptr->sessionid, total_cycles, 0, thread_ptr->peak_heap, thread_ptr->total_heap, total_leaks, leak_size,
				thread_ptr->total_pkts, thread_ptr->axi, thread_ptr->slot0_pkts, thread_ptr->slot1_pkts, thread_ptr->slot2_pkts, thread_ptr->slot3_pkts, thread_ptr->attributes);
	}
}

void init_profiling(void) {
	// add other session by default
	get_profile_session(SESSION_OTHER);
	init_profile_ext();
}

void deinit_profiling(void) {
	profile_t *thread_prof = _profile_head.next;
	heap_t *heap = thread_prof->heap_ptr;
	heap_t *temp_heap = NULL;
	while(heap != NULL) {
		temp_heap = heap;
		heap = heap->next;
		_wrappers.free(temp_heap);
	}
	attr_t *attr = thread_prof->attributes;
	attr_t *temp_attr = NULL;
	while(attr != NULL) {
		temp_attr = attr;
		attr = attr->next;
		_wrappers.free(temp_attr->name);
		_wrappers.free(temp_attr->value);
		_wrappers.free(temp_attr);
	}
	while(thread_prof != NULL) {
		remove_profile_thread(thread_prof);
		thread_prof = thread_prof->next;
	}
	deinit_profile_ext();
}

void reset_profiling(int sessionId) {
	profile_t *thread_ptr = get_profile_session(sessionId);
	if(thread_ptr->state == 0) {
		heap_t *heap = thread_ptr->heap_ptr;
		heap_t *temp = NULL;
		while(heap != NULL) {
			temp = heap;
			heap = heap->next;
			_wrappers.free(temp);
		}
		attr_t *attr = thread_ptr->attributes;
		attr_t *temp_attr = NULL;
		while(attr != NULL) {
			temp_attr = attr;
			attr = attr->next;
			_wrappers.free(temp_attr);
		}
		memset(thread_ptr, 0, sizeof(profile_t) - sizeof(profile_t*));
		thread_ptr->sessionid = sessionId;
	}
}

void start_profiling(int sessionId) {
	unsigned long long start_cycles[MAX_HW_THREADS] = {0};

	profile_t *thread_ptr = get_profile_session(sessionId);
	pmu_reset();
	pmu_config(0xffffffff, 0x3, 0x41, 0x33, 0x34);
#ifdef __HEXAGON_V55__
	pmu_config1(0xffffffff, 0x35, 0x36, 0x0, 0x0);
#endif
	get_tcycle((unsigned long long*)&start_cycles);
	thread_ptr->state = 1;
	memcpy(thread_ptr->thread_start_cycles, start_cycles, sizeof(start_cycles));
	pmu_enable();
	sessionid = sessionId;
}

void stop_profiling(int sessionId, int printFlag) {
	unsigned long long end_cycles[MAX_HW_THREADS] = {0}, total_cycles = 0;
	int total_leaks = 0, leak_size = 0;
	int hw_thread_index = 0;

	get_tcycle((unsigned long long*)&end_cycles);
	profile_t *thread_ptr = get_profile_session(sessionId);
	if(thread_ptr == NULL) {
		return;
	}
	thread_ptr->state = 0;
	thread_ptr->total_pkts += get_pmu_counter(0);
	thread_ptr->axi += get_pmu_counter(1);
	thread_ptr->slot0_pkts += get_pmu_counter(2);
	thread_ptr->slot1_pkts += get_pmu_counter(3);
	thread_ptr->slot2_pkts += 0;
	thread_ptr->slot3_pkts += 0;
	if(thread_ptr->total_pkts <= 0) {
		thread_ptr->total_pkts = 1;
	}
#ifdef __HEXAGON_V55__
	thread_ptr->slot2_pkts = get_pmu_counter(4);
	thread_ptr->slot3_pkts = get_pmu_counter(5);
#else
	thread_ptr->slot2_pkts = thread_ptr->slot0_pkts;
	thread_ptr->slot3_pkts = thread_ptr->slot1_pkts;
#endif
	pmu_reset();
	for(hw_thread_index = 0; hw_thread_index < MAX_HW_THREADS; hw_thread_index++) {
		thread_ptr->thread_cycles[hw_thread_index] += (end_cycles[hw_thread_index] - thread_ptr->thread_start_cycles[hw_thread_index]);
		total_cycles += (end_cycles[hw_thread_index] - thread_ptr->thread_start_cycles[hw_thread_index]);
	}
	if(printFlag) {
		heap_t *heap_ptr = thread_ptr->heap_ptr;
		while(heap_ptr != NULL) {
			total_leaks++;
			leak_size += heap_ptr->size;
			heap_ptr = heap_ptr->next;
		}
		total_cycles = total_cycles - thread_ptr->profile_cycles;	// doing this because the total_cycle was calculated before this print_profiling_call.
		profile_result(thread_ptr->sessionid, total_cycles, 0, thread_ptr->peak_heap, thread_ptr->total_heap, total_leaks, leak_size,
				thread_ptr->total_pkts, thread_ptr->axi, thread_ptr->slot0_pkts, thread_ptr->slot1_pkts, thread_ptr->slot2_pkts, thread_ptr->slot3_pkts, thread_ptr->attributes);
	}
	sessionid = SESSION_OTHER;
}

unsigned long long get_profiling_cycles(int sessionid) {
	unsigned long long cycles = get_cycle();
	int index = 0;
	profile_t *thread_prof = get_profile_session(sessionid);
	if(thread_prof == NULL) {
		return cycles;
	}
	for(index = 0; index < MAX_HW_THREADS; index++) {
		cycles += thread_prof->thread_cycles[index];
	}
	if(thread_prof->state == 1) {
		thread_prof->profile_cycles = get_cycle() - cycles;
	}
	return cycles;
}

void add_session_attributes(int sessionid, const char *name, const char *value) {
	unsigned long long cycles = get_cycle();
	profile_t *thread_prof = get_profile_session(sessionid);

	int name_length = strlen(name);
	int value_length = strlen(value);
	attr_t *new_attr = (attr_t*)_wrappers.malloc(sizeof(attr_t));
	new_attr->name = (char*)_wrappers.malloc(name_length + 1);
	memset(new_attr->name, 0, name_length + 1);
	new_attr->value = (char*)_wrappers.malloc(value_length + 1);
	memset(new_attr->value, 0, value_length + 1);
	strncpy(new_attr->name, name, name_length);
	strncpy(new_attr->value, value, value_length);
	new_attr->next = NULL;

	attr_t *attrib = thread_prof->attributes;
	if(attrib == NULL) {
		thread_prof->attributes = new_attr;
	} else {
		while(attrib->next != NULL) {
			attrib = attrib->next;
		}
		attrib->next = new_attr;
	}
	if(thread_prof->state == 1) {
		thread_prof->profile_cycles = get_cycle() - cycles;
	}
}

void* __wrap_malloc(size_t size) {
	unsigned long long prof_cycle = 0;
	FARF(HIGH, "Entering malloc\n");
	if(size <= 0) {
		return NULL;
	}
	void *ptr = _wrappers.malloc(size);
	if(ptr != NULL) {
		prof_cycle = get_cycle();
		int session_id = get_session_id();
		profile_t *thread_ptr = get_profile_session(session_id);
		if(thread_ptr == NULL) {
			return ptr;
		}
		add_heap_stats(thread_ptr, session_id, ptr, size);
		thread_ptr->profile_cycles += (get_cycle() - prof_cycle);
	}
	return ptr;
}

void* __wrap_realloc(void* ptr, size_t size) {
	unsigned long long prof_cycle = 0;

	if(size == 0) {
		return ptr;
	}
	if(ptr == NULL) {
		return __wrap_malloc(size);
	}

	void *new_ptr = _wrappers.malloc(size);
	if(new_ptr != NULL) {
		memcpy(new_ptr, ptr, size);
		_wrappers.free(ptr);
	}
	prof_cycle = get_cycle();
	if(ptr != NULL && new_ptr != NULL) {
		int session_id = get_session_id();
		profile_t *thread_ptr = get_profile_session(session_id);
		if(thread_ptr == NULL) {
			return new_ptr;
		}
		remove_heap_stats(thread_ptr, session_id, ptr);
		add_heap_stats(thread_ptr, session_id, new_ptr, size);
		thread_ptr->profile_cycles += (get_cycle() - prof_cycle);
	}
	return new_ptr;
}

void* __wrap_calloc(size_t num_elem, size_t size) {
	void *new_ptr = __wrap_malloc(size * num_elem);
	if(new_ptr != NULL) {
		memset(new_ptr, 0, size * num_elem);
	}
	return new_ptr;
}

void __wrap_free(void *ptr) {
	unsigned long long prof_cycle = 0;
	int session_id = get_session_id();

	if(ptr == NULL) {
		return;
	}
	_wrappers.free(ptr);
	prof_cycle = get_cycle();
	profile_t *thread_ptr = get_profile_session(session_id);
	if(thread_ptr == NULL) {
		return;
	}
	remove_heap_stats(thread_ptr, session_id, ptr);
	thread_ptr->profile_cycles += (get_cycle() - prof_cycle);
}
