/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file fast9.h
    \brief Brief description of file
*/

/*!
Performs an FAST feature detection.

\param img pointer to input image
\param stride stride of image
\param xsize width of the image block to be search
\param ysize height of the image block to be search
\param barrier threshold
\param border number of pixels to be excluded at boundary
\param xy  pointer to output arry of feature positions
\param maxnumcorners maximum feature points to be collected
\param numcorners number of feature points detected

\details

\b Assembly \b Assumptions
 - \a img must be aligned by HVX vector size
 - \a strides must be a multiple of HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/

void fast9(
    const unsigned char *img,
    unsigned int         stride,
    unsigned int         xsize,
    unsigned int         ysize,
    unsigned int         barrier,
    unsigned int         border,
    short int           *xy,
    int                  maxnumcorners,
    int                 *numcorners
    );
