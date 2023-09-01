/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file idct.h
    \brief Brief description of file
*/

/*! 
Performs an IEEE-1180 compliant 8x8 inverser discrete cosin transform (IDCT) 

\param input pointer to input buffer 
\param num_idcts number of 8x8 blocks to be transformed
\param output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a both input and output arrays are aligned by 8-bytes

\b Cycle-Count

 - 112*N + 22

\b Notes

 - input data is in Q12.4 format.
 - output array can be the same as input arrary (i.e., inplace implementation)

*/

void iDCT(Word16 *input, int num_idcts, Word16 *output);

