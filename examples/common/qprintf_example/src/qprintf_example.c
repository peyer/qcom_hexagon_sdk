/**=============================================================================
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
_all Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "qprintf_example.h"
#include "AEEStdErr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int test_main_start(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    return test_main_start(argc, argv);
}

// This is a simple program to initiate a test on the DSP. 
int test_main_start(int argc, char* argv[])
{
    char *qprintf_example_URI_Domain;
    remote_handle64 handle = -1;
	qprintf_example_URI_Domain = qprintf_example_URI "&_dom=cdsp";
	
	int retVal = qprintf_example_open(qprintf_example_URI_Domain, &handle);
    
    // if CDSP is not present, try ADSP.
    if (retVal != 0)
    {
        printf("cDSP not detected on this target (error code %d), attempting to use aDSP\n", retVal);
        qprintf_example_URI_Domain = qprintf_example_URI "&_dom=adsp";
        retVal = qprintf_example_open(qprintf_example_URI_Domain, &handle);
    }
	if (-1 == handle || 0 != retVal)
    {
        printf("Failed to open a channel to cDSP or aDSP\n");
        return -1;
    }
        
    retVal = qprintf_example_run(handle);
    
    qprintf_example_close(handle);
    handle = -1;
	
    if (retVal) 
    {
        printf("Test FAILED: %d \n", retVal);
    }
    else
    {
        printf("Test PASSED\n");
    }
    printf("See Mini-DM output for more details\n");

    return retVal;
}

