/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "lms.h"


Word16 Coeffs[] __attribute__((aligned(8))) =
{ 2000, 1893, 41, 16294, -22059, 29193, 16423, -5087, 
  -791, 6604, -20700, 27515, 15051, 22364, -7256, 32272 
};


Word16 States[16] __attribute__((aligned(8))) =
{ 763, 4070, -6491, 23982, -5672, -26473, -21849, 14015, 
  25833, -12705, 16969, -28755, -22406, 26103, -30230, 17025 
};


Word16 Expected_states[]=
{ 32, 32, 4070, -6491, 23982, -5672, -26473, -21849, 14015, 
  25833, -12705, 16969, -28755, -22406, 26103, -30230 
};

Word16 Expected_coeffs[]=
{ 2008, 2929, -1611, 22399, -23503, 22454, 10861, -1520, 
  5785, 3370, -16381, 20195, 9348, 29008, -14951, 32767 
};



int main()
{
    Word16 result;
    int i;
    long long start_time; 
    int overhead, Total_cycles;

    printf("Calling LMS Kernel\n");

    start_time = hexagon_sim_read_pcycles(); 
    result = lms(0, 0, 0, 0, States,Coeffs);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

    start_time = hexagon_sim_read_pcycles(); 
    result = lms(32, 0, 0x2666, 16, States,Coeffs);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

    if (result != -27803) {
        printf("### FAILED (output)!!!\n");
        return 1;
    }
    for (i = 0; i < 16; i++) {
        if (Coeffs[i]!=Expected_coeffs[i]) {
            printf("### FAILED (coefficients)!!!\n");
            return 1;
        }
        if (i!= 0) 	// states[0] is DO-NOT_CARE
          if (States[i]!=Expected_states[i]) {
              printf("### FAILED (states)!!!\n");
              return 1;
          }
    }
    printf("*** PASS!\n");
    printf("Cycle-count: %d\n", Total_cycles);
    return 0;
}
