/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 

/* ================================================================== */
/*  Cyclic Redundancy Code Encoder                                    */
/* ================================================================== */
/*  Description:                                                      */
/*  ------------                                                      */
/*  This program computes a CRC remainder vector of 31-bits or less,  */
/*  i.e.                                                              */
/*   Let binary data input vector                                     */
/*        U(X)= u(0) + u(1)X^1 + u(2)X^2 + ... u(k-1)X^(k-1)          */
/*                                                                    */
/*   generator polynomial:  G(X) = g(0) + g(1)X^1 + .... g(m)X^m      */
/*         where  g(i), u(i) = 0 or 1                                 */
/*                g(0) and g(m) are always 1                          */
/*                                                                    */
/*   CRC remainder is                                                 */
/*         B(X) = X^m *U(X) mod(GX)                                   */
/*                                                                    */
/*  The encoded vector output is of the form:                         */
/*                                                                    */
/*         B(X)+ X^(n-k)U(X) = b(0) + b(1)X^1 + b(m-1)X^(m-1)         */
/*                  + u(0)X^(m) + u(1)X^(m+1) + ... + u(k)X^(m+k-1)   */
/*                                                                    */
/*  IMPLEMENTATION RESTRICTIONS:                                      */
/*  ----------------------------                                      */
/*  - The order of G(X) is limited to 32, that is m <=32.  This is to */
/*    facilitate storing the generator polynomail as UWord32          */
/*                                                                    */
/*  - Only g(0)-g(m-1) are used for UWord32 poly, it is ajdusted to   */
/*    MSB of UWord32 as                                               */
/*             bit15(poly) = g(m-1)                                   */
/*             bit14(poly) = g(m-2)                                   */
/*             bit13(poly) = g(m-3)                                   */
/*             ......                                                 */
/*             bitn(poly) = g(0)                                      */
/*             rest bits = 0                                          */
/*                                                                    */
/*  - Inputs are in  bytes, where the LSB corresponds to the last     */
/*    received bit, and 0s MUST be appended at LSB if not fit into    */
/*    bytes                                                           */
/*                                                                    */
/*    For example, if a 5-bit input data vector is as follows:        */
/*        U(x) = a0*X^0 + a1*X^1 + a2*X^2 + a3*X^3 + a4*X^4           */
/*                                                                    */
/*    it would be packed as follows:                                  */
/*              bit15 = a4                                            */
/*              bit14 = a3                                            */
/*              bit13 = a2                                            */
/*              bit12 = a1                                            */
/*              bit11 = a0                                            */
/*              bit10-bit0 = 0                                        */
/*                                                                    */
/*  - Output CRC bits are adjusted to MSB of the returned UWord32     */
/*                                                                    */
/* ================================================================== */
#include <stdio.h>
#include <assert.h>
#include <hexagon_protos.h>

#include "typedef.h"
#include "q6basic_op.h"
#include "crc.h"


/* ------------------------------------------------------------------ */
/*         Bit-by-bit implementation                                  */
/* ------------------------------------------------------------------ */
UWord32 crc32(unsigned char *data, int nbits, UWord32 poly) 
{
  UWord32 crc = 0;
  int i, k;

  while (nbits>0) {
    crc  ^= (UWord32)*data++ <<24;
    k = (nbits > 8) ? 8 : nbits;
    for (i=0; i < k; ++i) {
      crc = (crc << 1) ^ (crc & 0x80000000 ? poly : 0);
    }
    nbits -= 8;
  }
  return crc;
}


UWord32 crc16(unsigned char *data, int nbits, UWord32 poly) 
{
// This program supports CRC where the order of G(x) is limited 
// to 24, though it is called "crc16" 

  UWord32 crc = 0;
  UWord32 spoly;
  int i, k;

  while (nbits>0) {
    crc  ^= (UWord32)*data++ <<24;
    spoly = poly >> 1 ;
    k = (nbits > 8) ? 8 : nbits;
    for (i=0; i < k; ++i) {
      crc    ^= ((crc>>(31-i))&1 ? spoly : 0);
      spoly >>= 1;
    }
    crc <<= k;
    nbits -= 8;
  }
  return crc;
}


/* ------------------------------------------------------------------ */
/*         lookup table implementation                                */
/*  - It is assumed the length of message are mutiple of 8-bits.      */
/* ------------------------------------------------------------------ */
UWord32 crc32_lut(unsigned char *data,int nbyte, UWord32 *crctab)
{
   UWord32 crc = 0;

   while(nbyte--) {
       crc = (crc<<8) ^ crctab[(crc>>24) ^ *data++];
   }
   return crc;
}


