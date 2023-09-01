/*===========================================================================

  Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved

  Qualcomm Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  QUALCOMM Incorporated and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of QUALCOMM Incorporated.

===========================================================================*/

/*===========================================================================
  
  DESCRIPTION: library of approximate math functions
  SOURCE     : Remi Gurski

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "typedef.h"
#include "q6basic_op.h"

#include "dsp_libs.h"

/*===========================================================================

                           MACRO DEFINITIONS

===========================================================================*/
#define SIGNED16(x) (x>32767? (short)(x-65536):(x))

/*===========================================================================

                           FUNCTIONS

===========================================================================*/



/*===========================================================================

  FUNCTION:  approx_uinv_linlut8

===========================================================================*/
/*
    @brief
    Approximate unsigned-inverse using linear 8-entry lookup table.

    @detail
    Given an unsigned input, return an approximation of 1/input, and a scale
    factor.
    If input is 0, the output returned is result=0xFFFFFFFF, scale=0.
    Valid input range is 0-0xFFFFFFFF

    With the input in Qi, the output is Qo=32-Qi. Right-shifting the result by
    the scale, converts the result to Qo.

    Note that the scale is signed & can be negative (scale=-1 for input=1). The
    >> C operator can't handle negative operands and alternately the intrinsic
    Q6_lsr_RR() may be used. Even in the case of a 1 bit left shift, the result
    wont overflow 32 bits.

    Any 32-bit input can be normalized to the range [.5,1) by trimming the
    leading 0s & interpreting the remaining bits (0-padded to get 32-bits)as a
    Q31 number. The first 4 bits of the normalized input are used for a rough
    inverse estimate, which is stored in a LUT with a 3 bit index( MSB=1 always
    for norm. input). The norm. input range [.5,1) is thus split into 8
    segments.

    The LUT contains the pair (m,c) that satisfy the equation for inverse:

    1/d = c - m*dn
    where,
      d = normalized input in range [.5,1)
      ind = d[30:28] in range [0,7]
      c = inverse of d[31:28] in Q14, treating d[31:28] as a fraction (Q4).
          i.e., c[ind] ~= 1/( 1 + ind/2^3 )
      m = slope of the linear approximation for 1/d in each segment, in Q15.
      dn= d[27:13] in Q15..fraction in range [0,1]

    d:   ||1 |   ind     |  dn     | ignored   ||
         31    30-28(3)  27-13(15)  12-0(13)

    The 'c' entries are skewed to give symmetrical error at either extreme of
    each segment. For e.g. in the 1st row, c=32690 ~= 16/8=32768(Q14)


    Example:
      input: 3.5625 (57 in Q4)
      output: scale=4, result=0x47dd8000=1205698560 (Q30)
                   shift right by 4 => 0x47dd800=75356160 (Q28=32-4)=0.280724

    @return
    A result/scale pair corresponding to 1/input.

    @note
    The optimized assembly function is 5 cycles.

    Maximum absolute relative error is 1.8e-3.
*/
/*=========================================================================*/

static Word32 approx_uinv_linlut8_LUT[8] =
{
   7273 | (32712<<16),  // m (Q15) | c (Q14) << 16
   5820 | (29087<<16),
   4762 | (26185<<16),
   3969 | (23809<<16),
   3359 | (21828<<16),
   2879 | (20151<<16),
   2496 | (18714<<16),
   2184 | (17467<<16)
  
};

result_scale_t approx_uinv_linlut8(UWord32 input)
{
  // Local variables: explanation appears near first usage 
  UWord32 d, sf, ind, dn, result;
  result_scale_t output;

  // Handle input=0. Report scale=0,result=0xFFFFFFFF 
  if (input==0) {
    output.result = 0xFFFFFFFFUL;
    output.scale = 0;
    return (output);
  }

  // Count #leading 0s in input
  sf = cl0(input);

  // Normalize input by scaling by 2^cl0. Norm. input=d is in Q32.
  // Hence, range of d is [.5,1).
  // d:   ||1 |   ind     |  dn     | ignored   ||
  //        31    30-28(3)  27-13(15)  12-0(13)     
  d = L_asl(input,sf);

  // Extract 3 MSBs of d after the 1st. ind=d[30:28]
  ind = (d >> 28) & 0x7 ;

  // Initialize the result. high hw=c & low hw=m.
  //  => result=c<<16 + m..Q30(14+16) 
  result = approx_uinv_linlut8_LUT[ind];


  // Extract 15 bits of d as the fraction fn. fn=d[27:13]..Q15.
  // Offset of 13 corresponds to ignored LSBs 
  dn =  (d >> 13) & 0x7FFF;

  // Add 1 to dn to compensate for m already loaded into the result
  dn += 1;

  // scale factor(SF)=30-cl0 ..lies in the range [-1,30].
  // The result, after a shift by SF, is in Qo but will never overflow 32 bits
  //   in case of a left shift by 1 
  sf = 30 -sf;

  // Calculate result=c<<16 - m*dn
  //                 =(c<<16 + m) - m*(dn+1)..Q30(15+15) 
  result -=  extract_l(result) * dn;

  // Format output 
  output.result = result;
  output.scale = sf;

  return ( output );
}/* approx_uinv_linlut8() */






/*===========================================================================

  FUNCTION:  approx_uinv_cubelut8

===========================================================================*/
/*
    @brief
    Approximate unsigned-inverse using cubic polynomial approximation.

    @detail
    Given an unsigned input, return an approximation of 1/input, and a scale
    factor.
    If input is 0, the output returned is result=0xFFFFFFFF, scale=0.
    Valid input range is 0-0xFFFFFFFF.

    With the input in Qi, the output is Qo=32-Qi. Right-shifting the result by
    the scale, converts the result to Qo.

    Note that the scale is signed & can be negative (scale=-1 for input=1). The
    >> C operator can't handle negative operands and alternately the intrinsic
    Q6_lsr_RR() may be used. Even in the case of a 1 bit left shift, the result
    wont overflow 32 bits.

    Any 32-bit input can be normalized to the range [.5,1) by trimming the
    leading 0s & interpreting the remaining bits (0-padded to get 32-bits)as a
    Q31 number. The norm. input range is split into 8 segments using 3 MSBs
    after the 1st (MSB=1 always for norm. input). The inverse is approximated
    by a cubic polynomial in each segment. Each equation is of the form:

    1/d = c0 + c1*dn + c2*dn^2 + c3*dn^3, 

    where,
      d = normalized input in range [.5,1)
      dn= fraction left after extracting the LUT index for the segment from d
          dn=d[27:0] in Q28 = 8*d[27:0] in Q31

    d:   ||1 |   ind     |  dn     ||
           31    30-28(3)  27-0(28)

    and the LUT for co-efficients in the cubic approximation for inverse is
    ( Cubic interpolation: Calculated using MATLAB polyfit() )
     1/d=
        c0       c1          c2           c3             Range of: d        dn
        Q15      Q17         Q20          Q22
      1.99997 -0.2498*dn + 0.0307*dn^2 -0.003078*dn^3    [0.5000,0.5625]-> [0,1)
      1.77777 -0.1975*dn + 0.0217*dn^2 -0.001971*dn^3    [0.5625,0.6250]-> [0,1)
      1.60001 -0.1600*dn + 0.0158*dn^2 -0.001320*dn^3    [0.6250,0.6875]-> [0,1)
      1.45456 -0.1322*dn + 0.0119*dn^2 -0.000917*dn^3    [0.6875,0.7500]-> [0,1)
      1.33334 -0.1111*dn + 0.0092*dn^2 -0.000657*dn^3    [0.7500,0.8125]-> [0,1)
      1.23077 -0.0947*dn + 0.0072*dn^2 -0.000483*dn^3    [0.8125,0.8750]-> [0,1)
      1.14285 -0.0816*dn + 0.0058*dn^2 -0.000362*dn^3    [0.8750,0.9375]-> [0,1)
      1.06668 -0.0711*dn + 0.0047*dn^2 -0.000278*dn^3    [0.9375,1.0000]-> [0,1)

    NOTE: The columns in the LUT are scaled to compensate for bit loss in dn^k.
          The Q of each product (ck*dn^k) is Q30..explained in the code
  

    Example:
      input: 3.5625 (57 in Q4)
      output: scale=4, result=0x47dc5800=1205622784 (Q30)
                   shift right by 4 => 0x47dc580=75351424 (Q28=32-4)=0.280706

    @return
    A result/scale pair corresponding to 1/input.

    @note
    The optimized assembly function is 7 cycles.

    Maximum absolute relative error is 1.525879e-5.
*/
/*=========================================================================*/

