/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:02 CST 2008 
****************************************************************************/ 


/*===========================================================================*
 *  Convert RGB to YCbCr color space as                                      *
 *                                                                           *
 *  Y  =  0.29900*R + 0.58700*G + 0.11400*B                                  *
 *  Cb = -0.16874*R - 0.33126*G + 0.50000*B + 128                            *
 *  Cr =  0.50000*R - 0.41869*G - 0.08131*B + 128                            *
 *                                                                           *
 *===========================================================================*/
#define	R2Y     9798
#define G2Y     19235
#define B2Y     3736
#define R2CB    -5529
#define G2CB    -10855
#define B2CB    16384
#define R2CR    16384
#define G2CR    -13720
#define B2CR    -2664
#define ONE_HALF ((int)1 << 14)       // rounding factor       



unsigned char clamp(int i)
{
  if (i & 0xFFFFFF00)
    i = (((~i) >> 31) & 0xFF);

  return ((unsigned char) i);
}



void RGB2YCbCr( unsigned char *pRGB, 
                int            width,
                int            nrows, 
                unsigned char *pYCrCb )
{
  int y, cb, cr;
  int short r, g, b;
  int i;

  for (i = 0; i < (nrows*width); i++) 
  {
      r = *pRGB++;  
      g = *pRGB++;  
      b = *pRGB++;  

      y  = R2Y *r + G2Y *g + B2Y *b;
      cb = R2CB*r + G2CB*g + B2CB*b;
      cr = R2CR*r + G2CR*g + B2CR*b;

      *pYCrCb++ = clamp( ((y  + ONE_HALF) >> 15)       );
      *pYCrCb++ = clamp( ((cr + ONE_HALF) >> 15) + 128 );
      *pYCrCb++ = clamp( ((cb + ONE_HALF) >> 15) + 128 );
  }
}



