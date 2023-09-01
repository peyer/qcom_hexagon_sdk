/*
 *======================= COPYRIGHT NOTICE =====================
 * Copyright (c) 2003-04 by Qualcomm, Inc.  All rights reserved.
 * All data and information contained in or disclosed by this   
 * document is confidental and proprietary information of       
 * Qualcomm, Inc and all rights therein are expressly reserved.
 * By accepting this material the recipient agrees that this    
 * material and the information contained therein is held in    
 * confidence and in trust and will not be used, copied,        
 * reproduced in whole or in part, nor its contents revealed in 
 * any manner to others without the express written permission  
 * of Qualcomm, Inc.                                            
 *==============================================================
 ***************************************************************
 * FILE NAME:   dsp_libs.cc                                      
 * DESCRIPTION: Library of QDSP4000 instructions and library 
 *              math functions.
 * PROGRAMMERS: Vivek Viswanathan
 *              Brian Banister
 ***************************************************************
 */

// Modified by MZ from the original codes

/* ------------------------------------------------------------- */
/*  INCLUDES                                                     */
/* ------------------------------------------------------------- */
#include <stdio.h>
#include "typedef.h"

#include "q6basic_op.h"
#include "dsp_libs.h"

/* ------------------------------------------------------------- */
/*  CONSTANTS                                                    */
/* ------------------------------------------------------------- */

#define DSP_INV_LUT_BITS   4   /* number of bits to index the inverse lookup table (actually 2^(val-1) segments) */
#define DSP_SQRT_LUT_BITS  5
#define DSP_INV_SQRT_LUT_BITS  5

#define DSP_INV_LUT_SIZE  (1+(1<<(DSP_INV_LUT_BITS-1)))  /* number of entries in the inverse LUT */
#define DSP_SQRT_LUT_SIZE ( (1<<(DSP_SQRT_LUT_BITS-2))*3 + 1)
#define DSP_INV_SQRT_LUT_SIZE ( (1<<(DSP_INV_SQRT_LUT_BITS-2))*3 + 1)

#define DSP_LOG_LUT_BITS      5
#define DSP_LOG_LUT_SIZE      ((1<<DSP_LOG_LUT_BITS) + 1)

#define INVLOG2Q13               0x6a4dLL
#define EXP10C0                  0x7fbdLL
#define EXP10C1                  0x556fLL
#define EXP10C2                  0x15efLL

/* ------------------------------------------------------------- */
/*  LOOK UP TABLES                                               */
/*                                                               */
/* this LUT has been generated using the matlab function         */
/* "find_optimal_lut_mse_cantoni.m" in                           */
/*  \\vivekv\Public\Jaguar\cdma1x\matlab. See the matlab file    */
/* for details of the mathematical analysis.                     */
/* ------------------------------------------------------------- */

Word16 invTable[DSP_INV_LUT_SIZE]= {32690, 29066, 26171, 23798, 21820, 20145, 18709, 17463, 16373};

UWord16 sqRootLut[DSP_SQRT_LUT_SIZE]= {16389, 17382, 18322, 19215, 20069, 20888, 21676, 22437, 
                                     23172, 23885, 24578, 25251, 25907, 26546, 27171, 27782, 
                                     28379, 28964, 29538, 30100, 30652, 31195, 31728, 32253, 
                                     32769};

Word16 invSqRootLut[DSP_INV_SQRT_LUT_SIZE]= {32738, 30870, 29290, 27930, 26743, 25696,
                                          24762, 23924, 23165, 22474, 21841, 21259,
                                          20721, 20222, 19757, 19323, 18917, 18535,
                                          18175, 17835, 17514, 17209, 16920, 16645,
                                          16383};

