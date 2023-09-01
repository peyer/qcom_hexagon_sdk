/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"


Word16 lms( Word16 xin, Word16 ydes, Word16 rate,   \
            int ntap, Word16 *states, Word16 *coef)
{ Word16 output, error, er;
  Word64 sum;
  int i;

    states[0] = xin;

    sum = 0;
    for(i = 0; i< ntap; i++)
      sum += coef[i] * states[i];
     
    output = saturate((Word32)LL_shr(sum,15));

    error = sub(ydes, output);
    er = mult_r(error, rate);

    for(i = 0; i < ntap; i++)
      coef[i] = add(coef[i], mult_r(er, states[i]));

    for(i = ntap-1; i > 0; i--)
      states[i] = states[i-1];
   
   return output;
}
