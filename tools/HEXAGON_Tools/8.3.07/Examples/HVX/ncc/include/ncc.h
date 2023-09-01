/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file ncc.h
    \brief Brief description of file
*/

/*!
Performs 8x8 template matching in 18x18 windows

\param ptch8x8 pointer to input 8x8 template
\param img pointer to input image
\param imgW the width of image
\param imgH the height of image
\param srchX the x coordinate of image for search window
\param srchY the y coordinate of image for search window
\param filterLowVariance the threshold to skip ncc value computation
\param bestX the best match of x coordinate in the image
\param bestY the best match of y coordinate in the image
\param bestNCC the value of ncc
\param doSubPixel the flag to do subpixel (not support)
\param subX the subpixel x coord (not support)
\param subY the subpixel y coord (not support)
\param scratch the scratch buffer of 1k + VLEN and VLEN byte aligned

\details

\b Assembly \b Assumptions
 - \a ptch8x8 is NOT aligned by HVX vector size
 - \a img is NOT aligned by HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/


int
fcvNCCPatchOnSquare8x8u8(
    const unsigned char* __restrict ptch8x8,
    const unsigned char* __restrict img,
    unsigned short            imgW,
    unsigned short            imgH,
    unsigned short            srchX,
    unsigned short            srchY,
    unsigned short            srchW,
    int                       filterLowVariance,
    unsigned short*           bestX,
    unsigned short*           bestY,
    unsigned int*             bestNCC,
    int                       doSubPixel,
    float*                    subX,
    float*                    subY,
    unsigned char            *scratch
);
