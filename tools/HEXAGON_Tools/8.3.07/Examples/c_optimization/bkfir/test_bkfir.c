/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <stdio.h>
#include <string.h>
#include <hexagon_sim_timer.h>

void iter(void) {
  static int i = 0;
  printf("Iteration%04d\n", i++);
  return;
}

short int Input[] __attribute__((aligned(8))) =
{ 
  0,32,64,96,128,160,192,224,
  256,288,320,352,384,416,448,480,
  512,544,576,608,640,672,704,736,
  768,800,832,864,896,928,960,992,
  1024,1056,1088,1120,1152,1184,1216,1248,
  1280,1312,1344,1376,1408,1440,1472,1504,
  1536,1568,1600,1632,1664,1696,1728,1760
};

short int Coeffs[] __attribute__((aligned(8))) =
{
  16384, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16384
};


short int Output[128]  __attribute__((aligned(8)));

short int Expected_results[]=
  {
    0x0000, 0x0000, 0x0000, 0x0000, 0x001e, 0x0022, 0x0026, 0x002a,
    0x002e, 0x0032, 0x0036, 0x003a, 0x003e, 0x0042, 0x0046, 0x004a,
    0x004e, 0x0052, 0x0056, 0x005a, 0x005e, 0x0062, 0x0066, 0x006a,
    0x006e, 0x0072, 0x0076, 0x007a, 0x007e, 0x0082, 0x0086, 0x008a,
    0x008e, 0x0092, 0x0096, 0x009a, 0x009e, 0x00a2, 0x00a6, 0x00aa,
    0x00ae, 0x00b2, 0x00b6, 0x00ba, 0x00ba
  };

typedef void (*FIR_fptr)(short int *, short int *, short int *, int, int, int);

#define REGISTER_FIR_VERSION(function) extern void function(short int *Inputs, short int *Coeffs, short int *Output, int unused, int taps, int length);

#define FIR_VERSION(function, desc) { #desc , function }

REGISTER_FIR_VERSION(FIR);
REGISTER_FIR_VERSION(FIR01);
REGISTER_FIR_VERSION(FIR02);
REGISTER_FIR_VERSION(FIR03);
REGISTER_FIR_VERSION(FIR04);
REGISTER_FIR_VERSION(FIR05);
REGISTER_FIR_VERSION(FIR06);
REGISTER_FIR_VERSION(FIR07);
REGISTER_FIR_VERSION(FIR08);

struct FIR_version
{
  char *desc;
  FIR_fptr function;
};

struct FIR_version funcs[] =
  {
    FIR_VERSION(FIR01, "Original code"),
    FIR_VERSION(FIR02, "Unroll and jam outer loop"),
    FIR_VERSION(FIR03, "Unroll inner loop"),
    FIR_VERSION(FIR04, "Reorder inner loop statements"),
    FIR_VERSION(FIR05, "Vectorize inner loop"),
    FIR_VERSION(FIR06, "Vectorize output writes"),
    FIR_VERSION(FIR07, "Scale index variables"),
    FIR_VERSION(FIR08, "Eliminate sign extensions"),
    FIR_VERSION(FIR, "Hand-coded assembly"),
  };

#define FUNC_COUNT sizeof(funcs) / sizeof(struct FIR_version)


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
    memset(Output, 0x0, sizeof(Output[0]) * 44);
    
    start = hexagon_sim_read_cycles();
    funcs[index].function(Input,Coeffs,Output,0,16,40);
    stop = hexagon_sim_read_cycles();

    times[index] = stop - start;

    for (i=0;i<44;i++) {
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

