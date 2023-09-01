/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file color_correct.h
    \brief Brief description of file
*/

typedef struct 
{   Word16   c0              ; // 12-bits; coefficients for R
    Word16   c1              ; // 12-bits
    Word16   c2              ; // 12-bits
    Word16   c3              ; // 12-bits; coefficients for G
    Word16   c4              ; // 12-bits
    Word16   c5              ; // 12-bits
    Word16   c6              ; // 12-bits; coefficients for B
    Word16   c7              ; // 12-bits
    Word16   c8              ; // 12-bits
    Word16   k0              ; // 12-bits; offset for R
    Word16   k1              ; // 12-bits; offset for G
    Word16   k2              ; // 12-bits; offset for B
    UWord16 qfactor          ; // 2 bits
} ColorCorrectParams;


/*! 
Performs color correction to one frame of RGB data

\param R pointer to buffer for R component
\param G pointer to buffer for G component
\param B pointer to buffer for B component
\param width number of pixels per line
\param height number of lines
\param params pointer to structure \a ColorCorrectParams

\details

\b Assembly \b Assumptions
 - \a width*height is even 
 - \a R/G/B buffers are aligned by 4-bytes

\b Cycle-Count
 - 4*width*height + 19

\b Notes
 - None.

*/

void color_correct ( UWord16 *R, UWord16 *G, UWord16 *B, 
                     int width, int height, ColorCorrectParams *params);
