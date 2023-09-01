/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <stdlib.h>

#include "data_macros.h"
#include "typedef.h"

#include "dsp_libs.h"
#include "sincostab.h"
#include "outdata.h"


#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

Word32 data[21] = { 14767,  5297, 13555, 18037, 17551, 6129, 22803, 9314, 
             27209, 12724, 20604, 31171, 10461, 2966,  8322,  284,
             25080, 16435, 22592, 30508, 12345  };


int main()
{
   int i;
   int exit_status;
   Word32 result;
   UWord32 y, x, ulexpOut;
   Word32 db, slexpOut, Qi;
   result_scale_t val, expOut, actOut;
   FILE *fdata;
   char *pFilename;

   printf("dsp math library I ...\n");

   exit_status = 0;
   for( i=0; i<20; i++) {

     val = dsplib_approx_invert( data[i]);
     if ( (val.result != Yinvert[2*i]) || (val.scale != Yinvert[2*i+1])) {
        printf("### invert FAILED!!!\n");
        exit_status = 1;
     }

     val = dsplib_newtons_invert( data[i]);
     if ( (val.result != Yinvert_newton[2*i]) || (val.scale != Yinvert_newton[2*i+1])) {
        printf("### newton_invert FAILED!!!\n");
        exit_status = 1;
     }

     val = dsplib_approx_invert_simple( data[i]);
     if ( (val.result != Yinvert_simple[2*i]) || (val.scale != Yinvert_simple[2*i+1])) {
        printf("### simple_invert FAILED!!!\n");
        exit_status = 1;
     }
  
     val = dsplib_approx_divide( data[i], data[i+1]);
     if ( (val.result != Ydiv[2*i]) || (val.scale != Ydiv[2*i+1])) {
        printf("### divide FAILED!!!\n");
        exit_status = 1;
     }

     result = dsplib_sqrt_lut( data[i], (1<<i));
     if ( result != Ysqrt[i] ) {
        printf("### sqrt_lut FAILED!!!\n");
        exit_status = 1;
     }
 
     result = dsplib_inv_sqrt_lut( data[i], (1<<i));
     if ( result != Yinvsqrt[i] ) {
        printf("### inv_sqrt_lut FAILED!!!\n");
        exit_status = 1;
     }

     result = dsplib_log10( data[i]);
     if ( result != Ylog[i] ) {
        printf("### log10 FAILED!!!\n");
        exit_status = 1;
     }

     result = dsplib_exp10(data[i]);
     if ( result != Yexp[i] ) {
        printf("### exp10 FAILED!!!\n");
        exit_status = 1;
     }

   }
 
   val = dsplib_invert(0x0F00);
     if ( (val.result != 17476) || (val.scale != 26)) {
        printf("### direct invert FAILED!!!\n");
        exit_status = 1;
   }

   // test atan()
   if ( dsplib_atan(-22767) != 0xe74242b8) {
        printf("### atan FAILED!!!\n");
        exit_status = 1;
   }
   if ( dsplib_atan(32767) != 0x2008bfee) {
        printf("### atan FAILED!!!\n");
        exit_status = 1;
   }

   // test sin() 
   if ( dsplib_sin(0x607200) != 1501775872) {
        printf("### sin FAILED!!!\n");
        exit_status = 1;
   }
   if ( dsplib_sin(0xe07200) != -1501775872) {
        printf("### sin FAILED!!!\n");
        exit_status = 1;
   }
   if ( dsplib_sin(-0x607200) != -1501775872) {
        printf("### sin FAILED!!!\n");
        exit_status = 1;
   }
 
   // test cos()
   if ( dsplib_cos(0x607200) != -1534987264) {
        printf("### cos FAILED!!!\n");
        exit_status = 1;
   }
   if ( dsplib_cos(0xe07200) != 1534987264) {
        printf("### cos FAILED!!!\n");
        exit_status = 1;
   }
   if ( dsplib_cos(-0x607200) != -1534987264) {
        printf("### cos FAILED!!!\n");
        exit_status = 1;
   }
   printf("done!\n");


   printf("dsp math library II...\n");

   /* ------------------------------------- */
   /*  test approx_uinv_linlut8             */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_uinv_linlut8_data.txt");
   fdata = fopen(pFilename,"r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%u %d %u" , &x, &expOut.scale, &expOut.result);
         actOut = approx_uinv_linlut8(x);
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_uinv_linlut8 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_uinv_cubelut8            */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_uinv_cubelut8_data.txt");
   fdata = fopen(pFilename,"r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%u %d %u" , &x, &expOut.scale, &expOut.result);
         actOut = approx_uinv_cubelut8(x);
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_uinv_cubelut8 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_uinv_newt                */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_uinv_newt_data.txt");
   fdata = fopen(pFilename,"r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%u %d %u" , &x, &expOut.scale, &expOut.result);
         actOut = approx_uinv_newt(x, 3 );
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_uinv_cubelut8 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_uinv_tylr                */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_uinv_tylr_data.txt");
   fdata = fopen(pFilename,"r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%u %d %u" , &x, &expOut.scale, &expOut.result);
         actOut = approx_uinv_tylr(x, 10);
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_uinv_tylr FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_dB_to_ulin_linlut8       */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_dB_to_ulin_linlut8_data.txt");
   fdata = fopen(pFilename, "r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%d %d %u" , &db, &expOut.scale, &expOut.result);
         actOut = approx_dB_to_ulin_linlut8(db);
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_dB_to_ulin_linlut8 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_dB_to_ulin_cubefit       */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_dB_to_ulin_cubefit_data.txt");
   fdata = fopen(pFilename, "r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%d %d %u" , &db, &expOut.scale, &expOut.result);
         actOut = approx_dB_to_ulin_cubefit(db);
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_dB_to_ulin_cubefit FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_dB_to_ulin_quadlut8      */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_dB_to_ulin_quadlut8_data.txt");
   fdata = fopen(pFilename, "r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%d %d %u" , &db, &expOut.scale, &expOut.result);
         actOut = approx_dB_to_ulin_quadlut8(db);
         if(actOut.scale != expOut.scale || actOut.result!=expOut.result) {
            printf("### approx_dB_to_ulin_cubefit FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_ulin_to_dB_linlut8       */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_ulin_to_dB_linlut8_data.txt");
   fdata = fopen(pFilename, "r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%u %d %d" , &x, &Qi, &slexpOut);
         db = approx_ulin_to_dB_linlut8(x, Qi);
         if(db != slexpOut) {
            printf("### approx_ulin_to_dB_linlut8 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_ulin_to_dB_cubelut8       */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_ulin_to_dB_cubelut8_data.txt");
   fdata = fopen(pFilename, "r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%u %d %d" , &x, &Qi, &slexpOut);
         db = approx_ulin_to_dB_cubelut8(x, Qi);
         if(db != slexpOut) {
            printf("### approx_ulin_to_dB_cubelut8 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }


   /* ------------------------------------- */
   /*  test approx_atan2_poly7              */
   /* ------------------------------------- */
   pFilename = FULL_FILE_NAME("test_approx_atan2_poly7_data.txt");
   fdata = fopen(pFilename,"r");
   if (fdata == NULL) {
      printf("Can not open test file %s\n", pFilename);
      exit_status = 1;
   }
   else {
      while (!feof(fdata)){
         fscanf(fdata,"%d %d %d" , &y, &x, &ulexpOut);
         if( approx_atan2_poly7(y, x) != ulexpOut) {
            printf("### atan2_poly7 FAILED!!!\n");
            exit_status = 1;
         }
      }   
      fclose(fdata);
   }

   printf("done!\n");
   if (exit_status == 0) {
      printf("*** PASS!\n");
   }

   exit(exit_status);
}



