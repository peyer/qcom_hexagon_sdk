/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file corr.h
    \brief Brief description of file
*/

/*! 
Calculates correlation  between x[] and y[]

\param x pointer to input buffer #1
\param y pointer to input buffer #2
\param length length of input samples
\param nc number of correlation outputs
\param Qshift shift factor after accumulation
\param cor pointer to output correlation buffer(S16)

\details

\b Assembly \b Assumptions
 - \a length is a multiple of 4 and no less than 4
 - \a x is aligned by 4
 - \a y is aligned by 8
 - \a cor is aligned by 8

\b Cycle-Count
 - [2*(length/4)-ceil(nc/4)+1]*ceil(nc/4)*3/2 + 7*ceil(nc/4) + 15

\b Notes
 - 64-bit accumulator used for summation.

*/

void CorrS16( Word16 *x, Word16 *y, int length, int nc, int QShift, Word16 *cor); 

    


/*! 
Calculates correlation  between x[] and y[]

\param x pointer to input buffer #1
\param y pointer to input buffer #2
\param length length of input samples
\param nc number of correlation outputs
\param Qshift shift factor after accumulation
\param cor pointer to output correlation buffer(S32)

\details

\b Assembly \b Assumptions
 - \a length is a multiple of 4 and no less than 4
 - \a x is aligned by 4
 - \a y is aligned by 8
 - \a cor is aligned by 8

\b Cycle-Count
 - [2*(length/4)-ceil(nc/4)+1]*ceil(nc/4)*3/2 + 7*ceil(nc/4) + 15

\b Notes
 - 64-bit accumulator used for summation.

*/

void CorrS32( Word16 *x, Word16 *y, int length, int nc, int QShift, Word32 *cor); 


/*! 
Calculates correlation  between complex array x[] and y[]

\param x pointer to input buffer #1
\param y pointer to input buffer #2
\param length length of input samples
\param nc number of correlation outputs
\param Qshift shift factor after accumulation
\param cor pointer to output correlation buffer(CWord2x16)

\details

\b Assembly \b Assumptions
 - \a length is a multiple of 2 and no less than 2
 - \a x is aligned by 8
 - \a y is aligned by 8
 - \a cor is aligned by 8

\b Cycle-Count
 - (length - ceil(nc/2) + 4)*ceil(nc/2) + 7  

\b Notes
 - 32-bit accumulator used for summation.

*/

void cxCorr( CWord2x16 *x, CWord2x16 *y, int length, int nc, int QShift, CWord2x16 *cor); 

