/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file bilateral.h
    \brief Brief description of file
*/

/*!
Performs 9x9 bilateral filtering on an image

\param input pointer to input buffer
\param stride stride of input image
\param width  width of the image block to be processed
\param height height of the image block to be processed
\param gaussLUT pointer to gaussian LUT
\param rangeLUT pointer to range LUT
\param output pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a input must be aligned by HVX vector size
 - \a output must be aligned by HVX vector size
 - \a rangeLUT must be aligned by HVX vector size
 - \a strides must be a multiple of HVX vector size
 - \a gaussLUT are symmetric

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/


void bilateral9x9(
    unsigned char   *input,
    int              stride,
    int              width,
    int              height,
    unsigned char   *gaussLUT,
    unsigned char   *rangeLUT,
    unsigned char   *output
    );

