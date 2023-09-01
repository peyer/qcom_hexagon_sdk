/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file vit3g.h
    \brief Brief description of file
*/

#define K                      9       // constraint length
#define NUM_STATES             256     // number of states per trellis stage


/*! 
Performs Viterbi decoding of convolutional codes (r=1/3) specified in WCDMA

\param insamp pointer to soft-decision input buffer
\param output pointer to output buffer for packed decoded bits
\param buffer pointer to a working buffer

\details

\b Assembly \b Assumptions
 - working buffer is aligned by 8-bytes.

\b Cycle-Count
 - 171*N + 2*ceil(N/8) + 78  (45,974 for N=268)

\b Notes
 - A total of (2x256+268x256/16)x2 bytes is required for working buffer 
 - 16-bit accumulation used for path metrics
 - Decoded bits are packed as
\par
   byte0 = I0I1..... I7

\par
   byte1 = I8I9..... I15

\par
   ....

\par
   byte33= I264..I267xxxxx

*/

void vit3g_1r3k9 (Word8 *insamp, UWord8 *output, Word16 *buffer, Word32 numbits);


/*! 
Performs Viterbi decoding of convolutional codes (rate = 1/2) specified in WCDMA

\param insamp pointer to soft-decision input buffer
\param output pointer to output buffer for packed decoded bits
\param buffer pointer to a working buffer
\param numbits number of decoded bits

\details

\b Assembly \b Assumptions
 - working buffer is aligned by 8-bytes.

\b Cycle-Count
 - 168*N + 2*ceil(N/8) + 82 

\b Notes
 - A total of (2x256+268x256/16)x2 bytes is required for working buffer 
 - 16-bit accumulation used for path metrics
 - Decoded bits are packed as
\par
   byte0 = I0I1..... I7

\par
   byte1 = I8I9..... I15

\par
   ....

\par
   byte33= I264..I267...

*/

void vit3g_1r2k9 (Word8 *insamp, UWord8 *output, Word16 *buffer, Word32 numbits);

