/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "typedef.h"
#include "hexagon_sim_timer.h"

#include "fft.h"


#define  N        2048
#define  LOG2N    11
#define  NTEST    6


typedef  __complex__ float   cmplxf_t;
typedef volatile unsigned long u32;
typedef volatile unsigned long long u64;

cfloat Input[N]   __attribute__((aligned(1<<16)));
cfloat Output[N]  __attribute__((aligned(8)));
cfloat Wtwiddles[3*N/4] __attribute__((aligned(8)));
cfloat ref[N]  __attribute__((aligned(8)));


int thread_get_tnum(void);
void gentwiddle( cfloat *twiddle, int NP, int log2NP);

#define PKTCOUNT_MON 0x1000
#define SSR_CE_SET 0x800000
/*
    This function enables the packet count register for monitor mode.
*/
void enable_pktcount(void)
{
  u32 mon_mode=PKTCOUNT_MON, set_ssrce=SSR_CE_SET;

  __asm__ __volatile__ (
    "r0 = usr\n"
	"r1 = %0\n"
	"r0 = or(r0,r1)\n"
	"usr = r0\n"
	"isync\n"
	"r0 = ssr\n"
	"r1 = %1\n"
	"r0 = or(r0, r1)\n"
	"ssr = r0\n"
	"isync\n"
	:
    : "r" (mon_mode), "r" (set_ssrce)
	: "r0", "r1"
  );
}

/*
    This function reads the packet count register for thread currently active.
*/
unsigned long long read_pktcount(void)
{
  u32 lo, hi;
  u64 pcyc;

  __asm__ __volatile__ (
    "%0 = pktcountlo\n"
    "%1 = pktcounthi\n"
	"isync\n"
    : "=r" (lo), "=r" (hi)
  );
  pcyc = (((u64) hi)<<32) + lo;
  return pcyc;
}
/*
    This function reads the packet count register for thread currently active.
*/
//void write_pktcount(int counthi, int countlo)
void write_pktcount(long long count)
{
	__asm__ __volatile__ (
		"PKTCOUNT = %0\n"
		//"c18 = r0\n"
		"isync\n"
		:
		: "r" (count)
		: "r1", "r0"
	);
}

void compareSequence( cfloat *a, cfloat *b, int npoint)
{
   double xr, xi;
   double er, ei, err, sig;

   int i;

   sig = 0.0;
   err = 0.0;

   /* --------------------------------------------------------- */
   /*    Direct implementation in floating point                */
   /* --------------------------------------------------------- */
   for (i = 0; i < npoint; i++)
   {
     xr = __real__(a[i]);   xi = __imag__(a[i]);
     er = __real__(b[i]);   ei = __imag__(b[i]);

     printf("Point #%d: (%f, %f)  (%f, %f)", i, xr, er, xi, ei);

     sig += er * er + ei * ei;
     er -= xr;  ei -= xi;

     printf(": err %f %f\n", er, ei);

     err += er * er + ei * ei;

   }
   printf("SNR= %.2f  \n\n", 10*log10(sig/err));
}


void PrecisionCheck( cfloat *Input, cfloat *FFTOut, int npoint)
{
   double xr, xi;
   double er, ei, err, sig, ampMax;
   double arg;
   double pi = acos (-1.0);
   cmplxf_t s, wk, xk;

   int i, j, k;

   sig = 0.0;
   err = 0.0;
   ampMax = 0.0;

   /* --------------------------------------------------------- */
   /*    Direct implementation in floating point                */
   /* --------------------------------------------------------- */
   for (i = 0; i < npoint; i++)
   {
     s = 0.0;

     for (j = 0; j < npoint; j++) {
         k = (i*j) %npoint;
         arg = - k*2*pi/npoint;
         wk  = cos (arg) + 1i*sin (arg);
         xk  = Input[j];
         s   = s + xk * wk;
     }

     xr = __real__(FFTOut[i]);   xi = __imag__(FFTOut[i]);
     er = __real__(s);           ei = __imag__(s);

//     printf("(%f, %f) (%f, %f)", xr, xi, er, ei);

     sig += er * er + ei * ei;

     if (ampMax < fabs(er) ) ampMax = fabs(er);
     if (ampMax < fabs(ei) ) ampMax = fabs(ei);

     er -= xr;  ei -= xi;

//     printf("Point #%d: err %f %f\n", i, er, ei);

     err += er * er + ei * ei;

   }
   printf("SNR= %.2f  of %.2f(max)\n", 10*log10(sig/err), 20*log10(ampMax));
}



