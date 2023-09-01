/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

/*==============================================================================
  Generate interleave & bit-reverse order
  twiddle factor array for radix-4 fixed-point
  implementation of FFT
==============================================================================*/

#include <stdio.h>
#include <math.h>

typedef __complex float cfloat; 

#define PI (acos(-1.0))

int brev(int x, int BITS)
{  int i;
  int y = 0;
  for (i = 0; i < BITS; i++) {
    y = (y << 1) | (x & 1);
    x >>= 1;
  }
  return y;
}

void gentwiddle(cfloat* twiddle1, cfloat* twiddle2, int NP, int log2NP)
{
  int i, k, k1, k2;
  float ar, ai;

  NP     = NP / 2;
  log2NP = log2NP - 1;

  // Generate twiddles for complexFFT
  // arrange in bit reversed order
  for (i = 0; i < NP / 4; i++) {
    k1 = brev(i, log2NP - 1);
    ar =  cos((double)(k1) * 2.0 * PI / (double)NP);
    ai = -sin((double)(k1) * 2.0 * PI / (double)NP);
    twiddle1[3 * i + 0] = ar + 1i * ai;

    k2 = brev(2 * i, log2NP - 1);
    ar =  cos((double)(k2) * 2.0 * PI / (double)NP);
    ai = -sin((double)(k2) * 2.0 * PI / (double)NP);
    twiddle1[3 * i + 1] = ar + 1i * ai;

    k = k1 + k2;
    ar =  cos((double)(k) * 2.0 * PI / (double)NP);
    ai = -sin((double)(k) * 2.0 * PI / (double)NP);
    twiddle1[3 * i + 2] = ar + 1i * ai;
  }

  for (i = 1; i <= NP / 2; i++) {
    ar = -0.5 * sin((double)(i) * PI / (double)NP);
    ai = -0.5 * cos((double)(i) * PI / (double)NP);
    twiddle2[i - 1] = ar + 1i * ai;
  }
}

