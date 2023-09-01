/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"


#include "typedef.h"
#include "idct.h"


Word16 Input[] __attribute__((aligned(8))) = {
      2288,16,2240,1232,-4608,-720,-96,2560,
      -608,-2416,-1728,-32,8336,5792,-5904,3200,
      -5712,-1408,-2464,4256,-1136,224,-2288,1136,
      -1040,192,816,-576,1200,1872,4720,1584,
      208,-1168,-1424,384,1040,-3360,-3232,-1568,
      3328,1344,-832,-208,-544,-2368,752,1984,
      368,1472,-4768,480,-1616,768,-1568,1920,
      752,7440,-656,-2320,6080,-464,1296,1440,
      32,-4576,4032,2640,2384,592,-1168,976,
      -4176,3104,-704,1120,-224,2656,832,4224,
      -2256,-3776,-480,4688,-5712,-1968,-432,-2768,
      400,400,960,-2080,3056,-1664,544,1472,
      2848,640,-48,-4352,1008,3056,2336,-672,
      6000,-3072,32,-1024,-1808,368,-4560,4416,
      -3136,1344,1584,2800,224,5856,-528,4016,
      -496,2576,1280,-1328,-3488,2304,576,-2336
     };


Word16 Expected_results[]= {
     8, -195, -115, 21, 255, -197, 122, -164, 
     -3, -226, -250, -66, -134, -79, 255, 21, 
     160, 88, 161, 168, 194, -209, 75, -111, 
     -92, 251, 12, -59, 64, 172, 255, 222, 
     220, 104, 155, 49, -67, -87, -181, 196, 
     -51, -22, 255, -225, -173, 255, 143, -55, 
     168, 155, 255, -256, -109, 153, -154, 54, 
     -256, 255, -109, -256, -35, 249, 43, -229, 
     42, -149, -2, -158, -15, -56, 224, -43, 
     -85, -71, -256, -204, 13, 118, -242, -60, 
     164, -153, -94, -162, -121, -117, 233, -256, 
     91, -61, 106, 140, 255, -160, 203, 255, 
     167, -41, 104, 186, -205, -132, -220, 219, 
     150, -175, -216, -123, 182, -169, 38, 130, 
     -182, 255, -150, -42, 152, 217, 237, 255, 
     -63, -41, -224, -25, -158, 255, 92, 63 
     };


Word16 output[128] __attribute__((aligned(8)));

int main()
{
    int i;
    long long start_time;
    int overhead, Total_cycles;

    printf(".... Calling iDCT Kernel\n");

    start_time = hexagon_sim_read_pcycles();
    iDCT(Input, -1, output);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

    start_time = hexagon_sim_read_pcycles();
    iDCT(Input, 1, output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;
    
    for (i=0;i<64;i++) {
        if (output[i]!=Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("Cycle Count(per %d blocks):  %d\n", 1, Total_cycles);

    start_time = hexagon_sim_read_pcycles();
    iDCT(Input, 2, output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;
    
    for (i=0;i<128;i++) {
        if (output[i]!=Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("Cycle Count(per %d blocks):  %d\n", 2, Total_cycles);

    printf("*** PASS!\n");
    return 0;
}
