/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hexagon_sim_timer.h"
#include "flt_bkfir.h"

#define		T              16
#define		N              80

float coeffs[T] __attribute__((aligned(8))) = {
0.025452, -0.977081, -0.828766, 0.619141, -0.668732, -0.279694, 0.375275, 0.213715, 
0.410980, -0.526672, 0.962097, 0.158844, -0.034271, 0.224792, 0.357727, -0.792664 
};

float input[N+T] __attribute__((aligned(8))) = {
-1.899977, -3.680657, 3.296419, -3.510036, 1.486542, -0.897696, 2.541173, -2.555543, 
-0.551665, -2.112112, 0.548700, 1.022696, 1.921077, 2.624202, -2.786952, 3.313298, 
-0.733919, -1.848654, -1.185789, -1.360515, 3.265169, 2.493613, 0.622149, 2.278057, 
-0.255817, -3.658075, -0.154881, -1.616674, -3.469548, 0.397240, -0.671533, -2.709512, 
1.025091, 3.582687, 1.655452, -0.428832, 2.716127, 0.109831, 2.642564, 2.430885, 
0.457801, -0.893362, 2.563983, -0.950274, -3.380702, -0.549612, 2.990192, -1.924840, 
1.663549, -0.940351, -3.637203, 2.655224, -2.580862, -2.345575, 2.146556, 2.384694,
-0.551665, -2.112112, 0.548700, 1.022696, 1.921077, 2.624202, -2.786952, 3.313298, 
-0.733919, -1.848654, -1.185789, -1.360515, 3.265169, 2.493613, 0.622149, 2.278057, 
-0.255817, -3.658075, -0.154881, -1.616674, -3.469548, 0.397240, -0.671533, -2.709512, 
1.025091, 3.582687, 1.655452, -0.428832, 2.716127, 0.109831, 2.642564, 2.430885, 
0.457801, -0.893362, 2.563983, -0.950274, -3.380702, -0.549612, 2.990192, -1.924840 
};

float output[N]  __attribute__((aligned(8)));
float ref_out[N];

void flt_bkfir_ref(float *xin, float *coefs, int taps, int length, float *yout);

int main()
{
   int i;
   long long start_time, Total_cycles;

   // Generate reference outputs
   flt_bkfir_ref(input,coeffs,T,N,ref_out);

   printf("Calling FIR Kernel\n");

   start_time = hexagon_sim_read_pcycles();
   flt_bkfir(input,coeffs,T,N,output);
   Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time);

   for (i = 0; i < N; i++) {
	if ( fabs(output[i]-ref_out[i]) > 1.0e-5) {
		printf("### FAILED!!!\n"); return 1;
	}
   }
   printf("*** PASS!\n");
   printf("Cycle-count: %lld\n", Total_cycles);

   return 0;
}