static Word16 approx_uinv_cubelut8_coefs[8][4] =
{
  {  SIGNED16(65535), -32748, -12909, 32241 }, // c0(Q15), c1(Q17), c3(Q22), c2(Q20)
  {  SIGNED16(58254), -25881,  -8266, 22715 }, // c0 is unsigned
  {  SIGNED16(52429), -20969,  -5536, 16597 },
  {  SIGNED16(47663), -17332,  -3846, 12491 },
  {  SIGNED16(43691), -14564,  -2754,  9635 },
  {  SIGNED16(40330), -12409,  -2024,  7586 },
  {  SIGNED16(37449), -10698,  -1520,  6079 },
  {  SIGNED16(34953),  -9323,  -1164,  4946 }
  
};

result_scale_t approx_uinv_cubelut8(UWord32 input)
{
  // Local variables: explanation appears near first usage 
  UWord32 d, sf, ind, result;
  Word32 dn1, dn2, dn3, sum0, sum1, sum3;
  Word16 *coefs;
  result_scale_t output;

  // Handle input=0. Report scale=0,result=0xFFFFFFFF 
  if (input==0) {
    output.result = 0xFFFFFFFFUL;
    output.scale = 0;
    return (output);
  }

  // Count #leading 0s in input
  sf = cl0(input);

  // Normalize input by scaling by 2^cl0. Norm. input=d is interpreted as
  //   Q32. Hence, range of d is [.5,1).
  //   d:   ||1 |   ind     |  dn     ||
  //          31    30-28(3)  27-0(28)     
  d = L_asl(input,sf);

  // Extract 3 MSBs of d after the 1st. ind=d[30:28]  */
  ind = (d >> 28) & 0x7 ;

  //  Load coefficients c0 through c3 as c0,c1,c3,c2 i.e. c0 in lowest hw, c1
  //  in the next, c3 in the penultimate and c2 in highest hw */
  coefs = approx_uinv_cubelut8_coefs[ind];

  // Extract 28 bits of the fraction dn. dn=d[27:0]..Q28.
  // Need to clear dn first for insert op.
  // Offset of 1 =>dn..Q28 to dn..Q29  
  dn1 = (d & 0x0FFFFFFF)<<1;

  // Calculate dn^2..Q26(=2*29-32) 
  dn2 = L_imult32h(dn1,dn1);

  //Note: to utilize  Q6_P_vmpywoh_PP_rnd_sat in assembly
  //  coefs.d= || c2  |  c3  |  c1  |  c0   ||
  //   dn_k.d= ||   dn^2     |  dn          ||
  //=>sum.d  = ||  c2*dn^2   | c1*dn        ||
  //  Qs     =  Q30(=20+26-16)  Q30(=17+29-16)
  // 
  sum0 = L_imult32hr( dn1, (coefs[1]<<16) );
  sum1 = L_imult32hr( dn2, (coefs[3]<<16) ); 

  // Calculate dn^3..Q23(=26+29-32) 
  dn3 = L_imult32h(dn1,dn2);

  // Calculate c0<<15 +c1*dn..Q30(15+15) 
  sum0 += (Word32)((UWord16)coefs[0]) << 15;

  // Calculate c3*fn^3..Q30(=22+23-16+1) 
  sum3 = L_mult32x16r( dn3, coefs[2] );

  // Calculate result =c0<<15 +c1*dn^1 +c2*dn^2 +c3*dn^3..Q30 
  result = sum0 + sum1 + sum3 ;

  // scale factor(SF)=30-cl0 ..lies in the range [-1,30].
  // The result, after a shift by SF, is in Qo but will never overflow 32 bits
  // in case of a left shift by 1 
  sf = 30 - sf;

  output.result = result;
  output.scale = sf;

  return ( output );
}/* approx_uinv_cubelut8 */




