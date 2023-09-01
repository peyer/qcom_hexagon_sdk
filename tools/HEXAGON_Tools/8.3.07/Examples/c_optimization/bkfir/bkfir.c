/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#include <hexagon_types.h>
#include <assert.h>
#include <typedef.h>

typedef short __attribute__((aligned(8)))  short_8B_align;


/* 	Original version */
/* 	This is the type of code we would expect to start with when
   	optimizing a program.  It may be the "reference" version from a
   	benchmark suite, or it could be the prototype version that is
   	written first to achieve correctness.  When writing a program from
   	scratch, it is best to write a simple version like this first to
   	ensure functional correctness.  After correctness is verified, then
   	incremental optimization can be applied to the program "hot spots".
   	The following versions of the FIR function show example steps that
   	could be taken to improve program performance without significantly
   	sacrificing source code readability. */
void FIR01(short Input[],
           short Coeff[],
           short Output[],
           int unused, int ntaps,
           int nsamples)
{
  int i, j;
  int sum;

  for (i =0; i < nsamples; i++) 
  {
      sum=0;
      for (j = 0; j < ntaps; j++) 
	  {
          sum += Input[i+j]*Coeff[j];
      }
      Output[i+4]=sum>>18;
  }
}

/* 	Unroll and jam outer loop */
/* 	This optimization can be used to improve data locality in the inner
   	loop and/or to better enable vectorization.  In this example, we
   	improve locality on the memory access to Coeff[j].  Also, we are
   	preparing to vectorize the code, and the unroll-and-jam will help
   	us with alignment issues later.  */
void FIR02(short Input[],
           short Coeff[],
           short Output[],
           int unused, int ntaps,
           int nsamples)
{
  int i, j;
  int sum0, sum1, sum2, sum3;

  for (i = 0; i < nsamples; i += 4) 
  {
      sum0 = sum1 = sum2 = sum3 = 0;
      for (j = 0; j < ntaps; j++) 
	  {
          sum0 += Input[i+0+j]*Coeff[j];
          sum1 += Input[i+1+j]*Coeff[j];
          sum2 += Input[i+2+j]*Coeff[j];
          sum3 += Input[i+3+j]*Coeff[j];
      }
      Output[i+0+4] = sum0 >> 18;
      Output[i+1+4] = sum1 >> 18;
      Output[i+2+4] = sum2 >> 18;
      Output[i+3+4] = sum3 >> 18;
  }
}

/* 	Unroll inner loop */
/* 	Loop unrolling should always be applied carefully, since 
   	excessive unrolling can make the code hard to read and difficult
   	to maintain. In this case however, we are using it to enable
   	better vectorization in a later step.  Unrolling allows us to make 
	assumptions about the induction variable for the inner loop (it is 
	always a multiple of 4) that will help with alignment issues during
   	vectorization. */
void FIR03(short Input[],
           short Coeff[],
           short Output[],
           int unused, int ntaps,
           int nsamples)
{
  int i, j;
  int sum0, sum1, sum2, sum3;
  for(i = 0; i < nsamples; i += 4)
  {
     sum0 = sum1 = sum2 = sum3 = 0;
     for(j = 0; j < ntaps; j += 4)
     {
        sum0 += Input[i+0+j+0]*Coeff[j+0];
        sum1 += Input[i+1+j+0]*Coeff[j+0];
        sum2 += Input[i+2+j+0]*Coeff[j+0];
        sum3 += Input[i+3+j+0]*Coeff[j+0];

        sum0 += Input[i+0+j+1]*Coeff[j+1];
        sum1 += Input[i+1+j+1]*Coeff[j+1];
        sum2 += Input[i+2+j+1]*Coeff[j+1];
        sum3 += Input[i+3+j+1]*Coeff[j+1];

        sum0 += Input[i+0+j+2]*Coeff[j+2];
        sum1 += Input[i+1+j+2]*Coeff[j+2];
        sum2 += Input[i+2+j+2]*Coeff[j+2];
        sum3 += Input[i+3+j+2]*Coeff[j+2];

        sum0 += Input[i+0+j+3]*Coeff[j+3];
        sum1 += Input[i+1+j+3]*Coeff[j+3];
        sum2 += Input[i+2+j+3]*Coeff[j+3];
        sum3 += Input[i+3+j+3]*Coeff[j+3];
     }
     Output[i+0+4] = sum0 >> 18;
     Output[i+1+4] = sum1 >> 18;
     Output[i+2+4] = sum2 >> 18;
     Output[i+3+4] = sum3 >> 18;
  }
}

/* 	Reorder statements in inner loop */
/* 	This step shouldn't change performance at all, but it is helpful to
   	see how we are going to structure the vectorization.  In the
   	previous iteration, the accumulation operations for each
   	sum-accumulator were not grouped.  Also, the accesses to the Coeff
   	array were not contiguous as they are for this version.  This
   	iteration strives to group the accumulators and to order the array
   	accesses so they are contiguous. */
