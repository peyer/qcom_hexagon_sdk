/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef _AXIMEM_
#define _AXIMEM_



#include "HexagonWrapper.h"

#define INVALID_ADDRESS 0x1
#define IsAddressInvalid(x) ((x) == INVALID_ADDRESS)

typedef struct OneIteration		// Holds an iteration from start thru end
{
	HEX_8u_t	startCycle;
	HEX_8u_t	endCycle;
	HEX_8u_t	numCycles;
	struct OneIteration *pNext;
} OneIteration;

class FunctionProfilerCosim
{
public:
	FunctionProfilerCosim(HexagonWrapper *wrapper);
	~FunctionProfilerCosim();

	int ParseValidateArgs(char *args);
	void PrintArgUsage(int error);
	int AddNewIteration(HEX_8u_t startCycle);
	int SetEndCycle(HEX_8u_t endCycle);
	HexagonWrapper *GetWrapper(void) { return pWrapper; }
	HEX_PA_t getRetAddr(void) { return retAddr; }
	void setRetAddr(HEX_PA_t adr) { retAddr = adr; }

	void AddName(char **cosimName, char *args)
	{
		name = "FunctionProfilerCosim";
		*cosimName = name;
	}

	char		*functionName;
	HEX_4u_t	funcAddr;

private:
	char *name;
	
  	HexagonWrapper *pWrapper; //used to talk to iss core

	OneIteration *pHead;

	HEX_PA_t	retAddr;

	HEX_8u_t	numIterations;
	HEX_8u_t	totCycles;
	HEX_8u_t	minCycles;
	HEX_8u_t	minCycleTime;	// Start cycle of iteration with max cycle count
	HEX_8u_t	minIteration;	// Iteration with max cycle count
	HEX_8u_t	maxCycles;
	HEX_8u_t	maxCycleTime;	// Start cycle of iteration with min cycle count
	HEX_8u_t	maxIteration;	// Iteration with min cycle count
};

void FunctionProfilerCosimStartAddressHit(void *handle);
void FunctionProfilerCosimEndAddressHit(void *handle);


#endif 
