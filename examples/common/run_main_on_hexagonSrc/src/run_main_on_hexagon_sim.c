/*==============================================================================
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "dlfcn.h"

#include "qurt.h"

#define FARF_ERROR 1
#define FARF_HIGH 1
#define FARF_MEDIUM 1
#include "HAP_farf.h"
#include "run_main_on_hexagon.h"

extern void qurt_process_cmdline_get(char *buf, unsigned buf_siz);

#define MAX_BUF_SIZE 2048
char args_buf[MAX_BUF_SIZE];
unsigned int stack_size = 0;
#define DEFAULT_STACK_SIZE 1024*256

struct thread_context {
   char* args;
   int result;
};

int main(void)
{
   struct thread_context context;
   char* stack_arg;
   int status;

   // init the loader
   DL_vtbl vtbl = { sizeof(DL_vtbl), HAP_debug_v2 };
   char *builtin[] = { (char *)"libc.so", (char *)"libgcc.so" };
   if (0 == dlinitex(2, builtin, &vtbl)) {
      FARF(ERROR, "Failed to init loader");
      return 1;
   }

   // get arguments
   args_buf[0] = '\0';
   qurt_process_cmdline_get(args_buf, MAX_BUF_SIZE);
   if (strlen(args_buf) == 0) {
      FARF(ERROR, "Failed to get args from QuRT");
      return 1;
   }
   FARF(HIGH, "Args from QuRT: %s", args_buf);

   // read optional stack size from user and malloc
   stack_arg = strstr(args_buf, "stack_size=");
   if (stack_arg) {
      stack_arg += strlen("stack_size=");
      stack_size = (int)strtol(stack_arg, NULL, 0);
   } else {
      stack_size = DEFAULT_STACK_SIZE;
   }
   
   if (stack_size < DEFAULT_STACK_SIZE) {
         FARF(HIGH, "Stack size less than 0x%X is not allowed",DEFAULT_STACK_SIZE);
         return -1;
   }

   // everything after "--" comes from the user, only pass that
   context.args = strstr(args_buf, "--");
   if (context.args) {
      context.args += strlen("--");
   } else {
      context.args = "";
   }

  status = run_main_on_hexagon_doit(0, stack_size, context.args, &context.result);
   if ((int)status) {
      FARF(ERROR, "Error %d while attempting to call main() on dsp", status);
      return 1;
   }
   FARF(HIGH, "Main() returned %d", context.result);

   return context.result;
}

