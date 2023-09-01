/*==============================================================================
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <test_profile.h>
#include <stdio.h>

extern void* __wrap_malloc(size_t size);
extern void __wrap_free(void* ptr);

void HAP_debug(const char* msg, int level, const char* filename, int line) {
	printf("HAP  HIGH: %s - %s:%d\n", msg, filename, line);
}

void lib_function(void) {

	void *ptr1 = NULL;
	void *ptr2 = NULL, *ptr3 = NULL, *ptr4 = NULL;
	void *ptr5 = NULL, *ptr6 = NULL, *ptr7 = NULL, *ptr8 = NULL, *ptr9 = NULL;
	
	//Total = 0, peak = 0
	ptr1 = __wrap_malloc(10);
	ptr2 = __wrap_malloc(20);
	//Total = 30, peak = 30
	__wrap_free(ptr2);
	ptr3 = __wrap_malloc(30);
	//Total = 60, peak = 40
	ptr4 = __wrap_malloc(40);
	//Total = 100 peak = 80
	__wrap_free(ptr1);
	//second free
	__wrap_free(ptr2);
	__wrap_free(ptr3);
	__wrap_free(ptr4);
	ptr5 = __wrap_malloc(50);
	//Total = 150 peak = 80
	ptr6 = __wrap_malloc(60);
	//Total = 210 peak = 110
	ptr7 = __wrap_malloc(70);
	//Total = 280 peak = 180
	ptr8 = __wrap_malloc(80);
	//Total = 360 peak = 260
	ptr9 = __wrap_malloc(90);
	//Total = 450 peak = 350
	__wrap_free(ptr5);
	__wrap_free(ptr7);
	__wrap_free(ptr8);
	__wrap_free(ptr9);
	//second free
	__wrap_free(ptr6);
	ptr5 = __wrap_malloc(50);
	//Total = 150 peak = 80
	ptr6 = __wrap_calloc(1, 60);
	//Total = 210 peak = 110
	ptr7 = __wrap_malloc(70);
	//Total = 280 peak = 180
	ptr8 = __wrap_calloc(1, 80);
	//Total = 360 peak = 260
	ptr9 = __wrap_malloc(90);
	//Total = 450 peak = 350
	ptr9 = __wrap_realloc(ptr9, 100);
	//Total = 460 peak = 350
	__wrap_free(ptr5);
	__wrap_free(ptr7);
	//__wrap_free(ptr8);
	__wrap_free(ptr9);
	//second free
	__wrap_free(ptr6);		
}


int main(void) {
	printf("Testing other profile\n");
	reset_profiling(1);
	start_profiling(1);
	lib_function();
	stop_profiling(1, 0);
	print_profile_result(1);

	printf("Dummy profile for calculating the overhead\n");
	start_profiling(2);
	stop_profiling(2, 1);

	return 0;
}


