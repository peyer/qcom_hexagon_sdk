/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"

void CorrS16(  Word16 *x, Word16 *y,
               int length, int nc, int Qshift, 
               Word16 *cor                      )

{
  int i, j;
  Word64 sum;

  for (i = 0; i < nc; i++)  {
      sum = 0; 
      for (j=i; j < length; j++) {
         sum += x[j] * y[j-i] ;
      } 
      sum = LL_shr(sum, Qshift);
      cor[i] = extract_h(L_sat(sum));
  }
}


void CorrS32(  Word16 *x, Word16 *y,
               int length, int nc, int Qshift, 
               Word32 *cor                      )

{
  int i, j;
  Word64 sum;

  for (i = 0; i < nc; i++)  {
      sum = 0; 
      for (j=i; j < length; j++) {
         sum += x[j] * y[j-i] ;
      } 
      sum = LL_shr(sum, Qshift);
      cor[i] = L_sat(sum);
  }
}


void cxCorr(  CWord2x16 *x, CWord2x16 *y,
              int length, int nc, int Qshift, 
              CWord2x16 *cor                    )

{
  int i, j;
  CWord2x32 sum;

  for (i = 0; i < nc; i++)  {
      sum = 0; 
      for (j=i; j < length; j++) {
         // sum += x[j] * conj(y[j-i])
         sum = L_cmac_c(sum, x[j], y[j-i]);
      } 
      sum = L_Vasr(sum, Qshift);
      cor[i] = Vsat(sum);
  }
}

