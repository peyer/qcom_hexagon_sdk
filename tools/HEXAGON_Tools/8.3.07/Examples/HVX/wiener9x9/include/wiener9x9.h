/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file weiner9x9.h
    \brief Brief description of file
*/

/*!
Performs Wiener filtering on an image

\param in pointer to input buffer
\param out pointer to input buffer
\param width  width of the image block to be processed
\param stride stride of image
\param height height of the image block to be processed
\param noise noise level used in wiener

\details

\b Assembly \b Assumptions
 - \a in must be aligned by HVX vector size
 - \a out must be aligned by HVX vector size
 - \a strides must be a multiple of HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/

#define WINDOWS 9

void filter_wiener9x9 (
    unsigned char      *out,
    unsigned char      *in,
    int                 width,
    int                 stride,
    int                 height,
    const unsigned char noise
    );