/*===========================================================================

  FUNCTION:  approx_uinv_newt

===========================================================================*/
/*
    @brief
    Approximate unsigned-inverse using Newton-Raphson iterative method.

    @detail
    Given an unsigned input and an iteration count, return an approximation of
    1/input, and a scale factor.
    If input is 0, the output returned is result=0xFFFFFFFF, scale=0.
    Valid input range is 0-0xFFFFFFFF. The iteration count must be in [1,3].

    With the input in Qi, the output is Qo=32-Qi. Right-shifting the result by
    the scale, converts the result to Qo.

    Note that the scale is signed & can be negative (scale=-1 for input=1). The
    >> C operator can't handle negative operands and alternately the intrinsic
    Q6_lsr_RR() may be used. Even in the case of a 1 bit left shift, the result
    wont overflow 32 bits.

    Newton's recursive equation for the solution of f(x)=1/x-d is
    x[n+1] =  2*x[n] - d*x[n]^2 where,
      d = normalized input in range [.5,1)
      x[n]= current solution of 1/x -d = 0

    The first approximation for inverse is x[0]=2.9142-2*d, which tracks actual
    inverse 1/d closely, for d in [.5,1).

    Newton's modified iterative solution for 1/d:
    x^2 loses 1 bit compared to x, for e.g x in Q31, x^2 is Q30. d*x^2 loses an
    additional bit as d is Q31. To keep the bit loss to a predictable 1 bit per
    iteration, modify the equation as
    x[n+1]=2*x[n] - d*x[n]*(x[n]<<n).
    With the 2nd term in Q-1, x[n] in Q can be interpreted as 2*x[n] in Q-1
    giving x[n+1] in Q-1, avoiding additional shift operations in the loop.
    Thus, x will be down to Q=31-'iterations' at the end.

    Loop instability:
    If x becomes >=2.0 in a stage (Q is variable in the loop), xp will overflow
    32 bits in the next iteration and the 2nd term becomes negligible compared
    to x. MSBs of x will get stuck at their value and we wont see the expected
    degradation in Q of x. Interpreting this result as Q=Q31-'iterations' is
    then incorrect.
    The solution is to signed saturate the result after a left shift of
    'iterations'-1. This gives the result in Q30, which saturates at 0x7FFFFFFF
    =1.999(Q30) and we get a stable result.


    Example:
       input: 3.5625 (57 in Q4)
       output: scale=4, result=0x47dc11f8=1205604856 (Q30)
               shift right by 4 => 0x47dc120=75350304 (Q28=32-4)=0.280702

    @return
    A result/scale pair corresponding to 1/input.

    @note
    The optimized assembly function is 4+2*(iterations) cycles.

    Maximum absolute relative error : 7.3e-3  6E-5  5.5E-9
                          iterations: 1       2     3
                              cycles: 6       8     10
    Quantization noise makes error worse at more than 3 iterations.
*/
/*=========================================================================*/
result_scale_t approx_uinv_newt(UWord32 input, UWord32 iter)
{
  // Local variables: explanation appears near first usage 
  UWord32 n, d, sf, result;
  Vect64 x, a, xp;
  result_scale_t output;

  // Handle input=0. Report scale=0,result=0xFFFFFFFF 
  if (input==0) {
    output.result = 0xFFFFFFFFUL;
    output.scale = 0;
    return (output);
  }

  // Count #leading 0s in input
  sf = cl0(input);

  // Normalize input by scaling by 2^cl0. Normalized input is interpreted as 
  //   Q32. Hence, range of d is [.5,1). 
  d = L_asl(input,sf);

  // Calculate initial approx. for 1/d as x=2.9142-2*d.
  // Const. term 1963229551UL is .9142 in Q31. d in Q32 can be interpreted as
  // 2*d if d were in Q31 w/o additional operations. Use the trick that 2-x=-x
  // for x in Q31. Thus, x=2.9142-2*d same as x=0.9142-2*d in Q31 

  x.uw[0] = 0;
  x.uw[1] = 1963229551UL - d ;

  // Newton's modified iterative solution for 1/d:
  // x[n+1]=2*x[n] - d*x[n]*(x[n]<<n).
  // Refer prototype for details on modified equation. 
  for (n=0; n < iter; n++) {
    // Calculate x[n]<<n  ..always stays at Q31 
    xp.d = LL_shl(x.d, n);

    // Calculate a=(d*x[n])>>32  ..Q =32+(31-n)-32= 31-n 
    a.d = LL_imult32u(d, x.uw[1]);

    // Calculate x[n+1] = 2*x[n]-d*x[n]*xp[n]  ..Q =(31-n)+31-32= 31-(n+1) 
    x.d -= LL_imult32u(a.w[1], xp.uw[1]);
  }

  // shift factor(SF)=30-cl0 ..lies in the range [-1,30].
  // The result, after a shift by SF, is in Qo but will never overflow 32 bits
  // in case of a left shift by 1 
  sf =30 - sf;

  // result is in Q(=31-'iter'). We get back the result in Q30 by left
  // shifting 'iter'-1 bits and saturating(signed), clipping any  values larger
  // than 1.9999 
  result = L_shl(x.w[1],n-1);

  // Format output 
  output.result = result;
  output.scale = sf;

  return ( output );
}/* approx_uinv_newt */





/*===========================================================================

  FUNCTION:  approx_uinv_tylr

===========================================================================*/
/*
    @brief
    Approximate unsigned-inverse using Taylor's series expansion.

    @detail
    Given an unsigned input and an iteration count, return an approximation of
    1/input, and a scale factor. The iteration count corresponds to the loop 
    that accumulates 1 term of the Taylor series expansion per pass.
    If input is 0, the output returned is result=0xFFFFFFFF, scale=0.
    Valid input range is 0-0xFFFFFFFF. The iteration count must be in [3,8].

    With the input in Qi, the output is Qo=32-Qi. Right-shifting the result by
    the scale, converts the result to Qo.

    Taylor's expansion of f(x)=1/(1-x)..|x| < 1:
        f(x)=1+x+x^2+x^3+..ad.inf
        Above is truncated to a fixed length=1+'iterations'. The noise added
        is directly proportional to |x|^(1+'iterations').
    Let a=1-d*x  ..lim a->0
    => d*x = 1-a  OR
    1/d = x/(1-a)
        = x*(1+a+a^2+a^3+..a^hi_pow) where,
      d = normalized input in [.5,1)
      x = init. approx for 1/d,

    If input is 0, the output is result=0xFFFFFFFF, scale=0.

    If the input is Qi, the output is Qo=32-Qi.
    Shifting right by scale, converts reported Q30 output to Qo.

    Choice of initial approximation:
    The first approximation for inverse is x=2.9142-2*d, as seen in Newton's
    method,  but this gives signed result for a=1-d*x which cannot be
    accomodated in a Q32 unsigned accumulator w/o additional shift operations.
    Hence choose x=2.8283-2*d, which gives unsigned a=1-d*x but at the expense
    of eror symmetry at two extremes of range of d. Hence for symmetric error,
    use x = 2.666 - 1.7778*d,
    which gives same value for a=1-d*x at either extremes .5 and 1


    Example:
      input: 3.5625 (57 in Q4)
      output: scale=5, result=0x8fb8235d=2411209565 (Q31)
                   shift right by 5 => 0x47dc11b=75350299 (Q28=32-4)=0.280702
                   NOTE: The result is twice the magnitude, compared to other 
                   algorithms and so the scale is also 1 more.

    @return
    A result/scale pair corresponding to 1/input.

    @note
    The optimized assembly function is 6+(hi_pow) cycles.

    Max absolute relative error: 1.6-4  1.8e-5  2.0e-6  2.5e-7  3.5e-8  3.5e-9
                  highest power: 3      4       5       6       7       8
                         cycles: 9      10      11      12      13      14
    Competitive only for hi_pow=4 but 2 cycles more than newt_2iter and 3 more 
    than cube8fit

*/
/*=========================================================================*/
result_scale_t approx_uinv_tylr(UWord32 input, UWord32 hi_pow)
{
  // Local variables: explanation appears near first usage 
  UWord32 d, sf, a, n, result;
  Vect64 sum, x, a_k;
  result_scale_t output;


  // Handle input=0. Report scale=0,result=0xFFFFFFFF 
  if (input==0) {
    output.result = 0xFFFFFFFFUL;
    output.scale = 0;
    return (output);
  }

  // Count #leading 0s in input
  sf = cl0(input);

  // Normalize input by scaling by 2^cl0. Normalized input is interpreted as 
  // Q32. Hence, range of d is [.5,1). 
  d = L_asl(input,sf);

  // Calculate initial approx. for 1/d as x=2.666 - 1.778*d
  // Const. terms in Q31: c=1430224110UL=.666 & m=3818258432UL=1.778.
  // d in Q32 can be interpreted as 2*d in Q31. Use the trick that 2-x=-x for x
  // in Q31. Thus, x= 2.666 - 1.778*d = 0.666 - 1.778*d in Q31. 
  x.uw[1] = 1430224110UL;
  x.uw[0] = 0;
  x.d -= LL_imult32u(3818258432UL,d);

  // Calculate a=1-d*x. With a<1,
  // 1/(1-a)~=1+a+a^2+a^3+..a^hi_pow. Then,
  // 1/d = x/(1-a) ~= x*(1+a+a^2+a^3+..a^hi_pow).
  // Q of a =32+31-32= 31 
  sum.d = 0x8000000000000000LL - LL_imult32u(d,x.uw[1]);

  // Convert a from Q31 to Q32 by a<<1 to avoid 1-bit loss for every calculated
  // higher power of a. If a is in Q31, a^2 is in Q30,a^3 is Q29 and so on 
  sum.d <<=1;
  a = sum.uw[1];

  // Initialize the a^k=a for k=1. a^k is in Q32 always 
  a_k.d = sum.d;

  // Calculate 1/(1-a) -1 =a+a^2+a^3+..a^hi_pow 
  for (n=0; n < hi_pow -1; n++) {
    sum.d += LL_imult32u(a_k.uw[1], a);
    a_k.d  = LL_imult32u(a_k.uw[1], a);
  }

  // shift factor(SF)=31-cl0 ..lies in the range [0,31].
  // The result, after a shift by SF, is in Qo. There is no chance of overflow
  // as the shift is always to the right 
  sf =31 - sf;

  // Calculate final result:
  //   1/d = x/(1-a)
  //      ~= x*(1+a+a^2+a^3+..a^hi_pow) 
  //       = x + x*(a+a^2+a^3+..a^hi_pow) 
  sum.d = LL_imult32u(sum.uw[1], x.uw[1]);
  result = sum.uw[1] + x.uw[1];

  // Format output 
  output.result = result;
  output.scale = sf;

  return ( output );
}/* approx_uinv_tylr */





