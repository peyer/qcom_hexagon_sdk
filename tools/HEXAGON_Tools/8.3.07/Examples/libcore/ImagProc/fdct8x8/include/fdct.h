/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file fdct.h
    \brief Brief description of file
*/

/*! 
Performs block 8x8 forward discrete cosin transform 

\param input pointer to input buffer 
\param num_idcts number of 8x8 blocks to be transformed
\param output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a both input and output arrays are aligned by 8-bytes

\b Cycle-Count

 - 96*N + 19

\b Notes

 - output array can be the same as input arrary (i.e., inplace implementation)

*/


void fDCT(Word16 *input, int num_fdcts, Word16 *output);

