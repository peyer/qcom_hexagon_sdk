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
#include "rfft.h"

#include "rfftwiddles.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define NP          256


Word16 Input[NP]   __attribute__((aligned(2*NP)));  // MUST be aligned by array size

CWord2x16 Output[NP/2+1]  __attribute__((aligned(8)));

CWord2x16 Expected_results[NP/2+1];

int main()
{
    FILE *Infile;
    int i; 
    long long start_time; 
    int overhead, Total_cycles;

   /* --------------------------------------------------------- */
   /*                     Test 256-point rFFT                   */
   /* --------------------------------------------------------- */

   /* --------------------------------------------------------- */
   /* read inputs from file                                     */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("rFFTin.bin"),"rb")) == NULL) {

     printf("Error: can not open rFFTin.bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(Word16), NP, Infile) != NP ) 
       printf("Error, Unable to read from rFFTin.bin\n");
   
   fclose(Infile);

   /* --------------------------------------------------------- */
   /* CALL 256-point rFFT                                       */
   /* --------------------------------------------------------- */
    printf("testing  256-point rFFT ....");

    start_time = hexagon_sim_read_pcycles();
    rfft(Input, 0, Wt1, Wt2, Output);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

    start_time = hexagon_sim_read_cycles();
    rfft(Input, 256, Wt1, Wt2, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */

   if((Infile = fopen(FULL_FILE_NAME("rFFTout.bin"),"rb")) == NULL) {
     printf("Error: can not open rFFTout.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), NP/2+1, Infile) != (NP/2+1) ) 
       printf("Error: Unable to read from rFFTout.bin\n");

   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i< (NP/2+1);i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }
    }
    printf("Cycle-count: %d\n", Total_cycles);

    printf("*** PASS!\n");
    return 0;
}

