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
/*[     CornerHarrisu8                                                     ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs Harris corner detection.                    ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     JAN-14-2015                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <stdio.h>
#include <stdlib.h>
/* ======================================================================== */
/*  Reference C version of CornerHarrisu8()                                 */
/* ======================================================================== */
static void blurRow5x5( 
    const short *src, 
    short       *dstImg, 
    unsigned int width, 
    int idx, int flush 
    )
{
    //----------------------------------------------------------
    // idx: 0 --> gradx, idx: 1 --> grady, idx: 2 --> gradxy
    //   x: row location of the pixel
    //----------------------------------------------------------
    int col0, col1, col2, col3, col4;
    int W0 = (idx + 3 * (flush % 5 ) ) * width + 1;
    int W1 = (idx + 3 * (( flush + 1 ) % 5 )) * width + 1;
    int W2 = (idx + 3 * (( flush + 2 ) % 5 )) * width + 1;
    int W3 = (idx + 3 * (( flush + 3 ) % 5 )) * width + 1;
    int W4 = (idx + 3 * (( flush + 4 ) % 5 )) * width + 1;
    short* dst    = dstImg + idx * width + 3;
    short* dstEnd = dstImg + idx * width + width - 3;

    col0 = (int)(src[W0+0]       + (src[W1+0] << 2) + (src[W2+0] << 2) +
                (src[W2+0] << 1) + (src[W3+0] << 2) + src[W4+0]          );
    col1 = (int)(src[W0+1]       + (src[W1+1] << 2) + (src[W2+1] << 2) +
                (src[W2+1] << 1) + (src[W3+1] << 2) + src[W4+1]          );
    col2 = (int)(src[W0+2]       + (src[W1+2] << 2) + (src[W2+2] << 2) +
                (src[W2+2] << 1) + (src[W3+2] << 2) + src[W4+2]          );
    col3 = (int)(src[W0+3]       + (src[W1+3] << 2) + (src[W2+3] << 2) +
                (src[W2+3] << 1) + (src[W3+3] << 2) + src[W4+3]          );
    src += 4;

    // Do 5 pixels per loop
    //
    while( dst < (dstEnd - 5) )
    {
        col4 = (int)(src[W0+0]       + (src[W1+0] << 2) + (src[W2+0] << 2) +
                    (src[W2+0] << 1) + (src[W3+0] << 2) + src[W4+0]          );

        *dst++ = (short)((col0       + (col1 << 2) + (col2 << 2) + (col2 << 1) +
                         (col3 << 2) + col4) >> 8);

        col0 = (int)(src[W0+1]       + (src[W1+1] << 2) + (src[W2+1] << 2) +
                    (src[W2+1] << 1) + (src[W3+1] << 2) + src[W4+1]          );

        *dst++ = (short)((col1       + (col2 << 2) + (col3 << 2) + (col3 << 1) +
                         (col4 << 2) + col0) >> 8);

        col1 = (int)(src[W0+2]       + (src[W1+2] << 2) + (src[W2+2] << 2) +
                    (src[W2+2] << 1) + (src[W3+2] << 2) + src[W4+2]          );

        *dst++ = (short)((col2       + (col3 << 2) + (col4 << 2) + (col4 << 1) +
                         (col0 << 2) + col1) >> 8);

        col2 = (int)(src[W0+3]       + (src[W1+3] << 2) + (src[W2+3] << 2) +
                    (src[W2+3] << 1) + (src[W3+3] << 2) + src[W4+3]          );

        *dst++ = (short)((col3       + (col4 << 2) + (col0 << 2) + (col0 << 1) +
                         (col1 << 2) + col2) >> 8 );

        col3 = (int)(src[W0+4]       + (src[W1+4] << 2) + (src[W2+4] << 2) +
                    (src[W2+4] << 1) + (src[W3+4] << 2) + src[W4+4]          );

        *dst++ = (short)((col4       + (col0 << 2) + (col1 << 2) + (col1 << 1) +
                         (col2 << 2) + col3) >> 8);

        src +=  5; 
    }

    // Do the remaining pixels (less than 5)
    //
    while( dst < dstEnd )
    {
        col4 = (int)(src[W0]       + (src[W1] << 2) + (src[W2] << 2) +
                    (src[W2] << 1) + (src[W3] << 2) + src[W4]          );

        *dst++ = (short)((col0       + (col1 << 2) + (col2 << 2) +
                         (col2 << 1) + (col3 << 2) + col4) >> 8);

        src++;
        col0 = col1;
        col1 = col2;
        col2 = col3;
        col3 = col4;
    }
}