/*===========================================================================

  FUNCTION:  approx_dB_to_ulin_linlut8

===========================================================================*/
/*
    @brief
    dB to linear conversion using linear 8-segment lookup table.

    @detail
    Given a signed input value( in dB..Q24), return an approximation of
    10^(input/10), and a scale factor.

    Details common to all approx_dB_to_ulin functions:
      The scale_factor is signed and lies in the range [-32,31].
      The scale_factor corresponds to the right shift amount in a 64-bit shift
      operation on the result & interpreting the resultant 64 bits
      as a Q32 number. For other output Q-factors, the scale_factor is to be
      adjusted appropriately (ensure no overflow/underflow).
      For e.g. Q16 result can be obtained w/o underflow for inputs in range
      -48.1647dB:127dB with right shift amount given by:
      right shift = scale_factor + 16.
  
      Considering only the positive input dB range upto 96.32dB, no shift is
      required if we treat the result as floating point. The Qo is given by:
      Qo = 32 + scale_factor..Qo is in the range [0,31]

      Valid input range is Q24 numbers corresponding to real values within
      +/-96.32 (dB range corresponding to linear 32-bit no. range). Inputs between
      96.33dB:127dB or -128dB:-96.33dB can be used, but the right shift for the
      result in 64 bits, now interpreted as Q21, is given by:
      right shift =scale_factor + sign(scale_factor)*11

      Consider, 10^y = 2^x with y = input/10
      => x = log2(10)*y = 3.321928*y
           = .3321928*input = i + f ..i is an integer & f is in [0,1)

      Thus, a power of 10 is converted to a power of 2, where the integer part is
      simply a bit-shift amount. Calculate 2^f using linear interpolation
      & bit-shift by 'i' to get the final result.

    Details specific to approx_dB_to_ulin_linlut8():
    The first 3 bits of f are used for a rough estimate of 2^f. A LUT with a
    3-bit index is used for this purpose. The fractional range is split into
    the 8 segments (hence the suffix 8): [0.0, 0.125),[0.125, 0.25),...,
    [0.875,1.0)

    The LUT contains c & m that satisfy the following equation:

    2^f = c + m*fn
    where,
      d       = .3321928*input  = i + f..Q25
      i       = integral part of d  = d[31:24] ..in range [-32,31]
      f       = fractional part of d= d[23:0]..Q24
      ind     = LUT index for the segment = d[23:21] in range [0, 7]
      fn      = d[20:6] in Q15..fraction left after extracting 'ind' from f
      c[ind] ~= 2^( ind/2^3 )..Q15
      m      ~= c[ind+1] - c[ind]..Q16

                     <-------f------------------->
    d:   ||  i      |  ind     | fn      |ignored ||
           31:24(8)   23:21(3)  20:6(15)  5:0(6)

    Final result:
    10^(input/10) = ( 2^f ) << i = (c + m*fn) << i

    Example:
    ||       input                  ||                  output               ||
    |      dB..Q24      | i  | f    || (scale, result)   |  2^f  |Final result|
-------------------------------------------------------------------------------
    -96.00dB=0xa0000000 =-32+ 0.1095 =>(31, 0x8a189ac0)  =1.0789 =2.511954e-10
    -16.00dB=0xf0000000 =-6 + 0.6849 =>(5, 0xcddfb7c0)   =1.6084 =2.513109e-02
    0.00dB=0x       0   =00 + 0.0000 =>(-1, 0x7ff00000)  =0.9995 =9.995117e-01
    16.00dB=0x10000000  =05 + 0.3151 =>(-6, 0x9f509220)  =1.2446 =3.982868e+01
    96.00dB=0x60000000  =31 + 0.8905 =>(-32, 0xed4130c0) =1.8536 =3.980473e+09

    @return
    A result corresponding to 10^(input/10).

    @note
    The optimized assembly function is 5 cycles.

    Maximum absolute relative error is 5.8121e-04
*/
/*=========================================================================*/
static UWord32 approx_dB_to_ulin_linlut8_LUT[8] =
{
    5931| (32752<<16),  // m(Q16) | c(Q15) <<16
    6467| (35717<<16),
    7053| (38949<<16),
    7691| (42474<<16),
    8387| (46319<<16),
    9146| (50511<<16),
    9974| (55082<<16),
   10877| (60068<<16)
};

result_scale_t approx_dB_to_ulin_linlut8(Word32 input)
{
  // Local variables: explanation appears near first usage 
  UWord32 ind, fn, result;
  Word32 d;
  result_scale_t output;

  //.1*log2(10)=.3321915, in Q17 is 0x0AA15 (43541)
  // Calculate d = i+f = input*.3321928..Q24(=24+17+15-32)
  d = L_imult32h(input, (0x0AA15<<15));
  
  //  
  //                   <-------f------------------->
  //  d:   ||  i      |  ind     | fn      |ignored ||
  //         31:24(8)   23:21(3)  20:6(15)  5:0(6)
  //

  // Extract 3bits of d as ind. ind=d[23:21] 
  ind = (d >> 21) & 0x7;

  //  Load coefficients: c in higher hword, & m in the lower hword */
  // Initialize the result. high hw=c & low hw=m.
  // => result=c<<16 + m..Q31(15+16) */
  result = approx_dB_to_ulin_linlut8_LUT[ind];

  // Extract 15 bits of d as the fraction fn. fn=d[20:6]..Q15.
  // Offset of 6 corresponds to ignored LSBs 
  fn = (d >> 6) & 0x7FFF;

  // i=d[31:24]..lies in range [-32, 31] 
  d >>= 24;

  // Subtract 1 from fn to compensate for m already loaded into the result 
  fn -= 1;

  // Calculate result=c<<16 + m*fn
  //                 =(c<<16 + m) + m*(fn-1)..Q31(16+15) 
  result += extract_l(result) * fn ;

  // scale = -1-i..lies in range [-32,31]..mapping to range of i is reversed */
  output.result = result;
  output.scale =  -1 -d ;

  return ( output );
}/* approx_dB_to_ulin_linlut8() */





