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

CWord2x16 cavg( CWord2x16 x, CWord2x16 y ) {
   Word32 zr, zi;
   zr = (Word32)real(x) + (Word32)real(y);
   zi = (Word32)imag(x) + (Word32)imag(y);

#ifdef TWOS_COMPLEMENT_ROUNDING
   zr += 1;
   zi += 1;
#else
   zr +=  (zr>>1) & 1; 
   zi +=  (zi>>1) & 1;
#endif
   zr >>= 1;
   zi >>= 1;

   return( complex((Word16)zr, (Word16)zi) );
}
 

CWord2x16 cnavg( CWord2x16 x, CWord2x16 y ) {
   Word32 zr, zi;
   zr = (Word32)real(x) - (Word32)real(y);
   zi = (Word32)imag(x) - (Word32)imag(y);
#ifdef TWOS_COMPLEMENT_ROUNDING
   zr += 1;
   zi += 1;
#else
   zr +=  (zr>>1) & 1;
   zi +=  (zi>>1) & 1;
#endif
   zr >>= 1;
   zi >>= 1;

   return( complex(saturate(zr), saturate(zi)) );
}
 


 
/***************************/
/*  Scaled version         */
/***************************/
void sRadix2BTFLY( CWord2x16 *x ) {
   CWord2x16 a,b;
   a =  cavg(x[0],x[1]);
   b = cnavg(x[0],x[1]);
   
   x[0] = a;
   x[1] = b;
   return;
}

void sRadix4BTFLYqv3( CWord2x16 *x ) {
   CWord2x16 a,b,c,d;

   a =  cavg(x[0],x[1]);
   b = cnavg(x[0],x[1]);
   c =  cavg(x[2],x[3]);
   d = cnavg(x[2],x[3]);
   
   // j*b
   b = complex(negate(imag(b)), real(b));

   x[0] =  cavg(a,c);
   x[1] =  cavg(b,d);
   x[2] = cnavg(a,c);
   x[3] = cnavg(b,d);
   return;
}


void sRadix4BTFLY( CWord2x16 *x ) {
   CWord2x16 a,b,c,d;

   a =  cavg(x[0],x[1]);
   b = cnavg(x[0],x[1]);
   c =  cavg(x[2],x[3]);
   d = cnavg(x[2],x[3]);
   
   // j*d
   d = complex(negate(imag(d)), real(d));

   x[0] =  cavg(a,c);
   x[1] = cnavg(b,d);
   x[2] = cnavg(a,c);
   x[3] =  cavg(b,d);
   return;
}

/***************************/
/*  Non-scaled version     */
/***************************/
void Radix2BTFLY( CWord2x16 *x ) {
   CWord2x16 a,b;
   a = cadd(x[0],x[1]);
   b = csub(x[0],x[1]);
   
   x[0] = a;
   x[1] = b;
   return;
}

void Radix4BTFLYqv3( CWord2x16 *x ) {
   CWord2x16 a,b,c,d;

   a = cadd(x[0],x[1]);
   b = csub(x[0],x[1]);
   c = cadd(x[2],x[3]);
   d = csub(x[2],x[3]);

   // j*b   
   b = complex(negate(imag(b)), real(b));

   x[0] = cadd(a,c);
   x[1] = cadd(b,d);
   x[2] = csub(a,c);
   x[3] = csub(b,d);
   return;
}


void Radix4BTFLY( CWord2x16 *x ) {
   CWord2x16 a,b,c,d;

   a = cadd(x[0],x[1]);
   b = csub(x[0],x[1]);
   c = cadd(x[2],x[3]);
   d = csub(x[2],x[3]);
   
   // j*d
   d = complex(negate(imag(d)), real(d));

   x[0] = cadd(a,c);
   x[1] = csub(b,d);
   x[2] = csub(a,c);
   x[3] = cadd(b,d);
   return;
}


/***************************/
/*     ifft                */
/***************************/
void ifftRadix2BTFLY( CWord2x16 *x ) {
   CWord2x16 a,b;
   a =  cavg(x[0],x[1]);
   b = cnavg(x[0],x[1]);
   
   x[0] = a;
   x[1] = b;
   return;
}


void ifftRadix4BTFLYqv3( CWord2x16 *x ) {
   CWord2x16 a,b,c,d;

   a =  cavg(x[0],x[1]);
   b = cnavg(x[0],x[1]);
   c =  cavg(x[2],x[3]);
   d = cnavg(x[2],x[3]);
   
   // -j*b
   b = complex(imag(b), negate(real(b)));

   x[0] =  cavg(a,c);
   x[1] =  cavg(b,d);
   x[2] = cnavg(a,c);
   x[3] = cnavg(b,d);
   return;
}


void ifftRadix4BTFLY( CWord2x16 *x ) {
   CWord2x16 a,b,c,d;

   a =  cavg(x[0],x[1]);
   b = cnavg(x[0],x[1]);
   c =  cavg(x[2],x[3]);
   d = cnavg(x[2],x[3]);
   
   // -j*b
   d = complex(imag(d), negate(real(d)));

   x[0] =  cavg(a,c);
   x[1] = cnavg(b,d);
   x[2] = cnavg(a,c);
   x[3] =  cavg(b,d);
   return;
}

