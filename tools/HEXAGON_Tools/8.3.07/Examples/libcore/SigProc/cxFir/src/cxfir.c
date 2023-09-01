/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"

void cxfir( CWord2x16 *xin, CWord2x16 *coefs, 
            int taps,       int length, 
            int Qshift,     CWord2x16 *yout )
{
  CWord2x32 sum;
  int i, j;

  for (i = 0; i < length; i++)  {
      sum = 0; 
      for (j=0; j < taps; j++) {
         sum = L_cmac(sum, coefs[j],xin[i+j]) ;
      } 
      sum = L_Vasr(sum, Qshift);	// sum >>= Qshift		
      yout[i] = Vsat(sum);
  }
}