/*===========================================================================

  FUNCTION:  approx_dB_to_ulin_cubefit

===========================================================================*/
/*
    @brief
    dB to linear conversion using a single cubic polynomial fit.

    @detail
    Given a signed input value( in dB), return an approximation of
    10^(input/10), and a scale factor.

    Please refer the details common to all approx_dB_to_ulin functions in
    approx_dB_to_ulin_linlut8() prototype.

    The cubic polynomial fit for 2^f is as follows:

          c0       c1         c2           c3
    2^f = 0.9998 + 0.6968*f + 0.2242*f^2 + 0.078979*f^3
    where,
        d = .3321928*input  = i + f..Q16
        i = integral part of d  = d[31:16]..in range [-32,31]
        f = fractional part of d= d[15:0]..Q16

    d:   ||  i      |   f      ||
           31:16(16)   15-0(16)

    Final result:
    10^(input/10) = ( 2^f ) << i = (c0 + c1*f +c2*f^2 + c3*f^3) << i

    Example:
    ||       input                  ||                  output               ||
    |      dB..Q24      | i  | f    || (scale, result)   |  2^f  |Final result|
-------------------------------------------------------------------------------
    -96.00dB=0xa0000000 =-32+ 0.1095 =>(31, 0x8a1b2575)  =1.0790 =2.512134e-10
    -16.00dB=0xf0000000 =-6 + 0.6849 =>(5, 0xcdc7d1a6)   =1.6077 =2.511970e-02
    0.00dB=0x       0   =00 + 0.0000 =>(-1, 0x7ffc0000)  =0.9999 =9.998779e-01
    16.00dB=0x10000000  =05 + 0.3151 =>(-6, 0x9f405f29)  =1.2442 =3.981286e+01
    96.00dB=0x60000000  =31 + 0.8905 =>(-32, 0xed4d9ed2) =1.8539 =3.981287e+09

    @return
    A result corresponding to 10^(input/10).

    @note
    The optimized assembly function is 6 cycles.

    Maximum absolute relative error is 1.3463e-04
*/
/*=========================================================================*/
result_scale_t approx_dB_to_ulin_cubefit(Word32 input)
{
  //Q15:c0 ,c1  , c2  c3
  UWord16 coefs[4] = { 32764, 22833, 7346, 2588 };
  UWord16 f1, f2, f3;
  Word32 d;
  UWord32 sum;
  result_scale_t output;

  // d = input * .3321928 (in Q24)
  d = L_imult32h(input, 5573270); 

  f1 = (UWord16)extract_l(d);

  // Init. accumulator to c0<<16..Q31(=15+16) 
  sum = coefs[0]<<16;

  // Add c1*f..Q31(=15+16) to accum. 
  sum += f1 * coefs[1];

  // Calculate f^2..Q32 
  f2 = (UWord16)extract_h(f1 * f1);

  // Add c2*f^2..Q31(=15+16) to accum. 
  sum += f2 * coefs[2];

  // Calculate f^3..Q32 
  f3 = (UWord16)extract_h(f1 * f2);

  // i=d[31:16]..lies in range [-32, 31] 
  d >>= 16;

  // result= c0<<16 +c1*f^1 +c2*f^2 +c3*f^3..Q31(=15+16) 
  sum += f3 * coefs[3]; 

  // scale = -1-i..lies in range [-32,31]..mapping to range of i is reversed 
  output.result = sum;
  output.scale =  -1 -d ;
  return (output);
}/* approx_dB_to_ulin_cubefit() */





/*===========================================================================

  FUNCTION:  approx_dB_to_ulin_quadlut8

===========================================================================*/
/*
    @brief
    dB to linear conversion using 2nd degree 8-segment lookup table.

    @detail
    Given a signed input value( in dB), return an approximation of
    10^(input/10), and a scale factor.

    Please refer the details common to all approx_dB_to_ulin functions in
    approx_dB_to_ulin_linlut8() prototype.

    The first 3 bits of f are used to split the range [0,1) into 8 segments
    (hence the suffix 8):[0.0, 0.125),[0.125, 0.25),...,[0.875,1.0). 2^f is
    approximated by a cubic polynomial in each of these segments & a LUT with
    a 3 bit index( MSB=1 always for the norm. input) stores the coefficients.

    The LUT contains c's that satisfy the following equation in each segment:
    2^f = c0 + c1*fn + c2*fn^2,  

    where,
        d   = .3321928*input  = i + f..Q24
        i   = integral part of d  = d[31:24] ..in range [-32,31]
        f   = fractional part of d= d[23:0]..Q24
        ind = LUT index for the segment = d[23:21] in range [0, 7]
        fn  = d[20:0] in Q21..fraction left after extracting 'ind' from f

                     <-------f----------->
    d:   ||  i      |  ind     | fn       ||
           31:24(8)   23:21(3)  20:0(21)

    The LUT for 2nd degree polyfit coefficients of 2^f is:
      2^f =
       c0        c1          c2          Range of: f   fn
  
      1.0000 + 0.0866*fn + 0.0039*fn^2  [1.000,1.125]-> [0,1)
      1.0905 + 0.0944*fn + 0.0043*fn^2  [1.125,1.250]-> [0,1)
      1.1892 + 0.1030*fn + 0.0047*fn^2  [1.250,1.375]-> [0,1)
      1.2968 + 0.1123*fn + 0.0051*fn^2  [1.375,1.500]-> [0,1)
      1.4142 + 0.1224*fn + 0.0055*fn^2  [1.500,1.625]-> [0,1)
      1.5422 + 0.1335*fn + 0.0061*fn^2  [1.625,1.750]-> [0,1)
      1.6818 + 0.1456*fn + 0.0066*fn^2  [1.750,1.875]-> [0,1)
      1.8340 + 0.1588*fn + 0.0072*fn^2  [1.875,2.000]-> [0,1)

    Final result:
    10^(input/10) = ( 2^f ) << i = (c0 + c1*fn + c2*fn^2) << i

    Example:
    ||       input                  ||                  output               ||
    |      dB..Q24      | i  | f    || (scale, result)   |  2^f  |Final result|
-------------------------------------------------------------------------------
    -96.00dB=0xa0000000 =-32+ 0.1095 =>(31, 0x8a17c28e)  =1.0789 =2.511894e-10
    -16.00dB=0xf0000000 =-6 + 0.6849 =>(5, 0xcdc60cfc)   =1.6076 =2.511885e-02
    0.00dB=0x       0   =00 + 0.0000 =>(-1, 0x8000184e)  =1.0000 =1.000003e+00
    16.00dB=0x10000000  =05 + 0.3151 =>(-6, 0x9f3e30a5)  =1.2441 =3.981073e+01
    96.00dB=0x60000000  =31 + 0.8905 =>(-32, 0xed4a2fbb) =1.8538 =3.981062e+09

    @return
    A result corresponding to 10^(input/10).

    @note
    The optimized assembly function is 7 cycles.

    Maximum absolute relative error is 4.4531e-06

*/
/*=========================================================================*/
static Word16 approx_dB_to_ulin_quadlut8_coefs[8][4] =
//    c0.l  c0.h     c1     c2
//    <----Q31--->   Q18    Q20           
{
  {  6222, 32768, 22697, 4110 },
  { 56918, 35733, 24751, 4483 },
  {  4425, 38968, 26991, 4889 },
  { 63455, 42494, 29434, 5332 },
  {  7301, 46341, 32097, 5818 },
  { 23046, 50535, 35002, 6344 },
  { 12892, 55109, 38170, 6917 },
  {   381, 60097, 41625, 7542 }
};

