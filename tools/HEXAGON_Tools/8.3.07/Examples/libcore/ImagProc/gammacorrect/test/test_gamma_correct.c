/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "gammacorrect.h"

UWord16 Expected_results1[]={
  256,512,704,864,128,384,576,768,384,576,768,880,261,514,705,864
};

UWord16 Expected_results0[]={
  64,128,176,216,32,96,144,192,96,144,192,220,65,128,176,216
};



int main()
{
  UWord16 LUTtable[1024]; 
  UWord16 ch[16] = {
     256,768,1536,2816,
     128,512,1024,2048,
     512,1024,2048,3072,
     266,778,1546,2826
    };
  int LUTmode;
  int i;
  int start_time, Total_cycles;


    for (i=0; i<1024; i++) LUTtable[i] = i;

    LUTmode = 1;
    start_time = hexagon_sim_read_pcycles();
    gamma_correct( ch, 4, 4, LUTtable, LUTmode);
    Total_cycles = hexagon_sim_read_pcycles() - start_time;

    for (i=0;i<16;i++) {
          if (ch[i]!=Expected_results1[i]) {
            printf("Mode 1 FAILED!!!\n");
            return 1;
          }
    }
    printf("** OK Mode 1\n");
    printf("Cycle-count: %d\n\n", Total_cycles);

    LUTmode = 0;
    start_time = hexagon_sim_read_pcycles();
    gamma_correct( ch, 4, 4, LUTtable, LUTmode);
    Total_cycles = hexagon_sim_read_pcycles() - start_time;

    for (i=0;i<16;i++) {
          if (ch[i]!=Expected_results0[i]) {
            printf("Mode 0 FAILED!!!\n");
            return 1;
          }
    }
    printf("** OK Mode 0\n");
    printf("*** PASS!\n");
    printf("Cycle-count: %d\n\n", Total_cycles);
    return 0 ;
}



