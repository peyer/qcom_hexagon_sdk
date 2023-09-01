/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <hexagon_sim_timer.h>
#define read_cycles hexagon_sim_read_cycles

#include "FFTsfr.h"


#define N 256
#define LOG2N 8
#define NTEST 6

typedef __complex__ float cmplxf_t;

float Input[N]__attribute__((aligned(4* N)));
cfloat Output[N]__attribute__((aligned(8)));
cfloat Wtwiddle1[3*N/8]__attribute__((aligned(8)));
cfloat Wtwiddle2[N/4]__attribute__((aligned(8)));
cfloat ref[N]__attribute__((aligned(8)));

void gentwiddle(cfloat* twiddle1, cfloat* twiddel2, int NP, int log2NP);


// this check takes a long time, make it optional
#ifdef FFT_PRECISION_CHECK
void PrecisionCheck(float* Input, cfloat* FFTOut, int npoint)
{
  double xr, xi;
  double er, ei, err, sig;
  double arg;
  double pi = acos(-1.0);
  cmplxf_t s, wk, xk;

  int i, j, k;

  sig = 0.0;
  err = 0.0;

  // Direct implementation in floating point
  for (i = 0; i < npoint; i++) {
    s = 0.0;

    for (j = 0; j < npoint; j++) {
      k = (i * j) % npoint;
      arg = -k * 2 * pi / npoint;
      wk  = cos(arg) + 1i * sin(arg);
      xk  = Input[j] + 1i * 0;
      s   = s + xk * wk;
    }

    xr = __real__(FFTOut[i]);   xi = __imag__(FFTOut[i]);
    er = __real__(s);           ei = __imag__(s);

    sig += er * er + ei * ei;

    er -= xr;  ei -= xi;
    err += er * er + ei * ei;
  }

  printf("SNR= %.2f dB\n", 10 * log10(sig / err));
}
#endif

int main(void)
{
  int i, j;
  long long start_time;
  int overhead, total_cycles;

  srand48(8888888);

  gentwiddle(Wtwiddle1, Wtwiddle2, N, LOG2N);

  start_time = read_cycles();
  FFTsfr(Input, 0, Wtwiddle1, Wtwiddle2, Output);
  overhead =  (int)(read_cycles() - start_time) - 1;
  
  // Test correctness
  for (j = 0; j < NTEST; j++) {
    printf("Test #%d...\n", j);

    for (i = 0; i < 256; i++) {
      Input[i] = mrand48() / 32768.0 + 1i * mrand48() / 32768.0;
    }

    printf("  Test 256-point FFT\n");
    FFTsfr(Input, 256, Wtwiddle1, Wtwiddle2, Output);
    #ifdef FFT_PRECISION_CHECK
    PrecisionCheck(Input, Output, 256);
    #endif
  }

  for (i = 0; i < N; i++) {
    Input[i] = mrand48() / 32768.0 + 1i * mrand48() / 32768.0;
  }

  start_time = read_cycles();
  FFTsfr(Input, N, Wtwiddle1, Wtwiddle2, Output);
  total_cycles =  (int)(read_cycles() - start_time) - overhead;

  printf(" 256-point Cycle-count: %d\n", total_cycles);

  return 0;
}

