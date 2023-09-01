/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 

// with convergent rounding
CWord2x16  cavg(CWord2x16 x, CWord2x16 y);
CWord2x16 cnavg(CWord2x16 x, CWord2x16 y);

Word32 bitrev( Word32 x, Word32 BITS );
void sRadix2BTFLY( CWord2x16 *x );
void sRadix4BTFLY( CWord2x16 *x );
void sRadix4BTFLYqv3( CWord2x16 *x );

