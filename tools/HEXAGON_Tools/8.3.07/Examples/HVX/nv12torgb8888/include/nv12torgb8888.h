/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file nv12torgb888.h
    \brief Brief description of file
*/

/*!
Performs a conversion from NV12 to RGB8888 format

\param yuv420sp pointer to input Y buffer
\param uv420sp pointer to input UV buffer
\param out pointer to output buffer
\param height height of the image block to be processed
\param width  width of the image block to be processed
\param stride stride of image

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

void
color_NV12toRGB8888(
    unsigned char* __restrict yuv420sp,
    unsigned char* __restrict uv420sp,
    unsigned char* __restrict rgb,
    int                       height,
    int                       width,
    int                       stride
    );
