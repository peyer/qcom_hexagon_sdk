/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "PCRangeProfiler.h"

extern "C"
{
	void INTERFACE *RegisterCosimArgs(char ** name, HexagonWrapper *simPtr, char *args)
	{
		int cerror;
		PCRangeProfilerCosim *pPCRangeProfiler = new PCRangeProfilerCosim(simPtr);

		pPCRangeProfiler->AddName(name, args);
		cerror = pPCRangeProfiler->ParseValidateArgs(args);
		if (cerror < 0)
		{ 
			pPCRangeProfiler->PrintArgUsage(cerror);
			exit(0);
		}

		return (void *)pPCRangeProfiler;
	}

	char INTERFACE *GetCosimVersion()
	{
		return (char *)HEXAGON_WRAPPER_VERSION;
	}

	void INTERFACE UnRegisterCosim(void *handle)
	{
		// Be sure to destroy the class so its destructor gets called
		PCRangeProfilerCosim *pCosim = (PCRangeProfilerCosim *) handle;
		delete pCosim;
	}
}

PCRangeProfilerCosim::PCRangeProfilerCosim(HexagonWrapper *pW)
{
	pWrapper = pW;
	pHead = NULL;
	name = NULL;
	startSymbol = NULL;
	endSymbol = NULL;
	startAddress = 0;
	endAddress = 0;
	numIterations = 0;
	totCycles = 0;
	minCycles = 0xffffffffffffffffULL;
	minCycleTime = 0;	// Start cycle of iteration with max cycle count
	maxCycles = 0;
	maxCycleTime = 0;	// Start cycle of iteration with min cycle count
}

