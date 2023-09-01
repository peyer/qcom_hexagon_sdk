/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file iir_cas_bq.h
    \brief Brief description of file
*/

/*! 
Implements a cascade of biquads IIR filter

\param xin pointer to input buffer
\param coef pointer to filter coefficents buffer
\param state pointer to filter states buffer
\param nsec number of biquads sections
\param nsamples number of input samples 
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - None.

\b Cycle-Count
 -  nsamples*(3*nsec + 2) + 5

\b Notes
 - None.

*/

void IIR_casBiquad( Word16 *xin, Word16 *coef, Word16 *state, \
                    int nsec, int nsamples, Word16 *out);
