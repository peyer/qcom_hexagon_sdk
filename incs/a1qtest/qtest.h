#ifndef QTEST_H
#define QTEST_H
/*
=======================================================================
              Copyright ©  2008 Qualcomm Technologies Incorporated.
                        All Rights Reserved.
                Qualcomm Confidential and Proprietary
=======================================================================
*/

//--------------------------------------------------------------------
// qtest.h
//
// Useful macros for "QTests": executable programs (command-line executables
// in Win32) that perform tests.  They implement main() and rely on <stdlib.h>
// and <stdio.h> services.  On success they sliently return zero.  On failure,
// they display a message and return a non-zero value.
//
// A typical QTest will exerice functions and perform a number of
// assertions, stopping execution when an error is detected.
//--------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static __inline void *Zalloc(size_t n)
{
   void *pv = malloc(n);
   if (pv) {
      memset(pv, 0, n);
   }
   return pv;
}

#define MALLOCNEW(t)      ((t *) malloc(sizeof(t)))
#define ZALLOCNEW(t)      ((t *) Zalloc(sizeof(t)))
#define MALLOCNEW_EX(t,n) ((t *) malloc(sizeof(t)+(size_t)n))
#define ZALLOCNEW_EX(t,n) ((t *) Zalloc(sizeof(t)+(size_t)n))

#if (((defined __linux__) && !(defined ANDROID)) || (defined __APPLE__))
#include <execinfo.h>

static __inline char* print_stack(void) {
   int bufsz = 0, sz = 0;
   char* buf = 0;
   void* callstack[256];
   int i, frames = backtrace(callstack, 256);
   char** strs = backtrace_symbols(callstack, frames);
   for (i = 0; i < frames; ++i) {
      bufsz += snprintf(0, 0, "%s\n", strs[i]);
   }
   buf = malloc(bufsz);
   assert(buf != 0);
   for (i = 0; i < frames && bufsz > 0; ++i) {
      sz += snprintf(buf + sz, bufsz, "%s\n", strs[i]);
      bufsz -= sz;
   }
   free(strs);
   return buf;
}

#else

static __inline char* print_stack(void) {
   return 0;
}

#endif //ANDROID

static __inline void exit_stack(void) {
   char* stack = print_stack();
   if(stack) {
      printf("%s", stack);
      free(stack);
   }
   exit(-1);
}

#define QERROR(fmt, ...) \
      ((void) printf("%s:%d: ERROR: " fmt "\n", __FILE__, __LINE__,  ##__VA_ARGS__), exit_stack())


#define QERROR1(str, arg1) QERROR(str, arg1) 

#define ASSERTMSG(exp,str) \
   ((exp) || (QERROR(str), FALSE))

#undef VERIFY
#undef ASSERT

#define VERIFY(exp) \
   ASSERTMSG(exp, "Assertion failed: " #exp)

#define ASSERT(exp) \
   (void)VERIFY(exp)

#define ASSERTEQ(a,b) \
   ASSERT((a) == (b))

#define ASSERT_SUCCESS(exp) \
   (void)ASSERTMSG(0 == (exp), "Failed: " #exp)


#endif /* QTEST_H */
