/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"


#include "typedef.h"
#include "fdct.h"


Word16 Input[] __attribute__((aligned(8))) = {
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


Word16 Expected_results[]= {
     131, 6, 141, 79, -266, -40, -8, 145, 
     -43, -143, -107, -11, 504, 344, -353, 201, 
     -355, -84, -156, 266, -64, 25, -133, 64, 
     -75, 8, 58, -37, 76, 115, 276, 92, 
     22, -74, -85, 24, 52, -198, -188, -86, 
     215, 76, -52, -14, -39, -138, 32, 116, 
     19, 85, -284, 34, -104, 56, -108, 132, 
     59, 461, -45, -147, 371, -21, 81, 88, 
     -10, -281, 246, 161, 151, 51, -71, 72, 
     -240, 190, -37, 66, -24, 153, 60, 253, 
     -146, -235, -30, 283, -348, -119, -30, -161, 
     27, 21, 52, -118, 186, -103, 40, 87, 
     173, 51, 2, -275, 62, 188, 130, -44, 
     359, -193, -12, -44, -116, 39, -280, 274, 
     -184, 76, 118, 162, 24, 347, -43, 243, 
     -30, 147, 83, -82, -208, 143, 43, -147
};


Word16 output[128] __attribute__((aligned(8)));

int main()
{
    int i;
    long long start_time; 
    int Total_cycles, overhead;

    printf(".... Calling forward DCT Kernel\n");

    start_time = hexagon_sim_read_pcycles();
    fDCT(Input, -1, output);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

    start_time = hexagon_sim_read_pcycles();
    fDCT(Input, 1, output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;
    
    for (i=0;i<64;i++) {
        if (output[i]!=Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("Cycle Count(per %d blocks):  %d\n", 1, Total_cycles);

    start_time = hexagon_sim_read_pcycles();
    fDCT(Input, 2, output);
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
