/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file bkfir.h
    \brief Brief description of file
*/

/*! 
Peforms FIR filtering on block of data

\param xin pointer to input buffer
\param coefs pointer to coefficents array
\param taps number of taps of FIR filter
\param length number of data to be filtered
\param Qshift shift factor after accumulation
\param yout pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a taps is a multiple of 4 and no less than 8
 - \a length is a multiple of 4 and no less than 4
 - \a xin, \a yout and \a coefs arrays are aligned by 8-bytes
 - coefficients must be arranged in reverse order.

\b Cycle-Count
 - length/4 * (3*taps/4 + 4) + 9

\b Notes
 - multi-sample implementation to reduce load bandwidth.
 - 64-bit accumulator used for summation.

*/

void bkfir(Word16 *xin, Word16 *coefs, int taps,  \
           int length, int Qshift, Word16 *yout);
