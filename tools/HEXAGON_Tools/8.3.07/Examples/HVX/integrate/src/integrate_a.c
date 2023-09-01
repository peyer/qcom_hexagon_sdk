/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.          */
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
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void IntegratePerRow(
    unsigned char   *src,
    int              width,
    unsigned int    *dst
    );

void IntegrateVertical(
    unsigned int    *iimgPrev,
    unsigned int    *iimg,
    int              stride,
    int              width,
    int              h
    );

/* ======================================================================== */
void IntegrateImage(
    unsigned char   *src,
    int              srcStride,
    int              srcWidth,
    int              srcHeight,
    unsigned int    *dst,
    int              dstStride
    )
{
   int i, j, n, k;
   unsigned int sum;
   unsigned char *img;
   unsigned int  *iimg;

   // First row
   img  = src;
   iimg = dst;
   IntegratePerRow( img, srcWidth, iimg );

   iimg += dstStride;
   img  += srcStride;

   // remaining rows
   n = 4;

   for ( i=1; i<srcHeight; i+=n )
   {
      k = (srcHeight-i) > n ? n : (srcHeight-i);

      // split into horizontal/vertical integration in order to do MT implementation
      for (j=0; j<k; j++)
      {
        IntegratePerRow( img + j*srcStride, srcWidth, iimg + j*dstStride );
      }

      IntegrateVertical( iimg-dstStride, iimg, dstStride, srcWidth, k );

      img  += n*srcStride;
      iimg += n*dstStride;
   }
}


