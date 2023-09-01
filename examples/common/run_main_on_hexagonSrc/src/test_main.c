/*==============================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include "HAP_farf.h"

int main(int argc, char *argv[])
{
   FARF(ALWAYS, "In main() with %d args", argc);
   for (int i = 0; i < argc; i++) {
      FARF(ALWAYS, "  argv[%d]: %s", i, argv[i]);
   }

   return 0;
}

