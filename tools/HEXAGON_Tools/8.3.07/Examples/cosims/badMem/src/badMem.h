/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef __BADMEM_H
#define __BADMEM_H

#include "HexagonWrapper.h"

typedef struct address_range_t {
    pair<unsigned int, unsigned int> element;
}Arange;

Arange  ArangeSet(unsigned int first, unsigned int second); 

typedef struct badMem_t{
	HexagonWrapper * m_simHandle;
    bool fatal;
    vector<Arange> address_range;
    vector<Arange> bad_address_range;

} BadMem;

pair<unsigned int, unsigned int> * el_bar(BadMem *bm, int i) { return &((bm->bad_address_range[i]).element);}
unsigned int  sz_bar(BadMem *bm) { return (bm->bad_address_range).size(); } 
// utility functions
int  ParseValidateArgs(BadMem *pbm, char *args);
void printArgUsage(int err);
void BadMemInit(BadMem *bm, HexagonWrapper *pSimhandle);
void BadMemFree(BadMem *bm);
void PrintSysState(BadMem *bm);

// callback functions
void MemReadCallback(void * pBM, HEX_PA_t addr,  HEX_8u_t value, HEX_4u_t sizeInBytes);
void MemWrittenCallback(void *pBM, HEX_PA_t addr, HEX_8u_t value, HEX_4u_t sizeInBytes);

#endif // __BADMEM_H
