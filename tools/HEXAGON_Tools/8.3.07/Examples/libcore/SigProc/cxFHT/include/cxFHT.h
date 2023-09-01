/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file cxFHT.h
    \brief Brief description of file
*/

/*! 
Peforms fast Hadmard transform (FHT) on complex numbers.

\param in pointer to input buffer
\param N number of FHT points
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a N is power of 2 and N>=4
 - \a input and \a output arrays are aligned by 8-bytes

\b Cycle-Count
 - (N+3)*K + 3*N/2 + 12, for N = 2*(4^k)
 - (N+3)*K +   N/2 + 9 , for N = 4^K

\b Notes
 - Radix-4 implementation.
 - Data types are different between input and output.

*/

void cxFHT_a32(CWord2x16 *in, int N, CWord2x32 *out);


/*! 
Peforms fast Hadmard transform (FHT) on complex numbers with a scaling factor of 1/N.

\param in pointer to input buffer
\param N number of FHT points
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a N is power of 2 and N>=4
 - \a input and \a output arrays are aligned by 8-bytes

\b Cycle-Count
 - (N/2+3)*K + N   + 10, for N = 2*(4^k)
 - (N/2+3)*K + N/2 + 7 , for N = 4^K

\b Notes
 - Radix-4 inplace implementation.

*/

void cxsFHT_a16(CWord2x16 *in, int N, CWord2x16 *out);
