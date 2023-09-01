/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file pnseq.h
    \brief Brief description of file
*/

/*! 
Generate PN sequence with generator polynomial of h(D) = D^20 + D^19 + D^16 + D^14 + 1

\param seed initial state for PN sequence
\param length number of output PN bits 
\param output pointer to output buffer

\details

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 3*ceil(N/8) + 4

\b Notes
 - The most current bit corresponds to MSB 
 - If length is not multiple of 8, the last byte is filled as xxxxpp, where xxxx are DO-NOT-CARE bits.

*/


void PNGeneration( UWord32 seed, int length, UWord8 *output);