/* Note: log2table[x]=25*2^10*log2(32+x) */
Word16  log2table[DSP_LOG_LUT_SIZE]=    { 0x7800, 0x782e, 0x785a, 0x7884, 0x78ae,
                                         0x78d7, 0x78fe, 0x7924, 0x794a, 0x796e,
                                         0x7992, 0x79b5, 0x79d7, 0x79f8, 0x7a18,
                                         0x7a38, 0x7a57, 0x7a76, 0x7a93, 0x7ab1,
                                         0x7acd, 0x7ae9, 0x7b05, 0x7b20, 0x7b3b,
                                         0x7b55, 0x7b6f, 0x7b88, 0x7ba1, 0x7bb9,
                                         0x7bd1, 0x7be9, 0x7c00};




/*--------------------------------------------------------------
 * dsplib_log10 ()
 *
 * DSP library function "log10". 
 * 
 * Returns 10*log10(x). 
 * Uses 10*log10(x)=3.0103*log2(x) approximation to calculate.
 * Uses lookup table for log2 with 7 MSB of input.
 *
 * Inputs:
 *     x              integer to find 10log10 of (> 0)
 *
 * Return:
 *     10log10(x) in 32Q23 unsigned.
 * 
 *----------------------------------------------------------------*/  
Word32 dsplib_log10 (Word32 x)
{
   Word32 expon=0, mantis,frac;
   Word32 logval, y;

   /* Input ranges (0,2^31). Saturate? */
   if ( x<=0 ) return -1;
   
   /* Saturate input                   */
   // x = (x>(Word32) 0x7fffffff) ? 0x7fffffff : x;

   /* Normalize input, y= normalized i/p, expon = SR */
   expon = norm_l(x);
   y = L_shl(x,(Word16)expon);


   /* Use only 7 MSB's of normalized input                    */
   /* to compute log. The 2 MSB's are 01                      */
   /* Right-align value. In effect using 5 LSB's to find log. */
   mantis = (y>>25)&(0x1f);
   

   /* DSP implementation uses a MAC instead of shift, so the low bits     */
   /* get truncated.                                                      */
   /* frac = (y>>9)&(0xff80);  // Q16 */

   frac = (y>>9)&(0xffff);  // Q16                                    
   expon <<=10;

   /* Interpolate log2 results w/ LPF */
   logval=(log2table[mantis]<<16)+0x8000-frac*log2table[mantis]; /* 0x8000 to round frac */
   logval+=(frac*log2table[mantis+1]);

   /* Approx. log10(x)=3.0103*log2(x)       */
   /* Use 5 LSB's to lookup log2 of input   */
   /* 0x6054=3.0103 (Q13)                   */
   logval=(0x6054*(logval>>16)- 0x6054*expon);

   /* returned 10log10(x) is 32Q23 fixed-point value */
   return(logval);
}


/*--------------------------------------------------------------
 * dsplib_approx_invert()
 *
 *     DSP library function "approx_invert".
 *     Approximates inversion:  out ~= 2^(31-floor(lg2(in)))/in
 *
 *     Approximation done with a linearly interpolated lookup table. With
 *     9 point entries (8 line segments) the maximum error is 0.238%. The 
 *     number to be inverted must be positive for valid results. If not 
 *     positive, then the lookup table is invalidly indexed.
 * 
 *     If input is Qi and the output is Qo, then
 *        Qo = 32 + (*shift_factor) - Qi.
 *
 * Inputs:
 *     input          integer to be inverted
 *
 * Return:
 *     Word64 (shift_factor : result)
 *     where
 *      result        Q-shifted inverse
 *                    result ~= 2^(31-floor(lg2(input))) / input
 *                             = 2^(32+(*shift_factor)) / input
 *      shift_factor  (output_Q_factor - 32) of integer inverse
 *                    shift_factor = -1-floor(lg2(input))
 *
 *      if input <=0:  return 
 *        result       = 0xFFFFFFFF
 *        shift_factor = 0xFFFFFFFF
 *----------------------------------------------------------------*/  
