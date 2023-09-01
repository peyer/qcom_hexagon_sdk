/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file vitgsm.h
    \brief Brief description of file
*/


#define N 189


/*! 
Performs Viterbi decoding of convolutional codes used in GSM full-rate

\param INsoft pointer to soft-decision input buffer
\param OutWord pointer to output buffer for packed decoded bits

\details

\b Assembly \b Assumptions
 - \a OutWord is aligned by 2-bytes

\b Cycle-Count
 - 11*N + 1*ceil(N/16) + 10 (2100 for N=189)

\b Notes
 - Decoded bits are packed as
\par
   Word0 = I15I14..... I0

\par
   Word1 = I31I30..... I16

\par
   ....

\par
   Word11= xxxI188....I177I176

*/


void vitgsm(Word8 *INsoft, UWord16 *OutWord);

