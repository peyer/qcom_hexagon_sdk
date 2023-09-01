/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/* ================================================================== */ 
/*  PN sequence Generation                                            */
/*                                                                    */
/*     Generator:  h(D) = D^20 + D^19 + D^16 + D^14 + 1               */
/*     i.e., jth output of register shall satisfy                     */
/*     I(j) = I(j-20) ^ I(j-19) ^ I(j-16) ^ I(j-14)                   */
/* ------------------------------------------------------------------ */ 
/*  Implementation:                                                   */
/*     - generate 8-bits at once. If length is not a multiple         */ 
/*       of 8, the last byte is filled as xxxxppp, where xxxx         */
/*       are DO-NOT-CARE bits                                         */
/*                                                                    */
/*     - the most current bit corresponds to MSB                      */
/* ================================================================== */ 

#include <stdio.h>
#include "typedef.h"

void PNGeneration( UWord32 seed, int length, UWord8 *output)
{
  UWord32 LFSRvalue, ShftBackbits;
  int i, nlc;

  // Take care of length = 0
  if (length <= 0) return; 

  // initialize LFSR
  LFSRvalue = seed; 

  nlc = (length + 7) >> 3;

  for (i = 0; i < nlc; i++) {

      output[i] = (UWord8)LFSRvalue;

      // generating 8 shift back bits (14-bits are valid) 
      ShftBackbits =  ( LFSRvalue>>(20-20) ) ^
                      ( LFSRvalue>>(20-19) ) ^
                      ( LFSRvalue>>(20-16) ) ^ 
                      ( LFSRvalue>>(20-14) );

      // updating content of LFSR
      LFSRvalue = ((LFSRvalue >> 8) & ((1<<(20-8))-1)) | ( ShftBackbits<<(20-8) );
  }

  return;
}

