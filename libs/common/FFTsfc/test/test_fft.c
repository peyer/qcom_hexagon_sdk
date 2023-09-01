/*==============================================================================
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <hexagon_sim_timer.h>
#define read_cycles hexagon_sim_read_cycles

#include "FFTsfc.h"

#define N 2048
#define LOG2N 11
#define NTEST 6

typedef __complex__ float cmplxf_t;

cfloat Input[N]__attribute__((aligned(1<<16)));
cfloat Output[N]__attribute__((aligned(8)));
cfloat Wtwiddles[3*N/4]__attribute__((aligned(8)));
cfloat ref[N]__attribute__((aligned(8)));

void gentwiddle(cfloat* twiddle, int NP, int log2NP);

void compareSequence(cfloat* a, cfloat* b, int npoint)
{
  double xr, xi;
  double er, ei, err, sig;

  int i;

  sig = 0.0;
  err = 0.0;

  // Direct implementation in floating point
  for (i = 0; i < npoint; i++) {
    xr = __real__(a[i]);   xi = __imag__(a[i]);
    er = __real__(b[i]);   ei = __imag__(b[i]);

    printf("Point #%d: (%f, %f)  (%f, %f)", i, xr, er, xi, ei);

    sig += er * er + ei * ei;
    er -= xr;  ei -= xi;

    printf(": err %f %f\n", er, ei);

    err += er * er + ei * ei;

  }
  printf("SNR= %.2f  \n\n", 10 * log10(sig / err));
}

// this check takes a long time, make it optional
#ifdef FFT_PRECISION_CHECK
void PrecisionCheck(cfloat* Input, cfloat* FFTOut, int npoint)
{
  double xr, xi;
  double er, ei, err, sig, ampMax;
  double arg;
  double pi = acos(-1.0);
  cmplxf_t s, wk, xk;

  int i, j, k;

  sig = 0.0;
  err = 0.0;
  ampMax = 0.0;

  // Direct implementation in floating point
  for (i = 0; i < npoint; i++) {
    s = 0.0;

    for (j = 0; j < npoint; j++) {
      k = (i * j) % npoint;
      arg = -k * 2 * pi / npoint;
      wk  = cos(arg) + 1i * sin(arg);
      xk  = Input[j];
      s   = s + xk * wk;
    }

    xr = __real__(FFTOut[i]);   xi = __imag__(FFTOut[i]);
    er = __real__(s);           ei = __imag__(s);

    // printf("(%f, %f) (%f, %f)", xr, xi, er, ei);

    sig += er * er + ei * ei;

    if (ampMax < fabs(er)) ampMax = fabs(er);
    if (ampMax < fabs(ei)) ampMax = fabs(ei);

    er -= xr;  ei -= xi;

    // printf("Point #%d: err %f %f\n", i, er, ei);

    err += er * er + ei * ei;

  }
  printf("SNR= %.2f  of %.2f(max)\n", 10 * log10(sig / err), 20 * log10(ampMax));
}
#endif

int main(void)
{
  int i, j;
  long long start_time;
  int overhead, total_cycles;

  srand48(8888888);

  gentwiddle(Wtwiddles, N, LOG2N);

  start_time = read_cycles();
  FFTsfc(Input, 0, Wtwiddles, Output);
  overhead =  (int)(read_cycles() - start_time) - 1;

  // Test correctness
  for (j = 0; j < NTEST; j++) {
    printf("Test #%d...\n", j);

    for (i = 0; i < 256; i++) {
      Input[i] = mrand48() / 32768.0 + 1i * mrand48() / 32768.0;
    }

    printf("  Test 128-point FFT\n");
    FFTsfc(Input, 128, Wtwiddles, Output);
    #ifdef FFT_PRECISION_CHECK
    PrecisionCheck(Input, Output, 128);
    #endif

    printf("  Test 256-point FFT\n");
    FFTsfc(Input, 256, Wtwiddles, Output);
    #ifdef FFT_PRECISION_CHECK
    PrecisionCheck(Input, Output, 256);
    #endif
  }

  for (i = 0; i < N; i++) {
    Input[i] = mrand48() / 32768.0 + 1i * mrand48() / 32768.0;
  }

  start_time = read_cycles();
  FFTsfc(Input, 256, Wtwiddles, Output);
  total_cycles =  (int)(read_cycles() - start_time) - overhead;
  printf("  256-point Cycle-count: %d\n", total_cycles);

  start_time = read_cycles();
  FFTsfc(Input, 512, Wtwiddles, Output);
  total_cycles =  (int)(read_cycles() - start_time) - overhead;
  printf("  512-point Cycle-count: %d\n", total_cycles);

  start_time = read_cycles();
  FFTsfc(Input, 1024, Wtwiddles, Output);
  total_cycles =  (int)(read_cycles() - start_time) - overhead;
  printf("  1024-point Cycle-count: %d\n", total_cycles);

  start_time = read_cycles();
  FFTsfc(Input, 2048, Wtwiddles, Output);
  total_cycles =  (int)(read_cycles() - start_time) - overhead;
  printf("  2048-point Cycle-count: %d\n", total_cycles);

  return 0;
}

