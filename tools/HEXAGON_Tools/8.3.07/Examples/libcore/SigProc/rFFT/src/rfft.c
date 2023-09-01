/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/*****************************************************************/
/*     This program is used to as a reference for                */
/*     Hexagon implementation.                                     */
/* ============================================================= */    
/*  Reference:                                                   */
/*  "Real-Valued Fast Fourier Transform Algorithms",             */
/*  IEEE Trans. on ASSP, vol 35, pp. 849-863, June 1987          */
/*****************************************************************/
#include <stdio.h>
#include "q6basic_op.h"
#include "fft_util.h"
#include "fft.h"




/***************************************************************/
/* Function: N-point scaled real-valued FFT                    */
/***************************************************************/
/* - scaling factor = 1/N                                      */
/***************************************************************/

void rfft(Word16 *input, int N, CWord2x16 *Wt1, CWord2x16 *Wt2, CWord2x16 *output) 
{
  int i;
  CWord2x16 X, Y;
  CWord2x16 *Z = output;

 /*************************************/
 /* Construct complex array with even */
 /* input as real and odd input as    */ 
 /* imaginary parts                   */
 /* Then do N/2-point complex FFT     */
 /*************************************/

  sfft16x16((CWord2x16 *)input, N/2, Wt1, Z); 
  
 /*************************************/
 /*  Calculate last stage butterflies */
 /**************************************/
  // calculate FFT at k=0, k=N/2
  X = complex(real(Z[0]), 0);
  Y = complex(imag(Z[0]), 0);

  output[0  ] = cavg(X,Y);
  output[N/2] = cnavg(X,Y);

  for (i=1; i<= N/4; i++) {

     X =  cavg(Z[i], conjugate(Z[N/2-i]));
     Y = cnavg(Z[i], conjugate(Z[N/2-i]));

     Y = cmult_r(Wt2[i-1],Y);
     output[i    ] = cnavg(X, Y);

     if (i!= N/4) {
       output[N/2-i] = conjugate(cavg(X, Y));
     }
  }
}

