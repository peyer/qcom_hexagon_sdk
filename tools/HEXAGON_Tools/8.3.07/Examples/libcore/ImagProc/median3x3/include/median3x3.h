/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file median3x3.h 
    \brief Brief description of file
*/

/*! 
Performs a 3x3 median filter on a frame of image 

\param xin pointer to input buffer
\param width number of pixels per line
\param height number of lines
\param yout pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is a multiple of 8 
 - \a xin and yout array must be aligned by 8-bytes

\b Cycle-Count

 - 11*(width/8)*(H-2) + 14  

\b Notes

 - In C reference, the boundary cases are not defined.

*/

void median3x3(UWord8 *xin, int width, int height, UWord8 *yout);

