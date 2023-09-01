/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include "q6basic_op.h"


Word32 bitrev( Word32 x, Word32 BITS ) {
   int i;
   Word32 y = 0;
   for ( i= 0; i<BITS; i++) {
      y = (y << 1)| (x & 1);
      x >>= 1;
   }
   return y;
}


CWord2x32 L_cmult32x16( CWord2x32 x, CWord2x16 y)
{
   Word32 xRe, xIm; 
   Word16 yRe, yIm;
   Word32 zRe, zIm;

   xRe = L_real(x);  xIm = L_imag(x);
   yRe = real(y);    yIm = imag(y);
   
   // the complex MPY is defined in such a way in order
   // to facilitate faster ASM implementation on Q6
   zRe = L_mult32x16r( xRe, yRe );
   zIm = L_mult32x16r( xIm, yRe );

   zRe = L_add( zRe, L_mult32x16r( L_negate(xIm), yIm ) );

   // to match Q6 definition of 32x16 MAC
   if ((xRe == -0x80000000) && (yIm == -0x8000))
       zIm = L_sat((Word64)zIm + 0x080000000LL);
   else 
       zIm = L_add( zIm, L_mult32x16r( xRe, yIm ) );

   return (L_complex(zRe, zIm));
}



void Radix2BTFLYsc32 ( CWord2x32 *x ) {

   CWord2x32 a, b;

   a = L_Vadd(x[0], x[1]);
   b = L_Vsub(x[0], x[1]);

   x[0] = a;
   x[1] = b;
   return;
}


void Radix4BTFLYqv3sc32 ( CWord2x32 *x ) {

   CWord2x32 a,b,c,d;

   a = L_Vadd(x[0],x[1]);
   b = L_Vsub(x[0],x[1]);
   c = L_Vadd(x[2],x[3]);
   d = L_Vsub(x[2],x[3]);

   // j*b
   b = L_complex(L_negate(L_imag(b)), L_real(b));

   x[0] = L_Vadd(a,c);
   x[1] = L_Vadd(b,d);
   x[2] = L_Vsub(a,c);
   x[3] = L_Vsub(b,d);
   return;
}


void Radix4BTFLYsc32 ( CWord2x32 *x ) {

   CWord2x32 a,b,c,d;

   a = L_Vadd(x[0],x[1]);
   b = L_Vsub(x[0],x[1]);
   c = L_Vadd(x[2],x[3]);
   d = L_Vsub(x[2],x[3]);

   // j*d
   d = L_complex(L_negate(L_imag(d)), L_real(d));

   x[0] = L_Vadd(a,c);
   x[1] = L_Vsub(b,d);
   x[2] = L_Vsub(a,c);
   x[3] = L_Vadd(b,d);
   return;
}