// The destructor should calculate the min / max / average for the region
// Be sure to free allocated memory
PCRangeProfilerCosim::~PCRangeProfilerCosim()
{
	OneIteration *pIter = pHead;
	OneIteration *pIter2;
	FILE *fp;
	char *fname, *cmd;
	int fnameLen;

	if(pHead)		// Only need to do this if there was some activity
	{
		printf("\nFor range ");
		if(startSymbol)
		{
			printf("%s at ", startSymbol);
		}
		printf("address 0x%x through ", startAddress);
		if(endSymbol)
		{
			printf("%s at ", endSymbol);
		}
		printf("address 0x%x:\n", endAddress);
		printf("\t%lld iterations\n\tavg = %.2f cycles\n\tmax = %lld cycles for iteration %lld starting at cycle %lld\n\tmin = %lld cycles for iteration %lld starting at cycle %lld\n",
			numIterations, (float) totCycles / (float) numIterations, maxCycles, maxIteration, maxCycleTime, minCycles, minIteration, minCycleTime);
		fflush(stdout);
	} else
	{
		printf("\nNo data collected from PC Range Profiler\n");
	}

	fnameLen = 0;
	if(startSymbol || endSymbol)
	{
		fnameLen += startSymbol ? strlen(startSymbol) : 10;
		fnameLen += endSymbol ? strlen(endSymbol) : 10;
		fname = (char *) calloc(1, fnameLen+10);	// 0x%08x-0x%08x.dat
		if(startSymbol)
		{
			strcpy(fname, startSymbol);
		} else
		{
			sprintf(fname, "0x%x", startAddress);
		}
		strcat(fname, "-");
		if(endSymbol)
		{
			strcat(fname, endSymbol);
			strcat(fname, ".dat");
		} else
		{
			sprintf(&fname[strlen(fname)], "0x%x.dat", endAddress);
		}
	} else
	{
		fname = (char *) calloc(1, 28);	// 0x%08x-0x%08x.dat
		sprintf(fname, "0x%x-0x%x.dat", startAddress, endAddress);
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

	free(fname);

	if(startSymbol) free(startSymbol);
	if(endSymbol) free(endSymbol);
}

// Start a new iteration range.  Note that the previous range must be completed or an error is returned.
// Returns 0 on success, 1 on error
int PCRangeProfilerCosim::AddNewIteration(HEX_8u_t start_cycle)
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
int PCRangeProfilerCosim::SetEndCycle(HEX_8u_t end_cycle)
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

int PCRangeProfilerCosim::ParseValidateArgs(char *args)
{
	char *tok= args;
	char *symbol1, *symbol2;
	HEX_VA_t VAStartAddress;
	HEX_VA_t VAEndAddress;
	HEXAPI_Status stat;

	//printf("ARGS='%s'\n", args);

	if(args == 0) 
		return -1; // Null arguments

	while(*tok == ' ') tok++;

	if((tok[0] == '0') && (tolower(tok[1]) == 'x'))
	{
		sscanf(tok, "0x%x", &(startAddress));
		while((*tok != ' ') && (*tok != '\0')) tok++;
		if(*tok == '\0')
		{
			return -4;
		}
	} else
	{
		symbol1 = tok;
		while((*tok != ' ') && (*tok != '\0')) tok++;
		if(*tok == '\0')
		{
			return -4;
		}
		*tok++ = '\0';

		stat = pWrapper->ReadSymbolValue(symbol1, &VAStartAddress);
		if(stat != HEX_STAT_SUCCESS)
		{
			return -3;
		}

		startSymbol = (char *) calloc(1, strlen(symbol1)+5);
		strcpy(startSymbol, symbol1);

#if 0
		stat = pWrapper->TranslateVirtualToPhysical(VAStartAddress, 0xFFFFFFFF, &startAddress);
		if(stat != HEX_STAT_SUCCESS)
		{
			return -3;
		}
#else
		startAddress = VAStartAddress;
#endif
	}

	while(*tok == ' ') tok++;

	if((tok[0] == '0') && (tolower(tok[1]) == 'x'))
	{
		sscanf(tok, "0x%x", &(endAddress));
	} else
	{
		symbol2 = tok;
		while((*tok != ' ') && (*tok != '\0')) tok++;
		*tok++ = '\0';

		stat = pWrapper->ReadSymbolValue(symbol2, &VAEndAddress);
		if(stat != HEX_STAT_SUCCESS)
		{
			return -4;
		}

		endSymbol = (char *) calloc(1, strlen(symbol2)+5);
		strcpy(endSymbol, symbol2);

#if 0
		stat = pWrapper->TranslateVirtualToPhysical(VAEndAddress, 0xFFFFFFFF, &endAddress);
		if(stat != HEX_STAT_SUCCESS)
		{
			return -3;
		}
#else
		endAddress = VAEndAddress;
#endif
	}

	printf("Profiling the region from ");
	if(startSymbol)
	{
		printf("%s ", startSymbol);
	}
	printf("(0x%x) to ", startAddress);
	if(endSymbol)
	{
		printf("%s ", endSymbol);
	}
	printf("(0x%x)\n", endAddress);

	stat = pWrapper->AddPCCallback((void *) this, startAddress, PCRangeProfilerCosimStartAddressHit);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("Cannot set PC callback at address 0x%x\n", startAddress);
		return -999;
	}

	stat = pWrapper->AddPCCallback((void *) this, endAddress, PCRangeProfilerCosimEndAddressHit);
	if(stat != HEX_STAT_SUCCESS)
	{
		printf("Cannot set PC callback at address 0x%x\n", endAddress);
		return -999;
	}

	return 1;
}

// Add a new start cycle at the current cycle
void PCRangeProfilerCosimStartAddressHit(void *handle)
{
	HEX_8u_t pcycles;
	HEX_4u_t hi, lo;
	HEXAPI_Status stat;
	PCRangeProfilerCosim *pCosim = (PCRangeProfilerCosim *) handle;
	HexagonWrapper *pWrapper = ((PCRangeProfilerCosim *) handle)->GetWrapper();

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
		printf("ERROR: Start address hit without previous end address being hit.\n");
		printf("  Iteration at cycle 0x%llx dropped.\n", pcycles);
	}
}

// Terminate the region.
void PCRangeProfilerCosimEndAddressHit(void *handle)
{
	HEX_8u_t pcycles;
	HEX_4u_t hi, lo;
	HEXAPI_Status stat;
	PCRangeProfilerCosim *pCosim = (PCRangeProfilerCosim *) handle;
	HexagonWrapper *pWrapper = ((PCRangeProfilerCosim *) handle)->GetWrapper();

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

void PCRangeProfilerCosim::PrintArgUsage(int error)
{
	printf("\n%s: Argument error for cosim. Usage: %s <StartAddr> <EndAddr>\n", name, name);
	if(error == -1) printf("No arguments passed to cosim\n");
	else if(error == -3) printf("Could not find start address\n");
	else if(error == -4) printf("Could not find end address\n");
}
