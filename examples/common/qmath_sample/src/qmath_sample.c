/**=============================================================================
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "qmath_sample.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __hexagon__     // some defs/stubs so app can build for Hexagon simulation
int apps_mem_heap_init(int id, int flags, int rflags, int size) {
   return 0;
}

void* apps_mem_heap_malloc(int size) { return malloc(size); }
void apps_mem_heap_free(void* po) { return free(po); }
#endif

int test_main_start(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}

// This is a simple program to initiate a test on the DSP. 
int test_main_start(int argc, char* argv[])
{
    char *qmath_sample_URI_Domain;
    remote_handle64 handle = -1;
	
	qmath_sample_URI_Domain = qmath_sample_URI "&_dom=cdsp";
    // first try opening handle on CDSP.
    
	int retVal = qmath_sample_open(qmath_sample_URI_Domain, &handle);
    
    // if CDSP is not present, try ADSP.
    if (retVal != 0)
    {
        printf("cDSP not detected on this target (error code %d), attempting to use aDSP\n", retVal);
        qmath_sample_URI_Domain = qmath_sample_URI "&_dom=adsp";
        retVal = qmath_sample_open(qmath_sample_URI_Domain, &handle);
    }
	if (-1 == handle || 0 != retVal)
    {
        printf("Failed to open a channel to cDSP or aDSP\n");
        return -1;
    }
        
    retVal = qmath_sample_quadratic(handle);
    
    qmath_sample_close(handle);
    handle = -1;
   
    if (retVal) 
    {
        printf("Test FAILED - return value from qmath_sample_quadratic: %d \n", retVal);
    }
    else
    {
        printf("Test PASSED\n");
    }
    printf("See Mini-DM output for more details\n");

    return retVal;
}

