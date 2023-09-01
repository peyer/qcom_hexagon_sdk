/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <stdlib.h>

#include "hexagon_sim_timer.h"
#include "typedef.h"
#include "q6basic_op.h"
#include "data_macros.h"

#include "corr.h"
#include "outdata.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define		NC             80 
#define		N              80


int main()
{
   Word16 in1[N], in2[N];
   Word16 outS16[NC];  
   Word32 outS32[NC];  
   CWord2x16 outSC16[NC/2];  
   FILE *Infile;
   int i; 
   long long start_time; 
   int overhead, Total_cycles;


   // load input and reference output
   if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) {
       printf("Error: can not open indata.bin\n");
       return 1;
   }

   if (fread(in1,  sizeof(Word16), N, Infile) != N) {
       printf("Error, Unable to read to indata.bin\n");
       return 1;
   }

   if (fread(in2,  sizeof(Word16), N, Infile) != N) {
       printf("Error, Unable to read to indata.bin\n");
       return 1;
   }
   fclose(Infile);

   //
   // TEST CorrS16()
   // 
   printf("Calling Correlation Kernel (output in S16)\n");

   // call function
   start_time = hexagon_sim_read_cycles();
   CorrS16(in1, in2, N, 0, 6, outS16);
   overhead = (int)(hexagon_sim_read_cycles() - start_time) - 1;

   // call function
   CorrS16(in1, in2, N, NC, 6, outS16);

   // compare results
   for (i = 0; i < NC; i++) {
        if (outS16[i]!=Expected_xycorrS16[i]) {
            printf("### CROSSCORR FAILED!!!\n");
            return 1;
        }
   }

   // call function
   start_time = hexagon_sim_read_cycles();
   CorrS16(in1, in1, N, NC, 6, outS16);
   Total_cycles = (int)(hexagon_sim_read_cycles() - start_time) - overhead;

   // compare results
   for (i = 0; i < NC; i++) {
        if (outS16[i]!=Expected_autocorrS16[i]) {
            printf("### AUTOCORR FAILED!!!\n");
            return 1;
        }
   }
   printf("Cycle-count: %d\n", Total_cycles);

   //
   // TEST CorrS32()
   // 
   printf("Calling Correlation Kernel (output in S32)\n");
   // call function
   start_time = hexagon_sim_read_cycles();
   CorrS32(in1, in2, N, NC, 6, outS32);
   Total_cycles = (int)(hexagon_sim_read_cycles() - start_time) - overhead;

   // compare results
   for (i = 0; i < NC; i++) {
        if (outS32[i]!=Expected_xycorrS32[i]) {
            printf("### CROSSCORR FAILED!!!\n");
            return 1;
        }
   }
   printf("Cycle-count: %d\n", Total_cycles);


   //
   // TEST cxCorr()
   // 
   // shift down sample to avoid saturation
   for (i=0; i<N; i++)
   {
      in1[i] >>= 6;
      in2[i] >>= 6;
   }
   printf("Calling Correlation Kernel (complex)\n");
   // call function
   start_time = hexagon_sim_read_cycles();
   cxCorr((CWord2x16 *)in1, (CWord2x16 *)in2, N/2, NC/2, 10, outSC16);
   Total_cycles = (int)(hexagon_sim_read_cycles() - start_time) - overhead;

   // compare results
   for (i = 0; i < NC/2; i++) {
        if ( (real(outSC16[i])!=Expected_xycorrSC16[2*i  ]) ||
             (imag(outSC16[i])!=Expected_xycorrSC16[2*i+1]) ) 
        {
            printf("### CROSSCORR (complex) FAILED!!!\n");
            return 1;
        }
   }

   printf("Cycle-count: %d\n", Total_cycles);


   printf("*** PASS!\n");
   return 0;
}

