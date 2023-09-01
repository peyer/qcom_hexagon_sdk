/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"


#define LUT_SIZE      512
extern Word16 COSSINTAB[LUT_SIZE+2];
/* ------------------------------------------------------------- *
 *  dsplib_sin()                                                 *
 *    - Approximates  sin(2*PI*x)                                *
 *      Use LUT with interploation for approximation             *
 *    - intput x in Q24                                          *
 *    - Output in Q31                                            * 
 *                                                               * 
 * ------------------------------------------------------------- */
Word32 dsplib_sin(Word32 x) 
{
  UWord32 xa;
  Word32 y;
  Word16 frac;
  int index, flag;

   if ( x<0 ) {
      xa = -x;
      flag = 1;
   }
   else  {
      xa = x;
      flag = 0;
   }

   index =  (xa >> 15)% (LUT_SIZE/2);
   frac = (Word16)(xa &0x07fff);

   y = L_mac( (Word32)COSSINTAB[2*index+1]<<16, 
              COSSINTAB[2*index+3]-COSSINTAB[2*index+1], frac);

   if ((xa>>15)&(LUT_SIZE/2) ) 
       flag ^= 1;

   if (flag)
       y = L_negate(y);

   return y;
}


/* ------------------------------------------------------------- *
 *  dsplib_cos()                                                 *
 *    - Approximates  cos(2*PI*x)                                *
 *      Use LUT with interploation for approximation             *
 *    - intput x in Q24                                          *
 *    - Output in Q31                                            * 
 *                                                               * 
 * ------------------------------------------------------------- */
Word32 dsplib_cos(Word32 x) 
{
  UWord32 xa;
  Word32 y;
  Word16 frac;
  int index;

   xa = (x<0) ? -x : x;

   index =  (xa >> 15)% (LUT_SIZE/2);
   frac = (Word16)(xa &0x07fff);

   y = L_mac( (Word32)COSSINTAB[2*index]<<16, 
              COSSINTAB[2*(index+1)]-COSSINTAB[2*index], frac);

   if ((xa>>15)&(LUT_SIZE/2) ) 
       y = L_negate(y);

   return y;
}

/* ------------------------------------------------------------- *
 *  dsplib_atan()                                                *
 *    - Approximates  atan(x)/PI for -1<= x <= 1                 *
 *      Use polynomial approximation with MSE.                   *
 *     (maximum error < 0.0003)                                  * 
 *    - Output in Q31                                            * 
 *                                                               * 
 * ------------------------------------------------------------- */
Word32 dsplib_atan(Word16 x) 
{
   Word16 C1, C2, C3;
   Word16 x2;
   Word32 y1, y2, y;

    // set coefficents (change C3 = 866)
    C1 = 10388; C2 = -3049; C3 = 863;  

    // calculate c1*x + c2*x^3 + c3*x^5
    x2 = mult(x, x);  
    y1 = L_mac((Word32)C2<<16, C3, x2);
    y2 = L_mac((Word32)C1<<16, extract_h(y1), x2);
    y  = L_mult(extract_h(y2), x);

    return y;

}



  
