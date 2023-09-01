/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file sobel.h
    \brief Brief description of file
*/

/*! 
Performs 3x3 soble filtering on one frame of image.

\param in pointer to input buffer
\param width number of pixels per line
\param height number of lines
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is a multiple of 4
 - \a input and output arrays are aligned by 4-bytes

\b Cycle-Count

 - 6*(width/4)*(height-2) + 15

\b Notes

 - The boundary cases are DO-NOT-CARE.

*/

void sobel( UWord8 *in, int width, int height, UWord8 *out);

