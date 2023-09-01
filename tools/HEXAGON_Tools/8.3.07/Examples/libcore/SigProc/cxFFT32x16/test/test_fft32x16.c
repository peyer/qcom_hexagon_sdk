/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <math.h>
#include "data_macros.h"
#include "typedef.h"
#include "q6basic_op.h"
#include "hexagon_sim_timer.h"

#include "fft32x16.h"
#include "fftwiddles.h"         

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

CWord2x32 Input[256]   __attribute__((aligned(8*256)));
CWord2x32 Output[256]  __attribute__((aligned(8)));
CWord2x32 ExpOut[256];

int main()
{
    int i, Nfft; 
    long long start_time; 
    int overhead, Total_cycles;
    FILE *Infile;

   /* --------------------------------------------------------- */
   /*                     Test FFT                              */
   /* --------------------------------------------------------- */

   /* --------------------------------------------------------- */
   /* read inputs from file                                     */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFTin.bin"),"rb")) == NULL) {
      printf("Error: can not open FFTin.bin\n");
      return 1;
   }

   if (fread(Input, sizeof(CWord2x32), 256, Infile) != 256)
      printf("Error: Unable to read from FFTin.bin\n");
   fclose(Infile);

    start_time = hexagon_sim_read_pcycles();
    fft32x16(Input,0, Wtwiddles, Output);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

   /* --------------------------------------------------------- */
   /* CALL 128-point FFT                                        */
   /* --------------------------------------------------------- */
    Nfft = 128;
    printf("testing  %d-point FFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    fft32x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /*  and compare results                                      */
   /* --------------------------------------------------------- */
    if((Infile = fopen(FULL_FILE_NAME("FFT128out.bin"),"rb")) == NULL) {
       printf("Error: can not open FFT128out.bin\n");
       return 1;
    }

    if (fread(ExpOut, sizeof(CWord2x32), 128, Infile) != 128)
       printf("Error: Unable to read from FFT128out.bin\n");
    fclose(Infile);

    for (i = 0; i < Nfft; i++) {
       if ( Output[i]!= ExpOut[i] ) {
            printf("FAILED!!!\n");
            return 1;
       }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);

   /* --------------------------------------------------------- */
   /* CALL 256-point FFT                                        */
   /* --------------------------------------------------------- */
    Nfft = 256;
    printf("testing  %d-point FFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    fft32x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /*  and compare results                                      */
   /* --------------------------------------------------------- */
    if((Infile = fopen(FULL_FILE_NAME("FFT256out.bin"),"rb")) == NULL) {
       printf("Error: can not open FFT256out.bin\n");
       return 1;
    }

    if (fread(ExpOut, sizeof(CWord2x32), 256, Infile) != 256)
       printf("Error: Unable to read from FFT256out.bin\n");
    fclose(Infile);

    for (i = 0; i < Nfft; i++) {
       if ( Output[i]!= ExpOut[i] ) {
            printf("FAILED!!!\n");
            return 1;
       }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);

    printf("*** PASS!\n");
    return 0;
}

