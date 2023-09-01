/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "FunctionProfiler.h"

extern "C"
{
	void INTERFACE *RegisterCosimArgs(char ** name, HexagonWrapper *simPtr, char *args)
	{
		int cerror;
		FunctionProfilerCosim *pFunctionProfiler = new FunctionProfilerCosim(simPtr);

		pFunctionProfiler->AddName(name, args);
		cerror = pFunctionProfiler->ParseValidateArgs(args);
		if (cerror < 0)
		{ 
			pFunctionProfiler->PrintArgUsage(cerror);
			exit(0);
		}

		return (void *)pFunctionProfiler;
	}

	char INTERFACE *GetCosimVersion()
	{
		return (char *)HEXAGON_WRAPPER_VERSION;
	}

	void INTERFACE UnRegisterCosim(void *handle)
	{
		// Be sure to destroy the class so its destructor gets called
		FunctionProfilerCosim *pCosim = (FunctionProfilerCosim *) handle;
		delete pCosim;
	}
}

FunctionProfilerCosim::FunctionProfilerCosim(HexagonWrapper *pW)
{
	pWrapper = pW;
	pHead = NULL;
	name = NULL;
	functionName = NULL;
	funcAddr = 0;
	retAddr = INVALID_ADDRESS;		// Invalid Hexagon address for an instruction
	numIterations = 0;
	totCycles = 0;
	minCycles = 0xffffffffffffffffULL;
	minCycleTime = 0;	// Start cycle of iteration with max cycle count
	maxCycles = 0;
	maxCycleTime = 0;	// Start cycle of iteration with min cycle count
}

// The destructor should calculate the min / max / average for the region
// Be sure to free allocated memory
FunctionProfilerCosim::~FunctionProfilerCosim()
{
	OneIteration *pIter = pHead;
	OneIteration *pIter2;
	FILE *fp;
	char *fname, *cmd;

	if(pHead)		// Only need to do this if there was some activity
	{
		printf("\nFor function ");
		if(functionName)
		{
			printf("%s ", functionName);
		}
		printf("at address 0x%x:\n", funcAddr);

		printf("\t%lld iterations\n\tavg = %.2f cycles\n\tmax = %lld cycles for iteration %lld starting at cycle %lld\n\tmin = %lld cycles for iteration %lld starting at cycle %lld\n",
			numIterations, (float) totCycles / (float) numIterations, maxCycles, maxIteration, maxCycleTime, minCycles, minIteration, minCycleTime);
		fflush(stdout);
	} else
	{
		printf("No data collected from PC Range Profiler\n");
	}

	if(functionName)
	{
		fname = (char *) calloc(1, strlen(functionName)+10);
		sprintf(fname, "%s.dat", functionName);
	} else
	{
		fname = (char *) calloc(1, 25);
		sprintf(fname, "0x%x.dat", funcAddr);
	}

	fp = fopen(fname, "w");

	while(pIter)
	{
		if(fp != NULL)
		{
			fprintf(fp, "Iteration: %lld Start cycle: %lld , End cycle: %lld", numIterations--, pIter->startCycle, pIter->endCycle);
			fprintf(fp, " Cycle count: %lld\n", pIter->numCycles);
		}
		pIter2 = pIter->pNext;
		free(pIter);
		pIter = pIter2;
	}

	if(fp != NULL)
	{
		fclose(fp);
		cmd = (char *) calloc(1, strlen(fname) * 2 + 20);
		sprintf(cmd, "sort -n -k 2 %s -o %s", fname, fname);
		system(cmd);
		free(cmd);
	}

	if(functionName) free(functionName);
	if(fname) free(fname);
}

// Start a new iteration range.  Note that the previous range must be completed or an error is returned.
// Returns 0 on success, 1 on error
int FunctionProfilerCosim::AddNewIteration(HEX_8u_t start_cycle)
{
	OneIteration *pIter = (OneIteration *) calloc(1, sizeof(struct OneIteration));

	pIter->startCycle = start_cycle;

	if(pHead == NULL)
	{
		pHead = pIter;
		return 0;
	}

	if(pHead->endCycle == (HEX_8u_t) 0)
	{
		return 1;	// Start before previous end
	}

	pIter->pNext = pHead;
	pHead = pIter;

	return 0;
}

// Set the end cycle count for a previously-set start cycle.  Return error if not yet started.
// Returns 0 on success, 1 on error
int FunctionProfilerCosim::SetEndCycle(HEX_8u_t end_cycle)
{
	if(pHead == NULL)
	{
		return 1;	// Nothing there!  End before Start
	}

	if(pHead->endCycle != (HEX_8u_t) 0)
	{
		return 1;	// Already ended this one - double end
	}

	numIterations++;

	pHead->endCycle = end_cycle;
	pHead->numCycles = pHead->endCycle - pHead->startCycle;
	if(pHead->numCycles < minCycles)
	{
		minCycles = pHead->numCycles;
		minCycleTime = pHead->startCycle;
		minIteration = numIterations;
	}

	if(pHead->numCycles > maxCycles)
	{
		maxCycles = pHead->numCycles;
		maxCycleTime = pHead->startCycle;
		maxIteration = numIterations;
	}

	if((totCycles + pHead->numCycles) < totCycles)
	{
		printf("Total cycles overflowed.  Average will be incorrect.\n");
	}

	totCycles += pHead->numCycles;

	return 0;
}

