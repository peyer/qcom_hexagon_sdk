/*==============================================================================
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include <AEEstd.h>

#include "run_main_on_hexagon.h"

#define FARF_ERROR 1
#define FARF_HIGH 1
#include "HAP_farf.h"

#define BAIL_IF_ZERO(_err_, _x_) do { if (0 == (_x_)) { _err_ = 1; goto bail; } } while(0)
#define FREE(_x_) { if (_x_) { free(_x_); } }
#define MAX_ARGS 100

// deconstruct args string into discrete arguments and module name
static int parse_arguments(char* args, int* argc, char** argv)
{
   int nErr = 0;
   const char delim[2] = " ";
   char* token;
   int largc;

   BAIL_IF_ZERO(nErr, args);
   BAIL_IF_ZERO(nErr, argc);
   BAIL_IF_ZERO(nErr, argv);

   FARF(HIGH, "Arguments passed from AP: %s", args);
   token = strtok(args, delim);
   BAIL_IF_ZERO(nErr, token);
   argv[0] = token;
   largc = 1;

   do {
      token = strtok(NULL, delim);
      if (0 != token) {
         argv[largc++] = token;
      }
   } while (0 != token);

   *argc = largc;

bail:
   return nErr;
}


struct thread_context {
   char* args;
   const char* module;
   int result;
};

static void *ribbon(void* pv) {
   int (*func_ptr)(int argc, char* argv[]);
   int len = 0;
   int argc = 0;
   char* args_local = 0;
   struct thread_context* context = (struct thread_context*)pv;
   char *args = context->args;
   context->result = 1;
   char* argv[MAX_ARGS];
   void* H = 0;
   
   // Open Module
   H = dlopen(context -> module, RTLD_NOW);
   if (!H) {
      FARF(ERROR, "Failed to load module %s", context->module);
      goto bail;
   }
   
   // alloc local args string we can use to break into tokens
   len = strlen(args) + 1;
   args_local = (char*)malloc(len);
   if (0 == args_local) {
      FARF(ERROR, "failed to malloc %d bytes", len);
   }
   strncpy(args_local, args, len);

   // break apart arg string into discrete args
   if (0 != parse_arguments(args_local, &argc, argv)) {
      FARF(ERROR, "Failed to parse args: %s", args);
      return NULL;
   }

   FARF(HIGH, "args to main main()");
   for (int i = 0; i < argc; i++) {
      FARF(HIGH, "  argv[%d]: %s", i, argv[i]);
   }

   // lookup main()
   func_ptr = (int (*)(int, char**))dlsym(H, "main");
   if (!func_ptr) {
      FARF(ERROR, "No main() found in module %s", context->module);
      goto bail;
   }

   // call main()
   FARF(HIGH, "Calling main() in module %s", context->module);
   context->result = (*func_ptr)(argc, argv);
   FARF(HIGH, "main() returned %d", context->result);
   
bail:
   FREE(args_local);
   if (H)
       dlclose(H);
   return NULL;
}

int run_main_on_hexagon_doit(remote_handle64 handle64, unsigned int stack_size, const char* args, int* result)
{
   int nErr = 0;
   char* module;
   int len = 0;

   char* args_local = 0;
   char* stack = 0;
   struct thread_context context;
     
   // Check args passed by user
   if (0 == args || 0 == strlen(args)) {
      FARF(ERROR, "Bad args");
      return 1;
   }
   // alloc local args string we can use to break into tokes
   len = strlen(args) + 1;
   args_local = (char*)malloc(len);
   if (0 == args_local) {
      FARF(ERROR, "failed to malloc %d bytes", len);
   }
   strncpy(args_local, args, len);

   // Get Module
   module = strtok(args_local, " ");
   if (0 == module) {
      FARF(ERROR, "module not found");
      return 1;
   }
   context.args = (char *)args;
   context.module = module;

   // Start ribbon thread.
   pthread_t pid = 0;
   pthread_attr_t thread_attr;
   pthread_attr_t* thread_attrp;
   int exit_status[1];
   int status;

   thread_attrp = NULL;

   thread_attrp = &thread_attr;
   status = pthread_attr_init(thread_attrp);
   if (status) {
      FARF(ERROR, "Error %d from pthread_attr_init", status);
      nErr = 1;
      goto bail;
   }
   stack = (char*)malloc(stack_size);
   if (0 == stack) {
      FARF(ERROR, "Failed to malloc stack size 0x%X", stack_size);
      return 1;
   }
   FARF(HIGH, " malloc stack size 0x%X", stack_size);
   pthread_attr_setthreadname(thread_attrp, "ribbon");
   pthread_attr_setstackaddr(thread_attrp,stack);
   pthread_attr_setstacksize(thread_attrp,stack_size);
   status = pthread_create(&pid, thread_attrp, ribbon, &context);
   if (status) {
      FARF(ERROR, "Error %d from pthread_create", status);
      nErr = 1;
      goto bail;
   }
   // Wait for ribbon to finish.
   status = pthread_join(pid, (void**) &exit_status);
   if (status) {
      FARF(ERROR, "Error %d from pthread_join", status);
      nErr = 1;
      goto bail;
   }
   FARF(HIGH, "Main() returned %d", context.result);
   *result = context.result;

bail:
   if (thread_attrp != NULL) {
     if (0 != (status = pthread_attr_destroy(thread_attrp))) {
        FARF(ERROR, "Error %d from pthread_attr_destroy", status);
        nErr = 1;
     }
   }
   FREE(args_local);
   FREE(stack);
   return nErr;
}

/**
 * @param handle, the value returned by open
 * @retval, 0 for success, should always succeed
 */
int run_main_on_hexagon_close(remote_handle64 handle)
{
   FREE((void*)handle);
   return 0;
}

int run_main_on_hexagon_open(const char* uri, remote_handle64* handle)
{
   void* tptr = 0;

   /* can be any value or ignored, rpc layer doesn't care
    * also ok
    * *handle = 0;
    * *handle = 0xdeadc0de;
    */
   tptr = (void*)malloc(1);
   if (0 == tptr) {
      FARF(ERROR, "failed to malloc 1 bytes");
      return -1;
   }
   *handle = (remote_handle64)tptr;
   return 0;
}

