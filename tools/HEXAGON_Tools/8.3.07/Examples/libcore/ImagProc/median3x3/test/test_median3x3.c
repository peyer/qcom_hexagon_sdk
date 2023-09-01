/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <stdlib.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "median3x3.h"


#define   WIDTH     16
#define   HEIGHT     8

UWord8 input[128] __attribute__((aligned(8))) = 
{
  178,159,219,112,215,227,249,139,
   11, 70, 22,133, 13,118,186,186,
   93,160,215,248, 90,241,249,169,
  139, 89,187, 55, 60,145,124,171,
   77, 95, 83,151, 69, 24, 63,54,
  173,102,162,209,155, 75,240,99,
  140,101,163,128,119,115,114,114,
   30, 23,193,164,166,221, 90,67,
  178,159,219,112,215,227,249,139,
   30, 23,193,164,166,221, 90,67,
   11, 70, 22,133, 13,118,186,186,
  140,101,163,128,119,115,114,114,
   93,160,215,248, 90,241,249,169,
  173,102,162,209,155, 75,240,99,
  139, 89,187, 55, 60,145,124,171,
   77, 95, 83,151, 69, 24, 63,54
};


UWord8 expected_out[128] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,159,159,151,151,215,169,139,102,102,102,133,118,124,145,0,
  0,101,151,128,119,114,114,114,102,139,162,164,155,145,124,0,
  0,140,128,128,119,115,114,114,54,102,164,166,166,166,90,0,
  0,140,128,128,119,119,139,139,101,101,163,164,164,119,114,0,
  0,159,159,133,133,215,186,173,139,140,162,163,155,119,114,0,
  0,93,133,90,118,124,171,171,140,102,128,151,119,114,99,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


UWord8 output[128] __attribute__((aligned(8)));



int main()
{
  int i, j;
  long long start_time; 
  int overhead, Total_cycles;

  /* ---------------------------------------------------------- */
  /* calling function                                           */
  /* ---------------------------------------------------------- */
  start_time = hexagon_sim_read_pcycles();
  median3x3(input, 0, 1, output);
  overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

  start_time = hexagon_sim_read_cycles();
  median3x3(input, WIDTH, HEIGHT, output);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  /* ---------------------------------------------------------- */
  /* compare results                                            */
  /* boundary is DO-NOT-CARE                                    */
  /* ---------------------------------------------------------- */
  for (i=1; i< HEIGHT-1; i++) {
      for (j=1; j< WIDTH-1; j++) {
          if ( output[i*WIDTH+j] != expected_out[i*WIDTH+j] ) {
             printf("#### FAILED!!!\n"); 
             return 1;
          }
      }
  }
  printf("*** PASS!\n");
  printf("Cycle-count: %d\n", Total_cycles);

  return 0;    
}
