/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file mipi2raw16.h
    \brief Brief description of file
*/

/*!
Decompresses the packed 10-bit bayer to 16-bit output.

\param input pointer to input buffer
\param istride stride of input image
\param width  width of the image block to be processed
\param height height of the image block to be processed
\param output pointer to output buffer
\param ostride stride of output

\details

\b Assembly \b Assumptions
 - \a input and output must be aligned by HVX vector size
 - \a istride and ostride must be aligned by HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/


void convertMipiToRaw16(
    unsigned char  *input,
    unsigned        istride,
    unsigned        width,
    unsigned        height,
    unsigned short *output,
    unsigned        ostride
    );