result_scale_t dsplib_approx_invert(Word32 input)
{
  Word32 norm_divisor;
  Word32 n1,interp;
  Word32 index;
  Word32 r;
  result_scale_t rval;
  
  /* check for negative input (invalid) */
  if (input <= 0) {
    rval.result = -1;
    rval.scale  = -1;
    return rval;
  }
  
  /* bit-align-normalize input */
  r = norm_l(input);
  norm_divisor = L_shl(input, (Word16)r);

  /* determine inverse LUT index and interpolation factor */
  n1     = norm_divisor >> (15-DSP_INV_LUT_BITS);
  interp = n1 % (1<<16);
  index  = (n1 >> 16) - (DSP_INV_LUT_SIZE-1);
  
  /* inverse linear interpolation between LUT entries */
  rval.result = (invTable[index]<<16) + interp * (invTable[index+1] - invTable[index]);
  rval.scale  = r-31;

  /* return values  */
  return rval;

}



/*--------------------------------------------------------------
 * dsplib_approx_divide()
 *
 *     DSP library function "approx_divide".
 *     Approximates inversion:  out ~= numer*2^(31-floor(lg2(in)))/denom
 *
 *     Approximation done with a linearly interpolated lookup table. With
 *     9 point entries (8 line segments) the maximum error is 0.238%. The 
 *     number to be inverted must be positive for valid results. If not 
 *     positive, then the lookup table is invalidly indexed.
 * 
 *
 * Inputs:
 *     numer          integer numerator
 *     denom          integer denominator
 *
 * Return:
 *     Word64 (shift_factor : result)
 *     where
 *      result        - quotient
 *                    result ~= numer*2^(31-floor(lg2(input))) / denom
 *                             = numer*2^(32+(*shift_factor)) / denom
 *      shift_factor  - Qfactor of (*result)
 *
 * 
 * Notes : (*result << *shift_factor) will be floating point result in Q0
 *
 *----------------------------------------------------------------*/  
result_scale_t dsplib_approx_divide(Word32 numer,Word32 denom)
{
  Word32 norm_num,r,s_d,s_n;
  result_scale_t norm_denom; 
  result_scale_t rval;

  if (denom <= 0) {
    rval.result = -1;
    rval.scale  = -1;
    return rval;
  }

  norm_denom = dsplib_approx_invert(denom);
  r   = norm_denom.result;
  s_d = norm_denom.scale + 1;
  s_n = norm_l(numer);

  norm_num = L_shl(numer, (Word16)s_n);

  rval.result = L_mult32x16r(norm_num, rnd(r));
  rval.scale  = s_d - s_n;

  return rval;
}


/*--------------------------------------------------------------
 * dsplib_sqrt_lut()
 *
 *     DSP library function "sq_root_lut".
 *     Approximates inversion:  out=sqrt(in)
 *
 *
 * Inputs:
 *     input          integer input number in Q0 ( > 0 )
 *     round_factor   integer rounding factor, any bit location
 *
 * Return:
 *     result        sqrt(input), in Q16
 * 
 * 
 *----------------------------------------------------------------*/  
Word32 dsplib_sqrt_lut(Word32 input,Word32 round_factor)
{
  Word32 norm_input, n1,interp,val;
  Word32 index, r, r2;
  Word32 result;

  /* check for negative input (invalid) */
  if (input < 0)
  {
    return (-1);
  }

  /* check for zero input               */
  if (input == 0)
  {
    return (0);
  }

  /* bit-align-normalize input */
  r2 = norm_l(input);
  norm_input = L_shl(input, (Word16)r2);



  r = (-r2) >> 1;
  n1 = norm_input >> 10;
  if (!(r2 & 1)) {
    n1 = n1 >> 1;
  }
  
  index = (n1>>16) - 8;
  interp = n1 % (1<<16);
  val = (sqRootLut[index] << 16) - (sqRootLut[index]*interp) + (sqRootLut[index+1]*interp);
//result = (val >> (-(r + 1))) + round_factor;
  result = L_shl(val, (r + 1)) + round_factor;
  return result;
}


