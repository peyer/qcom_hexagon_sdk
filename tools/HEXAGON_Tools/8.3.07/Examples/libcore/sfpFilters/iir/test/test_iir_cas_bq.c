/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>
#include <math.h>
#include "hexagon_sim_timer.h"
#include "flt_iir_cas_bq.h"

#define NS       81

float Coeffs[8] __attribute__((aligned(8))) =
{
// first section
0.067352,       // f0
0.999969,       // b0
-0.194000,      // f1
0.500000,       // b1
// second section
-0.032043,      // f0
0.999969,       // b0
-0.156982,      // f1
0.500000        // b1
};


float Input[] =
{ 471,2159,2695,2607,1319,488,625,1239,-42,-771,
  -92,-566,-1601,-794,-1380,-739,-750,-1513,-1604,-543,
  -153,-158,563,1330,1024,1596,2286,2985,2526,2263,
 2268,1840,1838,954,520,-337,-488,243,-1096,-1872,
  471,2159,2695,2607,1319,488,625,1239,-42,-771,
  -92,-566,-1601,-794,-1380,-739,-750,-1513,-1604,-543,
  -153,-158,563,1330,1024,1596,2286,2985,2526,2263,
 2268,1840,1838,954,520,-337,-488,243,-1096,-1872,
 100
};

float States[4] __attribute__((aligned(8)));
float Output[NS], ref_out[NS];

extern void flt_IIR_casBiquad_ref(float *in, float *coef, float *states, int nSec, int nsamples, float *out); 

int main()
{
    int i;
    long long start_time, Total_cycles;

    // Generate reference outputs
    for (i=0; i<4; i++)
      States[i] = 0;
    flt_IIR_casBiquad_ref(Input,Coeffs,States,2,NS, ref_out);

    printf("Calling IIR Kernel\n");

    for (i=0; i<4; i++)
      States[i] = 0;

    start_time = hexagon_sim_read_pcycles();
    flt_IIR_casBiquad(Input,Coeffs,States,2,NS, Output);
    Total_cycles = hexagon_sim_read_pcycles() - start_time;

    for (i=0;i<NS;i++) {
        if (fabs(Output[i]- ref_out[i]) > 0.001) {
            printf("### FAILED!!!\n");
            return 1;
        }
    }

    printf("Cycle-count: %lld\n", Total_cycles);
    printf("*** PASS!\n");
    return 0;
}

