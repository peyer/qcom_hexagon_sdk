/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/*

    Invalid memory regions

    This cosim module keeps track of invalid memory ranges and reports an
    error on access to these regions

*/

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;
#include "badMem.h"
#include <string.h>
#include <stdio.h>


#define DEBUG 0

void BadMemInit(BadMem *bm, HexagonWrapper *pSimHandle)
{
    bm->m_simHandle=pSimHandle;
    bm->fatal = false;

#if DEBUG
  printf("Construct BADMEM...\n");
#endif

}

void BadMemFree(BadMem *bm)
{
    bm->m_simHandle=NULL;
    bm->address_range.clear();
    bm->bad_address_range.clear();
	free(bm);
}

void PrintSysState(BadMem *pbm)
{
	HEX_4u_t curr_thread, pc_value, pcyclehi, pcyclelo;
    if( pbm->m_simHandle->GetCurrentHWThreadNum(&curr_thread) != HEX_STAT_SUCCESS)
		fprintf(stderr, "Cannot read thread number \n");
    if( pbm->m_simHandle->ReadThreadRegister(curr_thread, TH_REG_PC, &pc_value) != HEX_STAT_SUCCESS)
		fprintf(stderr, "Cannot read PC\n");
    if( pbm->m_simHandle->ReadGlobalRegister(G_REG_PCYCLEHI, &pcyclehi) != HEX_STAT_SUCCESS)
		fprintf(stderr, "Cannot read PCYCLEHI\n");
    if( pbm->m_simHandle->ReadGlobalRegister(G_REG_PCYCLELO, &pcyclelo) != HEX_STAT_SUCCESS)
		fprintf(stderr, "Cannot read PCYCLELO\n");
	printf("{PC:0x%x, PCYCLE: 0x%x%x, TNUM:%d}\n", pc_value, pcyclehi, pcyclelo, curr_thread);
}

void MemReadCallback(void *pBM, HEX_PA_t addr, HEX_8u_t value, HEX_4u_t sizeInBytes)
{
	BadMem *pbm = (BadMem *) pBM;
    printf("%s: ", (pbm->fatal)? "Fatal": "Warning");
    printf("Bad Memory: read addr=0x%llx ",  addr);
	PrintSysState(pbm);
    if(pbm->fatal) exit(0);
}

void MemWrittenCallback(void *pBM, HEX_PA_t addr, HEX_8u_t value, HEX_4u_t sizeInBytes)
{
	BadMem *pbm = (BadMem *) pBM;
    printf("%s: ", (pbm->fatal)? "Fatal": "Warning");
	if(sizeInBytes == 1)
    	printf("Bad Memory: write addr=0x%llx, value=0x%lx ", addr, (HEX_1u_t)value);
	if(sizeInBytes == 2)
    	printf("Bad Memory: write addr=0x%llx, value=0x%lx ", addr, (HEX_2u_t)value);
	if(sizeInBytes == 4)
    	printf("Bad Memory: write addr=0x%llx, value=0x%lx ", addr, (HEX_4u_t)value);
	if(sizeInBytes == 8)
    	printf("Bad Memory: write addr=0x%llx, value=0x%llx ", addr, (HEX_8u_t)value);
	PrintSysState(pbm);
	if(pbm->fatal) exit(0);
}

void printArgUsage(int err)
{
    fprintf(stderr, "BADMEM COSIM: Argument Usage Error\n");
    switch(err){
        case -1:
        fprintf(stderr, "Null arguments \n");
        break;
        case -2:
    	fprintf(stderr, "Invalid args. Arg list is: -w/f\
  0x{range1start@}--0x{range1end@};\
  0x{range2start@}--0x{range2end@}; ....\n");
        break;
        case -3:
        fprintf(stderr, "StartAddress > EndAddress \n");
        break;
    }
}

bool compare_range(Arange a1, Arange a2)
{
    if(a1.element.first < a2.element.first) return true;
    else return false;
}

Arange  ArangeSet(unsigned int first, unsigned int second) { 
	Arange *ar = (Arange *)calloc(1, sizeof(Arange));
	(ar->element).first = first; 
	(ar->element).second = second; 
	return *ar;
}



