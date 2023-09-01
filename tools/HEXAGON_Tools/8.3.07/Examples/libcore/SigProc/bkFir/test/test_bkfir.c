/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <stdlib.h>

#include "hexagon_sim_timer.h"

#include "data_macros.h"
#include "typedef.h"
#include "bkfir.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define		T              16
#define		N              40

Word16 coeffs[T] __attribute__((aligned(8))) =
{ -22520,-25671,22715,32534,4144,-17761,19444,30165,
  -15724,-11683,-2153,-29485,-3883,-1700,-27057,32687
};


int main()
{
   Word16 input[N+T] __attribute__((aligned(8)));
   Word16 output[N]  __attribute__((aligned(4)));
   Word16 Expected_results[N];
   FILE *Infile, *Outfile; 
   int i, Total_cycles, overhead;
   long long start_time;

   // load input and reference output
   if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) {
       printf("Error: can not open indata.bin\n");
       return 1;
   }
   if((Outfile = fopen(FULL_FILE_NAME("outdata.bin"),"rb")) == NULL) {
       printf("Error: can not open outdat.bin\n");
       return 1;
   }

   if (fread(input,  sizeof(Word16), (N+T), Infile) != (N+T) ) {
       printf("Error, Unable to read to indata.bin\n");
       return 1;
   }

   if (fread(Expected_results, sizeof(Word16), N, Outfile) != N) {
       printf("Error, Unable to read to outdata.bin\n");
       return 1;
   }

   fclose(Infile);
   fclose(Outfile);

   printf("Calling FIR Kernel\n");
   start_time = hexagon_sim_read_pcycles();
   bkfir(input,coeffs,4,0,4,output);
   overhead = (int)(hexagon_sim_read_pcycles() - start_time) -1;

   start_time = hexagon_sim_read_pcycles();

   bkfir(input,coeffs,T,N,4,output);
   Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   for (i = 0; i < N; i++) {
        if (output[i]!=Expected_results[i]) {
            printf("### FAILED!!!\n");
            return 1;
        }
   }
   printf("*** PASS!\n");
   printf("Cycle-count: %d\n", Total_cycles);

   return 0;
}

