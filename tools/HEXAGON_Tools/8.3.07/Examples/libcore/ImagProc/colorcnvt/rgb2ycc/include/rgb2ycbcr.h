/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file rgb2ycbcr.h
    \brief Brief description of file
 */

/*! 
Convert RGB to YCbCr color space

\param pRGB pointer to input RGB array
\param width pitch of image
\param nrows number of RGB rows
\param pYCrCb pointer to output array

\details

\b Assembly \b Assumptions
 - \a width*nrow is an even number
 - \a input and output arrays are aligned by 2-bytes

\b Cycle-Count

 - 6*(width*nrow/2) + 13

\b Notes

None.

*/



void RGB2YCbCr( unsigned char *pRGB, 
                int            width, 
                int            nrows, 
                unsigned char *pYCrCb  );