int main()
{
    int i, j;
    unsigned long long start_time, packet_count;
    int overhead, total_cycles, current_thread;
	enable_pktcount();
    srand48(8888888);

    gentwiddle( Wtwiddles, N, LOG2N);
	current_thread = thread_get_tnum();
    start_time = hexagon_sim_read_tcycles(current_thread);
    sfpFFT(Input, 0, Wtwiddles, Output);
    overhead =  (int)(hexagon_sim_read_tcycles(current_thread) - start_time) - 1;

	
/* 	start_time = hexagon_sim_read_tcycles(current_thread);
	// clear pktcount
	write_pktcount(0LL);
	packet_count = read_pktcount();
    sfpFFT(Input, 256, Wtwiddles, Output);
	packet_count = read_pktcount();
    total_cycles =  (int)(hexagon_sim_read_tcycles(current_thread) - start_time) - overhead;
    printf(" 256-point Thread_Cycle_count: %d, packet_count: 0x%llx\n", total_cycles, packet_count);
	exit(0); */

	
    // Test correctness
    for (j = 0; j < NTEST; j++)
    {
        printf("Test #%d...\n", j);

        for (i=0; i<256; i++)
        {
           Input[i] = mrand48()/32768.0 + 1i*mrand48()/32768.0;
        }

        printf("Test 128-point FFT: ");
        sfpFFT(Input, 128, Wtwiddles, Output);
        PrecisionCheck(Input, Output, 128);

        printf("Test 256-point FFT: ");
        sfpFFT(Input, 256, Wtwiddles, Output);
        PrecisionCheck(Input, Output, 256);
    }

    for (i=0; i<N; i++)
    {
       Input[i] = mrand48()/32768.0 + 1i*mrand48()/32768.0;
    }

    start_time = hexagon_sim_read_tcycles(current_thread);
	// clear pktcount
	write_pktcount(0LL);
    sfpFFT(Input, 256, Wtwiddles, Output);
	packet_count = read_pktcount();
    total_cycles =  (int)(hexagon_sim_read_tcycles(current_thread) - start_time) - overhead;
	printf(" 256-point packet_count: %lld\n", packet_count);

    start_time = hexagon_sim_read_tcycles(current_thread);
	// clear pktcount
	write_pktcount(0LL);
    sfpFFT(Input, 512, Wtwiddles, Output);
	packet_count = read_pktcount();
    total_cycles =  (int)(hexagon_sim_read_tcycles(current_thread) - start_time) - overhead;
    printf(" 512-point packet_count: %lld\n", packet_count);

    start_time = hexagon_sim_read_tcycles(current_thread);
	// clear pktcount
	write_pktcount(0LL);
    sfpFFT(Input, 1024, Wtwiddles, Output);
	packet_count = read_pktcount();
    total_cycles =  (int)(hexagon_sim_read_tcycles(current_thread) - start_time) - overhead;
	printf("1024-point packet_count: %lld\n", packet_count);

    start_time = hexagon_sim_read_tcycles(current_thread);
	// clear pktcount
	write_pktcount(0LL);
    sfpFFT(Input, 2048, Wtwiddles, Output);
	packet_count = read_pktcount();
    total_cycles =  (int)(hexagon_sim_read_tcycles(current_thread) - start_time) - overhead;
	printf("2048-point packet_count: %lld\n", packet_count);

    return 0;
}