result_scale_t approx_dB_to_ulin_quadlut8(Word32 input)
{
  // Local variables: explanation appears near first usage 
  UWord32 ind, result;
  Word32 d, sum0, sum1, fn1, fn2;
  Word16 *coefs;
  result_scale_t output;

  //.1*log2(10)= .3321928 in Q32 is 1426757212
  //d = i+f=input*.3321928..Q24
  d = L_imult32h(input, 1426757212); 

  // 
  //                   <-------f----------->
  //  d:   ||  i      |  ind     | fn       ||
  //         31:24(8)   23:21(3)  20:0(21)
  //

  // Extract 3bits of ind. ind=d[23:21]  
  ind = (d >> 21) & 0x7;

  //  Load coefficients c0 through c2 in order i.e. 0 in lowest hw, c0 in the
  //  next one, c1 after that and c2 in the highest hw 
  coefs = approx_dB_to_ulin_quadlut8_coefs[ind];

  // Extract 21 bits of the fraction fn. fn=d[20:0]..Q21.
  // Need to clear fn first for insert op.
  // Offset of 8 =>fn..Q21 to fn..Q29
  fn1 =  (d & 0x1FFFFF)<<8; 

  // i=d[31:24]..lies in range [-32, 31] 
  d >>= 24;

  // Calculate fn^2..Q26(=2*29-32) 
  fn2 = L_imult32h(fn1, fn1);

  // Init higher accumulator word to c0..Q31 
  sum1 = ((Word32)coefs[1] << 16)| (coefs[0] & 0x0000FFFF);

  //  sum1  = ||  c1*fn^1 +  c0<<16
  //  Qs     =    Q31(=18+29-16)
  //  (32s x 16u with rnd)
  sum1 += (Word32)LL_shr( (LL_imult32(fn1, (UWord16)coefs[2]) + 0x00008000), 16);

  // Calculate c2*fn^2..Q31(=20+26-16+1)  
  sum0 = L_mult32x16r(fn2, coefs[3]);

  // Calculate result= sum1+sum0 = c0<<16 +c1*fn^1 +c2*fn^2..Q31 
  result = L_add(sum1, sum0);

  // scale = -1-i..lies in range [-32,31]..mapping to range of i is reversed 
  output.result = result;
  output.scale =  -1 -d ;

  return ( output );
}/* approx_dB_to_ulin_quadlut8() */





/*===========================================================================

  FUNCTION:  approx_ulin_to_dB_linlut8

===========================================================================*/
/*
    @brief
    Linear range to dB conversion using linear 8-segment lookup table.

    @detail
    Given an unsigned input and the its Q-factor, return an approximation
    of 10*log10(input).
    Valid input range is 0 to 0xFFFFFFFF & Q-factor from 0 to 32 ( Q-factors
    outside this range should be checked for overflow/underflow before use).

    The output Q-factor is fixed at Q24. Valid output is in the range
    +/-96.32dB.

    If input is 0, the output returned is 0x80000000=-256dB in Q24.

    Any 32-bit input can be normalized to the range [1, 2) by trimming the
    leading 0s & interpreting the remaining bits (0-padded to get 32-bits)as a
    Q30 number. The first 4 bits of the normalized input are used for a rough
    estimate of 10*log10(norm. input). A LUT with a 3 bit index( MSB=1 always 
    for the norm. input) stores these values. The input range is split into the 
    8 segments (hence the suffix 8):
    [1.0, 1.125),[1.125, 1.25),...,[1.875,2.0)

    The LUT contains c's that satisfies the following equation:
    10*log10(d) = c[ind+1]*dn  + c[ind]*(1-dn) where,
        d       = normalized input in range [1,2)
        ind     = LUT index for the segment = d[30:28] in range [0, 7]
        c[ind] ~= 10*log10( 1+ ind/2^3 ) in Q12
        dn      = d[27:0] in Q28..fraction left after extracting 'ind' from d

    d:   ||1 |   ind     |  dn     ||
           31    30-28(3)  27-0(28)

    The actual 'c' entries have been skewed for symmetric error in each segment.

    Final result:
    10*log10(input) = 10*log10( d*2^-(Qi+cl0-31) )
                    = 10*log10(d) - 3.0103*(Qi+cl0-31)
                    = c[ind+1]*dn  + c[ind]*(1-dn) - 3.0103*(Qi+cl0-31)
                    The last term is the dB offset due to the relative scale
                    difference between the real value and norm. range [1,2) i.e.
                    its 0 for real value of input in [1,2)

    Example:
               input         =>    output..Q24
    --------------------------------------------------
    0xffffffff..Q0= 4.29e+09 	 0x6054bd9b = 96.33102 dB
    0xffffffff..Q32= 1.00e+00 	 0x00005cfb = 0.00142 dB
    0x0000001f..Q0= 3.10e+01 	 0x0ee9a414 = 14.91266 dB
    0x00000001..Q0= 1.00e+00 	 0x00010000 = 0.00391 dB
    0x000000ff..Q16= 3.89e-03 	 0xe7e6c7d3 = -24.09851 dB
    0x00000001..Q32= 2.33e-10 	 0x9fac9f60 = -96.32569 dB

    @return
    A result corresponding to 10*log10(input).

    @note
    The optimized assembly function is 6 cycles.

    Maximum absolute error is less than 0.004012 dB
*/
/*=========================================================================*/
static Word16 approx_ulin_to_dB_linlut8_LUT[9] =
//  Cs in Q12 
{ 16, 2108, 3980, 5674, 7220, 8643, 9961, 11187, 12336 };


Word32 approx_ulin_to_dB_linlut8(UWord32 input, Word32 Qi)
{
  // Local variables: explanation appears near first usage 
  UWord32 d, sf, ind, temp;
  Word32 c0, c1, sum0, sum1, dn, result;

  // Handle input=0. Report result=-256dB in Q24 
  if (input==0) {
    return ((Word32)0x80000000);
  }

  // Count #leading 0s in input (cl0)
  sf = cl0(input);

  // Normalize input by scaling by 2^cl0. Normalized input is interpreted as Q31.
  //   Hence, range of d is [1,2).
  d = L_asl(input,sf);

  // Extract 3 MSBs of d after the 1st. ind = d[30:28] 
  ind = (d >> 28) & 0x7;

  // Store the address of c[ind] to load from later 
  c0 = approx_ulin_to_dB_linlut8_LUT[ind  ];
  c1 = approx_ulin_to_dB_linlut8_LUT[ind+1];

  // Find dn = d[27:0] in Q28
  dn = d & 0x0FFFFFFF;

  // Calculate temp =  3.0103 in Q24
  temp = 50504453;

  Qi += (sf -31);  //Qi = (Qi+cl0-31)

  // Init the accumulator to 3.0103*(Qi+cl0-31) in Q24 
  // Need to add c[ind]..convert Q12 to Q24 
  sum0 = c0 << 12 ;
  sum1 = temp * Qi;

  //  this is to use Q6_P_vmpywehacc_PP_rnd_sat
  //  Calculate c[ind+1]*dn & c[ind]*dn +temp as follows:
  //    log_pair: ||  0 | c[ind]   | 0 | c[ind+1]  ||..'|' denotes hword boundary
  //          dn: ||   dn          |    dn         ||
  // =>   sum1:0: || c[ind]*dn +   | c[ind+1]*dn+  ||
  //                  temp           c[ind]
  //        Qs: =    24(=12+28-16)  24(=12+28-16) 
  sum0 += L_imult32hr( dn, (c1<<16) );
  sum1 += L_imult32hr( dn, (c0<<16) );
  
  // result = c[ind+1]*dn + ( c[ind] - (c[ind]*dn +temp) )
  //        = ( c[ind+1]*dn  + c[ind]*(1-dn) )- temp
  //        = 10*log10(d) - 3.0103*(Qi+cl0-31)
  result = sum0 - sum1;

  return ( result );
}/* approx_ulin_to_dB_linlut8() */





