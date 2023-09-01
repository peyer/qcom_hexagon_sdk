/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
void flt_bkfir_ref( float *xin, float *coefs, 
                    int taps, int length, 
                    float *yout  )
{
  int i, j;

  for (i = 0; i < length; i++)  {
      float sum = 0.0; 
      for (j=0; j < taps; j++) {
         sum += coefs[taps-1-j] * xin[i+j] ;
      } 
      yout[i] = sum;
  }
}

