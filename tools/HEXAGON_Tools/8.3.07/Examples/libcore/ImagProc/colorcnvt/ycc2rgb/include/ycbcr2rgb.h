/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file ycbcr2rgb.h
    \brief Brief description of file
 */

/*! 
Convert one line of an image from YCbCr (H1V1) to 24-bit RGB. 

\param src pointer to input buffer of (Minimum Coded Unit) MCUs
\param width number of pixels per line
\param dst pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is an even number
 - \a src is aligned by 2-bytes
 - \a dst is aligned by 2-bytes

\b Cycle-Count

 - 6*(width/2) + 10 (~ 3 cycles/pixel)

\b Notes

None.

*/

void H1V1Convert(UWord8 *src, int width, UWord8 *dst);



/*! 
Convert one line of an image from YCbCr (H1V2) to 24-bit RGB. 

\param ysrc pointer to Y in input buffer of MCUs
\param csrc pointer to CbCr in input buffer of MCUs
\param width number of pixels per line
\param dst pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is an even number
 - \a ysrc is aligned by 2-bytes
 - \a dst is aligned by 2-bytes

\b Cycle-Count

 - 8*(width/2) + 2*ceil(width/8) + 14 

\b Notes

None.

*/

void H1V2Convert(UWord8 *ysrc, UWord8 *csrc, int width, UWord8 *dst);



/*! 
Convert one line of an image from YCbCr (H1V1) to 24-bit RGB. 

\param src pointer to input buffer of MCUs
\param width number of pixels per line
\param dst pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is a multiple of 4
 - \a src is aligned by 4-bytes
 - \a dst is aligned by 4-bytes

\b Cycle-Count

 - 9*(width/4) + 11 

\b Notes

None.

*/

void H2V1Convert(UWord8 *src, int width, UWord8 *dst);


/*! 
Convert one line of an image from YCbCr (H2V2) to 24-bit RGB. 

\param ysrc pointer to Y in input buffer of MCUs
\param csrc pointer to CbCr in input buffer of MCUs
\param width number of pixels per line
\param dst pointer to output buffer

\details

\b Assembly \b Assumptions
 - \a width is a multiple of 4
 - \a ysrc is aligned by 4-bytes
 - \a dst is aligned by 4-bytes

\b Cycle-Count

 - 14*(width/4) + 13 

\b Notes

None.

*/

void H2V2Convert(UWord8 *ysrc, UWord8 *csrc, int width, UWord8 *dst);


