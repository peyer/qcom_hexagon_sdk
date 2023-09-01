#include "calculator_test.h"
#include "rpcmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
  int nErr = 0;
  int runLocal = 0;
  int num = 0;
  int domain = 0;

  if (argc < 4) {
    nErr = 1;
    goto bail;
  }
  runLocal = atoi(argv[1]);
  domain = atoi(argv[2]);
  num = atoi(argv[3]);

  printf("\n- starting calculator multi domains test\n");

  if (domain < 0 || domain > 3) {
     nErr = -1;
     printf("\nInvalid domain %d\n", domain);
     goto bail;
  }

  nErr = calculator_test(runLocal, domain,  num);

bail:
  if (nErr) {
    printf("\nusage: %s <1/0 run locally> <0/1/2/3 domain> <uint32 size>\n\n", argv[0]);
  } else {
    printf("- success\n\n");
  }

  return nErr;
}
