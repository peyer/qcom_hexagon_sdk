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
#include "q6basic_op.h"
#include "cxFHT.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

#define NF     256

void  rFHT_rc(int *data, int length);
CWord2x16 xsc16[NF]    __attribute__((aligned(8)));
CWord2x32 outsc32[NF]  __attribute__((aligned(8)));
Word32    RefOut1[NF], RefOut2[NF];
CWord2x16 outsc16[NF]  __attribute__((aligned(8)));
CWord2x16 Exp_out[NF];

int main()
{
  int i, nfht;
  long long start_time;
  int Total_cycles, overhead;
  FILE *Infile;

  /*---------------------------------------------------------------*/
  /*            Read inputs from file                              */
  /*---------------------------------------------------------------*/
  if((Infile = fopen(FULL_FILE_NAME("FHTin.bin"), "rb")) == NULL) {
    printf("Error: can not open FHTin.bin\n");
    return 1;
  }
  if (fread(xsc16, sizeof(CWord2x16), 256, Infile) != 256)
     printf("Error: Unable to read from FHTin.bin\n");
  fclose(Infile);


  start_time = hexagon_sim_read_pcycles(); 
  cxFHT_a32(xsc16, 2, outsc32);
  overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;
  /*---------------------------------------------------------------*/
  /*            Test 4-point FHT                                   */
  /*---------------------------------------------------------------*/
  nfht = 4;
  start_time = hexagon_sim_read_pcycles(); 
  cxFHT_a32(xsc16, nfht, outsc32);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  printf("  4-point FHT: Cycle-count is %d\n", Total_cycles);

  for (i=0; i<nfht; i++) {
     RefOut1[i] = real(xsc16[i]);
     RefOut2[i] = imag(xsc16[i]);
  }
  rFHT_rc(RefOut1, nfht);
  rFHT_rc(RefOut2, nfht);

  for (i=0; i<nfht; i++) {
     if((L_real(outsc32[i]) != RefOut1[i])||(L_imag(outsc32[i]) != RefOut2[i])) {
       printf("*** FAILED!!!\n");
       return 1;
     }
  }

  /*---------------------------------------------------------------*/
  /*            Test 8-point FHT                                   */
  /*---------------------------------------------------------------*/
  nfht = 8;
  start_time = hexagon_sim_read_pcycles(); 
  cxFHT_a32(xsc16, nfht, outsc32);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  printf("  8-point FHT: Cycle-count is %d\n", Total_cycles);

  for (i=0; i<nfht; i++) {
     RefOut1[i] = real(xsc16[i]);
     RefOut2[i] = imag(xsc16[i]);
  }
  rFHT_rc(RefOut1, nfht);
  rFHT_rc(RefOut2, nfht);

  for (i=0; i<nfht; i++) {
     if((L_real(outsc32[i]) != RefOut1[i])||(L_imag(outsc32[i]) != RefOut2[i])) {
       printf("*** FAILED!!!\n");
       return 1;
     }
  }
 
  /*---------------------------------------------------------------*/
  /*            Test 128-point FHT                                 */
  /*---------------------------------------------------------------*/
  nfht = 128;
  start_time = hexagon_sim_read_pcycles(); 
  cxFHT_a32(xsc16, nfht, outsc32);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  printf("128-point FHT: Cycle-count is %d\n", Total_cycles);

  for (i=0; i<nfht; i++) {
     RefOut1[i] = real(xsc16[i]);
     RefOut2[i] = imag(xsc16[i]);
  }
  rFHT_rc(RefOut1, nfht);
  rFHT_rc(RefOut2, nfht);

  for (i=0; i<nfht; i++) {
     if((L_real(outsc32[i]) != RefOut1[i])||(L_imag(outsc32[i]) != RefOut2[i])) {
       printf("*** FAILED!!!\n");
       return 1;
     }
  }

  /*---------------------------------------------------------------*/
  /*            Test 256-point FHT                                 */
  /*---------------------------------------------------------------*/
  nfht = 256;
  start_time = hexagon_sim_read_pcycles(); 
  cxFHT_a32(xsc16, nfht, outsc32);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  printf("256-point FHT: Cycle-count is %d\n", Total_cycles);

  for (i=0; i<nfht; i++) {
     RefOut1[i] = real(xsc16[i]);
     RefOut2[i] = imag(xsc16[i]);
  }
  rFHT_rc(RefOut1, nfht);
  rFHT_rc(RefOut2, nfht);

  for (i=0; i<nfht; i++) {
     if((L_real(outsc32[i]) != RefOut1[i])||(L_imag(outsc32[i]) != RefOut2[i])) {
       printf("*** FAILED!!!\n");
       return 1;
     }
  }

  printf("\n");
  /*---------------------------------------------------------------*/
  /*            Test 128-point scaled FHT                          */
  /*---------------------------------------------------------------*/
  nfht = 128;
  start_time = hexagon_sim_read_pcycles(); 
  cxsFHT_a16(xsc16, nfht, outsc16);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  printf("128-point scaled FHT: Cycle-count is %d\n", Total_cycles);

  if((Infile = fopen(FULL_FILE_NAME("sFHT128.bin"), "rb")) == NULL) {
    printf("Error: can not open sFHT128.bin\n");
    return 1;
  }

  if (fread(Exp_out, sizeof(CWord2x16), 128, Infile) != 128)
     printf("Error: Unable to read from sFHT128.bin\n");
  fclose(Infile);

  for (i=0; i<nfht; i++) {
     if( outsc16[i] != Exp_out[i]) {
       printf("*** FAILED!!!\n");
       return 1;
     }
  }

  /*---------------------------------------------------------------*/
  /*            Test 256-point scaled FHT                          */
  /*---------------------------------------------------------------*/
  nfht = 256;
  start_time = hexagon_sim_read_pcycles(); 
  cxsFHT_a16(xsc16, nfht, outsc16);
  Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

  printf("256-point scaled FHT: Cycle-count is %d\n", Total_cycles);

  if((Infile = fopen(FULL_FILE_NAME("sFHT256.bin"), "rb")) == NULL) {
    printf("Error: can not open sFHT256.bin\n");
    return 1;
  }

  if (fread(Exp_out, sizeof(CWord2x16), 256, Infile) != 256)
     printf("Error: Unable to read from sFHT256.bin\n");
  fclose(Infile);

  for (i=0; i<nfht; i++) {
     if( outsc16[i] != Exp_out[i]) {
       printf("*** FAILED!!!\n");
       return 1;
     }
  }

  printf("*** PASS!\n");

  return 0;
}




/*==========================================================================
FUNCTION  rFHT
DESCRIPTION     
perform an fht on the input data and return the results in the input array.

inputs:  data - pointer to array that contains data to be transformed     
length - length of fht                         

outputs: data[] is written with fht results in an ordered output 
============================================================================*/

void  rFHT_rc(int *data, int length)
{
  int *x_ptr;
  int *y_ptr;
  int span=(length>>1);
  int s, d;

  for(x_ptr=&data[0],y_ptr=&data[span];x_ptr<&data[span];x_ptr++,y_ptr++)
  {
      s = *x_ptr + *y_ptr;
      d = *x_ptr - *y_ptr;
      *x_ptr=s;
      *y_ptr=d;
  }

  if(span>1)
  {
      rFHT_rc(&data[0]   ,span);
      rFHT_rc(&data[span],span);
  }

  return;

}

