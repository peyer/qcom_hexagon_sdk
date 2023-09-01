#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "verify.h"

extern int rpcperf_main(int argc, char* argv[]);

int main(int argc, char* argv[]) {
   return rpcperf_main(argc, argv);
}

void HAP_debug_v2(int level, const char* file, int line, const char* format, ...) {
}

void HAP_debug(const char *msg, int level, const char *filename, int line) {
}

