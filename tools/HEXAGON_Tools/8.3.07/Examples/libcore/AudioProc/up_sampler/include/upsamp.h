/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file upsamp.h
    \brief Brief description of file
*/

/*! 
Performs upsampling audio signal by a factor of 8.

\param in pointer to input buffer
\param filtMem pointer to filter states
\param nsamples number of input samples
\param mode function call mode
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a filtMem is aligned by 8-bytes

\b Cycle-Count
 - 83*nsamples + 3

\b Notes
 - mode!=0 used for stero.
 - filtMem array has 28 elements. 
 - the most current input sample is the first in filtMem array.

*/

void upsamp8( Word16 *in, 
              Word16 *filtMem,
              Word16 nsamples,
              int mode,
              Word16 *out           );


