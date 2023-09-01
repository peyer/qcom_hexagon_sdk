/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file cxfir.h
    \brief Brief description of file
*/

/*! 
Peforms FIR filtering on block of complex data

\param xin pointer to input buffer
\param coefs pointer to coefficents array
\param taps number of taps of FIR filter
\param length number of data to be filtered
\param Qshift shift factor after accumulation
\param yout pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a taps and \a length are multiples of 2 and no less than 4 and 2 respectively
 - \a xin and \a yout arrays are aligned by 8-bytes
 - \a coefs arrays are aligned by 4-bytes
 - coefficients must be arranged in reverse order.

\b Cycle-Count
 - length/2 * (taps + 3) + 4

\b Notes
 - multi-sample implementation to reduce load bandwidth.
 - 32-bit accumulator used for summation of real/image part.

*/

void cxfir(CWord2x16 *xin, CWord2x16 *coefs, int taps,  \
           int length, int Qshift, CWord2x16 *yout);
