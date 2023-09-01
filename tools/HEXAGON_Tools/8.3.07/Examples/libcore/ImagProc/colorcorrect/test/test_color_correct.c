/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "hexagon_sim_timer.h"

#include "data_macros.h"
#include "typedef.h"
#include "color_correct.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

UWord16 RGB[64*3] __attribute__((aligned(8)));
UWord16 Expected_results[64*3];

int main()
{
  static  ColorCorrectParams TestParam = { 256,1,2,256,-256,4,0,0,128,1,2,3,7};
  FILE *Infile;
  int i;
  long long start_time; 
  int Total_cycles;

   // read input RGB, and reference output
   if((Infile = fopen(FULL_FILE_NAME("rgbin.bin"),"rb")) == NULL) {
     printf("Error: can not open rgbin.bin\n");
     return 1;
   }
   if (fread(RGB,  sizeof(UWord16), 64*3, Infile) != (64*3) ) 
       printf("Error, Unable to read from rgbin.bin\n");
   fclose(Infile);

   if((Infile = fopen(FULL_FILE_NAME("rgbout.bin"),"rb")) == NULL) {
     printf("Error: can not open rgbout.bin\n");
     return 1;
   }
   if (fread(Expected_results,  sizeof(UWord16), 64*3, Infile) != (64*3) ) 
       printf("Error, Unable to read from rgbout.bin\n");
   fclose(Infile);


   // call function
   start_time = hexagon_sim_read_pcycles();

   color_correct ( RGB, RGB+64, RGB+128, 8, 8, &TestParam );

   color_correct ( RGB, RGB+64, RGB+128, 0, 0, &TestParam );

   for (i = 0; i < 64*3; i++) {
          if (RGB[i]!= Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
          }
    }

    printf("*** PASS!\n");
    Total_cycles = hexagon_sim_read_pcycles() - start_time;
    printf("Cycle-count: %d\n", Total_cycles);

    return 0;
}


