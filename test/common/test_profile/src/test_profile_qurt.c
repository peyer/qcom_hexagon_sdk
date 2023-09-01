/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <hexagon_sim_timer.h>
#include <profile.h>
#include <test_profile.h>
#include <qurt_thread.h>
#include <qurt_cycles.h>
#include <qurt_alloc.h>
#include <HAP_farf.h>

void init_profile_ext(void) {
}

unsigned long long get_cycle(void) {
	return qurt_profile_get_thread_pcycles();
}

void get_tcycle(unsigned long long *tcycles) {
	qurt_profile_get_threadid_pcycles(qurt_thread_get_id(), tcycles);
}

void profile_result(int sessionid, unsigned long long total_cycles, unsigned long long total_time, int peak_heap, int total_heap, int num_leaks, int leak_size, unsigned long long total_pkts, unsigned long long axi, unsigned long long slot0, unsigned long long slot1, unsigned long long slot2, unsigned long long slot3, struct ATTR *attr) {

	char sessionName[20] = {0};
	switch(sessionid) {
	case 1:
		strncpy(sessionName, "SetMediaFormat", 15);
		break;
	case 2:
		strncpy(sessionName, "SetParam", 9);
		break;
	case 3:
		strncpy(sessionName, "GetParam", 9);
		break;
	case 4:
		strncpy(sessionName, "Process", 8);
		break;
	case SESSION_OTHER:
		strncpy(sessionName, "Other", 6);
		break;
	case SESSION_ALL:
		strncpy(sessionName, "All", 4);
	}

	FARF(HIGH,  "Profiling Information          :");
	FARF(HIGH,  "	       Session/Thread ID    : 0x%x", qurt_thread_get_id());
	FARF(HIGH,  "	       Cycles used          : %llu", total_cycles);
	FARF(HIGH,  "          Heap consumed        : %d bytes", total_heap);
	FARF(HIGH,  "          Peak heap            : %d bytes", peak_heap);
	FARF(HIGH,  "          Heap leak            : %d bytes", leak_size);
}

void deinit_profile_ext(void) {
}

void pmu_reset(void) {
}

void pmu_config( int tmask, char event0, char event1, char event2, char event3 ) {
}

void pmu_config1( int tmask, char event0, char event1, char event2, char event3 ) {
}

void pmu_enable(void) {
}

unsigned int get_pmu_counter( int event ) {
	return 0;
}

void *tp_malloc(size_t size) {
        return qurt_malloc(size);
}

void *tp_realloc(void* ptr, size_t size) {
        return qurt_realloc(ptr, size);
}

void *tp_calloc(size_t num, size_t size) {
        return qurt_calloc(num, size);
}

void tp_free(void* ptr) {
        qurt_free(ptr);
}


