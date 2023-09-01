/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <stdio.h>
#include <hexagon_sim_timer.h>

typedef unsigned int (*bitcount_fptr)(unsigned int);

#define REGISTER_bitcount_VERSION(function) extern unsigned int function (unsigned int);
#define bitcount_VERSION(function, desc) { #desc , function }

REGISTER_bitcount_VERSION(bitcount_orig);
REGISTER_bitcount_VERSION(bitcount_no_branch);

struct bitcount_version
{
  char *desc;
  bitcount_fptr function;
};

struct bitcount_version funcs[] =
  {
    bitcount_VERSION(bitcount_orig, "With control flow"),
    bitcount_VERSION(bitcount_no_branch, "Without control flow")
  };

#define FUNC_COUNT sizeof(funcs) / sizeof(struct bitcount_version)

unsigned long long times[FUNC_COUNT];

int
main()
{
  int index;
  unsigned long long start, stop;

  printf("Index\t%40s\tStatus\tCycles\tChange\n", "Name");
  for (index = 0 ; index < FUNC_COUNT ; index++) {
    int expected = 32;
    int count = 0;
    int passed;
    passed = 1;
    
    start = hexagon_sim_read_cycles();
    count += funcs[index].function(0xffffffff);
    count += funcs[index].function(0x00000000);
    stop = hexagon_sim_read_cycles();

    times[index] = stop - start;

    if (count != 32) {
      printf("FAILED!!!\n");
      printf("count = %x\n", count);
      printf("expected = %x\n", expected);
      passed = 0;
      return -1;
    }
    printf("%d\t%40s\t%s\t%6llu\t%5.2f%%\n",
           index,
           funcs[index].desc,
           (passed ? "PASSED" : "FAILED"),
           times[index],
           (index > 0) ? (((double) times[0] - (double)times[index]) / (double)times[0] * 100.0) : 0.0);

  }

  return 0;
}
