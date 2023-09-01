/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "lumafilt.h"


unsigned char Input[64] =
{  131,44,79,136,242,43,179,57,
   126,31,21,99,70,94,251,137,
   196,165,196,199,210,38,160,80,
   88,234,133,102,155,201,238,222,
   221,172,194,148,99,91,51,211,
   106,118,250,32,54,245,188,104,
   199,194,244,7,81,193,62,150,
   11,244,81,15,113,234,146,30,
};

unsigned char Expected_results0[] =
{  0,31,21,0,70,0,255,0,
   0,111,187,198,232,38,47,0,
   0,234,165,102,155,201,255,0,
   0,172,147,105,120,0,0,0,
   0,183,250,0,102,255,188,0,
   0,194,125,0,81,165,0,0
};

unsigned char Expected_results1[] =
{  0,0,0,99,0,94,111,0,
   0,111,187,199,210,38,160,0,
   0,171,133,102,132,201,238,0,
   0,172,194,105,99,91,51,0,
   0,118,255,0,0,245,188,0,
   0,146,255,0,0,193,0,0
};


unsigned char out[64] __attribute__((aligned(8)));


int main()
{
    static LumaFiltParams TestParam = {
       { -4,-5,-6,-7,72,-8,-9,-10,-11,0 },
         255,5,
       { 51,52,53,54,55,56,57,58,59,0 },
         128,9,
       { 0,0,0,0,1,0,0,0,0,0 },
         0 
    };
    int i, j;
    int start_time, Total_cycles;

    // test mode 0
    luma_filt(Input, 8, 8, &TestParam, out );  

    for (i=1; i<7; i++) {
       for(j=1; j<7; j++){
            if (out[i*8+j]!=Expected_results0[(i-1)*8+j]) {
            printf("Mode 0 FAILED!!! \n");
            return 1;
          }
       }
    }
    printf("** OK Mode 0\n\n");

    // test mode 1
    TestParam.mask_sel = 1;

    start_time = hexagon_sim_read_pcycles();

    luma_filt(Input, 8, 8, &TestParam, out );  

    Total_cycles = hexagon_sim_read_pcycles() - start_time;

    for (i=1; i<7; i++) {
       for(j=1; j<7; j++){
            if (out[i*8+j]!=Expected_results1[(i-1)*8+j]) {
            printf("Mode 1 FAILED!!! \n");
            return 1;
          }
       }
    }
    printf("** OK Mode 1\n\n");
    printf("*** PASS!\n\n");
    printf("Cycle-count: %d\n", Total_cycles);
    return 0 ;
}



