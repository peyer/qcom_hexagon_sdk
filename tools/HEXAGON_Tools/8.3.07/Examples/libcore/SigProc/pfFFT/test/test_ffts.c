/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "data_macros.h"
#include "typedef.h"
#include "hexagon_sim_timer.h"

#include "q6basic_op.h"
#include "pfFft.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define OVERHEAD    18

CWord2x16 Input[360]   __attribute__((aligned(2<<11)));
CWord2x16 Output[360]  __attribute__((aligned(2<<11)));

CWord2x16 Expected_results[360];

int main()
{
    FILE *Infile;
    int i; 
    long long start_time; 
    int Total_cycles;

   /* --------------------------------------------------------- */
   /*                     Test FFT                              */
   /* --------------------------------------------------------- */

   /* --------------------------------------------------------- */
   /* read inputs from file                                     */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFTsin.bin"),"rb")) == NULL) {

     printf("Error: can not open FFTsin.bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(CWord2x16), 360, Infile) != 360 ) 
       printf("Error, Unable to read from FFT320in.bin\n");
   
   fclose(Infile);

   /* --------------------------------------------------------- */
   /* CALL 320-point FFT                                        */
   /* --------------------------------------------------------- */
    printf("testing  320-point FFT ....");

    start_time = hexagon_sim_read_pcycles();
    fft320(Input, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - OVERHEAD;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */

   if((Infile = fopen(FULL_FILE_NAME("FFT320out.bin"),"rb")) == NULL) {
     printf("Error: can not open FFT320out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), 320, Infile) != 320 ) 
       printf("Error: Unable to read from FFT320out.bin\n");

   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<320;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);


   /* --------------------------------------------------------- */
   /* CALL 360-point FFT                                        */
   /* --------------------------------------------------------- */
    printf("testing  360-point FFT ....");

    start_time = hexagon_sim_read_pcycles();
    fft360(Input, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - OVERHEAD;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */

   if((Infile = fopen(FULL_FILE_NAME("FFT360out.bin"),"rb")) == NULL) {
     printf("Error: can not open FFT360out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), 360, Infile) != 360 ) 
       printf("Error: Unable to read from FFT360out.bin\n");

   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<360;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);


   /* --------------------------------------------------------- */
   /* CALL 192-point FFT                                        */
   /* --------------------------------------------------------- */
    printf("testing  192-point FFT ....");

    start_time = hexagon_sim_read_pcycles();
    fft192(Input, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - OVERHEAD;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */

   if((Infile = fopen(FULL_FILE_NAME("FFT192out.bin"),"rb")) == NULL) {
     printf("Error: can not open FFT192out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), 192, Infile) != 192 ) 
       printf("Error: Unable to read from FFT320out.bin\n");

   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<192;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);

    printf("*** PASS!\n");
    return 0;
}

