/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:19:59 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:19:59 CST 2008
****************************************************************************/


#ifndef _TYPEDEF_QDSP6_
#define _TYPEDEF_QDSP6_

//-------------------------
// Define Basic Data Types
//-------------------------
typedef signed   char        Word8;
typedef unsigned char        UWord8;
typedef short int            Word16;
typedef unsigned short       UWord16;
typedef int                  Word32;
typedef unsigned int         UWord32;
typedef signed long long     Word64;
typedef unsigned long long   UWord64;



//-------------------------
// Define Complex Number
//-------------------------

typedef int                  CWord2x16;

// typedef struct {
//    Word16 real;
//    Word16 imag;
//} CWord2x16;

typedef long long            CWord2x32;

// typedef struct {
//typedef struct {
//    Word32 real;
//    Word32 imag;
//} CWord2x32;


//-------------------------
// Define Vectors
//-------------------------
typedef union __attribute__((packed,aligned (4))) {
  Word8      b[4];
  UWord8     ub[4];
  Word16     h[2];
  UWord16    uh[2];
  Word32     w;
  UWord32    uw;
  CWord2x16  c;
} Vect32;


typedef union __attribute__((packed,aligned (8))) {
  Word8       b[8];
  UWord8      ub[8];
  Word16      h[4];
  UWord16     uh[4];
  Word32      w[2];
  UWord32     uw[2];
  Word64      d;
  Vect32      v[2];
  CWord2x16   c[2];
} Vect64;


#endif

