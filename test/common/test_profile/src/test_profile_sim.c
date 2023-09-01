/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <hexagon_sim_timer.h>
#include <profile.h>
#include <test_profile.h>
#include <HAP_farf.h>

#define	PROFILE_FILENAME	".profile.xml"

FILE *profile_xml_file = NULL;

void init_profile_ext(void) {
	profile_xml_file = fopen(PROFILE_FILENAME, "w");
	if(profile_xml_file == NULL) {
		return;
	}
	fprintf(profile_xml_file, "<profile>\n");
}

unsigned long long get_cycle(void) {
	return hexagon_sim_read_cycles();
}

void get_tcycle(unsigned long long *tcycles) {
	int index = 0;
	tcycles[0] = get_cycle();
	for(index = 1; index < MAX_HW_THREADS; index++) {
		tcycles[index] = 0;
	}
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

	if(profile_xml_file != NULL) {
		fprintf(profile_xml_file, "\t<runtime>\n\t\t<performance session_name=\"%s\" cycles=\"%llu\" time=\"%llu\" total_heap=\"%d\" peak_heap=\"%d\" num_leaks=\"%d\" leak_size=\"%d\"", sessionName, total_cycles, total_time, total_heap, peak_heap, num_leaks, leak_size);
		attr_t *temp_attr = attr;
		while(temp_attr != NULL) {
			fprintf(profile_xml_file, " %s=\"%s\"", temp_attr->name, temp_attr->value);
			temp_attr = temp_attr->next;
		}
		fprintf(profile_xml_file, "/>");
		fprintf(profile_xml_file, "\n\t\t<power total_pkts=\"%llu\" slot0_pkts=\"%llu\" slot1_pkts=\"%llu\" slot2_pkts=\"%llu\" slot3_pkts=\"%llu\" axi=\"%llu\"/>\n\t</runtime>\n",
				total_pkts, slot0, slot1, slot2, slot3, axi);
	}
	FARF(HIGH, "Profiling Information     :                                         ");
	FARF(HIGH, "     Session/Thread ID    : 0x%08x                              ", sessionid);
	FARF(HIGH, "          Cycles used     : %10llu                              ", total_cycles);
	FARF(HIGH, "          Heap consumed   : %10d bytes                        ", total_heap);
	FARF(HIGH, "          Peak heap       : %10d bytes                        ", peak_heap);
	FARF(HIGH, "          Heap leak       : %10d bytes                        ", leak_size);
}

void deinit_profile_ext(void) {
	fprintf(profile_xml_file, "</profile>\n");
	fclose(profile_xml_file);
}

void pmu_reset(void) {
	asm volatile(
			"r0 = #0\n"
			"pmucnt0 = r0\n"
			"pmucnt1 = r0\n"
			"pmucnt2 = r0\n"
			"pmucnt3 = r0\n"
#ifdef __HEXAGON_V55__
            "s44 = r0\n"
            "s45 = r0\n"
            "s46 = r0\n"
            "s47 = r0\n"
#endif
			"isync\n"
			: : : "r0" );
}

void pmu_config( int tmask, char event0, char event1, char event2, char event3 ) {
	unsigned int pmuevtcfg = (event3<<24) + (event2<<16) + (event1<<8) + event0;
	asm volatile(
			"pmucfg = %0\n"
			"pmuevtcfg = %1\n"
			:
			: "r"(tmask), "r"(pmuevtcfg)
			: "r0" );
}

void pmu_config1( int tmask, char event0, char event1, char event2, char event3 )
{
	unsigned int pmuevtcfg1 = (event3<<24) + (event2<<16) + (event1<<8) + event0;
	asm volatile(
			"pmucfg = %0\n"
			"s54 = %1\n"
			:
			: "r"(tmask), "r"(pmuevtcfg1)
			: "r0" );
}

void pmu_enable(void) {
	asm volatile(
			"r1 = syscfg\n"
			"r1 = setbit(r1, #9)\n"
			"syscfg = r1\n"
			:
			:
			: "r1");
}

unsigned int get_pmu_counter( int event ) {
	unsigned int counter = 0;
	switch (event) {
	case 0:
		asm volatile ("%0 = pmucnt0\n" : "=r"(counter));
		break;
	case 1:
		asm volatile ("%0 = pmucnt1\n" : "=r"(counter));
		break;
	case 2:
		asm volatile ("%0 = pmucnt2\n" : "=r"(counter));
		break;
	case 3:
		asm volatile ("%0 = pmucnt3\n" : "=r"(counter));
		break;
#ifdef __HEXAGON_V55__
	case 4:
		asm volatile ("%0 = s44\n" : "=r"(counter));
		break;
	case 5:
		asm volatile ("%0 = s45\n" : "=r"(counter));
		break;
	case 6:
		asm volatile ("%0 = s46\n" : "=r"(counter));
		break;
	case 7:
		asm volatile ("%0 = s47\n" : "=r"(counter));
		break;
#endif
		}
	return counter;
}

void *tp_malloc(size_t size) {
	return malloc(size);
}

void *tp_realloc(void* ptr, size_t size) {
	return realloc(ptr, size);
}

void *tp_calloc(size_t num, size_t size) {
	return calloc(num, size);
}

void tp_free(void* ptr) {
	free(ptr);
}


