/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "sobel.h"


#define   WIDTH     16
#define   HEIGHT     8

UWord8 input[128] __attribute__((aligned(8))) = 
{
   78, 59, 19, 12,215, 27, 49,139,
   11, 70, 22, 33, 13, 18, 86, 86,
   93, 60, 15, 48, 90, 41, 49,169,
   39, 89,  7, 55, 60, 45,124,171,
   77, 95, 83, 51, 69, 24, 63,54,
   73,  2, 62,  9, 15, 75, 40,99,
   40,  1, 63, 28,119,115, 14,114,
   30, 23,193, 64, 66,121, 90,67,
   78, 59, 19, 12, 15, 27, 49,139,
   30, 23, 93, 64,166, 21, 90,67,
   11, 70, 22, 33, 13,118, 86, 86,
   40, 01, 63, 28,119, 15,  4, 14,
   93, 60, 15, 48, 90, 41, 49, 69,
   73, 02, 62,209, 55, 75, 40,99,
   39, 89, 87, 55, 60, 45, 24, 71,
   77, 95, 83,151, 69, 24, 63,54
};


UWord8 expected_out[128] = {
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,255,255,255,255,255,255,142,255,98,110,56,68,255,255,0,
   0,166,90,140,138,255,238,180,255,154,255,255,255,255,216,0,
   0,142,240,255,255,244,200,136,255,255,255,255,255,96,52,0,
   0,152,112,176,255,64,255,94,255,255,255,224,78,255,255,0,
   0,128,162,196,255,242,132,184,255,128,255,255,226,255,82,0,
   0,228,255,255,104,206,255,110,255,252,255,244,255,178,244,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
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
  sobel(input, 1, 1, output);
  overhead = (int)(hexagon_sim_read_pcycles() - start_time) -1;

  start_time = hexagon_sim_read_pcycles();
  sobel(input, WIDTH, HEIGHT, output);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  /* ---------------------------------------------------------- */
  /* compare results                                            */
  /*  - boundary is DO-NOT-CARE                                 */
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
