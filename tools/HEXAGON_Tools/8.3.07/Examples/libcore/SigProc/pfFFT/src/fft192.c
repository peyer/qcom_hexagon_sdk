/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
#include <stdio.h>
#include "q6basic_op.h"
#include "fft_util.h"
#include "fft.h"


/***************************************************************/
/* DSP implementation of 3-point FFT                           */
/***************************************************************/
/* - scaling factor 1/3                                        */
/***************************************************************/
void ButterflyRadix3( CWord2x16 *x ) 
{
  CWord2x16 WN3x23 = 0x49E72AAB;   // 2/3 * (1/2 + j*sqrt(3)/2)
  CWord2x16 C13 = 0x2AAB;
  CWord2x16 C23 = 0x5555;

  CWord2x16 A, B; 
  CWord2x16 XS0, Y0, Y1, Y2;
  int tempr, tempi;

     A  =  cavg(x[1],x[2]);
     B  = cnavg(x[1],x[2]);

     XS0 = cmult_r(x[0], C13);

     Y0  = cmult_r(A, C23); 
     x[0] = cadd(XS0,Y0);

     tempr = L_add(L_mult(real(A), real(WN3x23)), L_mult(imag(B), -imag(WN3x23)));
     tempi = L_add(L_mult(imag(A), real(WN3x23)), L_mult(real(B),  imag(WN3x23)));
     Y1 = complex( rnd(tempr), rnd(tempi) ); 
     x[1] = csub(XS0,Y1);

     tempr = L_add(L_mult(real(A), real(WN3x23)), L_mult(imag(B),  imag(WN3x23)));
     tempi = L_add(L_mult(imag(A), real(WN3x23)), L_mult(real(B), -imag(WN3x23)));
     Y2 = complex( rnd(tempr), rnd(tempi) ); 
     x[2] = csub(XS0,Y2);
  
}


/***************************************************************/
/* DSP implementation of 192-point FFT                         */
/***************************************************************/
/* - scaling factor 1/192                                      */
/***************************************************************/
void fft192(CWord2x16 *input, CWord2x16 *output)
{
      CWord2x16 x[3], xb1[3][64], xb2[3][64];
      int j, k;

      // 3-point FFT
      for (k=0;k<64;k++)
      {
          for (j=0;j<3;j++)
            x[j]= input[(64*j+3*k)%192];

          ButterflyRadix3(x);
            
          for (j=0;j<3;j++)
            xb1[j][k] = x[j];
      }

      // 64-point FFT on rows
      for (j=0;j<3;j++)
         sfft16x16( xb1[j], 64, FFTtwiddles, xb2[j]);

      // re-order output
      for (j=0;j<3;j++)
      {
         for (k=0;k<64;k++)
             output[(64*j+129*k)%192]=xb2[j][k];
      }

      return;
}
