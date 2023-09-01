/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#include <hexagon_types.h>
#include <typedef.h>

typedef short __attribute__((aligned(8)))  short_8B_align;

/*  The original procedure implements a pair-wise array-addition
    between two input arrays and stores the result in the output
    array.  For this example, the test harness ensures that the number
    of samples is not a multiple of the vector length.  The vector
    length is 4 since we are dealing with 2-byte halfwords.  The
    run-time number of samples is 127, which results in 31 vector
    operations with 3 remaining scalar operations.  */
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

/*  This example shows the straightforward naive vectorization.  THIS
    ONLY WORKS WHEN samples IS A MULTIPLE OF 4.  If samples is not a
    multiple of 4, then the 1-3 remaining elements will be ignored and
    the incorrect result returned.  As the next examples show, we
    must use a cleanup loop to handle this.  */
void
VecADD_naive(short_8B_align *In1, 
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

/* To correctly handle the situation where the data length is not a
   multiple of the vector length, we must use a cleanup loop.  In this
   example, the first loop iterates 127/4=31 times, operating on 4
   elements at a time.  Then, the final loop operates on the remaining
   3 elements, 1 at a time.  */
void
VecADD_cleanup(short_8B_align *In1, 
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
  for (i = samples & ~0x3; i < samples; i++) {
    Out[i] = In1[i] + In2[i];
  }
}


/* The last example showed how to use a cleanup loop to handle
   remaining elements that are not a multiple of the vector length.
   From a software engineering point of view, however, it is sometimes
   easier to separate the concerns.  In this last example, we will
   extract the functionality for the vector add and the scalar add.
   This allows each of the individual functions to be more clean and
   concise.  Then, we have a general function that will combine the
   functionality of the two specialized functions.  If we declare our
   specialized functions as "inline static", then it is likely that
   the compiler will inline them, resulting in the same performance as
   the previous example.  The code is now much cleaner, however,
   becuase we have isolated the special cases in their own functions.
   This is a great example of the separation of concerns.

   An added benefit of this approach is that the programmer can use
   the specialized functions (they could no longer be static, however)
   in cases for which the constraints are met.  This gives the
   programer the flexibility to use high performance specialized
   function when it is safe, but also the ease of using a general
   function when the specialized version is not applicable.
*/
inline static void
VecADD_vector(Word64 *vIn1,
              Word64 *vIn2,
              Word64 *vOut,
              int samples)
{
  int i;
  for (i = 0; i < samples; i++) {
    vOut[i] = Q6_P_vaddh_PP(vIn1[i], vIn2[i]);
  }
}

inline static void
VecADD_scalar(short int *In1, 
                  short int *In2,
                  short int *Out,
                  int samples)
{
  int i;
  for (i = 0; i < samples; i++) {
    Out[i] = In1[i] + In2[i];
  }
}

void
VecADD_separate(short_8B_align *In1, 
                short_8B_align *In2,
                short_8B_align *__restrict Out,
                int samples)
{
  int i;
  int offset = samples & ~ 0x3;
  VecADD_vector((Word64*)In1, (Word64*)In2, (Word64*)Out, samples/4);
  VecADD_scalar(In1 + offset, In2 + offset, Out + offset, samples % 4);
}
