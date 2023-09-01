/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file freqcorrect.h
    \brief Brief description of file
*/

/*! 
Performs phase rotation on complex input vector to compensate for the frequency offset

\param input pointer to input signal buffer
\param ft normalized frequency in Q24
\param phacc accumulation of absolute ft in Q24
\param nsamp number of input samples
\param output pointer to output buffer

\details

\return
Accumulation of absolute ft

\b Assembly \b Assumptions
 - \a width is an even number
 - \a src is aligned by 2-bytes
 - \a dst is aligned by 2-bytes

\b Cycle-Count
 - 5* nsamp + 8

\b Notes
 - normalized frequency ft is defined as f*Ts
 - instantaneous phase = sign(ft)*2*PI*phacc 

*/

UWord32 freqcorrect(CWord2x16 *input, Word32 ft, UWord32 phacc, 
                    int nsamp, CWord2x16 *output);
