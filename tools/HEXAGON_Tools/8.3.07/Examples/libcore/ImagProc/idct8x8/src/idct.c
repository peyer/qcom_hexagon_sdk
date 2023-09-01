/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"


#define FIX_N0_707106781 (-23170)       /* -cos(PI/4)*2^15      */
#define FIX_0_707106781 (23170)         /* cos{PI/4)*2^15       */
#define FIX_0_382683432 (12540)         /* cos(6*PI/16)*2^15    */
#define FIX_0_923879532 (30274)         /* cos(2*PI/16)*2^15    */
#define FIX_0_195090322 (6393)          /* cos(7*PI/16)*2^15    */
#define FIX_0_980785280 (32138)         /* cos(PI/16)*2^15      */
#define FIX_0_831469612 (27246)         /* cos(3*PI/16)*2^15    */
#define FIX_0_555570233 (18205)         /* cos(5*PI/16)*2^15    */



void iDCT(Word16 *input, int num_idcts, Word16 *output) {
  
  /* Create an array to allow easy indexing of output */
  Word16 x0,x1,x2,x3,x4,x5,x6,x7;              /* Flowgraph inputs/outputs */
  Word16 s0,s1,s2,s3,s4,s5,s6,s7;              /* Stage 1 outputs          */
  Word16 t0,t1,t2,t3,t4,t5,t6,t7;              /* Stage 2 outputs          */
  Word16 u0,u1,u2,u3,u4,u5,u6,u7;              /* Stage 3 outputs          */
  Word16 w0,w1,w2,w3,w4,w5,w6,w7;              /* Stage 4 outputs          */

  int i, ctr;                                 

  for(i=0; i< num_idcts; i++)  {
    for ( ctr = 0;ctr < 8;ctr++) {
      /* Perform IDCT  on the 8 rows */
      x0 = input[i*64+ctr*8+0];
      x1 = input[i*64+ctr*8+1];
      x2 = input[i*64+ctr*8+2];
      x3 = input[i*64+ctr*8+3];
      x4 = input[i*64+ctr*8+4];
      x5 = input[i*64+ctr*8+5];
      x6 = input[i*64+ctr*8+6];
      x7 = input[i*64+ctr*8+7];

      /****************/
      /* 1st stage    */
      /****************/

      s0 = mac_r(L_mult(FIX_0_707106781,x0), FIX_0_707106781, x4);
      s1 = msu_r(L_mult(FIX_0_707106781,x0), FIX_0_707106781, x4);
      s2 = mac_r(L_mult(FIX_0_923879532,x2), FIX_0_382683432, x6);
      s3 = msu_r(L_mult(FIX_0_382683432,x2), FIX_0_923879532, x6);
      s4 = mac_r(L_mult(FIX_0_980785280,x1), FIX_0_195090322, x7);
      s5 = msu_r(L_mult(FIX_0_195090322,x1), FIX_0_980785280, x7);
      s6 = mac_r(L_mult(FIX_0_831469612,x3), FIX_0_555570233, x5);
      s7 = msu_r(L_mult(FIX_0_831469612,x5), FIX_0_555570233, x3);
      
      /****************/
      /* 2nd stage    */
      /****************/

      t0 = s0 + s2;
      t1 = s1 + s3;
      t2 = s0 - s2;
      t3 = s1 - s3;
      t4 = s4 + s6;
      t5 = s5 + s7;
      t6 = s4 - s6;
      t7 = s5 - s7;

      /****************/
      /* 3rd stage    */
      /****************/

      u0 = t0;
      u1 = t1;
      u2 = t2;
      u3 = t3;
      u4 = t4;
      u5 = msu_r(L_mult(FIX_0_707106781,t7), FIX_N0_707106781, t6);
      u6 = t5;
      u7 = mac_r(L_mult(FIX_0_707106781,t6), FIX_N0_707106781, t7);

      /****************/
      /* 4th stage    */
      /****************/

      output[i*64+ctr*8+0] = u0 + u4;
      output[i*64+ctr*8+1] = u1 + u5;
      output[i*64+ctr*8+3] = u2 + u6;
      output[i*64+ctr*8+2] = u3 + u7;
      output[i*64+ctr*8+7] = u0 - u4;
      output[i*64+ctr*8+6] = u1 - u5;
      output[i*64+ctr*8+4] = u2 - u6;
      output[i*64+ctr*8+5] = u3 - u7;

    }


    for ( ctr = 0;ctr < 8;ctr++) {
      /* Perform IDCT  on the 8 columns */
      
      x0 = output[i*64+0*8+ctr];
      x1 = output[i*64+1*8+ctr];
      x2 = output[i*64+2*8+ctr];
      x3 = output[i*64+3*8+ctr];
      x4 = output[i*64+4*8+ctr];
      x5 = output[i*64+5*8+ctr];
      x6 = output[i*64+6*8+ctr];
      x7 = output[i*64+7*8+ctr];

      /****************/
      /* 1st stage    */
      /****************/

      s0 = mac_r(L_mult(FIX_0_707106781,x0), FIX_0_707106781, x4);
      s1 = msu_r(L_mult(FIX_0_707106781,x0), FIX_0_707106781, x4);
      s2 = mac_r(L_mult(FIX_0_923879532,x2), FIX_0_382683432, x6);
      s3 = msu_r(L_mult(FIX_0_382683432,x2), FIX_0_923879532, x6);
      s4 = mac_r(L_mult(FIX_0_980785280,x1), FIX_0_195090322, x7);
      s5 = msu_r(L_mult(FIX_0_195090322,x1), FIX_0_980785280, x7);
      s6 = mac_r(L_mult(FIX_0_831469612,x3), FIX_0_555570233, x5);
      s7 = msu_r(L_mult(FIX_0_831469612,x5), FIX_0_555570233, x3);
      
      s0 = s0 + 31;
      s1 = s1 + 32;

      /****************/
      /* 2nd stage    */
      /****************/

      t0 = s0 + s2;
      t1 = s1 + s3;
      t2 = s0 - s2;
      t3 = s1 - s3;
      t4 = s4 + s6;
      t5 = s5 + s7;
      t6 = s4 - s6;
      t7 = s5 - s7;

      /****************/
      /* 3rd stage    */
      /****************/

      u0 = t0;
      u1 = t1;
      u2 = t2;
      u3 = t3;
      u4 = t4;
      u5 = msu_r(L_mult(FIX_0_707106781,t7), FIX_N0_707106781, t6);
      u6 = t5;
      u7 = mac_r(L_mult(FIX_0_707106781,t6), FIX_N0_707106781, t7);

      /****************/
      /* 4th stage    */
      /****************/

      w0 = u0 + u4;
      w1 = u1 + u5;
      w2 = u2 + u6;
      w3 = u3 + u7;
      w4 = u0 - u4;
      w5 = u1 - u5;
      w6 = u2 - u6;
      w7 = u3 - u7;

      w0 = w0>0x3FFF?0x3FFF : w0<-0x4000?-0x4000 : w0;
      w1 = w1>0x3FFF?0x3FFF : w1<-0x4000?-0x4000 : w1;
      w2 = w2>0x3FFF?0x3FFF : w2<-0x4000?-0x4000 : w2;
      w3 = w3>0x3FFF?0x3FFF : w3<-0x4000?-0x4000 : w3;
      w4 = w4>0x3FFF?0x3FFF : w4<-0x4000?-0x4000 : w4;
      w5 = w5>0x3FFF?0x3FFF : w5<-0x4000?-0x4000 : w5;
      w6 = w6>0x3FFF?0x3FFF : w6<-0x4000?-0x4000 : w6;
      w7 = w7>0x3FFF?0x3FFF : w7<-0x4000?-0x4000 : w7;
          
      output[i*64+0*8+ctr] = w0 >> 6;
      output[i*64+1*8+ctr] = w1 >> 6;
      output[i*64+3*8+ctr] = w2 >> 6;
      output[i*64+2*8+ctr] = w3 >> 6;
      output[i*64+7*8+ctr] = w4 >> 6;
      output[i*64+6*8+ctr] = w5 >> 6;
      output[i*64+4*8+ctr] = w6 >> 6;
      output[i*64+5*8+ctr] = w7 >> 6;
    }
  }
}


