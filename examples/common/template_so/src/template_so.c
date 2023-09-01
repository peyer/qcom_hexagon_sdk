/*==============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "HAP_farf.h"

#include "template_so.h"

int template_so(int n)
{
   FARF(ALWAYS, "template_so received %d", n);
   return n;
}