/*===========================================================================

  FUNCTION:  approx_ulin_to_dB_cubelut8

===========================================================================*/
/*
    @brief
    Linear range to dB conversion using cubic 32-segment lookup table.

    @detail
    Given an unsigned input and the its Q-factor, return an approximation
    of 10*log10(input).

    Valid input range is 0 to 0xFFFFFFFF & Q-factor from 0 to 32 ( Q-factors
    outside this range should be checked for overflow/underflow before use).

    The output Q-factor is fixed at Q24. Valid output is in the range
    +/-96.32dB.

    If input is 0, the output returned is 0x80000000=-256dB in Q24.

    Any 32-bit input can be normalized to the range [1, 2) by trimming the
    leading 0s & interpreting the remaining bits (0-padded to get 32-bits)as a
    Q30 number. The normalized input range [1,2) is split into 8 segments using 
    3 MSBs after the 1st (MSB=1 always for norm. input). The corresponding dB 
    value is approximated by a cubic polynomial in each segment as follows:

    10*log10(d) = c0 + c1*dn + c2*dn^2 + c3*dn^3,  

    where,
        d = normalized input in range [1,2)
        dn= d[27:2] in Q26..fraction left after extracting 'ind' from d

    d:   ||1 |   ind     |  dn     |ignored ||
          31    30-28(3)  27-2(26)  1-0(2)

    The LUT for the coefficients of cubic polynomials is:

    10*log10(d) =
      c0        c1         c2          c3           Range of: d        dn
  
    0.0000 + 0.5428*dn -0.0336*dn^2 + 0.002363*dn^3 [1.000,1.125]-> [0,1)
    0.5115 + 0.4825*dn -0.0266*dn^2 + 0.001692*dn^3 [1.125,1.250]-> [0,1)
    0.9691 + 0.4343*dn -0.0216*dn^2 + 0.001253*dn^3 [1.250,1.375]-> [0,1)
    1.3830 + 0.3948*dn -0.0179*dn^2 + 0.000953*dn^3 [1.375,1.500]-> [0,1)
    1.7609 + 0.3619*dn -0.0150*dn^2 + 0.000742*dn^3 [1.500,1.625]-> [0,1)
    2.1085 + 0.3341*dn -0.0128*dn^2 + 0.000589*dn^3 [1.625,1.750]-> [0,1)
    2.4304 + 0.3102*dn -0.0110*dn^2 + 0.000475*dn^3 [1.750,1.875]-> [0,1)
    2.7300 + 0.2895*dn -0.0096*dn^2 + 0.000389*dn^3 [1.875,2.000]-> [0,1)


    Final result:
    10*log10(input) = 10*log10( d*2^-(Qi+cl0-31) )
                    = 10*log10(d) - 3.0103*(Qi+cl0-31)
                    = c0 + c1*dn + c2*dn^2 + c3*dn^3 - 3.0103*(Qi+cl0-31)
                    The last term is the dB offset due to the relative scale
                    difference between the real value and norm. range [1,2) i.e.
                    its 0 for real value of input in [1,2)

    Example:
               input         =>    output..Q24
    --------------------------------------------------
    0xffffffff..Q0= 4.29e+09 	 0x60546034 = 96.32959 dB
    0xffffffff..Q32= 1.00e+00 	 0xffffff94 = -0.00001 dB
    0x0000001f..Q0= 3.10e+01 	 0x0ee9e18c = 14.91360 dB
    0x00000001..Q0= 1.00e+00 	 0x00000000 = 0.00000 dB
    0x000000ff..Q16= 3.89e-03 	 0xe7e68d5a = -24.09941 dB
    0x00000001..Q32= 2.33e-10 	 0x9fab9f60 = -96.32960 dB

    @return
    A result corresponding to 10*log10(input).

    @note
    The optimized assembly function is 7 cycles.

    Maximum absolute error is less than 0.0000483 dB
*/
/*=========================================================================*/

static Word16 approx_ulin_to_dB_cubelut8_coefs[8][4] =
// c0 & c3 are unsigned
//    c0             c1       c3             c2   
//    Q14            Q13      Q24            Q19    
{
  { SIGNED16(    0), 4447, SIGNED16(39647), -17639 },
  { SIGNED16( 8381), 3953, SIGNED16(28385), -13960 },
  { SIGNED16(15877), 3558, SIGNED16(21015), -11321 },
  { SIGNED16(22660), 3234, SIGNED16(15992),  -9365 },
  { SIGNED16(28850), 2965, SIGNED16(12450),  -7875 },
  { SIGNED16(34546), 2737, SIGNED16( 9882),  -6714 },
  { SIGNED16(39820), 2541, SIGNED16( 7974),  -5791 },
  { SIGNED16(44728), 2372, SIGNED16( 6527),  -5047 }
};

Word32 approx_ulin_to_dB_cubelut8(UWord32 input, Word32 Qi)
{
  // Local variables: explanation appears near first usage 
  UWord32 d, sf, ind;
  Word32 temp, sum0, sum1, dn1, dn2, dn3, result;
  Word16 *coefs;

  // Handle input=0. Report result=-256dB in Q24 
  if (input==0) {
    return ((Word32)0x80000000);
  }

  // Count #leading 0s in input
  sf = cl0(input);

  // Normalize input by scaling by 2^cl0. Normalized input is interpreted as
  //   Q32. Hence, range of d is [1,2).
  //     d:   ||1 |   ind     |  dn     |ignored ||
  //           31    30:28(3)  27:2(26)  1:0(2)
  d = L_asl(input,sf);

  // Extract 3 MSBs of d after the 1st. ind=d[30:28] 
  ind = (d >> 28) & 0x7;

  //  Load cubic poly. coefficients in the order c0, c1, c3, & c2. i.e
  //    c0 in lowest hword, c1 in the next, then c3 and c2 in the highest hword
  coefs = approx_ulin_to_dB_cubelut8_coefs[ind];

  // Extract the 26 bit fraction dn from d ignoring 2 LSBs. Result is more 
  // sensitive to cubic coef. quantization; we allocate more bits for coefs and
  // less to dn to maintain output Q at Q24
  dn1 = (d >> 2) & 0x03FFFFFF;

  // Calculate dn^2..Q20(=2*26-32) 
  dn2 = L_imult32h(dn1, dn1);

  temp = -50504453;//-3.0103 in Q24

  Qi += (sf -31);  //Qi = (Qi+cl0-31)

  // sum1:0  = || c2*dn^2 - norm_offset  | c1*dn^1     ||
  //     Qs: =    Q24(=19+20-16+1)         Q24(=13+26-16+1)
  // where -norm_offset =-3.0103*(Qi+cl0-31) 
  
  sum0 = 0        + L_mult32x16r(dn1, coefs[1]);
  sum1 = temp*Qi  + L_mult32x16r(dn2, coefs[3]);

  // Calculate dn^3..Q15(=20+26-16-16+1)
  dn3 = L_mult32x16r(dn2,extract_h(dn1));

  // Scaling factor for c0 Q14 to Q24 conversion = 2^10 
  //    sum1:0= || c3*dn^3 +   | c0 +        ||
  //            || c2*dn^2 -   | c1*dn^1     ||
  //            || norm_offset |             ||
  //    Qs: =   Q24(=24+15-16+1)   Q24(=14+25-16+1)
  // Note: using 32x16u multiplication (fractional)
  sum0 +=  LL_shr( (LL_imult32((1<<25), (UWord16)(coefs[0])) + 0x00004000), 15) ;
  sum1 +=  LL_shr( (LL_imult32(    dn3, (UWord16)(coefs[2])) + 0x00004000), 15) ;

  // result= c0 + c1*dn^1 + c2*dn^2 + c3*dn^3 - norm_offset
  //   ..in Q24
  result = L_add(sum0, sum1);

  return ( result );
}/* approx_ulin_to_dB_cubelut8 */






