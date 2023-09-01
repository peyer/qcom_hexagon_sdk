/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "hexagon_sim_timer.h"
#include "typedef.h"
#include "pnseq.h"

#define  M         65


UWord8 Expected_out[9] = { 0x6a, 0x1e, 0x09, 0x4c, 0xd2, 0x7f, 0x90, 0x63, 0xf0 };

int main()
{
  UWord8 out[(M+7)/8];
  int i;
  long long start_time; 
  int overhead, cycle_count;


  start_time = hexagon_sim_read_pcycles();
  PNGeneration( 0x91E6A, 0, (UWord8 *)out);
  overhead = (int)(hexagon_sim_read_pcycles() - start_time) -1 ;

  start_time = hexagon_sim_read_pcycles();
  PNGeneration( 0x91E6A, M, (UWord8 *)out);
  cycle_count = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  for(i=0; i< (M+7)/8; i++) {
     if (out[i] != Expected_out[i]) { 
         printf("### FAILED !!!\n");
         return 1;
     }
  }

  printf("*** PASS!\n");
  printf("CYCLE-COUNT: %d\n", cycle_count );

  return 0;

}