UWord8 crc8_lut(unsigned char *data,int nbyte, UWord8 *crctab)
{
   UWord32 crc = 0;

   while(nbyte--) {
       crc = crctab[crc ^ *data++];
   }
   return crc;
}



/* ------------------------------------------------------------------ */
/*         Block update implementation                                */
/* ------------------------------------------------------------------ */
/*  - Generate the CRC using 64 message bits at a time.               */
/*  - smaller table                                                   */
/*  - less cycles for m < 24                                          */
/*  - regular memory access pattern, but more bandwidth               */
/*    Source: Remi Gurski                                             */
/* ------------------------------------------------------------------ */
#define NUM_BYTES_BLK     8

UWord32 crc32_blk(unsigned char *data, int nbyte, UWord64 *crctab, int mG) {
  UWord64 d;
  UWord32 crc = 0;
  int i;

  assert((nbyte%NUM_BYTES_BLK) == 0);

  nbyte >>=3;

  while (nbyte--) {
    d = 0;
    // Message MSb first
    for (i=0; i < NUM_BYTES_BLK; i++) {
      d = (d<<8) | *data++;
    }
    // XOR CRC into upper bits for feedback
    d ^= ((UWord64)crc << 32);

    crc = 0;

    // Matrix multiply (mod-2): BLK*M 
    for (i=0; i < mG; ++i) {
      crc = (crc<<1) | parity(d, crctab[i]);
    }
    crc <<= (32-mG);
  }

  return crc;
}


/* ------------------------------------------------------------------ */
/*         Congruent Equivalence Compression implementation           */
/* ------------------------------------------------------------------ */
/*  - Generate the CRC using 64 message bits at a time.               */
/*  - large table, must be aligned to 1024 byte boundary              */
/*  - less cycles for n > 16                                          */
/*  - regular memory access pattern, but more bandwidth               */
/*  - max resource usage 16 lookups in 11 cycles                      */
/*    Source: David Hoyle                                             */
/* ------------------------------------------------------------------ */
UWord32 crc32_clut(uword8_8B_align *data,int nbyte, UWord32 *crctab, UWord32 *contab)
{
   int i, n16;
   UWord32 crc = 0;
   UWord8  b0;
   UWord64 v0, v1, v2, v3, v4, v5, v6, v7;
   UWord64 v8, v9, va, vb, vc, vd, ve, vf;
   UWord64 d7to0, dfto8;
   UWord64 d7to0_, dfto8_;

   if(!nbyte) return(0);
   n16 = nbyte >> 4;
   n16--;

   d7to0 = * (UWord64 *) data; data += 8;
   dfto8 = * (UWord64 *) data; data += 8;
   if(n16) for(i=0; i < n16; i++) {
       d7to0_ = * (UWord64 *) data; data += 8;
       dfto8_ = * (UWord64 *) data; data += 8;
                                      /* contab entries must be byte reversed */
       v0  = (UWord64) contab[  0 + (((UWord32)d7to0 >> 0 ) &0x0FF)];
       v1  = (UWord64) contab[256 + (((UWord32)d7to0 >> 8 ) &0x0FF)];
       v2  = (UWord64) contab[512 + (((UWord32)d7to0 >> 16) &0x0FF)];
       v3  = (UWord64) contab[768 + (((UWord32)d7to0 >> 24) &0x0FF)];

       v4  = (UWord64) contab[  0 + (((UWord32)(d7to0>>32) >> 0 ) &0x0FF)];
       v5  = (UWord64) contab[256 + (((UWord32)(d7to0>>32) >> 8 ) &0x0FF)];
       v6  = (UWord64) contab[512 + (((UWord32)(d7to0>>32) >> 16) &0x0FF)];
       v7  = (UWord64) contab[768 + (((UWord32)(d7to0>>32) >> 24) &0x0FF)];

       v0 =   v0 | (v4 << 32); v1 =   v1 | (v5 << 32);
       d7to0 =v2 | (v6 << 32); v3 =   v3 | (v7 << 32);
       d7to0 ^= v1     ^ v0;
       d7to0 ^= d7to0_ ^ v3;
       /* ------------------------------------------------------------ */
       v8 = (UWord64) contab[  0 + (((UWord32)dfto8 >> 0 ) &0x0FF)];
       v9 = (UWord64) contab[256 + (((UWord32)dfto8 >> 8 ) &0x0FF)];
       va = (UWord64) contab[512 + (((UWord32)dfto8 >> 16) &0x0FF)];
       vb = (UWord64) contab[768 + (((UWord32)dfto8 >> 24) &0x0FF)];

       vc = (UWord64) contab[  0 + (((UWord32)(dfto8>>32) >> 0 ) &0x0FF)];
       vd = (UWord64) contab[256 + (((UWord32)(dfto8>>32) >> 8 ) &0x0FF)];
       ve = (UWord64) contab[512 + (((UWord32)(dfto8>>32) >> 16) &0x0FF)];
       vf = (UWord64) contab[768 + (((UWord32)(dfto8>>32) >> 24) &0x0FF)];

       dfto8 = v8 | (vc << 32); v9 = v9 | (vd << 32);
       va = va | (ve << 32); vb = vb | (vf << 32);
       dfto8 ^= v9 ^ va;
       dfto8 ^= dfto8_ ^ vb;
   }
   for(i=0; i < 8; i++) {
      b0 = d7to0 & 0xff; d7to0 >>= 8;
      crc = (crc << 8) ^ crctab[(crc >> 24) ^ b0];
   }
   for(i=0; i < 8; i++) {
      b0 = dfto8 & 0xff; dfto8 >>= 8;
      crc = (crc << 8) ^ crctab[(crc >> 24) ^ b0];
   }
   return crc;
}

