/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2016-2018 QUALCOMM TECHNOLOGIES Incorporated.     */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     IntegrateImage                                                     ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function calculates a 2D integration of an image.             ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Reference C version of IntegrateImage()                                 */
/* ======================================================================== */
void integrate_ref(
    unsigned char   *src,
    int              srcStride,
    int              srcWidth,
    int              srcHeight,
    unsigned int    *dst,
    int              dstStride
    )
{
   int i, j;
   unsigned int sum;
   unsigned char *img;
   unsigned int *iimg, *iimgPrev;

   // First row
   img  = src;
   iimg = dst;

   //the parameter is in byte unit, when used on int *, should be divided by 4.
   dstStride = (dstStride / sizeof(int));
   
   sum = 0;
   for ( j=0; j<srcWidth; ++j )
   {
      sum    += img[j];
      iimg[j] = sum;
   }

   // remaining rows
   iimgPrev = iimg;
   iimg += dstStride;
   img  += srcStride;

   for ( i=1; i<srcHeight; ++i )
   {
      sum = 0;

      for ( j=0; j<srcWidth; ++j )
      {
         sum    += img[j];
         iimg[j] = sum + iimgPrev[j];
      }

      iimgPrev = iimg;
      img  += srcStride;
      iimg += dstStride;
   }
}


