/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file fft.h
    \brief Brief description of file
*/

/*! 
Performs (scaled) N-point complex FFT  

\param Input pointer to input buffer
\param points number of FFT points
\param twiddles pointer to twiddles factor array
\param Output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a Input must be aligned by array size
 - \a Output must be aligned by 8-bytes
 - \a twiddles must be aligned by 4-bytes

\b Cycle-Count
 - (7N/8+5)*K - (N+32)/24   + 10, for N = 4^K
 - (7N/8+5)*K + (17N-16)/24 + 13, for N= 2*(4^K) 

\b Notes
 - Radix-4 DIF implementation
 - 3N/4 twiddle factors are interleaved and stored in bit-reverse order.
 - TWOS_COMPLEMENT_ROUNDING defines convergent/2's-complement rounding mode.

*/

void  fft16x16( CWord2x16 *Input, int points, CWord2x16 *twiddles,
                CWord2x16 *Output);

void sfft16x16( CWord2x16 *Input, int points, CWord2x16 *twiddles,
                CWord2x16 *Output);


/*! 
Performs N-point complex inverse FFT  

\param Input pointer to input buffer
\param points number of IFFT points
\param twiddles pointer to twiddles factor array
\param Output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a Input must be aligned by array size
 - \a Output must be aligned by 8-bytes
 - \a twiddles must be aligned by 4-bytes

\b Cycle-Count
 - (7N/8+5)*K - (N+32)/24   + 10, for N = 4^K
 - (7N/8+5)*K + (17N-16)/24 + 13, for N= 2*(4^K) 

\b Notes
 - Radix-4 DIF implementation
 - Twiddle factors are the same as FFT

*/


void ifft16x16( CWord2x16 *Input, int points, CWord2x16 *twiddles,
                CWord2x16 *Output  );
