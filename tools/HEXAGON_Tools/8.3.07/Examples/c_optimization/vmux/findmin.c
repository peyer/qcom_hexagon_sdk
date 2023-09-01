/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

#include <hexagon_types.h>
#include <typedef.h>

typedef short __attribute__((aligned(8)))  short_8B_align;

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

/* This procedure shows how the reduction can be vectorized by
   spliting it into 4 parallel subproblems.  The vMin vector holds the
   4 parallel accumulators, each keeping track of the minimum element
   for a quarter of the array.  The vIndex vector keeps track of the
   corresponding indices of the elements in vMin.  Then, in each
   iteration of the vectorized loop, vMax and vIndex are updated with
   the vmax and vmux intrinsics.  The vmux intrinsic is controlled by
   the predicate set in a vcmph_gt operation.  After the vector loop
   finishes, the final "global" reduction can be completed simply by
   passing the vMin vector to the scalar findmin function with a
   constant length of 4.  This finishes the reduction sequentially.  */ 

#define Q6_p_vcmph_lt_PP(A, B) Q6_p_vcmph_gt_PP(B, A) /* A<B == B>A */

void findmin_vector(short_8B_align *in, int n, short int *out)
{
  int i;
  HEXAGON_Pred p;

  Word64 *vIn = (Word64*)in;

  Word64 vIndexCur = HEXAGON_V64_CREATE_H(3, 2, 1, 0);
  Word64 vIndex = vIndexCur;
  Word64 vIndexIncr = Q6_P_vsplath_R(0x0004);

  Word64 vMin = vIn[0];
  for (i = 0; i < n/4; i++) {
    p = Q6_p_vcmph_lt_PP(vIn[i], vMin);
    vIndex = Q6_P_vmux_pPP(p, vIndexCur, vIndex);
    vMin = Q6_P_vminh_PP(vIn[i], vMin);
    vIndexCur = Q6_P_vaddh_PP(vIndexCur, vIndexIncr);
  }

  findmin_orig((short int *)&vMin, 4, out);
  out[1] = ((short int *)&vIndex)[out[1]];
  return;
}

/* This last version is the same as the previous vectorized version,
   except that the final reduction is completed in parallel.  To
   reduce 4 elements in a single vector, a common log(n) trick is
   used.  The vMin element is shifted by a single element and compared
   with itself.  This eliminates half of the elements.  Then, it is
   shifted by 2 elements and, again, compared with itself.  This
   eliminates the remaining two elements.  The single element
   remaining is the minimum.  A similar trick is used with vcmp and
   vmux to find the correct index.  This version is slightly faster
   than the previous, because the final reduction is done in parallel,
   requiring only a logarithmic number of sequential steps.  */
void findmin_vector_opt(short_8B_align *in, int n, short int *out)
{
  int i;
  HEXAGON_Pred p;

  Word64 *vIn = (Word64*)in;

  Word64 vIndexCur = HEXAGON_V64_CREATE_H(3, 2, 1, 0);
  Word64 vIndex = vIndexCur;
  Word64 vIndexIncr = Q6_P_vsplath_R(0x0004);

  Word64 vMin = vIn[0];
  for (i = 0; i < n/4; i++) {
    p = Q6_p_vcmph_lt_PP(vIn[i], vMin);
    vIndex = Q6_P_vmux_pPP(p, vIndexCur, vIndex);
    vMin = Q6_P_vminh_PP(vIn[i], vMin);
    vIndexCur = Q6_P_vaddh_PP(vIndexCur, vIndexIncr);
  }

  p = Q6_p_vcmph_lt_PP(Q6_P_valignb_PPI(vMin, vMin, 2), vMin);
  vMin = Q6_P_vminh_PP(Q6_P_valignb_PPI(vMin, vMin, 2), vMin);
  vIndex = Q6_P_vmux_pPP(p, Q6_P_valignb_PPI(vIndex, vIndex, 2), vIndex);
  
  p = Q6_p_vcmph_lt_PP(Q6_P_valignb_PPI(vMin, vMin, 4), vMin);
  vMin = Q6_P_vminh_PP(Q6_P_valignb_PPI(vMin, vMin, 4), vMin);
  vIndex = Q6_P_vmux_pPP(p, Q6_P_valignb_PPI(vIndex, vIndex, 4), vIndex);
  
  out[0] = HEXAGON_V64_GET_H0(vMin);
  out[1] = HEXAGON_V64_GET_H0(vIndex);
  return;
}
