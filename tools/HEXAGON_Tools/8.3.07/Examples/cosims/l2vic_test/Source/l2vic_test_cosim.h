/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef _L2VIC_TEST_COSIM_
#define _L2VIC_TEST_COSIM_

#include <HexagonWrapper.h>

typedef struct
{
  HexagonWrapper *issHandler; //used to talk to iss core
}L2VIC_Test_Cosim;

void L2VIC_Test_Cosim_Init (L2VIC_Test_Cosim *pContext, HexagonWrapper *pHexWrapper);
void L2VIC_Test_Cosim_Free (L2VIC_Test_Cosim *pContext);
void L2VIC_Test_Cosim_TriggerINT(void *pContext);

#endif