/* ======================================================================== */
void CornerHarrisu8( 
    const unsigned char* __restrict srcImg,
    unsigned int             width,
    unsigned int             height,
    unsigned int             stride,
    unsigned int             border,
    unsigned int* __restrict xy,
    unsigned int             maxnumcorners,
    unsigned int* __restrict numcorners,
    int                      threshold 
    )
{
   *numcorners = 0;
   int flushcorner = 0;
   int flushgrads = 0;
   int responserow = 0;
   int allocsize = 5 * width; // 5 rows of corner response
   int   *corner;
   short *grads; 
   short *blurgrads;

   corner    = (int   *)malloc(allocsize*sizeof(int));        // memory for corner response
   grads     = (short *)malloc(3*allocsize*sizeof(short));    // grads will have rows of gxx, gyy, gxy
   blurgrads = (short *)malloc(3*width*sizeof(short));        // blurgrads will have rows of blurred gxx, gyy, gxy

   // # of boundary pixels should be >= 5
   //unsigned int boundary = border > 5 ? border : 5;
   unsigned int boundary = 5;

   // # of pixels for which to check the corner features
   unsigned int yboundary = height - boundary;
   unsigned int xboundary = width - boundary;
   unsigned int x, y;

   // Calculate 5 rows of gradient to begin with
   for( y = 1; y < 6; ++y )
   { 
      for( x = 1; x < width - 1; ++x )
      {
         int idx = y * width + x;
         short ix = (short) ( srcImg[idx + 1] - srcImg[idx - 1] ) >> 1; 
         short iy = (short) ( srcImg[idx + width] - srcImg[idx - width] ) >> 1;
         grads[3 * (y-1) * width + x] = ix * ix;
         grads[3 * (y-1) * width + width + x] = iy * iy;
         grads[3 * (y-1) * width + 2 * width + x] = ix * iy;
      }
   }

   //Setting y to 5 as the 1st 5 rows have been read
   y = 5;

   // Loop until y reaches it's boundary
   while( y < (height - 1) )
   { 
      // Blurring Ixx, Iyy, Ixy
      blurRow5x5( grads, blurgrads, width, 0, flushgrads );
      blurRow5x5( grads, blurgrads, width, 1, flushgrads );
      blurRow5x5( grads, blurgrads, width, 2, flushgrads );

      // calculate corner response
      for( x = 3; x < width - 3; ++x )
      {
         // Det(M) = Ixx * Iyy - (Ixy)^2
         int det = (int) ( blurgrads[x] * blurgrads[x + width] - 
                           blurgrads[x + 2*width] * blurgrads[x + 2*width] );

         // Trace(M) = Ixx + Iyy
         int trace = (int) ( blurgrads[x] + blurgrads[x + width] );

         // Corner response
         int rowloc = (responserow + flushcorner) % 5;
         corner[ rowloc * width + x] = det - ( (trace * trace) >> 4 );
      }

      responserow++;

      // if we have a filled 5x5 block, output the corners
      if( responserow == 5 )
      {

         // Do the non-max supression and output the Harris corner
         if( ( (y-4) >= boundary ) && ( (y-4) < yboundary ) )
         {
            for( x = boundary; x < xboundary; ++x )
            {
               const int* s0 = &corner[( flushcorner % 5) * width];
               const int* s1 = &corner[( (flushcorner + 1) % 5) * width];
               const int* s2 = &corner[( (flushcorner + 2) % 5) * width];
               const int* s3 = &corner[( (flushcorner + 3) % 5) * width];
               const int* s4 = &corner[( (flushcorner + 4) % 5) * width];
               int val = s2[x];

               // Check a 5x5 nbhd around the pixel and check if its a local max
               if( val > threshold && 
                   ( val > s0[x-2] ) && ( val > s0[x-1] ) && ( val > s0[x] ) && 
                   ( val > s0[x+1] ) && ( val > s0[x+2] ) &&
                   ( val > s1[x-2] ) && ( val > s1[x-1] ) && ( val > s1[x] ) && 
                   ( val > s1[x+1] ) && ( val > s1[x+2] ) &&
                   ( val > s2[x-2] ) && ( val > s2[x-1] ) &&  
                   ( val > s2[x+1] ) && ( val > s2[x+2] ) &&
                   ( val > s3[x-2] ) && ( val > s3[x-1] ) && ( val > s3[x] ) && 
                   ( val > s3[x+1] ) && ( val > s3[x+2] ) && 
                   ( val > s4[x-2] ) && ( val > s4[x-1] ) && ( val > s4[x] ) && 
                   ( val > s4[x+1] ) && ( val > s4[x+2]) )
               {
                  // coords will be (x, y-4) as 5 rows have been read before corner detection
                  *(xy++) = x;
                  *(xy++) = y - 4;
                  ++(*numcorners);
               }

               if( *numcorners >= maxnumcorners )
               {
                  break;
               }
            }
         }

         // move 'corner' memory by 1 row by masking ptrs
         flushcorner++;
         responserow--;
      }

      // Flush out the bottom row of gxx, gyy and gyy by masking ptrs
      // and increment the row of the original image
      flushgrads++;
      y++;
      if ( y >= (height - 1) ) break;

      // Read in a new row of gradients
      for( x = 1; x < width - 1; ++x )
      {
         int idx = y * width + x;
         short ix = (short) ( srcImg[idx + 1] - srcImg[idx - 1] ) >> 1; 
         short iy = (short) ( srcImg[idx + width] - srcImg[idx - width] ) >> 1;
         int lastrow = 3 * ( ( flushgrads + 4 ) % 5 ) * width;
         grads[lastrow + x] = ix*ix;
         grads[lastrow + width + x] = iy*iy;
         grads[lastrow + 2*width + x] = ix*iy;
      }
   }

   free(blurgrads);
   free(grads);
   free(corner);
}

