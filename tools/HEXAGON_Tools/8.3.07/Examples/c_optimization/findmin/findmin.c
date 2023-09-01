/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#include <hexagon_types.h>

/*  The original procedure searches an array for the minimum element,
    returning the minimum element and the index of the minimum
    element.  The body of the loop contains a conditional block that
    updates the smallest element found so far (and its index) if the
    current element is the new smallest.  The control flow in this
    loop can restrict performance if the compiler does not detect that
    the mux operation can be used.  At the time this test was written,
    the compiler did not automatically detect the ability to use the
    mux operation for this code, but it may improve in the future.
    The point of this example still remains, however: unnecessary
    control flow should be removed to improve compiler optimization.
    Other, more complex examples may still evade the compiler's
    analysis.
*/
void findmin_orig(short int *in, int n, short int *out)
{
  int min = in[0];
  int idx = 0;
  int i;
  for (i = 1; i < n; i++) {
    if (in[i] < min) {
      min = in[i];
      idx = i;
    }
  }

  out[0] = min;
  out[1] = idx;
  return;
}

/*  This procedure shows how the control flow can be eliminated in the
    body of the loop by using the mux intrinsic.  The min intrinsic is
    used, as well, but it is just a special case of the mux
    intrinsic.

    The main difference between this procedure and the original
    procedure is that the idx and min variables are now written to on
    every iteration of the loop.  In the original loop they were
    conditionally updated, depending on the result of the less-than
    comparison.  So, at first glance it looks like this loop may do
    "more work" than the original loop, becuase the idx and min
    variables are updated on every iteration.  The running time
    reflects the opposite, however, since the instruction schedule for
    the loop is much better.
*/
void findmin_mux(short int *in, int n, short int *out)
{
  int min = in[0];
  int idx = 0;
  int i;
  for (i = 1; i < n; i++) {
    HEXAGON_Pred p = (in[i] < min);
    idx = Q6_R_mux_pRR(p,i,idx);
    min = Q6_R_min_RR(in[i],min);
  }

  out[0] = min;
  out[1] = idx;
  return;
}

/* This final procedure shows how the mux operation can be leveraged
   without using the mux intrinsic in the source code.  The original
   control flow is restructured to use two ternary operations instead
   of an if-block.  It is structured in such a way that the compiler
   is able to recognize this pattern and replace it with a mux
   operation and a min operation.  To verify that the desired assembly
   is emitted, just compile with -S and view the compiler output. */
void findmin_ternary(short int *in, int n, short int *out)
{
  int min = in[0];
  int idx = 0;
  int i;
  for (i = 1; i < n; i++) {
    int cur = in[i];
    int p = (cur < min);
    idx = (p ? i : idx);
    min = (p ? cur : min);
  }

  out[0] = min;
  out[1] = idx;
  return;
}
