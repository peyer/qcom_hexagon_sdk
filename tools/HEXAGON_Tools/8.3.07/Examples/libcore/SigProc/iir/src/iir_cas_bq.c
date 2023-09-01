/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"


/*
 *-----------------------------------------------*
 * Implement a cascade of biquads, i.e.,         *
 * transfer function of:                         *
 *             1 + c0*z^-1 + c1*z^-2             *
 *     H(z) = -----------------------            *
 *             1 - c2*z^-1 - c3*z^-2             *
 *                                               *
 * for each biquad in the filter                 *
 *-----------------------------------------------*
*/
void IIR_casBiquad( Word16 *xin, Word16 *pCoef, Word16 *pState, \
                    int nSec, int nsamples, Word16 *out)
{
   Word16 *coef, *state;
   Word32 t, x;
   int i, j;

   for(i = 0; i < nsamples; i++) {
      coef = pCoef;
      state = pState; 
      x = xin[i] << 16;

      for (j=0; j< nSec; j++) {

//        t = x + coef[0]*state[0] + coef[1]*state[1];
//        x = t + coef[2]*state[0] + coef[3]*state[1];

          t = L_sat( (Word64)x + coef[0]*state[0] + coef[1]*state[1]);
          x = L_add(coef[2]*state[0], coef[3]*state[1]);
          x = L_add(x, t);

          state[1] = state[0];
          state[0] = extract_h(t);

          coef += 4;
          state += 2;
      }
      out[i] = extract_h(x);
   }
}
