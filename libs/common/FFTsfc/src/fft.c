/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>

#include <hexagon_protos.h>

#include "FFTsfc.h"

int bitrev(int x, int BITS)
{
  int i;
  int y = 0;
  for (i = 0; i < BITS; i++) {
    y = (y << 1) | (x & 1);
    x >>= 1;
  }
  return y;
}

void Radix2BTFLY(cfloat* x)
{
  cfloat a, b;

  a = x[0] + x[1];
  b = x[0] - x[1];

  x[0] = a;
  x[1] = b;
  return;
}

void Radix4BTFLY(cfloat* x)
{
  cfloat a, b, c, d;

  a = x[0] + x[1];
  b = x[0] - x[1];
  c = x[2] + x[3];
  d = x[2] - x[3];

  // -j*d
  d = (0 + 1i * (-1)) * d;

  x[0] = a + c;
  x[1] = b + d;
  x[2] = a - c;
  x[3] = b - d;
  return;
}

// Floating-point implementation of radix-4 FFT
void FFTsfc(cfloat* input, int N, cfloat* w, cfloat* output)
{
  int i, j, k1, k2, n, m;
  cfloat Wa, Wb, Wc;
  cfloat A[4];

  int LOG2N = Q6_R_ct0_R(N);

  // Stage 1
  // read input in bit-reversed order
  for (i = 0, m = 0; i < N; i += 4) {
    A[0] = input[bitrev(i, LOG2N)];
    A[1] = input[bitrev(i + 1, LOG2N)];
    A[2] = input[bitrev(i + 2, LOG2N)];
    A[3] = input[bitrev(i + 3, LOG2N)];

    Radix4BTFLY(A);

    Wb = w[m++]; // Wb = w[j];
    Wa = w[m++]; // Wa = w[2*j+1];
    Wc = w[m++]; // Wc = cmult_r(Wa,Wb);

    output[i] = A[0];
    output[i + 1] = A[1] * Wa;
    output[i + 2] = A[2] * Wb;
    output[i + 3] = A[3] * Wc;
  }


  // Other Radix-4 stages
  k1 =  4; // # in each group
  k2 = N / 16; // # of groups

  for (n = LOG2N - 2; n > 2; n -= 2) {
    for (i = 0, m = 0; i < k2; i++) {
      Wb = w[m++]; // Wb = w[i];
      Wa = w[m++]; // Wa = w[2*i+1];
      Wc = w[m++]; // Wc = cmult_r(Wa,Wb);

      for (j = 0; j < k1; j++) {
        A[0] = output[(4 * i + 0) * k1 + j];
        A[1] = output[(4 * i + 1) * k1 + j];
        A[2] = output[(4 * i + 2) * k1 + j];
        A[3] = output[(4 * i + 3) * k1 + j];

        Radix4BTFLY(A);

        output[(4 * i + 0) * k1 + j] = A[0];
        output[(4 * i + 1) * k1 + j] = A[1] * Wa;
        output[(4 * i + 2) * k1 + j] = A[2] * Wb;
        output[(4 * i + 3) * k1 + j] = A[3] * Wc;
      }
    }
    k1 = k1 << 2;
    k2 = k2 >> 2;
  }

  if (n == 2) {
    // last Radix-4 stage
    for (j = 0; j < N / 4; j++) {
      A[0] = output[0 * (N / 4) + j];
      A[1] = output[1 * (N / 4) + j];
      A[2] = output[2 * (N / 4) + j];
      A[3] = output[3 * (N / 4) + j];

      Radix4BTFLY(A);

      // No multiplcations needed
      output[0 * (N / 4) + j] = A[0];
      output[1 * (N / 4) + j] = A[1];
      output[2 * (N / 4) + j] = A[2];
      output[3 * (N / 4) + j] = A[3];
    }
  } else {
    // last Radix-2 stage
    for (i = 0; i < N / 2; i++) {
      A[0] = output[i];
      A[1] = output[i + N / 2];

      Radix2BTFLY(A);

      output[i] = A[0];
      output[i + N / 2] = A[1];
    }
  }
}

