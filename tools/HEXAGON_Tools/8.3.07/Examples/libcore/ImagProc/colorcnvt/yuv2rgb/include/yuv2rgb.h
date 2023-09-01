/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file yuv2rgb.h
    \brief Brief description of file
*/

/*! 
Convert an image from YUV 4:2:2 to 24-bit RGB. 

\param in pointer to input buffer
\param width number of pixels per line
\param height number of lines
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is even number
 - \a out is aligned by 2-bytes

\b Cycle-Count

 - ceil(height/2) * ( 9*(width/2) + 3*(width/10) + 3 ) + 9

\b Notes

 - Implementation is based on Eric Plondke's approximation method
   with 256-byte LUT.

*/

void YUVConvert( UWord8 *in, int width, int height, UWord8 *out);

