/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* This original procedure shows how the compiler must assume that
   pointers might alias one another.  The body of the loop contains
   two loads, a compoutation, followed by a store.  If the pointers
   might alias one another, then the two loads must always precede the
   store for the same iteration.  This forces the compiler to generate
   sequential code for the body fo the loop.  */
void
VecADD_orig(short int *In1, 
            short int *In2,
            short int *Out,
            int samples)
{
  int i;
  for (i = 0; i < samples; i++) {
    Out[i] = In1[i] + In2[i];
  }
}

/* This example applies the __restrict keyword to the Out pointer.
   Restrict tells the compiler that the Out pointer will not alias
   with any other pointers in the same scope (specifically, In1 and
   In2).  Now, in the body of the loop, the compiler can reorder the
   memory accesses.  Any of the loads can be moved before the store
   for a single iteration.  Thus, software pipelining (or loop
   unrolling) can result in more-compact schedules with higher
   ILP.  */
void
VecADD_restrict(short int *In1, 
                short int *In2,
                short int *__restrict Out,
                int samples)
{
  int i;
  for (i = 0; i < samples; i++) {
    Out[i] = In1[i] + In2[i];
  }
}
