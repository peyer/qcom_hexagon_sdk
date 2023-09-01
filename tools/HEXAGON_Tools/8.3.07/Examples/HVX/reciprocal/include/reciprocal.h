/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008
****************************************************************************/


/*! \file reciprocal.h
    \brief Brief description of file
*/

/*!
Computes 1 / x using interpolation.

\param input pointer to input buffer
\param recip_val  pointer to output buffer for values
\param recip_shft pointer to output buffer for shift amounts
\param width number of the entries to be processed

\details

\b Assembly \b Assumptions
 - \a input must be aligned by HVX vector size
 - \a recip_val must be aligned by HVX vector size
 - \a recip_shft must be aligned by HVX vector size

\b Cycle-Count
 - TO ADD

\b Notes
 - None
*/

void reciprocal(
    unsigned short *input,
    unsigned short *recip_val,
    short          *recip_shft,
    int             width
    );
