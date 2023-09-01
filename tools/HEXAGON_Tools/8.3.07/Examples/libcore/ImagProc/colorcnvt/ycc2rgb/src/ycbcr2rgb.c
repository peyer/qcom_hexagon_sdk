/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:02 CST 2008 
****************************************************************************/ 


/*===========================================================================*
 *  Convert YCbCr to RGB color space as                                      *
 *                                                                           *
 *  R = Y                    + 1.40200*(Cr-128)                              *
 *  G = Y - 0.34414*(Cb-128) - 0.71414*(Cr-128)                              *
 *  B = Y + 1.77200*(CB-128)                                                 *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  Implementation:                                                          *
 *    - input data are stored in blocks per MCU                              *
 *    - compute one row, and example for computing multiple rows is          *
 *      as follows                                                           *
 *                                                                           *
 *     // stride = imageXsize * OUTPUT_PER_PIXEL                             *
 *     case JPGD_YH2V2:                                                      *
 *       { uint8 *csrc = input + 64*4;                                       *
 *         for ( row = 0; row < num_rows; row+=2) {                          *
 *            H2V2Convert(src, csrc, imageXsize, dst);                       *
 *            src  += 16;                                                    *
 *            if (row == 6)                                                  *
 *              src += 64;                                                   *
 *            csrc += 8;                                                     *
 *            dst += 2*stride;                                               *
 *         }                                                                 *
 *        }                                                                  *
 *        break;                                                             *
 *                                                                           *
 *      case JPGD_YH2V1:                                                     *
 *       for ( row = 0; row < num_rows; row++) {                             *
 *             H2V1Convert(src, imageXsize, dst);                            *
 *             src += 8;                                                     *
 *             dst += stride;                                                *
 *       }                                                                   *
 *       break;                                                              *
 *                                                                           *
 *     case JPGD_YH1V2:                                                      *
 *       { uint8 *csrc = input + 64*2;                                       *
 *         for ( row = 0; row < num_rows; row+=2 ) {                         *
 *             H1V2Convert(src, csrc, imageXsize, dst);                      *
 *             src += 16;                                                    *
 *             csrc += 8;                                                    *
 *             dst += 2*stride;                                              *
 *         }                                                                 *
 *       }                                                                   *
 *       break;                                                              *
 *                                                                           *
 *    case JPGD_YH1V1:                                                       *
 *       for ( row = 0; row < num_rows; row++) {                             *
 *             H1V1Convert(src, imageXsize, dst);                            *
 *             src += 8;                                                     *
 *             dst += stride;                                                *
 *       }                                                                   *
 *       break;                                                              *
 *                                                                           *
 *===========================================================================*/
#include <stdio.h>
#include "typedef.h"

#define OUTPUT_PER_PIXEL   3
#define  CR2R   22970                    // (0x59ba) FIX(1.40200/2)
#define  CR2G   -11700                   // (0xd24c) FIX(-0.71414/2)
#define  CB2G   -5638                    // (0xe9fa) FIX(-0.34414/2)
#define  CB2B   29032                    // (0x7168) FIX(1.77200/2)
#define ONE_HALF ((Word32)1 << 14)       // rounding factor       


//------------------------------------------------------------------------------
UWord8 clamp(Word32 i)
{
  if (i & 0xFFFFFF00)
    i = (((~i) >> 31) & 0xFF);

  return ((UWord8) i);
}


//------------------------------------------------------------------------------
// YCbCr H1V1 (1x1:1:1, 3 blocks per MCU) to 24-bit RGB

void H1V1Convert(UWord8 *src, int width, UWord8 *dst)
{
  Word32 y, cb, cr;
  int i, j, k; 

  for (i = width; i > 0; i -=8) {
  
    k = (i > 8) ? 8 : i;

    for (j = 0; j < k; j++) {
     
      y  = src[    j];
      cb = src[ 64+j];
      cr = src[128+j];

      cb = 2* cb - 255;
      cr = 2* cr - 255;

      *dst++ = clamp( y + ((            CR2R * cr + ONE_HALF) >> 15) );
      *dst++ = clamp( y + ((CB2G * cb + CR2G * cr + ONE_HALF) >> 15) );
      *dst++ = clamp( y + ((CB2B * cb             + ONE_HALF) >> 15) );
    }
    src += 64*3;
  }
}



//------------------------------------------------------------------------------
// YCbCr H2V2 (2x2:1:1, 6 blocks per MCU) to 24-bit RGB
// This case is very popular, so it's important that it's fast.

