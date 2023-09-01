/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "data_macros.h"
#include "typedef.h"
#include "ycbcr2rgb.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define WIDTH     102
#define WIDTH1    100

unsigned char Input[64*8*8] __attribute__((aligned(4)));
unsigned char Output[3*128*2] __attribute__((aligned(4)));
unsigned char Expected_Out[3*128*2];


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
   if (fread(Input,  sizeof(char), 64*64, Infile) != 64*64 ) 
       printf("Error, Unable to read from indata.bin\n");
   fclose(Infile);

   // call H1V1Convert() 
   start_time = hexagon_sim_read_pcycles();
   H1V1Convert(Input, WIDTH, Output);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   H1V1Convert(Input, 0, Output); //dummy call test 0 length

   // compare outputs
   if((Infile = fopen(FULL_FILE_NAME("h1v1out.bin"),"rb")) == NULL) {
     printf("Error: can not open h1v1out.bin\n");
     return 1;
   }
   if (fread(Expected_Out,  sizeof(char), 3*WIDTH, Infile) != 3*WIDTH ) 
       printf("Error, Unable to read from h1v1out.bin\n");
   fclose(Infile);

   for (i=0; i< WIDTH*3; i++) {
     if ( Output[i] != Expected_Out[i] ) {  
         printf("### FAILED - H1V1 !!!\n");
         return 1;
     }
   }
   printf("Cycle-count:  H1V1 -- %d\n", Total_cycles);


   // call H2V2Convert() 
   start_time = hexagon_sim_read_pcycles();
   H2V2Convert(Input, Input + 64 *4, WIDTH1, Output);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   H2V2Convert(Input, Input + 64 *4, 0, Output); //dummy call test 0 length

   // compare outputs
   if((Infile = fopen(FULL_FILE_NAME("h2v2out.bin"),"rb")) == NULL) {
     printf("Error: can not open h2v2out.bin\n");
     return 1;
   }
   if (fread(Expected_Out,  sizeof(char), 2*3*WIDTH1, Infile) != 2*3*WIDTH1 ) 
       printf("Error, Unable to read from h2v2out.bin\n");
   fclose(Infile);

   for (i=0; i< 2*3*WIDTH1; i++) {
     if ( Output[i] != Expected_Out[i] ) {  
         printf("### FAILED - H2V2 !!!\n");
         return 1;
     }
   }
   printf("Cycle-count:  H2V2 -- %d\n", Total_cycles);

   // call H2V1Convert() 
   start_time = hexagon_sim_read_pcycles();
   H2V1Convert(Input, WIDTH1, Output);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   H2V1Convert(Input, 0, Output); //dummy call test 0 length

   // compare outputs
   if((Infile = fopen(FULL_FILE_NAME("h2v1out.bin"),"rb")) == NULL) {
     printf("Error: can not open h2v1out.bin\n");
     return 1;
   }
   if (fread(Expected_Out,  sizeof(char), 3*WIDTH1, Infile) != 3*WIDTH1 ) 
       printf("Error, Unable to read from h2v1out.bin\n");
   fclose(Infile);

   for (i=0; i< 3*WIDTH1; i++) {
     if ( Output[i] != Expected_Out[i] ) {  
         printf("### FAILED - H2V1 !!!\n");
         return 1;
     }
   }
   printf("Cycle-count:  H2V1 -- %d\n", Total_cycles);


   // call H1V2Convert() 
   start_time = hexagon_sim_read_pcycles();
   H1V2Convert(Input, Input + 64 *4, WIDTH, Output);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   H1V2Convert(Input, Input + 64 *4, 0, Output);
   // compare outputs
   if((Infile = fopen(FULL_FILE_NAME("h1v2out.bin"),"rb")) == NULL) {
     printf("Error: can not open h1v2out.bin\n");
     return 1;
   }
   if (fread(Expected_Out,  sizeof(char), 2*3*WIDTH, Infile) != 2*3*WIDTH ) 
       printf("Error, Unable to read from h1v2out.bin\n");
   fclose(Infile);

   for (i=0; i< 2*3*WIDTH; i++) {
     if ( Output[i] != Expected_Out[i] ) {  
         printf("### FAILED - H1V2 !!!\n");
         return 1;
     }
   }
   printf("Cycle-count:  H1V2 -- %d\n", Total_cycles);

   printf("*** PASS!\n");
   return 0;

}

