/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2009                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
/*  FUNCTIONS      : Decimated 42bit LFSR generator for 1x              */
/*  ARCHITECTURE   : Hexagon                                            */
/*  VERSION        :                                                    */
/*                                                                      */
/*======================================================================*/
/*  REVISION HISTORY:                                                   */
/*  =================                                                   */
/*                                                                      */
/*  Author              Date           Comments                         */
/*  -------------------------------------------------------------       */
/*  DJH                 10/10/09   behavioral model created             */
/*  DJH                 11/10/09   incorportated Mao's idea of          */ 
/*                                 makeing a stater splitter            */  
/*                                 deinterleaving consequetive states   */
/*                                 to make odd and even decimated       */
/*  DJH                 11/11/09   optimized mask transform, generated  */
/*                                 42 bits of masked sequence then      */
/*                                 applied inverse fibonacci to galois  */
/*                                 to deimated matrix to make single    */
/*                                 table                                */
/*======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "longpn.h"
#include "decimate_tab.h"

#define N 42
#define PACKET 4*64

/* test arrays */
UWord64 long_code32_o[PACKET/64];
UWord64 long_code32_c[PACKET/64];
UWord64 long_code64_o[PACKET/64];
UWord64 long_code64_c[PACKET/64];


/* behavior models */
UWord64 gmpy42(UWord64 a, UWord64 b) {
    UWord64 cond, sum = 0;
    UWord64 poly = 0x0000040a8e6f04efLL;
    int i;
    for(i=0; i < N-1; i++)
    {
      if(a & 0x0000020000000000LL) sum = sum ^ b;
      cond = (sum & 0x0000020000000000LL);
      a = a + a; sum = sum + sum;
      if(cond) sum = sum ^ poly;
    }
    if(  a & 0x0000020000000000LL) sum = sum ^ b;
    sum = sum & 0x3FFFFFFFFFFLL;
    return(sum);
}

UWord64 long_code_gen_c( UWord64 state, UWord64 mask, UWord64 *long_code, int decimation, int num_64chips)
{
  int i1, i2, i3;
  UWord64 feedback, bit;
  UWord64 poly = 0x0000040a8e6f04efLL;

  for(i1 = 0; i1 < num_64chips; i1++){
    feedback = 0;
    for(i2 = 0; i2 < 64; i2++){
      bit = mask & state;
      bit ^= bit >> 32;
      bit ^= bit >> 16;
      bit ^= bit >> 8;
      bit ^= bit >> 4;
      bit ^= bit >> 2;
      bit ^= bit >> 1;
      bit &= 1;
      feedback = feedback | (bit << i2);
      for(i3 = 0; i3 < decimation; i3++){
         state = (state << 1);
         if (state & 0x40000000000LL) state ^= poly;
      }
    }
    long_code[i1] =  feedback;
  }
  return state;
}
      
UWord64 lrand() {
    UWord64 r = (UWord64) rand();
    r = r ^ ((UWord64) rand())<<16;
    r = r ^ ((UWord64) rand())<<26;
    r = r & 0x3ffffffffffLL;
    return(r);
}

int main() {
  int i, k;
  UWord64 error, rms ;
  UWord64 mask, seed0, seed64, seed32 ;
  UWord64 invalpha = (0x0000040a8e6f04efLL)>>1;
  UWord64 states[4];
  UWord64 seedg640, seedg320, seedg641, seedg321; 
  
  unsigned int start_time, cycle_count;

  srand(0xa55aa55a);

  rms = 0;

  for(k=0; k < 16; k++)
  {
    seed0= lrand();
    mask = lrand(); 

    /* do it the hard way, decimating by n */
    seed64 = seed0;
    //retard state by 1 cycle
    seed32 = gmpy42(seed0, invalpha);

    //mechanically read off bits every 32 and 64 cycles
    long_code_gen_c(seed64, mask, long_code64_c, 64, PACKET/64);
    long_code_gen_c(seed32, mask, long_code32_c, 32, PACKET/64);

    /* do it the cool way, pass in 32 or 64 table */
    /* takes ~230 start up cycles + then generates bits/2 cycles*/

    //generate equivalent alpha^n for mask
    //includes retard masked state by 1 cycle
    mask_gen_asm(&seedg320, &seedg640, seed0, mask);  //90

    //apply delay to generate equivalent decimated states for odd and even 64
    seedg640 = calc_dec_asm(seedg640, decimate64); //44

    //apply delay to generate equivalent decimated states for odd and even 32
    seedg320 = calc_dec_asm(seedg320, decimate32); //44

    states[0] = seedg640;
    states[2] = seedg320;
    split_states_asm(states, 42);   //54
    seedg640 = states[0] ; seedg641 = states[1] ;
    seedg320 = states[2] ; seedg321 = states[3] ;

    start_time = hexagon_sim_read_pcycles();

    //generate 64 decimation sequence using 2 parallel 128 and interleave
    long_code_gen_f_asm(&seedg640,&seedg641, long_code64_o, PACKET/64);

    //generate 64 decimation sequence using 2 parallel 64 and interleave
    long_code_gen_f_asm(&seedg320,&seedg321, long_code32_o, PACKET/64); 

    cycle_count = hexagon_sim_read_pcycles() - start_time;

    /* verify its the same as the generic model*/

    for(i=0; i < PACKET/64; i++) {
      UWord64 a = long_code64_c[i];
      UWord64 b = long_code64_o[i];
      error = a - b;
      if (error == 0 && a && b) rms +=  error;
      else  rms += 200;
      a = long_code32_c[i];
      b = long_code32_o[i];
      error = a - b;
      if (error == 0 && a && b) rms +=  error;
      else  rms += 200;
    }
    if(rms != 0x0LL) {
         printf("FAIL!!!!\n"); 
         return 1; 
    }
  }

  printf("*** PASS!\n");
  printf("Cycle-count: %d\n", cycle_count/2);
  return(0);
}
    
/**********************************************************************/