void H2V2Convert(UWord8 *ysrc, UWord8 *csrc, int width, UWord8 *dst)
{
  Word32 cb, cr, rc, gc, bc; 
  Word32 y00,y01,y10,y11; 
  UWord8 *dst0, *dst1;
  int i, j, k;

  dst0 = dst;
  dst1 = dst + width*OUTPUT_PER_PIXEL;

  for (i = width; i > 0; i -= 16) {

      k = (i > 16) ? 16 : i;
 
      for (j = 0; j < k; j += 2) {

        cb = csrc[ 0]; 
        cr = csrc[64]; 
        cb = 2* cb - 255;
        cr = 2* cr - 255;
        rc = (            CR2R * cr + ONE_HALF) >> 15 ;
        gc = (CB2G * cb + CR2G * cr + ONE_HALF) >> 15 ;
        bc = (CB2B * cb             + ONE_HALF) >> 15 ;

        y00 = ysrc[0];
        *dst0++ = clamp(y00 + rc);
        *dst0++ = clamp(y00 + gc);
        *dst0++ = clamp(y00 + bc);

        y01 = ysrc[1];
        *dst0++ = clamp(y01 + rc);
        *dst0++ = clamp(y01 + gc);
        *dst0++ = clamp(y01 + bc);

        y10 = ysrc[8];
        *dst1++ = clamp(y10 + rc);
        *dst1++ = clamp(y10 + gc);
        *dst1++ = clamp(y10 + bc);

        y11 = ysrc[8+1];
        *dst1++ = clamp(y11 + rc);
        *dst1++ = clamp(y11 + gc);
        *dst1++ = clamp(y11 + bc);

        ysrc += 2;
        csrc++;
        if (j == 6) ysrc += 64 -8; 
      }
 
      ysrc += 64*6 - (64+8);
      csrc += 64*6 - 8;
  }

}






//------------------------------------------------------------------------------
// YCbCr H2V1 (2x1:1:1, 4 blocks per MCU) to 24-bit RGB

void H2V1Convert(UWord8 *ysrc, int width, UWord8 *dst)
{
  Word32 cb, cr, rc, gc, bc, y0, y1;
  UWord8 *csrc = ysrc + 2*64 ;
  int i, j, k;

  for (i= width; i > 0; i -=16) {

    k = (i > 16) ? 16 : i;
    for (j = 0; j < k; j += 2) {
      
        cb = csrc[ 0]; 
        cr = csrc[64]; 
        cb = 2* cb - 255;
        cr = 2* cr - 255;

        rc = (            CR2R * cr + ONE_HALF) >> 15 ;
        gc = (CB2G * cb + CR2G * cr + ONE_HALF) >> 15 ;
        bc = (CB2B * cb             + ONE_HALF) >> 15 ;

        y0 = ysrc[0];
        *dst++ = clamp(y0 + rc);
        *dst++ = clamp(y0 + gc);
        *dst++ = clamp(y0 + bc);

        y1 = ysrc[1];
        *dst++= clamp(y1 + rc);
        *dst++= clamp(y1 + gc);
        *dst++= clamp(y1 + bc);

        ysrc += 2;
        csrc++;
	if (j == 6) ysrc += 64-8;
    }

    ysrc += 64*4 - (64+8);
    csrc += 64*4 - 8;
  }

}





//------------------------------------------------------------------------------
// YCbCr H1V2 (1x2:1:1, 4 blocks per MCU) to 24-bit RGB

void H1V2Convert(UWord8 *ysrc, UWord8 *csrc, int width, UWord8 *dst)
{
  Word32 cb, cr, rc, gc, bc, y0, y1;
  UWord8 *dst0, *dst1;
  int i, j, k;

  dst0 = dst;
  dst1 = dst + width*OUTPUT_PER_PIXEL;

  for (i = width; i > 0; i -=8) {
   
    k = (i > 8) ? 8 : i;
    for (j = 0; j < k; j++) {
    
      cb = csrc[ 0+j];
      cr = csrc[64+j];

      cb = 2* cb - 255;
      cr = 2* cr - 255;

      rc = (            CR2R * cr + ONE_HALF) >> 15 ;
      gc = (CB2G * cb + CR2G * cr + ONE_HALF) >> 15 ;
      bc = (CB2B * cb             + ONE_HALF) >> 15 ;

      y0 = ysrc[j];
      *dst0++ = clamp(y0 + rc);
      *dst0++ = clamp(y0 + gc);
      *dst0++ = clamp(y0 + bc);

      y1 = ysrc[8+j];
      *dst1++ = clamp(y1 + rc);
      *dst1++ = clamp(y1 + gc);
      *dst1++ = clamp(y1 + bc);
    }

    ysrc += 64*4;
    csrc += 64*4;
  }
}

