/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <stdio.h>
#include <string.h>
#include <hexagon_sim_timer.h>

short int Input[] __attribute__((aligned(8))) =
{
  0x6b8b,0x643c,0x74b0,0x2ae8,0x238e,0x3d1b,0x2eb1,0x79e2,0x515f,0x1220,0x0216,0x1190,0x140e,0x109c,0x7fdc,0x41a7,
  0x4e6a,0x519b,0x3f2d,0x2571,0x436c,0x333a,0x2443,0x6763,0x08ed,0x4353,0x189a,0x71f3,0x0836,0x3a95,0x1e7f,0x737b,
  0x2222,0x3006,0x419a,0x440b,0x3804,0x7724,0x2463,0x51ea,0x580b,0x3855,0x6a23,0x1d4e,0x2cd8,0x7a6d,0x5422,0x3843,
  0x32ff,0x5794,0x3dc2,0x79a1,0x12e6,0x520e,0x4f4e,0x649b,0x3938,0x1801,0x4739,0x15b5,0x0d34,0x3f6a,0x7e0c,0x579b,
  0x5ff8,0x25a7,0x4ad0,0x1381,0x100f,0x1501,0x098a,0x06b9,0x168e,0x661e,0x540a,0x51d9,0x0bf7,0x4296,0x08f2,0x3b0f,
  0x4962,0x06a5,0x7fff,0x71ea,0x7fb7,0x6f6d,0x0088,0x4c04,0x14e1,0x74de,0x2df6,0x4a2a,0x57fc,0x43f1,0x26f3,0x49da,
  0x5fb8,0x0488,0x6aa7,0x6fc7,0x7d5e,0x73a1,0x555c,0x14fc,0x71c9,0x5329,0x5092,0x59ad,0x2a15,0x097e,0x1ca0,0x415e,
  0x23d8,0x5c10,0x3c59,0x78df,0x2b0d,0x379e,0x2c27,0x6aa7,0x5675,0x3db0,0x5b25,0x4f97,0x34fd,0x5643,0x2c6e,0x4df7
};

#define INPUT_SIZE (sizeof(Input) / sizeof(short int))

short int Expected_results[] __attribute__((aligned(8))) =
{
  0x0088, 0x0056
};

#define OUTPUT_SIZE (sizeof(Expected_results) / sizeof(short int))

short int Output[OUTPUT_SIZE]  __attribute__((aligned(8)));

extern void findmin(short int *In, int samples, short int *Out);

typedef void (*findmin_fptr)(short int *, int, short int *);

#define REGISTER_findmin_VERSION(function) extern void function (short int *, int, short int *);

#define findmin_VERSION(function, desc) { #desc , function }

REGISTER_findmin_VERSION(findmin_orig);
REGISTER_findmin_VERSION(findmin_mux);
REGISTER_findmin_VERSION(findmin_ternary);

struct findmin_version
{
  char *desc;
  findmin_fptr function;
};

struct findmin_version funcs[] =
  {
    findmin_VERSION(findmin_orig, "Original"),
    findmin_VERSION(findmin_mux, "Mux intrinsic"),
    findmin_VERSION(findmin_ternary, "Ternary operators")
  };

#define FUNC_COUNT sizeof(funcs) / sizeof(struct findmin_version)

unsigned long long times[FUNC_COUNT];

int
main()
{
  int i, index;
  unsigned long long start, stop;
    
  printf("Index\t%40s\tStatus\tCycles\tChange\n", "Name");

  for (index = 0 ; index < FUNC_COUNT ; index++) {
    int passed;
    passed = 1;
    memset(Output, 0x0, sizeof(Output[0]) * OUTPUT_SIZE);
    
    start = hexagon_sim_read_cycles();
    funcs[index].function(Input, INPUT_SIZE, Output);
    stop = hexagon_sim_read_cycles();

    times[index] = stop - start;

    for (i=0;i<OUTPUT_SIZE;i++) {
      if (Output[i]!=Expected_results[i]) {
        printf("FAILED!!!\n");
        printf("Output[%d] = 0x%x\n", i, Output[i]);
        printf("Expected[%d] = 0x%x\n", i, Expected_results[i]);
        passed = 0;
        return -1;
      }
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
