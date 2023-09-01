/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/* ===================================================================== */
/*  FUNCTION: UWord32 freqcorrect()                                      */
/*  DESCRIPTION:                                                         */
/*      Perform frequency shift on complex input vector.                 */ 
/*      Input:                                                           */
/*          CWord2x16 *input  -- pointer to complex input samples        */
/*          Word32    ft      -- normalized frequency, i.e.,             */
/*                               f*Ts in Q(15+9)                         */
/*          UWord32   phacc   -- accumulation of absolute ft in Q24.     */
/*                               instantaneous phase =                   */
/*                                      sign(ft)*2*PI*phacc              */
/*          int       nsamp   -- number of samples                       */			
/*          CWord2x16 *output -- pointer to output samples               */
/*      output:                                                          */
/*          - samples in *output                                         */
/*          - return value: phacc                                        */
/* ===================================================================== */
#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"

#define LUT_SIZE     512		//actual table size is LUT_SIZE+2
extern Word16 COSSINTAB[LUT_SIZE+2];

UWord32 freqcorrect(CWord2x16 *input, Word32 ft, UWord32 phacc, 
                    int nsamp, CWord2x16 *output)
{
    Word16 frac;
    Word32 xi, xq, x;
    UWord32 ftabs;
    int index;
    int i;
    
    ftabs = L_abs(ft);

    for (i = 0; i < nsamp; i++) {
        /* ------------------------------------------------------------- */
	/* calculate index for cosin/sine  in LUT                        */
        /* ------------------------------------------------------------- */
	index =   (phacc >> 15)% (LUT_SIZE/2);

        /* ------------------------------------------------------------- */
        /* Interpolate LUT values using fractional portion of phacc      */
        /* ------------------------------------------------------------- */
	frac = (Word16)(phacc & 0x07fff);

        xi = L_mac( (Word32)COSSINTAB[2*index  ]<<16, 
                    COSSINTAB[2*index+2] - COSSINTAB[2*index  ], frac);

        xq = L_mac( (Word32)COSSINTAB[2*index+1]<<16, 
                    COSSINTAB[2*index+3] - COSSINTAB[2*index+1], frac);

        /* ------------------------------------------------------------- */
        /* Calculate the sign and truncate to 16-bit                     */
        /* ------------------------------------------------------------- */
        if ( (phacc >> 15) & (LUT_SIZE/2) ) {
           xi = L_negate(xi);
           xq = L_negate(xq);
        }

        x = complex( extract_h(xi), extract_h(xq) );

        /* ------------------------------------------------------------- */
        /* Rotate input sample                                           */
        /* ------------------------------------------------------------- */
        if ( ft < 0) 
            output[i] = cmult_cr(input[i], x);
        else
            output[i] = cmult_r(input[i], x);

	phacc += ftabs;

    }  
  
    return(phacc);
}


