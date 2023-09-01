/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file gammacorrect.h
    \brief Brief description of file
*/

/*! 
Performs gamma correction to pre-adjusting the brightness of the image. 

\param src pointer to image buffer
\param width number of pixels per line
\param height number of lines
\param table pointer to lookup table 
\param mode defines the mode


\details

\b Assembly \b Assumptions
 - \a width*height is even

\b Cycle-Count
 - 1.5* width *height +  8 (mode 0)
 - 2.0* width *height + 11 (mode 1)

\b Notes
 - multiple-segment LUT is used in mode 1

*/

void gamma_correct( UWord16 *ch, int width, int height, UWord16 *table, int mode);

