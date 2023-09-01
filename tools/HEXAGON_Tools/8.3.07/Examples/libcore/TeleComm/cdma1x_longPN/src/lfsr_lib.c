/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2009                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
/*  FUNCTIONS      : Decimated 42bit LFSR generator for 1x              */
/*  ARCHITECTURE   : Hexagon                                            */
/*  VERSION        :                                                    */
/*                                                                      */
/*======================================================================*/
/*  REVISION HISTORY:                                                   */
/*  =================                                                   */
/*                                                                      */
/*  Author              Date           Comments                         */
/*  -------------------------------------------------------------       */
/*  DJH                 11/9/09       asm completed model created       */
/*  DJH                 11/10/09      added state split incorporating   */
/*                                    idea from Mao                     */
/*======================================================================*/
#include "typedef.h"
#define N 42

UWord64 _hi32(UWord64 a) {
   UWord32 r = (UWord32) (a >> 32);
   return((UWord64)r);
}

UWord64 _lo32(UWord64 a) {
   UWord32 r = (UWord32) (a);
   return((UWord64)r);
}

UWord64 interleave(UWord64 a) {
   int i;
   UWord64 y = 0;
   UWord64 u =  (a>>32)& 0xffffffffLL;
   UWord64 l =   a     & 0xffffffffLL;
   for(i=0; i < 32; i++)
   {
     y = y | (l & 1)<<(2*i) | (u & 1)<<(2*i+1); 
     u >>= 1;
     l >>= 1;
   }
   return(y);
}

UWord64 deinterleave(UWord64 a) {
   int i;
   UWord64 y = 0;
   UWord64 u =  (a>>1) & 0x5555555555555555LL;
   UWord64 l =   a     & 0x5555555555555555LL;
   for(i=0; i < 32; i++)
   {
     y = y | (l & 1)<<i | (u & 1)<<(i+32); 
     u >>= 2;
     l >>= 2;
   }
   return(y);
}

UWord64 parity(UWord64 a, UWord64 b) {
   UWord64 c = a & b;
   int i,p = 0;
   for(i=0; i < 64; i++) {
     p += c & 1; c >>= 1;
   }
   p = p & 1;
   return(p);
}

UWord64 lfs(UWord64 x, UWord64 y)
{
    x = (x >> 1) | parity(x, y)<<63;
    return(x);
}

UWord64 bitr(UWord64 x) {
 int i;
 UWord64 y = 0;
 for (i=0; i < 42; i++) {
      y = (y<<1) | (x & 0x1LL); x = x>>1;
 }
 return(y);
}

/* ----------------------------------------------------------------------- *
 * apply power matrix in "Decimation of m-Sequences leading to any 
   desired phase offset - B Arazi - Electronics Letters 31st March 1977" 
   to fibonacci state equivalent, uses input table of 0th row of 
   power matrices for alpha^(i*decimation) *
 * ----------------------------------------------------------------------- */

UWord64 calc_dec_asm(UWord64 seed, UWord64 * decimate)
{
    int i;
    UWord64 fib64 = 0;
    for(i=0; i < N; i+=1) {
        fib64 |= parity(decimate[i], seed)<<i;
    }
    return(fib64);
}
/* --------------------------------------------------------------- *

   Generate output lfsr sequence takes odd and even decimated sequences
   and interleaves them to double bandwdith so decimate by 64 is 2 decimated
   sequences of 128 interleaved, one delayed by 64
 * --------------------------------------------------------------- */

 void long_code_gen_f_asm( UWord64 *state0, UWord64 *state1, UWord64 *long_code, 
                           int num_64chips)
 {
   int i1, i2;
   UWord64 s0,s1;
   UWord64 poly = (0x0000040a8e6f04efLL)<<22;
   UWord64 * ptr = (UWord64 *) long_code;
 
   s0 = *state0<<22;
   s1 = *state1<<22;
   for(i1 = 0; i1 < num_64chips; i1++){
     ptr[i1] = interleave(((s1>>22) << 32) | ((s0>>22)&0xFFFFFFFFLL));
     for(i2 = 0; i2 < 32; i2++){
       s0 = lfs(s0, poly);
       s1 = lfs(s1, poly);
     }
   }
   *state0 = s0;
   *state1 = s1;
   return;
 }
/* --------------------------------------------------------------- *

      Advance decimated states by 42 cycles to get next 42 deicmated
      state bits, deinterleave them to get input for generator.
 * --------------------------------------------------------------- */

void split_states_asm( UWord64 * states, int n)
 {
   int i;
   UWord64 s0, s1, s2, s3, d0, d1, d2, d3;
   UWord64 poly = (0x0000040a8e6f04efLL)<<22;

   s0 = states[0];
   s2 = states[2];
   s1 = s0<<22;
   s3 = s2<<22;
   d0 = deinterleave(s0);
   d2 = deinterleave(s2);
   for(i = 0; i < n; i++) {
      s1 = lfs(s1, poly);
      s3 = lfs(s3, poly);
   }
   s1 >>= 22;
   s3 >>= 22;

   d1 = deinterleave(s1);
   d3 = deinterleave(s3);

   s0 = _lo32(d1)<<21 | _lo32(d0);
   s1 = _hi32(d1)<<21 | _hi32(d0);
   s2 = _lo32(d3)<<21 | _lo32(d2);
   s3 = _hi32(d3)<<21 | _hi32(d2);

   states[0] = s0;
   states[1] = s1;
   states[2] = s2;
   states[3] = s3;
   return;
 }

/* --------------------------------------------------------- *
   Generate initial state based on seed and mask
 * --------------------------------------------------------- */
void mask_gen_asm(UWord64 *fib_1, UWord64 *fib0, UWord64 state, UWord64 mask)
{
  int i;
  UWord64 fib42, bit;
  UWord64 poly = 0x0000040a8e6f04efLL;

  fib42 = 0;
  for(i = 0; i < N; i++){
    bit = parity(mask, state);
    fib42 = (fib42<<1) | bit;
    state = (state << 1); if (state & 0x40000000000LL) state ^= poly;
  }
  *fib_1 = fib42;
  bit = parity(mask, state);
  fib42 = (fib42<<1) | bit;
  *fib0 = fib42;

  return ;
}
/* --------------------------------------------------------------- *
   end of lfsr library file
 * --------------------------------------------------------------- */
