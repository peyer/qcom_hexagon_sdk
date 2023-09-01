/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file scale.h
    \brief Brief description of file
*/

/*! 
Downsample the image to half in both horizontal and vertical directions. 

\param imgSrc pointer to input image buffer
\param width number of pixels per line
\param height number of lines
\param imgDst pointer to output image buffer

\details

\b Assembly \b Assumptions
 - \a width and height are multiples of 2
 - \a input image array is aligned by 2-bytes

\b Cycle-Count

 - (height/2)*(4 + 2*ceil(width/4)) + 8 (ideal)

*/

int scale_DownSampleToHalf( const unsigned  char * __restrict imgSrc,
                            int width,
                            int height,
                            unsigned  char * __restrict imgDst);



/*! 
Downsample the image to quarter in both horizontal and vertical directions. 

\param imgSrc pointer to input image buffer
\param width number of pixels per line
\param height number of lines
\param imgDst pointer to output image buffer

\details

\b Assembly \b Assumptions
 - \a width and height are multiples of 4
 - \a input image array is aligned by 4-bytes

\b Cycle-Count

 - (height/4)*(5 + 5*ceil(width/16)) + 11 (ideal)


*/

int scale_DownSampleToQuarter( const unsigned  char * __restrict imgSrc,
                               int width,int height,
                               unsigned  char * __restrict imgDst );


