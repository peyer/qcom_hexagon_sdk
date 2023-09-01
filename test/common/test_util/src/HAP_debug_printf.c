#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WINNT) || defined (_WIN32_WINNT)
#ifndef __MINGW32__
#define vsnprintf(a,b,c,d) vsnprintf_s(a, b, b,c,d)
#pragma warning( disable : 4068 )
#endif
#endif

#include "HAP_debug.h"
static __inline const char* level2str(int level)
{
  switch(level) {
    case HAP_LEVEL_LOW:
      return "   LOW";
      break;
    case HAP_LEVEL_MEDIUM:
      return "MEDIUM";
      break;
    case HAP_LEVEL_HIGH:
      return "  HIGH";
      break;
    case HAP_LEVEL_ERROR:
      return " ERROR";
      break;
    case HAP_LEVEL_FATAL:
      return " FATAL";
      break;
    default:
      return "UNDEF ";
  }
}

void HAP_debug_v2(int level, const char* file, int line, const char* format, ...)
{
  char *buf = 0;
  int n = 0;
  va_list args;
  va_start(args, format);
  n = vsnprintf(0, 0, format, args);
  buf = calloc(n + 1, 1);
  if(buf) {
     vsnprintf(buf, n + 1, format, args);
     va_end(args);
     HAP_debug(buf, level, file, line);
     free(buf);
  }
}
#ifdef __hexagon__
#pragma weak qurt_thread_get_id
extern int qurt_thread_get_id(void);
#endif
void HAP_debug(const char* msg, int level,
               const char* filename, int line)
{
  char* newline = 0;
  int tid = 0;
#ifdef __hexagon__
  if(qurt_thread_get_id) {
      tid = qurt_thread_get_id();
      if(0xFFFFFFF == tid) {
        tid = 0;
      }
  }
#endif

  // remove newlines
  while (0 != (newline = (char*)strstr(msg, "\n"))) {
    newline[0] = ' ';
  }
  //printf("HAP %s: %s - %s:%d\n", level2str(level), msg, filename, line);
  printf("%s:%s:0x%X:%d:%s\n", msg, level2str(level), tid, line, filename);
}

void HAP_debug_runtime(int level, const char* file, int line,
                  const char* format, ...) {
#ifdef FARF_RUNTIME
  char *buf = 0;
  int n = 0;
  va_list args;
  va_start(args, format);
  n = vsnprintf(0, 0, format, args);
  buf = calloc(n, 1);
  if(buf) {
     vsnprintf(buf, n, format, args);
     va_end(args);
     HAP_debug(buf, level, file, line);
     free(buf);
  }
#endif
}

void __android_log_print(int level, const char* tag, const char* fmt,...) {
  char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  HAP_debug(buf, level, tag, -1);
}

void rtld_msg(const char* msg, int level,
              const char* filename, int line);

void rtld_msg(const char* msg, int level,
              const char* filename, int line)
{
  HAP_debug(msg, level, filename, line);
}
