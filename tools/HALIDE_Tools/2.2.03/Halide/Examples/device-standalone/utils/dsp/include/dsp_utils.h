#ifndef DSP_UTILS_H
#define DSP_UTILS_H

#include <hexagon_standalone.h>
#include "qurt_hvx.h"
/* Returns a vector that has the first byte of the vector v0
   repeated 'times' number of times at the end. Implements
   a boundary condition equivalent to Halide's repeat_edge boundary
   conditiion on the left edge. */
HVX_Vector repeat_edge_byte_left(HVX_Vector v0, int times) {
  int i;
  HVX_Vector vrot;
  HVX_Vector vzero = Q6_V_vzero();
  v0 = Q6_V_valign_VVI(v0, vzero, 1); /* One time. */
  for (i = 1; i < times; ++i) {
    vrot = Q6_V_vror_VR(v0, 1);
    v0 = Q6_V_valign_VVI(vrot, v0, 1);
  }
  return v0;
}
/* Returns a vector that has the last byte of the vector v0
   repeated 'times' number of times at the beginning. Implements
   a boundary condition equivalent to Halide's repeat_edge boundary
   condition on the right edge. */
HVX_Vector repeat_edge_byte_right(HVX_Vector v0, int times) {
  int i;
  HVX_Vector vrot;
  HVX_Vector vzero = Q6_V_vzero();
  v0 = Q6_V_vlalign_VVI(vzero, v0, 1);
  for (i = 1; i < times; ++i) {
    vrot = Q6_V_vror_VR(v0, 1);
    v0 = Q6_V_vlalign_VVI(v0, vrot, 1);
  }
  return v0;
}


#endif

