#ifndef CALCULATOR_TEST_H
#define CALCULATOR_TEST_H
/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "AEEStdDef.h"

#define DOMAIN_ID_ADSP  (0)
#define DOMAIN_ID_MDSP  (1)
#define DOMAIN_ID_SDSP  (2)
#define DOMAIN_ID_CDSP  (3)

#ifdef __cplusplus
extern "C" {
#endif

int calculator_test(int runMode, int domain, int num);

#ifdef __cplusplus
}
#endif

#endif // CALCULATOR_TEST_H

