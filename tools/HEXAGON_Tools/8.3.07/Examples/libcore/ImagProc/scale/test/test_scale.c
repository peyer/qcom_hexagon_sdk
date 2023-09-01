#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scale.h"

#include "hexagon_sim_timer.h"
#include "data_macros.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define  WIDTH           480
#define  HEIGHT          320

unsigned char img[WIDTH*HEIGHT];
unsigned char scaledImg[WIDTH*HEIGHT/4];
unsigned char refImg[WIDTH*HEIGHT/4];



int main()
{
   FILE *Infile;
   int i;
   long long start_time;
   int Total_cycles0, Total_cycles1;


   // read inputs from file
   if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) 
   {
      printf("Error: can not open indata.bin\n");
      return 1;
   }
   if (fread(img, sizeof(char), (WIDTH*HEIGHT), Infile) != (WIDTH*HEIGHT) )
   {
      printf("Error: can not read from indata.bin\n");
      return 1;
   }
   fclose(Infile);

   start_time = hexagon_sim_read_cycles();
   scale_DownSampleToHalf(img, WIDTH, HEIGHT, scaledImg);
   Total_cycles0 = (int)(hexagon_sim_read_cycles() - start_time);

   // Compare results
   if((Infile = fopen(FULL_FILE_NAME("outdataH.bin"), "rb")) == NULL)
   {
      printf("Error: can not open outdataH.bin\n");
      return 1;
   }
   if (fread(refImg, sizeof(char), (WIDTH/2)*(HEIGHT/2), Infile) != ((WIDTH/2)*(HEIGHT/2)))
   {
      printf("Error: can not read from outdataH.bin\n");
      return 1;
   }
   fclose(Infile);

   for (i=0; i<(WIDTH/2)*(HEIGHT/2); i++)
   {
      if ( scaledImg[i] != refImg[i] )
      {
         printf("#### FAILED !!! (scale down by half)\n");
         return 1;
      }
   }

   start_time = hexagon_sim_read_cycles();
   scale_DownSampleToQuarter(img, WIDTH, HEIGHT, scaledImg);
   Total_cycles1 = (int)(hexagon_sim_read_cycles() - start_time);

   // Compare results
   if((Infile = fopen(FULL_FILE_NAME("outdataQ.bin"), "rb")) == NULL)
   {
      printf("Error: can not open outdataQ.bin\n");
      return 1;
   }
   if (fread(refImg, sizeof(char), (WIDTH/4)*(HEIGHT/4), Infile) != ((WIDTH/4)*(HEIGHT/4)))
   {
      printf("Error: can not read from outdataQ.bin\n");
      return 1;
   }
   fclose(Infile);

   for (i=0; i<(WIDTH/4)*(HEIGHT/4); i++)
   {
      if ( scaledImg[i] != refImg[i] )
      {
         printf("#### FAILED !!! (scale down by quarter)\n");
         return 1;
      }
   }
   printf("Scale down %dx%d by Half   : Total cycles = %d\n", WIDTH, HEIGHT, Total_cycles0);
   printf("Scale down %dx%d by Quarter: Total cycles = %d\n", WIDTH, HEIGHT, Total_cycles1);
   printf("*** PASS!\n");

   return 0;
}
