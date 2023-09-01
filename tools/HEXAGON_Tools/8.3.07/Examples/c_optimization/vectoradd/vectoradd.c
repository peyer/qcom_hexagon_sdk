/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#include <hexagon_types.h>
#include <typedef.h>

typedef short __attribute__((aligned(8)))  short_8B_align;

/* The original procedure implements a pair-wise array-addition
   between two input arrays and stores the result in the output array.
   The data is contiguous (because it is in an array).  In this case,
   the test harness ensures that the pointers are 8-byte aligned and
   that the number of samples is a multiple of the vector size.  The
   vector size, in this case, is 4 since the element size is 2-bytes
   (short).  The example contains no control flow or loop-carried data
   dependences.  */
void
VecADD_orig(short int *In1, 
            short int *In2,
            short int *__restrict Out,
            int samples)
{
  int i;
  for (i = 0; i < samples; i++) {
    Out[i] = In1[i] + In2[i];
  }
}

/*  Because hand-vectorization requires replacing multiple
    scalar-operations with a single vector-operation, it is sometimes
    helpful to clearly identify the group of scalar operations.  This
    step is certainly not necessary, but it is especially useful when
    learning to vectorize or when vectorizing complex codes.
    Basically, this procedure shows the original loop unrolled
    manually by four.

    After unrolling the loop, we can more clearly see the contiguous
    data accesses (multiple data) as well as the duplicated add
    operation (single instruction) that are required for SIMD
    operations.  Four contiguous elements from the In1 and In2 arrays
    are loaded each iteration, and four contiguous elements from the
    Out array are stored each iteration.  The add operation is applied
    uniformly to each array element in In1 and In2.  Thus, the vector
    operation that we would like to use is vaddh (vector add
    halfwords).

    Note that unrolling the code may or may not result in an
    improvement in program performance.  However, this step is just an
    intermediate step to aid in the vectorization process.  As long as
    the final vectorized code achieves a performance improvement, we
    are not concerned with the performance of this intermediate step.
*/
void
VecADD_unrolled(short int *In1, 
            short int *In2,
            short int *__restrict Out,
            int samples)
{
  int i;
  for (i = 0; i < samples; i+=4) {
    Out[i+0] = In1[i+0] + In2[i+0];
    Out[i+1] = In1[i+1] + In2[i+1];
    Out[i+2] = In1[i+2] + In2[i+2];
    Out[i+3] = In1[i+3] + In2[i+3];
  }
}

/* This final procedure shows the vectorized code.  First, we must
   create Word64 pointers to access the short arrays as vector arrays
   instead.  This allows us to utilize the 8-byte load and store
   operations in Hexagon.  We can just create the Word64 pointers and
   cast the short pointers (this is safe because we have ensured
   correct alignment in the test harness).

   Vectorization allows us to operate on 4 data elements at a time;
   thus, we must divide the loop bound by 4.  This is safe, again,
   because we've ensured that the data length is a multiple of 4 in
   the test harness.

   Lastly, the actual vector operation takes the place of 4 scalar
   operations in the body of the loop.  The C intrinsic is
   Q6_P_vaddh_PP.  The operands are two vector elements from the input
   arrays, and the result is stored in a vector element of the output
   array.
*/
void
VecADD_vectorized(short_8B_align *In1, 
            short_8B_align *In2,
            short_8B_align *__restrict Out,
            int samples)
{
  int i;
  Word64 *vIn1 = (Word64*)In1;
  Word64 *vIn2 = (Word64*)In2;
  Word64 *vOut = (Word64*__restrict)Out;
  for (i = 0; i < samples/4; i++) {
    vOut[i] = Q6_P_vaddh_PP(vIn1[i], vIn2[i]);
  }
}