/*===========================================================================

  FUNCTION:  approx_atan2_poly7

===========================================================================*/
/*
    @brief
    Approximate atan2(y,x)/pi using 7th degree polynomial approximation.

    @detail
    Given a pair of signed inputs, return an approximation of atan2(y,x)/pi.
    If the input pair is (0,0), the output returned is 0.
    Valid input range is 0-0xFFFFFFFF

    Adjust the inputs so that they are in the same Q-factor Qi. The output is
    Qo=31. The numerical value returned is in the range (-1,1] (as a multiple
    of pi).

    The input pair represents a vector in the coordinate space. With both
    x, y >=0, atan2(y,x)=atan(y/x). In general,
      atan2(y,x)= +/- atan(|y/x|) +/- pi/2 * (0/1/2).

    Hence we project the vector onto the 1st quadrant to calculate atan(|y/x|),
    while the other coefficients used above are calculated from the signs of x
    & y. Once in the 1st quadrant, we need to go a step further & reflect the
    vector about y=x line if |y|>|x| as we can now narrow down the input range
    for atan(f) to f in [0,1] instead of f in [0,inf]..f=|y/x|. This change can
    be undone by flipping the sign & adding pi/2 i.e.
      angle_1st_quad = pi/2 -angle_1st_oct if |x|<|y|
                     = angle_1st_oct otherwise.

    atan(f) is approximated using a 7th degree polynomial approximation.
    The expression used for the complete inverse tranformation is as follows:

    Actual_angle= (1-2*s3)*angle_1st_oct + offset where,

      Actual_angle = atan2(y, x)/pi
      angle_1st_oct =atan2(|y/x|)/pi
      offset = .5*(2*is_y_ge_0-1)( is_ay_gt_ax + 2* !(is_ay_gt_ax|is_x_ge_0) )
      (Listed below are those variables derived from the sign of x & y.)
      s3 =  is_x_ge_0^is_y_ge_0^is_ay_gt_ax
      is_x_ge_0 = (x>=0)..as 0/1
      is_y_ge_0 = (y>=0)..as 0/1
      is_ay_gt_ax=(|x|<|y|)..as 0/1


    Example:
      input:y=-2147483648 x=-2147483648

      output: -1610611935 = 0xA0000321 =-7.499996e-01..Q31 (pi * -.75)

    @return
    A result corresponding to atan2(y,x)/pi.

    @note
    The optimized assembly function is 22 cycles.

    Maximum absolute relative error is 3.9419e-007.
*/
/*=========================================================================*/
static Word32 approx_atan2_poly7_coefs[] =
/******************************************************************
 Coefficients in the 7th degree polynomial approximation of
 proj_angle = atan(f)/pi,  where f in [0,1]
 proj_angle = 9.586300e-8 + 3.183e-1*f^1 -4.650e-5*f^2 -1.048e-1*f^3 -
             1.024e-2*f^4 +1.011e-1*f^5 -7.122e-2*f^6 +1.685e-2*f^7

 Order is c1:c7 & c0
 Qs (in the same order) =[(24:2:36) 31] 
*******************************************************************/
{
  5340306, -3121, -28127105, -10989807,
  434352049, -1223588557, 1157799985, 206
};


Word32 approx_atan2_poly7(Word32 y, Word32 x)
{
  Word8 is_x_ge_0, is_y_ge_0, is_ay_gt_ax;
  Word32 i, offset, xn;
  Word64 frac, sum;
  result_scale_t den;

  is_y_ge_0 = (y >= 0);  
  is_x_ge_0 = (x >= 0);  

  x = L_abs(x);  
  y = L_abs(y);  

  // is_ay_gt_ax = (|x|<|y|)?
  is_ay_gt_ax = ( y > x );  

  // if |x|<|y|, swap the values to get the bigger value as the divisor.
  // This gives |f|<=1 & projects the angle into the 1st octant 
  if (is_ay_gt_ax) 
  {
    offset = x;
    x = y;
    y = offset;
  }

  // if max(|x|,|y|)==0 then actual angle =0 
  if (x==0) 
     return(0);

  // Approximate 1/|x| using approx_uinv_newt with 3 iterations 
  // (for max accuracy) 
  den = approx_uinv_newt(x ,3);

  // frac = |y|* 1/|x|..Q32+scale(=32+scale-Qi+Qi). 
  // Refer approx_uinv_newt() for details on formula 
  // for returned Q used above 
  frac = LL_imult32u(den.result, y);

  // scale+=2 to ensure unsigned product is within +/-1 in Q30. 
  // Now, frac is in Q30+scale 
  den.scale +=2;

  // f..Q30+scale --> f..Q30 (f=frac.w[0]) 
  x = (Word32)LL_shr(frac, den.scale);

  xn = x;  
  sum = 0;  //init sum of polynomial approximation

  for (i=0;i<7;i++){
      sum += LL_imult32(xn, approx_atan2_poly7_coefs[i]);   //sum += c(i+1)*f^(i+1)
      xn = L_imult32h(xn, x);                              //f^(i+2)..Q28-2i
  }

  // We have sum = c1*f + ..+c7*f^7. But proj_angle = c0+ sum.
  // Load c0 into frac & add to sum after Q-factor alignment i.e.
  // sum..Q54 --> sum..Q31 
  y = (Word32)(approx_atan2_poly7_coefs[7] + LL_shr(sum, 23 ));

  // Calculate the multiple of 90 deg. to re-rotate the projected angle in
  // 1st octant back to the original quadrant.
  // offset = (2*is_y_ge_0-1)*.5*(is_ay_gt_ax + 2*!(is_ay_gt_ax | is_x_ge_0))
  // NOTE: The conditional variables is_XXXX are treated as 0/1 above  
  if (is_ay_gt_ax) {
    offset = 0x40000000;  //.5..Q31 (for +pi/2)
  }else if (!is_x_ge_0) {
    offset =(Word32)0x80000000; //-1..Q31 (for pi; pi=-pi due to wraparound)
  }else {
    offset = 0;
  }
  // Assign the correct sign to the pi/2 multiple 
  if (!is_y_ge_0) {
    offset = -offset;
  }

  // actual_angle= (1-2*s3)*proj_angle + offset..Q31
  // s3 = is_y_ge_0^is_x_ge_0^is_ay_gt_ax
  // NOTE: The conditional variables are treated as 0/1 above 
  if ((is_y_ge_0 ^ is_x_ge_0) ^is_ay_gt_ax) {
    y = offset - y;
  }else {
    y = offset + y;
  }
  return (y);
}
