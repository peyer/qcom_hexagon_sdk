/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "iir_cas_bq.h"

short int Coeffs[8] __attribute__((aligned(8))) =
{
// first section
 2207,			// f0
 -6357,			// f1
 32767,			// b0
 16384,			// b1
// second section
 -1050,			// f0
 -5144,			// f1
 32767,			// b0
 16384			// b1
};

short int States[4] __attribute__((aligned(8))); 

short int Input[40] =
{ 471,2159,2695,2607,1319,488,625,1239,-42,-771,
  -92,-566,-1601,-794,-1380,-739,-750,-1513,-1604,-543,
  -153,-158,563,1330,1024,1596,2286,2985,2526,2263,
 2268,1840,1838,954,520,-337,-488,243,-1096,-1872
};

short int Output[40];  

short int Expected_results[40] =
{ 471, 2638, 5170, 6667, 5727, 3490, 1948, 2101, 1525, -64, 
  -828, -1188, -2321, -2725, -3161, -2724, -2305, -2760, -3577, -3079, 
  -1740, -690, 369, 1850, 2706, 3477, 4603, 6267, 6975, 6703, 
  6183, 5518, 5071, 3958, 2594, 788, -558, -484, -1202, -2873
};


extern void IIR(short int in, short int *coef, short int *states, short int *out); 

int main()
{
    int i; 
    long long start_time;
    int overhead, Total_cycles;

    printf("Calling IIR Kernel\n");

    for (i=0; i<4; i++)
      States[i] = 0;

    start_time = hexagon_sim_read_pcycles();
    IIR_casBiquad(Input,Coeffs,States,2,0, Output);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

    start_time = hexagon_sim_read_pcycles();
    IIR_casBiquad(Input,Coeffs,States,2,40, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

    for (i=0;i<40;i++) {
        if (Output[i]!=Expected_results[i]) {
            printf("### FAILED!!!\n");
            return 1;
        }
    }

    printf("Cycle-count: %d\n", Total_cycles);
    printf("*** PASS!\n");
    return 0;
}

