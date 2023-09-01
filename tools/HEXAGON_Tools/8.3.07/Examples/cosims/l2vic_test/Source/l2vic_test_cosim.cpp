/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/

/*
    Second-level Vectored Interrupt Controller

    This cosim takes 2 parameters:
        The number of slices (number of 32-interrupt controllers)
        The base address of the register block

    This cosim emulates <n> L2VICs as described in the ADSP documentation.
    All L2VIC controllers are tied to interrupt 31 of the QDSP6, which is
    required to be positive-edge-sensitive.  The interrupt vector ID is
    ordered from 0 to 1023, with 0 having the highest priority (i.e., if
    multiple interrupts are present, the VID register of the Hexagon processor
    will be filled with the index of the highest-priority asserted interrupt.
    The interrupt is cleared after the Hexagon processor is interrupted.

*/

#include <iostream>
#include "l2vic_test_cosim.h"
#include "l2vic_registers.h"
#include "HexagonWrapper.h"
#include <stdlib.h>
#include <assert.h>

#define DBG	 0
#define DBG_PRINTF(x) if(DBG) printf(x);
#define DBG_PRINTF2(x,y) if(DBG) printf(x,y);

#define REL	 1
#define REL_PRINTF(x) if(REL) printf(x);
#define REL_PRINTF2(x,y) if(REL) printf(x,y);

#define L2VIC_INT_PENDING_COSIM(n)	((L2VIC_BASE) + 0x500 + 4 * (n/32))

void L2VIC_Test_Cosim_Init (L2VIC_Test_Cosim *pContext, HexagonWrapper *pHexWrapper)
{
	DBG_PRINTF("\nL2VIC_Test_Cosim_Init\n");
	HEXAPI_Status statusHEXAPI = HEX_STAT_ERROR;

	pContext->issHandler = pHexWrapper;
	statusHEXAPI = pContext->issHandler->AddTimedCallback((void *) pContext, 5, HEX_MILLISEC, L2VIC_Test_Cosim_TriggerINT);
}

void L2VIC_Test_Cosim_Free (L2VIC_Test_Cosim *pContext)
{
    DBG_PRINTF("\nL2VIC_Test_Cosim_Free\n");
    free(pContext);
}

void L2VIC_Test_Cosim_TriggerINT(void *pContext)
{
    DBG_PRINTF("L2VIC_Test_Cosim_TriggerINT\n");
    static int count = 1;

    // Trigger an Interrupt here
    L2VIC_Test_Cosim *pL2VIC_context = (L2VIC_Test_Cosim *) pContext;
    HexagonWrapper *pHexWrapPtr = pL2VIC_context->issHandler;

    HEXAPI_Status hexstatus = HEX_STAT_ERROR;
    int i = 0;
    int *pi = &i;

	if(count == 4)
	{
		// Remove calling anymore INT
		pHexWrapPtr->RemoveTimedCallback((void *) pL2VIC_context);
	}

    uint32 irq1_bit = (1 << (IRQ1 % 32));
    uint32 irq2_bit = (1 << (IRQ2 % 32));

	// Get the value of the PENDING register
    uint32 pl2vic_int_pending = L2VIC_INT_PENDING_COSIM(IRQ1);
    hexstatus = pHexWrapPtr->ReadMemory(pl2vic_int_pending, sizeof(uint32), (void *) pi);

	// The INT that will be triggered
	if(count == 1)
	{
		// Just trigger IRQ1
		REL_PRINTF2("\tTEST_COSIM >>>>!!!! Triggering INT_%d from cosim <<<<\n", IRQ1);
		i |= irq1_bit;
	}
	else if(count == 2)
	{
		// Just trigger IRQ2
		REL_PRINTF2("\tTEST_COSIM >>>>!!!! Triggering INT_%d from cosim <<<<\n", IRQ2);
		i |= irq2_bit;
	}
	else if(count ==3)
	{
		// Trigger IRQ1 and IRQ2
		REL_PRINTF2("\tTEST_COSIM >>>>!!!! Triggering INT_%d from cosim <<<<\n", IRQ1);
		REL_PRINTF2("\tTEST_COSIM >>>>!!!! Triggering INT_%d from cosim <<<<\n", IRQ2);
		i |= irq1_bit;
		i |= irq2_bit;
	}
	count++;

	// Trigger INT by writing to the pending register
	hexstatus =  pHexWrapPtr->WriteMemory(pl2vic_int_pending, sizeof(uint32), i);
}

extern "C" {

    void INTERFACE *RegisterCosimArgs(char **name, HexagonWrapper *simPtr, char *args)
    {
		DBG_PRINTF("\nRegisterCosimArgs\n");

        L2VIC_Test_Cosim *pContext = (L2VIC_Test_Cosim *)calloc(1, sizeof(L2VIC_Test_Cosim));
        L2VIC_Test_Cosim_Init(pContext, simPtr);

        return (void *)pContext;
    }

    char INTERFACE *GetCosimVersion()
    {
		DBG_PRINTF("GetCosimVersion\n");
		return (char *)HEXAGON_WRAPPER_VERSION;
    }

    void INTERFACE UnRegisterCosim(void *handle)
    {
		DBG_PRINTF("UnRegisterCosim\n");
        L2VIC_Test_Cosim_Free((L2VIC_Test_Cosim *)handle);
    }
}