void FIR04(short Input[],
           short Coeff[],
           short Output[],
           int unused, int ntaps,
           int nsamples)
{
  int i, j;
  int sum0, sum1, sum2, sum3;

  for (i = 0; i < nsamples; i += 4) 
  {
      sum0 = sum1 = sum2 = sum3 = 0;
      for (j = 0; j < ntaps; j += 4) 
	  {
          sum0 += Input[i+0+j+0]*Coeff[j+0];
          sum0 += Input[i+0+j+1]*Coeff[j+1];
          sum0 += Input[i+0+j+2]*Coeff[j+2];
          sum0 += Input[i+0+j+3]*Coeff[j+3];

          sum1 += Input[i+1+j+0]*Coeff[j+0];
          sum1 += Input[i+1+j+1]*Coeff[j+1];
          sum1 += Input[i+1+j+2]*Coeff[j+2];
          sum1 += Input[i+1+j+3]*Coeff[j+3];

          sum2 += Input[i+2+j+0]*Coeff[j+0];
          sum2 += Input[i+2+j+1]*Coeff[j+1];
          sum2 += Input[i+2+j+2]*Coeff[j+2];
          sum2 += Input[i+2+j+3]*Coeff[j+3];

          sum3 += Input[i+3+j+0]*Coeff[j+0];
          sum3 += Input[i+3+j+1]*Coeff[j+1];
          sum3 += Input[i+3+j+2]*Coeff[j+2];
          sum3 += Input[i+3+j+3]*Coeff[j+3];
      }
      Output[i+0+4] = sum0 >> 18;
      Output[i+1+4] = sum1 >> 18;
      Output[i+2+4] = sum2 >> 18;
      Output[i+3+4] = sum3 >> 18;
  }
}

/* 	Vectorize inner loop */
/* 	Now we can vectorize the inner loop.  Each group of 4
   	multiply-accumulate operations can be replaced with a single
   	Q6_P_vrmpyhacc_PP intrinsic.  This intrinsic is directly replaced,
   	by the compiler, with the equivalent vector operation in the final
   	executable.  The vector operation multplies for 16-bit halfwords
   	and accumulates the sum in an accumulator, all in a single cycle.
   	There are alignment issues with the array accesses, however, since
   	a double-word load needs to be 8-byte aligned.  Unrolling the two
   	loops in the previous step helps with the alignment, because we
   	know that both induction variables are always multiples of 4.
   	Thus, the access into Coeff array will always be aligned, and the
   	first access into the Input array will be aligned.  The following
   	three accesses into the Input array will not be aligned, but they
   	do overlap with each other.  Therefore, we can issue two vector
   	load operations for each iteration (8 elements), and then use the
   	appropriate Q6_P_valignb_PPI intrinsic to select the desired 4
   	elements.  The inner loop requires the following operations: 2 x
   	memd loads, 4 x vrmpyhacc, and 3 x valignb.  */
void FIR05(short_8B_align Input[],
           short_8B_align Coeff[],
           short_8B_align Output[],
           int unused, int ntaps,
           int nsamples)
{
  Word64 * vInput = (Word64*)Input;
  Word64 * vCoeff = (Word64*)Coeff;
  int i, j;
  int sum0, sum1, sum2, sum3;

  for (i = 0; i < nsamples; i+= 4) 
  {
      sum0 = sum1 = sum2 = sum3 = 0;
      for (j = 0; j < ntaps; j+= 4) 
	  {
          Word64 vIn1 = vInput[(i+j)/4];
          Word64 vIn2 = vInput[(i+j)/4+1];
          Word64 curCoeff = vCoeff[j/4];
          Word64 curIn;

          curIn = vIn1;
          sum0 = Q6_P_vrmpyhacc_PP(sum0, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 2);
          sum1 = Q6_P_vrmpyhacc_PP(sum1, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 4);
          sum2 = Q6_P_vrmpyhacc_PP(sum2, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 6);
          sum3 = Q6_P_vrmpyhacc_PP(sum3, curIn, curCoeff);
      }
      Output[i+0+4] = sum0 >> 18;
      Output[i+1+4] = sum1 >> 18;
      Output[i+2+4] = sum2 >> 18;
      Output[i+3+4] = sum3 >> 18;
  }
}

/* 	Vectorize output writes */
/* 	The output writes in the outer loop can be vectorized next.  Each
   	accumulator needs to be shifted by 18 bits.  Therefore, we can
   	simply extract the upper 16 bits to accomplish a 16-bit shift.  The
   	Q6_P_shuffoh_PP intrinsic works great for this by packing the "odd
   	halfwords" of two 64-bit vectors into a new 64-bit vector.  Then,
   	we can use the Q6_P_vasrh_PI intrinsic to shift each half-word the
   	remaining 2-bits to the right.  The advantage of this approach is
   	that the accumulators are shifted in parallel.  Then, the output
   	vector can be stored to the output array with a 64-bit store.  */
