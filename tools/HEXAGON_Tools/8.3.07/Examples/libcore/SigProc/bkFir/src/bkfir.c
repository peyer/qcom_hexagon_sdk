/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"

void bkfir( Word16 *xin, Word16 *coefs, 
            int taps,    int length, 
            int Qshift,  Word16 *yout  )
{
  int i, j;
  Word64 sum;

  for (i = 0; i < length; i++)  {
      sum = 0; 
      for (j=0; j < taps; j++) {
         sum += coefs[j] * xin[i+j] ;
      } 
      sum = LL_shr(sum, Qshift);
      yout[i] = extract_h(L_sat(sum));
  }
}

