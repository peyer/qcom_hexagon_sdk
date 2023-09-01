/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef _AXIMEM_
#define _AXIMEM_



#include "HexagonWrapper.h"

typedef struct OneIteration		// Holds an iteration from start thru end
{
	HEX_8u_t	startCycle;
	HEX_8u_t	endCycle;
	HEX_8u_t	numCycles;
	struct OneIteration *pNext;
} OneIteration;

class PCRangeProfilerCosim
{
public:
	PCRangeProfilerCosim(HexagonWrapper *wrapper);
	~PCRangeProfilerCosim();

	int ParseValidateArgs(char *args);
	void PrintArgUsage(int error);
	int AddNewIteration(HEX_8u_t startCycle);
	int SetEndCycle(HEX_8u_t endCycle);
	HexagonWrapper *GetWrapper(void) { return pWrapper; }

	void AddName(char **cosimName, char *args)
	{
		name = "PCRangeProfiler";
		*cosimName = name;
	}

	HEX_4u_t	startAddress, endAddress;
	char 		*startSymbol, *endSymbol;

private:
	char *name;
	
  	HexagonWrapper *pWrapper; //used to talk to iss core

	OneIteration *pHead;

	HEX_8u_t	numIterations;
	HEX_8u_t	totCycles;
	HEX_8u_t	minCycles;
	HEX_8u_t	minCycleTime;	// Start cycle of iteration with max cycle count
	HEX_8u_t	minIteration;	// Iteration with max cycle count
	HEX_8u_t	maxCycles;
	HEX_8u_t	maxCycleTime;	// Start cycle of iteration with min cycle count
	HEX_8u_t	maxIteration;	// Iteration with min cycle count
};

void PCRangeProfilerCosimStartAddressHit(void *handle);
void PCRangeProfilerCosimEndAddressHit(void *handle);


#endif 
