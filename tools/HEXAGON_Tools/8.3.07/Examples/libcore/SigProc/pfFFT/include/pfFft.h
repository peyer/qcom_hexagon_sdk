/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file fft320_360.h
    \brief Brief description of file
*/



/*! 
Performs scaled 320-point complex FFT  

\param Input pointer to input buffer
\param Output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a Input must be aligned by 2048-bytes
 - \a Output must be aligned by 2048-bytes

\b Cycle-Count
 - 1845

\b Notes
 - None

*/

void fft320( CWord2x16 *Input, CWord2x16 *Output);


/*! 
Performs scaled 360-point complex FFT  

\param Input pointer to input buffer
\param Output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a Input must be aligned by 2048-bytes
 - \a Output must be aligned by 256-bytes

\b Cycle-Count
 - 2105

\b Notes
 - None

*/

void fft360( CWord2x16 *Input, CWord2x16 *Output);



/*! 
Performs scaled 192-point complex FFT  

\param Input pointer to input buffer
\param Output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a Input must be aligned by 1024-bytes
 - \a Output must be aligned by 1024-bytes

\b Cycle-Count
 - 1091

\b Notes
 - None

*/

void fft192( CWord2x16 *Input, CWord2x16 *Output);
