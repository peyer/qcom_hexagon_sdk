/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 

typedef signed   char        Word8;
typedef unsigned char        UWord8;
typedef short int            Word16;
typedef unsigned short       UWord16;
typedef int                  Word32;
typedef unsigned int         UWord32;
typedef signed long long     Word64;
typedef unsigned long long   UWord64;
typedef int                  CWord2x16;
typedef long long            CWord2x32;

typedef struct
{
  Word32 result;    /*!< Result value */
  Word32 scale;     /*!< Scale factor */
} result_scale_t;


/*! 
Approximates inversion of a 32-bit positive number

\param x number to be inverted

\details
\return
  64-bit integer where the high 32-bit is shift factor
  and the low 32-bit is Q-shifted inverse

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 6 

\b Notes
 - Approximation done with a linearly interpolated lookup table.

 - Data format 

\par
   IF input is in Qn and output in Qm, then
   m = 30 - SF - n .
\par
   For example, 
\par
   input is 0x0F000000 in Q26, i.e., 3.75
\par
   return values are
   result = 0x44730000, SF = -28
\par
   Thus, m = 30-(-28)-26 = 32, i.e.,
   inversion is  (0x44730000)/2^32 = 0.266464

*/
result_scale_t dsplib_approx_invert(Word32 x);


/*! 
Inversion of a 16-bit positive number

\param x number to be inverted

\details
\return
  64-bit integer where the high 32-bit is shift factor
  and the low 16-bit is Q-shifted inverse

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 18 

\b Notes
 - Inversion has 15-bit full precision.

 - Data format 

\par
   IF input is in Qn and output in Qm, then
   m = SF - n .
\par
   For example, 
\par
   input is 0x0F00 in Q10, i.e., 3.75
\par
   return values are
   result = 17476, SF = 26 
\par
   Thus, m = 26 - 10 = 16, i.e.,
   inversion is  17476/2^16 = 0.2666626

*/
result_scale_t dsplib_invert(Word16 x);		


/*! 
Approximates sqrt(x)

\param x input number in Q0
\param rnd_fac rounding factor


\details

\return
sqrt(x) in Q16

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 8

\b Notes
 - ref. to QDSP4000 library for implementation

*/
Word32 dsplib_sqrt(Word32 x, Word32 rnd_fac);

void hvx_invert(short *in, int f_in, short *inverse, short *inverse_exp, int size);
void hvx_sqrt(const uint32_t* src, uint32_t N, uint32_t* dst, uint32_t VLEN);
void hvx_float2frac(HVX_Vector* in, HVX_Vector* fracBitsVec, HVX_Vector* dst, int width);
