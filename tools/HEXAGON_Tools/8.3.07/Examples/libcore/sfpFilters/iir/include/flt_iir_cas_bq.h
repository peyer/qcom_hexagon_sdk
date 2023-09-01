/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/* file - flt_iir_cas_bq.h
   Brief description of file
*/

/* 
Implements a cascade of biquads IIR filter

 - xin pointer to input buffer
 - coef pointer to filter coefficents buffer
 - state pointer to filter states buffer
 - nsec number of biquads sections
 - nsamples number of input samples 
 - out pointer to output buffer


 Assembly  Assumptions
 - None.

 Cycle-Count gives the number of cycles to execute filter

 Notes
 - None.

*/

void flt_IIR_casBiquad( float *xin, float *coef, float *state, \
                        int nsec, int nsamples, float *out);