//----------------------------------------------
// Read argument string and form address ranges
//----------------------------------------------
int ParseValidateArgs(BadMem *pbm, char * args)
{
    char * tok;
    unsigned int temp_start, temp_end;
    if(args == NULL) return -1;

    tok = strstr(args, "-");
    if(tolower(*(tok+1)) == 'w')
    {
        pbm->fatal = false;
        tok +=2;
    }
    else if(tolower(*(tok+1)) == 'f')
    {
        pbm->fatal = true;
        tok +=2;
    }
    else
        tok = args;
    tok = strtok(tok, ";");
    while(tok)
    {
        char * str;
        str = tok;
#if DEBUG
        printf("%s\n", str);
#endif
        str = strstr(str, "0x");
        if(!str) break;                        // NULL sub-string? 
        sscanf(str, "0x%x", &temp_start);
        str = strstr(str+2, "0x");
        if(!str) return -2;
        sscanf(str, "0x%x", &temp_end);
        if (temp_start > temp_end)
            return -3;
        (pbm->address_range).push_back(ArangeSet(temp_start, temp_end));

        if(pbm->address_range.size() > 9)
        {
            cout << "WARNING: COSIM does not support more than 10 memory ranges" << endl;
            break;
        }
        tok = strtok(NULL, ";");
    }
    // Sort based on start address
    //address_range.sort(compare_range);
    sort(pbm->address_range.begin(), pbm->address_range.end(), compare_range);
    // Find and delete overlapping ranges
    
    vector<Arange>::iterator r = pbm->address_range.begin();

    for(unsigned int i=0; i < pbm->address_range.size(); i++)
    {
        unsigned int j = i+1;
        if(j < pbm->address_range.size())
        {
            if(pbm->address_range[j].element.first < pbm->address_range[i].element.second)
            {
                if(pbm->address_range[i].element.second < pbm->address_range[j].element.second) // check partial overlap
                {
                    pbm->address_range[i].element.second = pbm->address_range[j].element.second;  
                }
                r +=j;
                pbm->address_range.erase(r);  // Delete subset range      
            }
        }
    }
    // Find Invalid memory ranges
    r = pbm->address_range.begin();
    if(r->element.first > 0)
    {
        (pbm->bad_address_range).push_back(ArangeSet(0, (r->element.first-1)));
#if 1
        printf("Adding address 0x0--0x%x to the bad address list\n", r->element.first-1);
#endif
    }
    while(r!= pbm->address_range.end())
    {
       temp_start = r->element.second+1;
       r++;
       if(r== pbm->address_range.end())
           temp_end   = 0xffffffff;
       else
           temp_end   = r->element.first-1;
       if(temp_start != 0xffffffff)
       {
           pbm->bad_address_range.push_back(ArangeSet(temp_start, temp_end));
#if 1 
           printf("Adding address 0x%x--0x%x to the bad address list\n", temp_start, temp_end);
#endif
       }
    }

    return 0;
}

//-------------------------------------------------------------------------
// Implement RegisterCosimArgs as arguments need to be passed
//-------------------------------------------------------------------------

extern "C" {

  void INTERFACE *RegisterCosimArgs(char *name, HexagonWrapper *simPtr, char *args)
  {
    int retval;
    unsigned int i;
    pair<unsigned int, unsigned int> *addressRange;
	BadMem *pbm = (BadMem *)calloc(1, sizeof(BadMem));
	BadMemInit(pbm, simPtr);

    retval = ParseValidateArgs(pbm, args);
    if (retval!=0)
    {
        printArgUsage(retval);
        exit(0);
    }

    for(i=0; i< sz_bar(pbm); i++)
    {
        addressRange = el_bar(pbm, i);
        simPtr->AddMemWasReadCallback((void *)pbm, (*addressRange).first, (*addressRange).second, MemReadCallback);
        simPtr->AddMemWasWrittenCallback((void *)pbm,(*addressRange).first, (*addressRange).second, MemWrittenCallback );
    }
	return (void *)pbm;
  }
  
  char INTERFACE *GetCosimVersion()
  {
	return (char *)HEXAGON_WRAPPER_VERSION;
  }

	void UnRegisterCosim(void * handle)
	{
		BadMemFree((BadMem *)handle);
	}

}
