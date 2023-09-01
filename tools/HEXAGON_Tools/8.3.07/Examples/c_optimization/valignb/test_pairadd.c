/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <stdio.h>
#include <string.h>
#include <hexagon_sim_timer.h>


unsigned short int Input[] __attribute__((aligned(8))) =
{
  0x6b8b,0x643c,0x74b0,0x2ae8,0x238e,0x3d1b,0x2eb1,0x79e2,0x515f,0x1220,0x0216,0x1190,0x140e,0x109c,0x7fdc,0x41a7,
  0x4e6a,0x519b,0x3f2d,0x2571,0x436c,0x333a,0x2443,0x6763,0x08ed,0x4353,0x189a,0x71f3,0x0836,0x3a95,0x1e7f,0x737b,
  0x2222,0x3006,0x419a,0x440b,0x3804,0x7724,0x2463,0x51ea,0x580b,0x3855,0x6a23,0x1d4e,0x2cd8,0x7a6d,0x5422,0x3843,
  0x32ff,0x5794,0x3dc2,0x79a1,0x12e6,0x520e,0x4f4e,0x649b,0x3938,0x1801,0x4739,0x15b5,0x0d34,0x3f6a,0x7e0c,0x579b,
  0x5ff8,0x25a7,0x4ad0,0x1381,0x100f,0x1501,0x098a,0x06b9,0x168e,0x661e,0x540a,0x51d9,0x0bf7,0x4296,0x08f2,0x3b0f,
  0x4962,0x06a5,0x7fff,0x71ea,0x7fb7,0x6f6d,0x0088,0x4c04,0x14e1,0x74de,0x2df6,0x4a2a,0x57fc,0x43f1,0x26f3,0x49da,
  0x5fb8,0x0488,0x6aa7,0x6fc7,0x7d5e,0x73a1,0x555c,0x14fc,0x71c9,0x5329,0x5092,0x59ad,0x2a15,0x097e,0x1ca0,0x415e,
  0x23d8,0x5c10,0x3c59,0x78df,0x2b0d,0x379e,0x2c27,0x6aa7,0x5675,0x3db0,0x5b25,0x4f97,0x34fd,0x5643,0x2c6e,0x4df7,
  0xa3f3
};

unsigned short int Expected_results[] __attribute__((aligned(8))) =
{
  0xcfc7,0xd8ec,0x9f98,0x4e76,0x60a9,0x6bcc,0xa893,0xcb41,0x637f,0x1436,0x13a6,0x259e,0x24aa,0x9078,0xc183,0x9011,
  0xa005,0x90c8,0x649e,0x68dd,0x76a6,0x577d,0x8ba6,0x7050,0x4c40,0x5bed,0x8a8d,0x7a29,0x42cb,0x5914,0x91fa,0x959d,
  0x5228,0x71a0,0x85a5,0x7c0f,0xaf28,0x9b87,0x764d,0xa9f5,0x9060,0xa278,0x8771,0x4a26,0xa745,0xce8f,0x8c65,0x6b42,
  0x8a93,0x9556,0xb763,0x8c87,0x64f4,0xa15c,0xb3e9,0x9dd3,0x5139,0x5f3a,0x5cee,0x22e9,0x4c9e,0xbd76,0xd5a7,0xb793,
  0x859f,0x7077,0x5e51,0x2390,0x2510,0x1e8b,0x1043,0x1d47,0x7cac,0xba28,0xa5e3,0x5dd0,0x4e8d,0x4b88,0x4401,0x8471,
  0x5007,0x86a4,0xf1e9,0xf1a1,0xef24,0x6ff5,0x4c8c,0x60e5,0x89bf,0xa2d4,0x7820,0xa226,0x9bed,0x6ae4,0x70cd,0xa992,
  0x6440,0x6f2f,0xda6e,0xed25,0xf0ff,0xc8fd,0x6a58,0x86c5,0xc4f2,0xa3bb,0xaa3f,0x83c2,0x3393,0x261e,0x5dfe,0x6536,
  0x7fe8,0x9869,0xb538,0xa3ec,0x62ab,0x63c5,0x96ce,0xc11c,0x9425,0x98d5,0xaabc,0x8494,0x8b40,0x82b1,0x7a65,0xf1ea
};


#define OUTPUT_SIZE (sizeof(Expected_results) / sizeof(unsigned short int))
unsigned short int Output[OUTPUT_SIZE]  __attribute__((aligned(8)));


extern void pairadd(unsigned short int *In, unsigned short int *Out, int samples);




typedef void (*pairadd_fptr)(unsigned short int*, unsigned short int*, int);

#define REGISTER_pairadd_VERSION(function) extern void function (unsigned short int *In, unsigned short int *Out, int samples);

#define pairadd_VERSION(function, desc) { #desc , function }

REGISTER_pairadd_VERSION(pairadd_orig);
REGISTER_pairadd_VERSION(pairadd_vector);
REGISTER_pairadd_VERSION(pairadd_vector_forward);

struct pairadd_version
{
  char *desc;
  pairadd_fptr function;
};

struct pairadd_version funcs[] =
  {
    pairadd_VERSION(pairadd_orig, "Original code"),
    pairadd_VERSION(pairadd_vector, "Vectorized with valignb"),
    pairadd_VERSION(pairadd_vector_forward, "Vectorized with valignb and forwarding")
  };

#define FUNC_COUNT sizeof(funcs) / sizeof(struct pairadd_version)


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
    funcs[index].function(Input, Output, OUTPUT_SIZE);
    stop = hexagon_sim_read_cycles();

    times[index] = stop - start;

    for (i=0;i<OUTPUT_SIZE;i++) {
      if (Output[i]!=Expected_results[i]) {
        printf("FAILED!!!\n");
        printf("Output[%d] = %x\n", i, Output[i]);
        printf("Expected[%d] = %x\n", i, Expected_results[i]);
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
