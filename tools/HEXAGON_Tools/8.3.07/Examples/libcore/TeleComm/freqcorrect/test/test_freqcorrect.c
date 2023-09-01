/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:19:54 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:19:54 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "data_macros.h"
#include "freqcorrect.h"
#include "sincostab.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

CWord2x16 input[100];
CWord2x16 output[100];
CWord2x16 Expected_output[100];
   
   

int main() 
{
  UWord32 phase;
  int i;
  int start_time, Total_cycles;
  FILE *Infile;

   // load input from file
   if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) {
     printf("Error: can not open indata.bin\n");
     return 1;
   }
   if (fread(input,  sizeof(CWord2x16), 100, Infile) != 100 ) 
       printf("Error, Unable to read from indata.bin\n");
   fclose(Infile);

   phase = freqcorrect( input, 0x293C48, 0x080000, 0, output); //dummy call
   // call function
   // ft set positive
   phase = freqcorrect( input, 0x293C48, 0x080000, 100, output); 

   // check return phase
   if ( phase != 0x10238C20 ) {
        printf("### FAILED!!!\n");
        return 1;
   }

   // load expect results from file
   if((Infile = fopen(FULL_FILE_NAME("outdata0.bin"),"rb")) == NULL) {
     printf("Error: can not open outdata0.bin\n");
     return 1;
   }
   if (fread(Expected_output,  sizeof(CWord2x16), 100, Infile) != 100 ) 
       printf("Error, Unable to read from outdata0.bin\n");
   fclose(Infile);

   // Compare results
   for (i=0; i<100; i++) {
       if ( output[i] != Expected_output[i] ) {
          printf("### FAILED!!!\n");
          return 1;
       }
   }

   phase = freqcorrect( input, -0x1293C48, 0x080000, 0, output); 
   // call function
   // ft set negative
   start_time = hexagon_sim_read_pcycles();
   phase = freqcorrect( input, -0x1293C48, 0x080000, 100, output); 
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   // check return phase
   if ( phase != 0x74238C20 ) {
        printf("### FAILED!!!\n");
        return 1;
   }

   // load expect results from file
   if((Infile = fopen(FULL_FILE_NAME("outdata1.bin"),"rb")) == NULL) {
     printf("Error: can not open outdata1.bin\n");
     return 1;
   }
   if (fread(Expected_output,  sizeof(CWord2x16), 100, Infile) != 100 ) 
       printf("Error, Unable to read from outdata1.bin\n");
   fclose(Infile);

   // Compare results
   for (i=0; i<100; i++) {
       if ( output[i] != Expected_output[i] ) {
          printf("### FAILED!!!\n");
          return 1;
       }
   }

   printf("*** PASS!\n");
   printf("Cycle-count: %d\n", Total_cycles);
   return 0;
}
