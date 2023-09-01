/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file integrate.h
    \brief Brief description of file
*/

/*!
Performs two-dimensional integration on an image block

\param src pointer to image buffer
\param srcStride stride of the image
\param srcWidth  width of the image block to be processed
\param srcHeight height of the image block to be processed
\param dst pointer to output buffer of integrations
\param dstStride stride of output

\details

\b Assembly \b Assumptions
 - \a src and dst must be aligned by HVX vector size
 - \a strides must be a multiple of HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/


void IntegrateImage(
    unsigned char   *src,
    int              srcStride,
    int              srcWidth,
    int              srcHeight,
    unsigned int    *dst,
    int              dstStride
    );
