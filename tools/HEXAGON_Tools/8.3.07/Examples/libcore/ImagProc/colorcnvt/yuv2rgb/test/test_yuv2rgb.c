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
#include "yuv2rgb.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define   WIDTH		36
#define   HEIGHT	16
#define   NP           (WIDTH*HEIGHT + (WIDTH*HEIGHT)/2)


int main() {
   UWord8 output[WIDTH*HEIGHT*3];
   UWord8 expected_out[WIDTH*HEIGHT*3];
   UWord8 input[NP];
   FILE *Infile, *Outfile; 

   int i;
   long long start_time; 
   int Total_cycles;

   // load input and reference output for files
   // 
   if((Infile = fopen(FULL_FILE_NAME("yuvin.bin"),"rb")) == NULL) {
     printf("Error: can not open yuvin.bin\n");
     return 1;
   }
   if((Outfile = fopen(FULL_FILE_NAME("rgbout.bin"),"rb")) == NULL) {
     printf("Error: can not open rgbout.bin\n");
     return 1;
   }
   if (fread(input,  sizeof(char), NP, Infile) != NP ) {  
       printf("Error, Unable to read yuvin.bin\n");
       return 1;
   }
   if (fread(expected_out, sizeof(char), 3*WIDTH*HEIGHT, Outfile) != (3*WIDTH*HEIGHT)) {
       printf("Error, Unable to read rgbout.bin\n");
       return 1;
   }
   fclose(Infile);
   fclose(Outfile);

   // call function
   // 
   start_time = hexagon_sim_read_pcycles();
   YUVConvert( input, WIDTH, HEIGHT, output);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   YUVConvert( input, 0, HEIGHT, output); //test 0 length
   YUVConvert( input, WIDTH, 0, output);

   // compare result
   // 
   for (i=0; i< 3*WIDTH*HEIGHT; i++) {
	if( output[i] != expected_out[i]) {
	   printf("### FAILED !!!!\n");
	   return 1;
        }
   }
   printf("*** PASS!\n");
   printf("TOTAL CYCLE-COUNT: %d\n", Total_cycles);
   return 0;
}    
