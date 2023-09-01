/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file histogram.h
    \brief Brief description of file
*/

/*!
Caculate histogram of an image

\param src pointer to input buffer
\param stride stride of input image
\param width  width of the image block to be processed
\param height height of the image block to be processed
\param hist pointer to histogram buffer

\details

\b Assembly \b Assumptions
 - \a src must be aligned by HVX vector size (can be removed)
 - \a stride is a multiple of HVX vector size (can be removed)
 - \a hist must be aligned by HVX vector size


\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/


void histogram(
    unsigned char   *src,
    int              stride,
    int              width,
    int              height,
    int             *hist
    );