void FIR06(short_8B_align Input[],
           short_8B_align Coeff[],
           short_8B_align Output[],
           int unused, int ntaps,
           int nsamples)
{
  Word64 * vInput = (Word64*)Input;
  Word64 * vCoeff = (Word64*)Coeff;
  Word64 *__restrict vOutput = (Word64*)Output;
  int i, j;
  int sum0, sum1, sum2, sum3;

  for (i = 0; i < nsamples; i+=4) 
  {
      sum0 = sum1 = sum2 = sum3 = 0;
      for (j = 0; j < ntaps; j+=4) 
	  {
          Word64 vIn1 = vInput[(i+j)/4];
          Word64 vIn2 = vInput[(i+j)/4 + 1];
          Word64 curCoeff = vCoeff[j/4];
          Word64 curIn;

          curIn = vIn1;
          sum0 = Q6_P_vrmpyhacc_PP(sum0, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 2);
          sum1 = Q6_P_vrmpyhacc_PP(sum1, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 4);
          sum2 = Q6_P_vrmpyhacc_PP(sum2, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 6);
          sum3 = Q6_P_vrmpyhacc_PP(sum3, curIn, curCoeff);
      }

          Word64 curOut = Q6_P_combine_RR(Q6_R_combine_RhRh(sum3, sum2), Q6_R_combine_RhRh(sum1, sum0));
         vOutput[(i/4) + 1] = Q6_P_vasrh_PI(curOut, 2);
  }
}

/* Scale index variables */
/* So far, the vector memory accesses have relied on "scaling" the
   induction variables to achieve the correct index.  This can result
   in inefficient code if the compiler isn't able to properly optimize
   the code.  We can simplify the induction variable now, since all of
   our array accesses are vector loads/stores.  The induction
   variables in this example are all "scaled-down" by 4.  Each vector
   access can use the induction variable directly, instead of
   computing the correct index.  */
void FIR07(short_8B_align Input[],
           short_8B_align Coeff[],
           short_8B_align Output[],
           int unused, int ntaps,
           int nsamples)
{
  Word64 * vInput = (Word64*)Input;
  Word64 * vCoeff = (Word64*)Coeff;
  Word64 *__restrict vOutput = (Word64*)Output;
  int i, j;
  int sum0, sum1, sum2, sum3;

  for (i = 0; i < nsamples/4; i++) 
  {
      sum0 = sum1 = sum2 = sum3 = 0;
      for (j = 0; j < ntaps/4; j++) 
	  {
          Word64 vIn1 = vInput[i+j];
          Word64 vIn2 = vInput[i+j+1];
          Word64 curCoeff = vCoeff[j];
          Word64 curIn;

          curIn = vIn1;
          sum0 = Q6_P_vrmpyhacc_PP(sum0, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 2);
          sum1 = Q6_P_vrmpyhacc_PP(sum1, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 4);
          sum2 = Q6_P_vrmpyhacc_PP(sum2, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 6);
          sum3 = Q6_P_vrmpyhacc_PP(sum3, curIn, curCoeff);
      }

	  Word64 curOut = Q6_P_combine_RR(Q6_R_combine_RhRh(sum3, sum2), Q6_R_combine_RhRh(sum1, sum0));
      vOutput[i + 1] = Q6_P_vasrh_PI(curOut, 2);
  }
}


/* 	Fix data-types to prevent unnecessary sign extension */
/* 	This step just recognizes that the data type of the accumulator for
   	the Q6_P_vrmpyhacc_PP intrinsic is a 64-bit integer.  Previous
   	iterations used a 32-bit integer, resulting in several sign-extension
   	operations in the final code.  By changing the data type of the sum
   	accumulators, we can eliminate these unnecessary operations.  Be
   	careful when changing a variable from 32-bits to 64-bits, however,
   	because it will increase the register pressure for that region of
   	the code (64-bit values need a register pair instead of a single
   	register).  In this example, the change does not seem to result in
   	too many extra register spills.  The sign-extension operations are
   	eliminated, however, resulting in better performance.  */
void FIR08(short_8B_align Input[],
           short_8B_align Coeff[],
           short_8B_align Output[],
           int unused, int ntaps,
           int nsamples)
{
  Word64 * vInput = (Word64*)Input;
  Word64 * vCoeff = (Word64*)Coeff;
  Word64 *__restrict vOutput = (Word64*)Output;
  int i, j;
  Word64 sum0, sum1, sum2, sum3;

  for (i = 0; i < nsamples/4; i++)
  {
      sum0 = sum1 = sum2 = sum3 = 0;
      for (j = 0; j < ntaps/4; j++)
      {
          Word64 vIn1 = vInput[i+j];
          Word64 vIn2 = vInput[i+j+1];
          Word64 curCoeff = vCoeff[j];
          Word64 curIn;

          curIn = vIn1;
          sum0 = Q6_P_vrmpyhacc_PP(sum0, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 2);
          sum1 = Q6_P_vrmpyhacc_PP(sum1, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 4);
          sum2 = Q6_P_vrmpyhacc_PP(sum2, curIn, curCoeff);

          curIn = Q6_P_valignb_PPI(vIn2, vIn1, 6);
          sum3 = Q6_P_vrmpyhacc_PP(sum3, curIn, curCoeff);
      }

      Word64 curOut = Q6_P_combine_RR(Q6_R_combine_RhRh(sum3, sum2), Q6_R_combine_RhRh(sum1, sum0));
      vOutput[i + 1] = Q6_P_vasrh_PI(curOut, 2);
  }
}

