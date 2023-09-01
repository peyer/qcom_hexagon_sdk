/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

/* This original procedure counts the number of bits that are set to 1
   in a 32-bit unsigned integer. (Yes, there are faster ways of doing
   this, but it is for illustrative purposes.)  Notice the main body
   of the loop contains control flow (an if-block); this can prevent
   the compiler from generating code with high ILP.  */
int bitcount_orig(unsigned x)
{
  int count = 0;
  int i = 0;
  for (i = 0; i < 32; i++)
    {
      if (x & 1)
        count++;
      x >>= 1;
    }
  return count;
}

/* This optimized procedure no longer contains control flow in the
   body of the loop; instead, the counter is incremented
   unconditionally.  The increment, however, is 0 or 1, depending on
   the value of the current bit.  Thus, the semantics of the algorithm
   are preserved even though the control flow was eliminated.  Again,
   may appear to be a small and unrealistic example, but the main
   point is to show that sometimes an algorithm can be modified to
   eliminate unnecessary control flow. */
int bitcount_no_branch(unsigned x)
{
  int count = 0;
  int i = 0;
  for (i = 0; i < 32; i++)
    {
      count += (x & 1);
      x >>= 1;
    }
  return count;
}