#if __HEXAGON_ARCH__ >= 4
/* ------------------------------------------------------------------ */
/*         Polynomial Implementation                                  */
/* ------------------------------------------------------------------ */
/*  - Generate the CRC using a polynomial calculation.                */
/*  - no table                                                        */
/*  - regular memory access pattern                                   */
/*    Source: David Hoyle                                             */
/* ------------------------------------------------------------------ */
unsigned int gmpy_poly(unsigned int x, unsigned int y, 
                        unsigned int poly, unsigned int ipoly)
{
    unsigned long long pp0, rr0;

    rr0 = Q6_P_pmpyw_RR(x, y);
    pp0 = rr0; 
    rr0 = Q6_P_pmpywxacc_RR(rr0, ipoly, rr0>>32);
    pp0 = Q6_P_pmpywxacc_RR(pp0, rr0>>32, poly) ;

    return((unsigned int)pp0);
}

unsigned int crc32poly(uword8_8B_align *ptr8_data, int len,
                       unsigned int poly, unsigned int ipoly, unsigned int alpha64)
{
  int i;
  unsigned int b0123, alphai;
  unsigned int * ptr32_data;
  unsigned short * ptr16_data;
  unsigned int start, end, mid, mask;
  unsigned long long lcrc_co, rr0;
  unsigned int crc_co = 0;

  start = (- ((int) ptr8_data));
  mask = 0x7fffffff >> Q6_R_cl0_R(len);
  start = start & mask & 3;
  mid = len - start;
  end = mid & 3;
  mid = mid >> 2;

  if(start & 1) {
     crc_co= Q6_R_swiz_R(*ptr8_data); ptr8_data+= 1;
     alphai = 0x100;
  }
  if(start & 2) {
     ptr16_data = (UWord16 *) ptr8_data; ptr8_data+= 2;
     b0123 = Q6_R_swiz_R(*ptr16_data);
     crc_co = b0123 ^ gmpy_poly(crc_co, alphai, poly, ipoly);
     alphai = 0x10000;
  }
  lcrc_co = (UWord64) crc_co;
  if(mid) for(i=0; i < mid; i++) {
    ptr32_data = (unsigned int * ) ptr8_data; ptr8_data+= 4;
    b0123 = Q6_R_swiz_R(*ptr32_data);
    rr0 = Q6_P_pmpyw_RR((UWord32)lcrc_co, alphai);
    lcrc_co = rr0 ^ ((UWord64) b0123); 
    rr0 = Q6_P_pmpywxacc_RR(rr0, ipoly, rr0>>32);
    lcrc_co = Q6_P_pmpywxacc_RR(lcrc_co, rr0>>32, poly) ;
    alphai = poly; 
  }
  crc_co = (unsigned int) lcrc_co;
  if(end & 2) {
     ptr16_data = (UWord16 *) ptr8_data; ptr8_data+= 2;
     b0123 = Q6_R_swiz_R(*ptr16_data);
     crc_co = b0123 ^ gmpy_poly(crc_co, alphai, poly, ipoly);
     alphai = 0x10000;
  }
  if(end & 1) {
     b0123 = Q6_R_swiz_R(*ptr8_data);
     crc_co = b0123 ^ gmpy_poly(crc_co, alphai, poly, ipoly);
     alphai = 0x100;
  }
  crc_co = gmpy_poly(crc_co, alphai, poly, ipoly);
  return(crc_co);
}
#endif

/* ------------------------------------------------------------------ */
