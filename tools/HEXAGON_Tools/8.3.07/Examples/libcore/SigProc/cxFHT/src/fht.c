/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"


/***************************************************************************/
/*   FUNCTION:  cxFHT_a32                                                  */
/***************************************************************************/
/*   Description: Radix-4 implementation of FHT                            */
/*   INPUTS:                                                               */
/*           CWord2x16 *in - pointer to array that contains 32-bit         */
/*                           (16bit I and 16bit Q) complex data            */
/*           int N  - number of FHT points  (N is power of 2 and >= 4)     */
/*                                                                         */
/*   OUTPUTS:                                                              */
/*            CWord2x32 *out - pointer to an array written with fht        */
/*                             results in an ordered output                */
/***************************************************************************/
void cxFHT_a32(CWord2x16 *in, int N, CWord2x32 *out)
{
  Word64 A, B, C, D;  
  Word64 A1, B1, C1, D1;  
  Word32 *xin = (Word32 *)in;
  Word64 *yout = (Word64 *)out;
  int stage, i, j, nG, nS, LOG2N, LOG4N;

  if (N < 4) return;

  LOG2N = 31-cl0(N); 

  for (i= 0; i< N/4; i++) {
       A= L_Vsxt(xin[i*4 + 0]);
       B= L_Vsxt(xin[i*4 + 1]);
       C= L_Vsxt(xin[i*4 + 2]);
       D= L_Vsxt(xin[i*4 + 3]);

       A1 = L_Vadd(A, B);
       B1 = L_Vsub(A, B);
       C1 = L_Vadd(C, D);
       D1 = L_Vsub(C, D);

       yout[i*4 + 0] = L_Vadd(A1, C1);
       yout[i*4 + 1] = L_Vadd(B1, D1);
       yout[i*4 + 2] = L_Vsub(A1, C1);
       yout[i*4 + 3] = L_Vsub(B1, D1);
  }

  LOG4N = LOG2N >> 1;
  nG = N/16; 
  nS = 4;

  for (stage= 1; stage< LOG4N; stage++) {

    for (i= 0; i< nG; i++) {
      for (j= 0; j< nS; j++) {

           A= yout[(4*i+0)*nS+j];
           B= yout[(4*i+1)*nS+j];
           C= yout[(4*i+2)*nS+j];
           D= yout[(4*i+3)*nS+j];

           A1 = L_Vadd(A, B);
           B1 = L_Vsub(A, B);
           C1 = L_Vadd(C, D);
           D1 = L_Vsub(C, D);

           yout[(4*i+0)*nS+j] = L_Vadd(A1, C1);
           yout[(4*i+1)*nS+j] = L_Vadd(B1, D1);
           yout[(4*i+2)*nS+j] = L_Vsub(A1, C1);
           yout[(4*i+3)*nS+j] = L_Vsub(B1, D1);
      }
    }
    nS <<= 2;
    nG >>= 2;
  }

  // if N = 2* 4^k
  if ( LOG2N & 1 ) { 
      for (j= 0; j< N/2; j++) {
           A= yout[    j];
           B= yout[N/2+j];
           yout[  j  ] = L_Vadd(A, B);
           yout[N/2+j] = L_Vsub(A, B);
      }
  }

  return;

}




/***************************************************************************/
/*   FUNCTION:  cxsFHT_a16                                                 */
/***************************************************************************/
/*   Description: Radix-4 implementation of FHT with scaling factor of 1/N */
/*   INPUTS:                                                               */
/*           CWord2x16 *in - pointer to array that contains 32-bit         */
/*                           (16bit I and 16bit Q) complex data            */
/*           int N  - number of FHT points  (N is power of 2 and >= 4)     */
/*                                                                         */
/*   OUTPUTS:                                                              */
/*            CWord2x16 *out - pointer to an array written with fht        */
/*                             results in an ordered output                */
/***************************************************************************/
CWord2x16 cavg( CWord2x16 x, CWord2x16 y ) {
   Word32 zr, zi;
   zr = (Word32)real(x) + (Word32)real(y);
   zi = (Word32)imag(x) + (Word32)imag(y);

#ifdef TWOS_COMPLEMENT_ROUNDING
   zr += 1;
   zi += 1;
#else
   zr +=  (zr>>1) & 1; 
   zi +=  (zi>>1) & 1;
#endif
   zr >>= 1;
   zi >>= 1;

   return( complex((Word16)zr, (Word16)zi) );
}
 

CWord2x16 cnavg( CWord2x16 x, CWord2x16 y ) {
   Word32 zr, zi;
   zr = (Word32)real(x) - (Word32)real(y);
   zi = (Word32)imag(x) - (Word32)imag(y);
#ifdef TWOS_COMPLEMENT_ROUNDING
   zr += 1;
   zi += 1;
#else
   zr +=  (zr>>1) & 1;
   zi +=  (zi>>1) & 1;
#endif
   zr >>= 1;
   zi >>= 1;

   return( complex(saturate(zr), saturate(zi)) );
}
 


void cxsFHT_a16(CWord2x16 *in, int N, CWord2x16 *out)
{
  CWord2x16 A, B, C, D;  
  CWord2x16 A1, B1, C1, D1;  
  CWord2x16 *xin = (Word32 *)in;
  int stage, i, j, nG, nS, LOG2N, LOG4N;

  if (N < 4) return;

  LOG2N = 31-cl0(N); 
  LOG4N = LOG2N >> 1;

  nG = N/4; 
  nS = 1;

  for (stage= 0; stage< LOG4N; stage++) {

    for (i= 0; i< nG; i++) {
      for (j= 0; j< nS; j++) {

           A= xin[(4*i+0)*nS+j];
           B= xin[(4*i+1)*nS+j];
           C= xin[(4*i+2)*nS+j];
           D= xin[(4*i+3)*nS+j];

           A1 =  cavg(A, B);
           B1 = cnavg(A, B);
           C1 =  cavg(C, D);
           D1 = cnavg(C, D);

           out[(4*i+0)*nS+j] =  cavg(A1, C1);
           out[(4*i+1)*nS+j] =  cavg(B1, D1);
           out[(4*i+2)*nS+j] = cnavg(A1, C1);
           out[(4*i+3)*nS+j] = cnavg(B1, D1);
      }
    }
    nS <<= 2;
    nG >>= 2;
    xin = out;
  }

  // if N = 2* 4^k
  if ( LOG2N & 1 ) { 
      for (j= 0; j< N/2; j++) {
           A= out[    j];
           B= out[N/2+j];
           out[  j  ] =  cavg(A, B);
           out[N/2+j] = cnavg(A, B);
      }
  }

  return;

}

