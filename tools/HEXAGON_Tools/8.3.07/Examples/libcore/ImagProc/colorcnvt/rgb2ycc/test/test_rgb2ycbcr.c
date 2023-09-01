/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "data_macros.h"
#include "rgb2ycbcr.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define WIDTH    320
#define HEIGHT   240
#define NOUT     (WIDTH*HEIGHT)

unsigned char Input[3*NOUT] __attribute__((aligned(32)));  // aligned by $line
unsigned char YCrCb[3*NOUT] __attribute__((aligned(32)));  // aligned by $line
unsigned char Expected_YCrCb[3*NOUT];


int main()
{
  int i;
  long long start_time; 
  int Total_cycles;
  FILE *Infile;

   // read inputs from file
   if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) {
     printf("Error: can not open indata.bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(char), 3*NOUT, Infile) != (3*NOUT) )
       printf("Error, Unable to read from indata.bin\n");
   fclose(Infile);

   // call RGB2YCbCr()
   start_time = hexagon_sim_read_pcycles();
   RGB2YCbCr(Input, WIDTH, HEIGHT, YCrCb );
   Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time);

   // compare outputs
   if((Infile = fopen(FULL_FILE_NAME("outdata.bin"),"rb")) == NULL) {
     printf("Error: can not open outdata.bin\n");
     return 1;
   }
   if (fread(Expected_YCrCb,  sizeof(char), 3*NOUT, Infile) != 3*NOUT )
       printf("Error, Unable to read from outdata.bin\n");
   fclose(Infile);

   for (i=0; i< 3*NOUT; i++) {
     if ( YCrCb[i]!=Expected_YCrCb[i] )
     {
         printf("### FAILED - RGB2YCbCr !!!\n");
         return 1;
     }
   }

   printf("Cycle-count:  RGB2YCbCr -- %d\n", Total_cycles);
   printf("*** PASS!\n");
   return 0;

}

