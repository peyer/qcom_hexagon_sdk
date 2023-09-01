/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
 *****************************************************************/
/*

   Semi-hosted function cosim module 

   This cosim module demonstrates a method for semi-hosting of a 
   run-time function call. In this context, semi-hosting means that 
   the functionality provided by the function call is handled by 
   the cosim. This allows complex, time consuming functionality to
   be rapidly simulated on the host system rather than being 
   simulated on the Hexagon simulator. 

 */

#include "HexagonWrapper.h"
#include <stdlib.h>
#include <assert.h>
#include "hfiles.h"

// Callback declarations. The name of the callback is arbitrary. 
void BSSCallback (void *);
void SemiHostCallback (void *);

// The following three functions are required in every cosim, and must
// be declared as extern "C" for visibility to the simulator at run time.
extern "C" {

	// RegisterCosim declares a name for the cosim and registers for
	// the user-defined instruction callback.
	INTERFACE void  *RegisterCosim(char **name, HexagonWrapper *pHexWrapper)
	{
		// Register the name of the cosim.
		*name="SHFCosim"; 

		// We cannot register for the PC callback required to do the semihosting
		// until configuration of the simulator is complete, since the 
		// executable binary is not loaded yet. Therefore, we must first
		// register for a "Before simulation Starts" callback. When this callback
		// is triggered, we can then register for the callback to do the semi-
		// hosting. 
		assert(pHexWrapper->AddBeforeSimulationStartsCallback((void *) pHexWrapper, 
					BSSCallback) == HEX_STAT_SUCCESS);

		// Return an arbitrary pointer used by the simulator to identify the cosim.
		// This value must be unique! Allocating a pointer on the heap ensures
		// uniqueness, although it would be just as valid to pass a handle to a 
		// cosim object, if C++ was being used to develop the cosim. 
		int *handle = (int *) calloc(1, sizeof(int));
		return handle ; 
	}

	// GetCosimVersion returns the API version,  as defined in HexagonWrapper.h. 
	// This is to help ensure compatibility between cosims and the simulator. 
	INTERFACE char * GetCosimVersion()
	{ // Return the Hexagon wrapper version.
		return (char *)HEXAGON_WRAPPER_VERSION; // Defined in HexagonWrapper.h.
	}

	// UnRegisterCosim is called on system exit and provides an opportunity for 
	// the  cosim to clean up any resources it used during a run. We are not 
	// using any resources in this cosim, so nothing needs to be done here. 
	INTERFACE void UnRegisterCosim(void *pSHFCosim)
	{    
		//free pSHFCosim;
		return;
	}
}

void BSSCallback(void *HexWrapperPtr)
{
    // Convert the handle back to a pointer to the HexagonWrapper object.
	HexagonWrapper *pHexWrapPtr = (HexagonWrapper *) HexWrapperPtr;
	HEX_4u_t funcAddr;

    // Read the virtual address of the function to be semi-hosted.  
	assert(pHexWrapPtr->ReadSymbolValue("Cosim_based_func", &funcAddr) == HEX_STAT_SUCCESS);

	// Use this address to registr for a PC callback. When program execution 
	// hits the address of the function to be semi-hosted, the semi-host callback
	// will be triggered. 
	assert(pHexWrapPtr->AddPCCallback(HexWrapperPtr, 
				(HEX_VA_t) funcAddr, 
				SemiHostCallback) == HEX_STAT_SUCCESS);

}

void SemiHostCallback(void *HexWrapperPtr)
{
	// Convert the handle back to a pointer to the HexagonWrapper object.
    HexagonWrapper *pHexWrapPtr = (HexagonWrapper *) HexWrapperPtr;
	
    printf("\n\n\t>>>>Entering SemiHostCallback\n");

    HEXAPI_Status status = HEX_STAT_SUCCESS;
    // Declare space for function arguments and current thread
    HEX_4u_t arg1, arg2, threadId;

    // Determine the active HW thread 
    if (HEX_STAT_SUCCESS != (status = pHexWrapPtr->GetCurrentHWThreadNum(&threadId)))
    {
        printf("GetCurrentHWThreadNum failed status = %d\n", status);
        fflush(stdout);
    }
    
    // Read the function arguments from r0 to r5 registers if necessary 
	// In this case read the r0 and r1
    assert(pHexWrapPtr->ReadThreadRegister(threadId, TH_REG_R0, &arg1)==HEX_STAT_SUCCESS);
    assert(pHexWrapPtr->ReadThreadRegister(threadId, TH_REG_R1, &arg2)==HEX_STAT_SUCCESS);

    // Create local variables to store arguments to this semi hosted function
    SIMPLE_STRUCT Struct_local;
    int32 i_local;
    pint32 p_i_local = &i_local;    
    pSIMPLE_STRUCT p_Struct_local = &Struct_local;

    // Assign the values to local variables
    assert(pHexWrapPtr->ReadMemory((HEX_PA_t)arg2, 4, p_i_local)==HEX_STAT_SUCCESS);
    assert(pHexWrapPtr->ReadMemory((HEX_PA_t)arg1, 8, p_Struct_local)==HEX_STAT_SUCCESS);

    // Do some computation to modify those values
    printf("Square all input variables\n");
    printf("Input values\ts.a = %d;\ts.b = %d;\ti = %d\n", Struct_local.a, Struct_local.b, i_local);
    Struct_local.a *= Struct_local.a;
    Struct_local.b *= Struct_local.b;
    i_local *= i_local;

    printf("Output values\ts.a = %d;\ts.b = %d;\ti= %d\n", Struct_local.a, Struct_local.b, i_local);	
    
    // Write to these two memory locations
    unsigned long long int temp64bit = *(unsigned long long int *)p_Struct_local;
    assert(pHexWrapPtr->WriteMemory((HEX_PA_t)arg1, 8, temp64bit)==HEX_STAT_SUCCESS);
	unsigned int temp32bit = *(unsigned long long int *)p_i_local;
	temp64bit = temp32bit;
    assert(pHexWrapPtr->WriteMemory((HEX_PA_t)arg2, 4, temp64bit)==HEX_STAT_SUCCESS);

    printf("\t<<<<Exiting SemiHostCallback\n");
}