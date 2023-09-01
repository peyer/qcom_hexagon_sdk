/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>

#include "hexagon_sim_timer.h"
#include "data_macros.h"

#include "conv3x3.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define WIDTH     480
#define HEIGHT    320
#define SHIFT     (4)
#define NOUT      (WIDTH*(HEIGHT-2))

unsigned char Input[WIDTH*HEIGHT]      __attribute__((aligned(2)));
unsigned char Output[WIDTH*(HEIGHT-2)] __attribute__((aligned(2)));
unsigned char Expected_Out[WIDTH*(HEIGHT-2)];


int main()
{
  int i;
  long long start_time; 
  int Total_cycles;
  FILE *Infile;

  signed char  mask[9] =
  {
      1, 2, 1,
      2, 4, 2,
      1, 2, 1
  };

   // read inputs from file
   if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) {
     printf("Error: can not open indata.bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(char), (WIDTH*HEIGHT), Infile) != (WIDTH*HEIGHT) )
       printf("Error, Unable to read from indata.bin\n");
   fclose(Infile);

   // call conv3x3()
   start_time = hexagon_sim_read_cycles();
   conv3x3(Input, mask, WIDTH, SHIFT, Output, HEIGHT-2);
   Total_cycles = (int)(hexagon_sim_read_cycles() - start_time);

   // compare outputs
   if((Infile = fopen(FULL_FILE_NAME("outdata.bin"),"rb")) == NULL) {
     printf("Error: can not open outdata.bin\n");
     return 1;
   }
   if (fread(Expected_Out,  sizeof(char), NOUT, Infile) != NOUT )
       printf("Error, Unable to read from outdata.bin\n");
   fclose(Infile);

   for (i=0; i< NOUT -2; i++) {  // the last 2 outputs are undefined for this test case
     if ( Output[i] != Expected_Out[i] ) {
         printf("### FAILED - conv3x3 !!!\n");
         return 1;
     }
   }

   printf("Cycle-count:  conv3x3 -- %d\n", Total_cycles);

   printf("*** PASS!\n");
   return 0;

}