/*--------------------------------------------------------------
 * dsplib_inv_sqrt_lut()
 *
 *     DSP library function "inv_square_root_lut".
 *     Approximates inversion:  out=1/sqrt(in)
 *
 *
 * Inputs:
 *     input          integer input number in Q0 ( > 0 )
 *     round_factor   integer rounding factor, any bit location
 *
 * Return:
 *     result        1/sqrt(input), in Q30
 * 
 * 
 *----------------------------------------------------------------*/  
Word32 dsplib_inv_sqrt_lut(int input,int round_factor)
{
  Word32 norm_input, n1, interp, val;
  Word32 index, r, r2;
  Word32 result;

  /* check for negative input (invalid) */
  if (input <= 0)
  {
    return (-1);
  }

  /* bit-align-normalize input */
  r2 = norm_l(input);
  norm_input = L_shl(input, (Word16)r2);

  r = (-r2) >> 1;
  n1 = norm_input >> 10;
  if (!(r2 & 1)) {
    n1 = n1 >> 1;
  }
  
  r=-r;
  r=16-r;

  index = (n1>>16) - 8;
  interp = n1 % (1<<16);
  val = (invSqRootLut[index] << 16) - (invSqRootLut[index]*interp) + (invSqRootLut[index+1]*interp);
//result = (val >> r) + round_factor;
  result = L_shr(val, r) + round_factor;
  return result;
}



/*--------------------------------------------------------------
 *  dsplib_exp10()
 *  
 *            Calculate 10^x where x is in the range [-1,1].
 *  Input:
 *       x    Word32 input, Q26
 *  Return:
 *            10^x  in Q15
 *----------------------------------------------------------------*/  
Word32 dsplib_exp10(Word32 x) {
  Word64 acc;
  Word64 inv;
  Word32 sh;

  acc = x * INVLOG2Q13 >> (16+7);
  sh = (acc >> 16) - 15;
  inv = ~acc & 0xFFFF;

  acc = EXP10C2*(inv*inv >> 16) -  EXP10C1*inv + (EXP10C0 << 16);

  return (Word32)(LL_shl(acc,sh));
}

/*--------------------------------------------------------------
 *  dsplib_newton_invert()
 *  
 *            Inverts a 32-bit positive number using Newton's method
 *  Input:
 *       input    Word32 input to be inverted, Q0
 *
 *  Return:
 *       Word64 (sr: result)
 *       where
 *         result   Inverted rounded result in Q(45-sr)
 *                  0xFFFFFFFF for input <= 0
 *         sr       Word32, shift normalization factor
 *----------------------------------------------------------------*/  

result_scale_t dsplib_newtons_invert(Word32 input) {
  Word32 i;
  Word32 num;
  Word32 x, sf;
  result_scale_t rval;

  if (input <= 0) {
    rval.result = -1;
    rval.scale  = -1;
    return rval;
  }

  sf = norm_l(input);
  num = L_shl(input,sf) >>16;

  x = 0x5555;
  for (i=0; i < 5; ++i) {
   x = (((((0x40008000 - x*num)>>16)*x)<< 3) + 0x8000) >> 16;
  }

  rval.result = x;
  rval.scale  = sf;

  return rval;
}




/*--------------------------------------------------------------
 * dsplib_invert()
 *
 *     DSP library function "dsplib_invert".
 *     Calculate inversion:  1/in
 *
 * Inputs:
 *     Word16 input         integer to be inverted
 *
 * Return:
 *     Word64 (shift_factor : result)
 *
 *      if input <=0:  return 
 *        result       = 0xFFFFFFFF
 *        shift_factor = 0xFFFFFFFF
 *
 *      - result has 16-bit precision in LSB
 *        
 *      - Decimal-point:
 *        If input is in Qi and the output is Qo, then
 *         o = (*shift_factor) -i
 *
 *        For example, input is 0x0F00 in Q10, i.e., x= 3.75 
 *        the function returns 
 *        result = 17476, shift_factor is 26, and thus the 
 *        actual result is in Q(26-10), i.e., inversion is
 *        17476/(2^16) = 0.26666260
 *      
 *----------------------------------------------------------------*/  
