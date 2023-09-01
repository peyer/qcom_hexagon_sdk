/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/* ======================================================================== */
/*  FUNCTION:   void cov_3x3                                                */
/*  DESCRIPTION:                                                            */
/*      Performs convolution with 3x3 masks                                 */
/*                                                                          */
/* ======================================================================== */

void conv3x3( unsigned char *img,
              signed char   *mask,
              int           width,
              int           shift, 
              unsigned char *outptr,
              int           nrows           )
{
     int sum;
     int      i, j;

     for (j = 0; j < (nrows*width) ; j++)
     {
         sum = 0;

         for (i = 0; i < 3; i++)
         {
             sum += (short)img[i + 0*width] * (short)mask[i + 0] + 
                    (short)img[i + 1*width] * (short)mask[i + 3] + 
                    (short)img[i + 2*width] * (short)mask[i + 6];
         }

         img++;

         sum = (sum >> shift);

         sum = (sum < 0)? 0 : (sum > 255)? 255 : sum;

         *outptr++ = (unsigned char)sum;
     }
}

