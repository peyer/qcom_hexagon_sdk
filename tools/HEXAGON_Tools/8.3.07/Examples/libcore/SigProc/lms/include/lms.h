/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file lms.h
    \brief Brief description of file
*/

/*! 
Implements an adaptive filter with least mean square (LMS) algorithm

\param xin input sample
\param yes desired output
\param rate step size of adaptation
\param ntap number of filter taps
\param state pointer to filter state buffer
\param coef pointer to filter coefficent buffer

\details

\b Assembly \b Assumptions
 - \a ntap is a multiple of 8
 - \a state and \a coef buffer are aligned by 8-bytes

\b Cycle-Count
 - 3*ntap/4 + 11

\b Notes
 - None.

*/

Word16 lms( Word16 xin, Word16 ydes, int rate,   \
            int ntap, Word16 *state, Word16 *coef);