result_scale_t dsplib_invert(Word16 input)
{
  UWord32 L_num, L_denom; 
  Word16 nshft, result;
  int iteration;
  result_scale_t rval;
  
  /* check for negative input (invalid) */
  if (input <= 0) {
    rval.result = -1;
    rval.scale  = -1;
    return rval;
  }
  
  /* bit-align-normalize input */
  L_denom = (UWord32)input << 16;
  nshft = norm_l(L_denom);
  L_denom = L_shl(L_denom, nshft);

  L_num = 0x80000000;
  result = 0;

  for (iteration = 0; iteration < 15; iteration++) {

       result <<= 1;

       if (L_num >= L_denom) {
          L_num -= L_denom;
          result++;
       }
       L_num <<= 1;
  }

  nshft = 29 - nshft;

  /* return values  */
  rval.result = result;
  rval.scale  = nshft;
  return rval;
}



/*--------------------------------------------------------------
 * dsplib_approx_invert_simple()
 *
 *     DSP library function "approx_invert".
 *     Approximates inversion:  out ~= 2^(31-floor(lg2(in)))/in
 *
 *     Approximation done with a linearly interpolated lookup table. With
 *     9 point entries (8 line segments) the maximum error is 0.238%. The 
 *     number to be inverted must be positive for valid results. If not 
 *     positive, then the lookup table is invalidly indexed.
 * 
 *     If input is Qi and the output is Qo, then
 *        Qo = 32 + (*shift_factor) - Qi.
 *
 * Inputs:
 *     input          integer to be inverted
 *
 * Return:
 *     Word64 (shift_factor : result)
 *     where
 *      result        Q-shifted inverse
 *                    result ~= 2^(31-floor(lg2(input))) / input
 *                             = 2^(32+(*shift_factor)) / input
 *      shift_factor  (output_Q_factor - 32) of integer inverse
 *                    shift_factor = -1-floor(lg2(input))
 *
 *      if input <=0:  return 
 *        result       = 0xFFFFFFFF
 *        shift_factor = 0xFFFFFFFF
 * 
 * Note:
 *      This is a modified version of previous implementation.
 *      It is intended to reduce cycle-count. 
 *----------------------------------------------------------------*/  

Word32 invTable1[DSP_INV_LUT_SIZE-1]= 
{ (32690<<16) | (32690-29066), 
  (29066<<16) | (29066-26171), 
  (26171<<16) | (26171-23798), 
  (23798<<16) | (23798-21820), 
  (21820<<16) | (21820-20145), 
  (20145<<16) | (20145-18709), 
  (18709<<16) | (18709-17463), 
  (17463<<16) | (17463-16373) 
};

result_scale_t dsplib_approx_invert_simple(Word32 input)
{
  Word32 norm_divisor;
  Word32 n1,interp;
  Word32 index;
  Word32 r;
  result_scale_t rval;
  
  /* check for negative input (invalid) */
  if (input <= 0) {
    rval.result = -1;
    rval.scale  = -1;
    return rval;
  }
  
  /* bit-align-normalize input */
  r = norm_l(input);
  norm_divisor = L_shl(input, (Word16)r);

  /* determine inverse LUT index and interpolation factor */
  n1     = norm_divisor >> (15-DSP_INV_LUT_BITS);
  interp = n1 % (1<<16);
  index  = (n1 >> 16) - (DSP_INV_LUT_SIZE-1);
  
  /* inverse linear interpolation based on LUT entries */
  rval.result = invTable1[index] - interp * (invTable1[index]&0x0000FFFF); 
  rval.scale  = r-31;

  /* return values  */
  return rval;

}




