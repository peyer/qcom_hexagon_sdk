/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <math.h>
#include "data_macros.h"
#include "typedef.h"
#include "hexagon_sim_timer.h"

#include "vit3g.h"
#define DEC_BITS_3GPP          260      /* 12.2 kbps, 20msec frame */

#define OVERHEAD                0

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

int main()
{  FILE *fp;
   Word8 insample[3*(DEC_BITS_3GPP+K-1)];
   UWord8 output[(DEC_BITS_3GPP+K-1+7)/8];
   UWord8 Expected_output[(DEC_BITS_3GPP+K-1+7)/8];

   //  state metrics and trace buffer 
   Word16 buffer[2*NUM_STATES + (NUM_STATES/16)*(DEC_BITS_3GPP+K-1)] __attribute__((aligned(8)));
   Word16 Expected_bm[2*NUM_STATES];

   long long start_time; 
   int Total_cycles, Total_cycles1; 
   int i;

   // read input 
   if((fp = fopen(FULL_FILE_NAME("vit3gin.bin"),"rb")) == NULL) {
     printf("Error: can not open vit3gin.bin\n");
     return 1;
   }
   if (fread(insample,  sizeof(Word8), 3*(DEC_BITS_3GPP+K-1), fp) != 3*(DEC_BITS_3GPP+K-1) ) 
       printf("Error, Unable to read from vit3gin.bin\n");
   fclose(fp);

   /*-------------------------------------------*/
   /*  Test rate = 1/3                          */
   /*-------------------------------------------*/

   // call function
   start_time = hexagon_sim_read_pcycles();
   vit3g_1r3k9(insample, output, buffer, DEC_BITS_3GPP+K-1);
   Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - OVERHEAD;

   // check output
   if((fp = fopen(FULL_FILE_NAME("vit3gout.bin"),"rb")) == NULL) {
     printf("Error: can not open vit3gout.bin\n");
     return 1;
   }
   if (fread(Expected_output,  sizeof(UWord8), (int)((DEC_BITS_3GPP+K-1+7)/8), fp) 
         != (int)((DEC_BITS_3GPP+K-1+7)/8) ) {
       printf("Error, Unable to read from vit3gout.bin\n");
       return 1;
   }
   fclose( fp );

   for ( i = 0; i < (DEC_BITS_3GPP+K-1+7)/8; i++) {
       if(output[i] != Expected_output[i]) {
           printf("### FAILED!!!\n");
           return 1;
       }
   }

   // check branch metrics
   if((fp = fopen(FULL_FILE_NAME("vit3gbm.bin"),"rb")) == NULL) {
     printf("Error: can not open vit3gbm.bin\n");
     return 1;
   }
   if (fread(Expected_bm,  sizeof(Word16), 2*NUM_STATES, fp) != 2*NUM_STATES ) {
       printf("Error, Unable to read from vit3gbm.bin\n");
       return 1;
   }
   fclose( fp );

   for ( i = 0; i < 2*NUM_STATES; i++) {
       if ( buffer[i] != Expected_bm[i] ) {
           printf("### FAILED!!!\n");
           return 1;
       }
   }


   /*-------------------------------------------*/
   /*  Test rate = 1/2                          */
   /*-------------------------------------------*/

   // call function
   start_time = hexagon_sim_read_pcycles();
   vit3g_1r2k9 (insample, output, buffer, DEC_BITS_3GPP+K-1);
   Total_cycles1 = (int)(hexagon_sim_read_pcycles() - start_time) - OVERHEAD;

   // check output
   if((fp = fopen(FULL_FILE_NAME("vit3gout2.bin"),"rb")) == NULL) {
     printf("Error: can not open vit3gout2.bin\n");
     return 1;
   }
   if (fread(Expected_output,  sizeof(UWord8), (int)((DEC_BITS_3GPP+K-1+7)/8), fp) 
         != (int)((DEC_BITS_3GPP+K-1+7)/8) ) {
       printf("Error, Unable to read from vit3gout2.bin\n");
       return 1;
   }
   fclose( fp );

   for ( i = 0; i < (DEC_BITS_3GPP+K-1+7)/8; i++) {
       if(output[i] != Expected_output[i]) {
           printf("### FAILED!!!\n");
           return 1;
       }
   }

   // check branch metrics
   if((fp = fopen(FULL_FILE_NAME("vit3gbm2.bin"),"rb")) == NULL) {
     printf("Error: can not open vit3gbm2.bin\n");
     return 1;
   }
   if (fread(Expected_bm,  sizeof(Word16), 2*NUM_STATES, fp) != 2*NUM_STATES ) {
       printf("Error, Unable to read from vit3gbm2.bin\n");
       return 1;
   }
   fclose( fp );

   for ( i = 0; i < 2*NUM_STATES; i++) {
       if ( buffer[i] != Expected_bm[i] ) {
           printf("### FAILED!!!\n");
           return 1;
       }
   }

   printf("*** PASS!\n");
   printf(" Cycle-count(rate = 1/3): %d\n", Total_cycles );
   printf(" Cycle-count(rate = 1/2): %d\n", Total_cycles1);
   return 0;
}

