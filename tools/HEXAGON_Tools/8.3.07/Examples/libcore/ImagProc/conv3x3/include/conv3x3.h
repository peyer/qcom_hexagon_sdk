/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/

/*! \file conv3x3.h
    \brief Brief description of file
 */

/*!
Calculate 3x3 convolution on multiple lines of image

\param inptr pointer to input array
\param mask pointer to 3x3 mask array
\param width number of pixels per line
\param shift number of bits to shift
\param outptr pointer to output array
\param nrows number of output rows to be computed

\details

\b Assembly \b Assumptions
 - \a width is an even number
 - \a input and output array are aligned by 2bytes

\b Cycle-Count

 - 4*(width*nrows/2) + 17

\b Notes

None.

*/


void conv3x3( unsigned char *inptr,
              signed char   *mask,
              int           width,
              int           shift,
              unsigned char *outptr,
              int nrows                   );