int FunctionProfilerCosim::ParseValidateArgs(char *args)
{
	char *tok= args;
	char *symbol1;
	HEX_PA_t FuncAddress;
	HEX_VA_t VAFuncAddress;
	HEXAPI_Status stat;

	//printf("ARGS='%s'\n", args);

	if(args == 0) 
		return -1; // Null arguments

	while(*tok == ' ') tok++;

	if((tok[0] == '0') && (tolower(tok[1]) == 'x'))
	{
		sscanf(tok, "0x%llx", &(FuncAddress));
		functionName = NULL;
		funcAddr = FuncAddress;
	} else
	{
		symbol1 = tok;
		while((*tok != ' ') && (*tok != '\0')) tok++;
		*tok++ = '\0';

		stat = pWrapper->ReadSymbolValue(symbol1, &VAFuncAddress);
		if(stat != HEX_STAT_SUCCESS)
		{
			return -3;
		}

#if 0
		stat = pWrapper->TranslateVirtualToPhysical(VAFuncAddress, 0xFFFFFFFF, &FuncAddress);
		if(stat != HEX_STAT_SUCCESS)
		{
			return -3;
		}
#else
		FuncAddress = VAFuncAddress;
		funcAddr = FuncAddress;
#endif

		functionName = (char *) calloc(1, strlen(symbol1)+5);
		strcpy(functionName, symbol1);
	}

	printf("Profiling the function ");
	if(functionName != NULL)
	{
		printf("%s ", functionName);
	}
	printf("whose address is 0x%llx\n", FuncAddress);

	stat = pWrapper->AddPCCallback((void *) this, FuncAddress, FunctionProfilerCosimStartAddressHit);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("Cannot set PC callback at address 0x%llx\n", FuncAddress);
		return -999;
	}

	return 1;
}

// Add a new start cycle at the current cycle
void FunctionProfilerCosimStartAddressHit(void *handle)
{
	HEX_8u_t pcycles;
	HEX_4u_t hi, lo;
	HEXAPI_Status stat;
	HEX_4u_t thread;
	HEX_4u_t retaddr;
	FunctionProfilerCosim *pCosim = (FunctionProfilerCosim *) handle;
	HexagonWrapper *pWrapper = ((FunctionProfilerCosim *) handle)->GetWrapper();

	if(!IsAddressInvalid(pCosim->getRetAddr()))
	{
		printf("This appears to be a recursive function.  Results are unpredictable.\n");
	}

	stat = pWrapper->GetCurrentHWThreadNum(&thread);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("GetCurrentHWThreadNum failed!\n");
		thread = 0;
	}

	stat = pWrapper->ReadThreadRegister(thread, TH_REG_R31, &retaddr);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("ReadThreadRegister failed!\n");
		retaddr = 0;
	}

	stat = pWrapper->ReadGlobalRegister(G_REG_PCYCLEHI, &hi);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("ReadGlobalRegister(PCYCLEHI) failed!\n");
		hi = 0xffffffff;
	}

	stat = pWrapper->ReadGlobalRegister(G_REG_PCYCLELO, &lo);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("ReadGlobalRegister(PCYCLELO) failed!\n");
		lo = 0xffffffff;
	}

	pcycles = ((HEX_8u_t) hi << 32) | (HEX_8u_t) lo;
	//printf("StartAddress hit: pcycle=0x%llx\n", pcycles);

	if(pCosim->AddNewIteration(pcycles))
	{
		printf("ERROR: Start address hit without previous end address being hit (recursive function?).\n");
		printf("  Iteration at cycle 0x%llx dropped.\n", pcycles);
	}

	pCosim->setRetAddr(retaddr);
	stat = pWrapper->AddPCCallback(handle, pCosim->getRetAddr(), FunctionProfilerCosimEndAddressHit);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("Cannot set PC callback at address 0x%llx\n", pCosim->getRetAddr());
	}
}

// Terminate the region.
void FunctionProfilerCosimEndAddressHit(void *handle)
{
	HEX_8u_t pcycles;
	HEX_4u_t hi, lo;
	HEXAPI_Status stat;
	FunctionProfilerCosim *pCosim = (FunctionProfilerCosim *) handle;
	HexagonWrapper *pWrapper = ((FunctionProfilerCosim *) handle)->GetWrapper();

	stat = pWrapper->RemovePCCallback(handle, pCosim->getRetAddr());
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("RemovePCCallback failed!\n");
	}
	pCosim->setRetAddr(INVALID_ADDRESS);

	stat = pWrapper->ReadGlobalRegister(G_REG_PCYCLEHI, &hi);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("ReadGlobalRegister(PCYCLEHI) failed!\n");
		hi = 0xffffffff;
	}

	stat = pWrapper->ReadGlobalRegister(G_REG_PCYCLELO, &lo);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("ReadGlobalRegister(PCYCLELO) failed!\n");
		lo = 0xffffffff;
	}

	pcycles = ((HEX_8u_t) hi << 32) | (HEX_8u_t) lo;
	//printf("EndAddress hit: pcycle=0x%llx\n", pcycles);

	if(pCosim->SetEndCycle(pcycles))
	{
		printf("ERROR: End address hit without previous start address being hit.\n");
		printf("  Iteration at cycle 0x%llx dropped.\n", pcycles);
	}
}

void FunctionProfilerCosim::PrintArgUsage(int error)
{
	printf("\n%s: Argument error for cosim. Usage: %s <StartAddr> <EndAddr>\n", name, name);
	if(error == -1) printf("No arguments passed to cosim\n");
	else if(error == -3) printf("Could not find start address\n");
	else if(error == -4) printf("Could not find end address\n");
}
