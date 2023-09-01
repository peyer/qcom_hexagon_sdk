/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#include <hexagon_types.h>
#include <typedef.h>

typedef short __attribute__((aligned(8)))  short_8B_align;

/* This is the original scalar version of vector add.  The function
   argument list contains two int pointers that are not necessarily
   8-byte aligned.  For this example, the assumption is that the
   programmer cannot always ensure the pointers are 8-byte aligned.
   In fact, the alignment is only known at compile time.  Thus,
   dynamic loop peeling is needed for vectorization.  */
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


/* This version shows how dynamic peeling works.  The first loop
   executes iterations one at a time, until the pointer is 8-byte
   aligned.  The following loop can then be vectorized.  */
void VecADD_peel(short int *In1, 
                 short int *In2,
                 short int *__restrict Out,
                 int samples)
{
  int i = 0;
  while ((((unsigned int)&In1[i]) % 8) != 0) {
    Out[i] = In1[i] + In2[i];
    i++;
  }
  for (; i < samples; i++) {
    Out[i] = In1[i] + In2[i];
  }
}


/* This last version shows how to complete the vectorization with
   dynamic peeling.  This solution uses 2 helper functions, however,
   and is encouraged for code readability.  The VecADD_misaligned
   function first calls a scalar version of vector add to implement
   the dynamic peeling.  Then, the remaining iterations are handled by
   calling a vectorized vector add function that implements cleanup.
   This solution is preferred because it isolates the assumptions for
   each routine.  The scalar version makes no assumptions, while the
   vectorized version assumes that the data is aligned and the correct
   length.  Composing these functions allows the creation of a
   vectorized function that makes no assumptions about the input
   data. */
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

inline static void
VecADD_cleanup(short_8B_align *In1, 
               short_8B_align *In2,
               short_8B_align *__restrict Out,
               int samples)
{
  int i;
  int offset = samples & ~0x3;
  VecADD_vector((Word64*)In1, (Word64*)In2, (Word64*)Out, samples/4);
  VecADD_scalar(In1 + offset, In2 + offset, Out + offset, samples % 4);
}

void
VecADD_misaligned(short int *In1, 
                  short int *In2,
                  short int *__restrict Out,
                  int samples)
{
  int i;
  int offset = (8 - (((unsigned int)Out) % 8)) / sizeof(short int);
  VecADD_scalar(In1, In2, Out, offset);
  VecADD_cleanup(In1 + offset, In2 + offset, Out + offset, samples - offset);
}

