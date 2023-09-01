/**=============================================================================
    An example illustrating how to invoke the Qualcomm Hexagon Libraries (QHL)

 Copyright (c) 2019 Qualcomm Technologies Incorporated.
 All Rights Reserved. Qualcomm Proprietary and Confidential.
 =============================================================================**/

#include <stdio.h>
#include <stdlib.h>

#include "qhmath.h"
#include "qhblas.h"
#define ARRAY_SIZE 3

int main(int argc, char *argv[])
{

  printf("----- In main() with %d args\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("  argv[%d]: %s\n", i, argv[i]);
  }
  
  float in_f=-.123;
  int16_t in_h=-123;
  float_a8_t in_af[ARRAY_SIZE];
  int16_a8_t in_ah[ARRAY_SIZE];
  float_a8_t out_af[ARRAY_SIZE];
  int16_a8_t out_ah[ARRAY_SIZE];
  
  printf("----- Usage example for math scalar and array library functions\n");
  printf("Absolute value of float %f is %f\n",in_f,qhmath_abs_f(in_f));
  printf("Absolute value of int16_t %d is %d\n",in_h,qhmath_abs_h(in_h));
  
  for (int i=0;i<ARRAY_SIZE;i++) {
    in_af[i]=-.1234-i;
    in_ah[i]=-i*111;
  }
  
  int response;
  response = qhmath_abs_af(in_af, out_af, ARRAY_SIZE);
  if (response!=0) {
    printf("qhmath_abs_af returned error code %0x\n",response);
    return -1;
  }
  
  response = qhmath_abs_ah(in_ah, out_ah, ARRAY_SIZE);
  if (response!=0) {
    printf("qhmath_abs_ah returned error code %0x\n",response);
    return -1;
  }
  
  
  for (int i=0;i<ARRAY_SIZE;i++) {
    printf("Float abs(in_af[%d] = %f) = %f\n",i,in_af[i],out_af[i]);
  }
  for (int i=0;i<ARRAY_SIZE;i++) {
    printf("16-bit int abs(in_ah[%d] = %d) = %d\n",i,in_ah[i],out_ah[i]);
  }
  
  printf("----- Usage example for BLAS library functions\n");
  float_a8_t in2_af[ARRAY_SIZE];
  int16_a8_t in2_ah[ARRAY_SIZE];
  for (int i=0;i<ARRAY_SIZE;i++) {
    in2_af[i]=.1234-i;
    in2_ah[i]=2*in_ah[i];
  }
  
  response = qhblas_vector_add_af(in_af, in2_af, out_af, ARRAY_SIZE);
  if (response!=0) {
    printf("qhblas_vector_add_af returned error code %0x\n",response);
    return -1;
  }
  
  response = qhblas_vector_add_ah(in_ah, in2_ah, out_ah, ARRAY_SIZE);
  if (response!=0) {
    printf("qhblas_vector_add_ah returned error code %0x\n",response);
    return -1;
  }
  
  for (int i=0;i<ARRAY_SIZE;i++) {
    printf("%f + %f = %f\n",in_af[i],in2_af[i],out_af[i]);
  }
  for (int i=0;i<ARRAY_SIZE;i++) {
    printf("%d + %d = %d\n",in_ah[i],in2_ah[i],out_ah[i]);
  }
  
  return 0;
}
