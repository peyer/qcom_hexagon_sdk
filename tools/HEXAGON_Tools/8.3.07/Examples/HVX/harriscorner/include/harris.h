/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file harris.h
    \brief Brief description of file
*/

/*!
Performs Harris corner detection.

\param srcImg pointer to input image
\param width width of the image block to be search
\param height height of the image block to be search
\param stride stride of image
\param border number of pixels to be excluded at boundary
\param xy  pointer to output arry of corner positions
\param maxnumcorners maximum number of corners to be collected
\param numcorners number of corners detected
\param threshold threshold 

\details

\b Assembly \b Assumptions
 - \a img must be aligned by HVX vector size
 - \a strides must be a multiple of HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/

void CornerHarrisu8( 
    const unsigned char* __restrict srcImg,
    unsigned int             width,
    unsigned int             height,
    unsigned int             stride,
    unsigned int             border,
    unsigned int* __restrict xy,
    unsigned int             maxnumcorners,
    unsigned int* __restrict numcorners,
    int                      threshold 
    );

