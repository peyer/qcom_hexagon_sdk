/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/

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
void flt_IIR_casBiquad_ref( float *xin, float *pCoef, float *pState, \
                            int nSec, int nsamples, float *out)
{
   float *coef, *state;
   float t, x;
   int i, j;

   for(i = 0; i < nsamples; i++) {
      coef = pCoef;
      state = pState;
      x = xin[i];

      for (j=0; j< nSec; j++) {

          t = x + coef[0]*state[0] + coef[2]*state[1];
          x = t + coef[1]*state[0] + coef[3]*state[1];

          state[1] = state[0];
          state[0] = t;

          coef  += 4;
          state += 2;
      }
      out[i] = x;
   }
}
