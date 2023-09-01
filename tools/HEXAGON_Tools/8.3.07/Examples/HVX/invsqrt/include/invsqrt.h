/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file invsqrt.h
    \brief Brief description of file
*/

/*!
Computes 1 / squareroot(x) using interpolation.

\param input pointer to input buffer
\param sqrt_recip_shft pointer to output buffer
\param sqrt_recip_val pointer to input buffer
\param width number of the entries to be processed

\details

\b Assembly \b Assumptions
 - \a input must be aligned by HVX vector size
 - \a sqrt_recip_shft must be aligned by HVX vector size
 - \a sqrt_recip_val must be aligned by HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/

void invsqrt(
    unsigned short *input,
    unsigned short *sqrt_recip_shft,
    unsigned short *sqrt_recip_val,
    unsigned        width
    );

