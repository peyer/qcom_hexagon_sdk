/*======================= COPYRIGHT NOTICE ======================*/ 
/* Copyright (c) 2006 by QUALCOMM, Inc. All rights reserved.     */ 
/* All data and information contained in or disclosed by this    */ 
/* document is confidential and proprietary information of       */ 
/* QUALCOMM, Inc and all rights therein are expressly reserved.  */ 
/* By accepting this material the recipient agrees that this     */ 
/* material and the information contained therein is held in     */ 
/* confidence and in trust and will not be used, copied,         */ 
/* reproduced in whole or in part, nor its contents revealed in  */ 
/* any manner to others without the express written permission   */ 
/* of QUALCOMM, Inc.                                             */ 
/*===============================================================*/ 
/*****************************************************************/ 
/*FileName: dct.c  FileType: C Source                            */ 
/*                                                               */ 
/*Description:                                                   */ 
/*   C function of forward DCT in MPEG-4                         */                                                           
/*                                                               */ 
/*Function List:                                                 */ 
/*   void fDCT (Word16 *input, int num_fdcts,                    */
/*                             Word16    int *output)            */
/*                                                               */
/*Revision History:                                              */ 
/*      Author          Date            Comments                 */ 
/*      -------         ------          ----------               */ 
/*      sxiao           05/24/06        Initial file             */
/*      mzeng           04/22/08        changed for performace   */
/*                                      and cycle-count          */ 
/*                                                               */ 
/*****************************************************************/ 

#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"


/* local definitions */
#define FIX_COS_4_16_Q15 (23170)        /* cos{PI/4)*2^15       */
#define FIX_COS_6_16_Q15 (12540)        /* cos(6*PI/16)*2^15    */
#define FIX_COS_2_16_Q15 (30274)        /* cos(2*PI/16)*2^15    */
#define FIX_COS_7_16_Q15 (6393)         /* cos(7*PI/16)*2^15    */
#define FIX_COS_1_16_Q15 (32138)        /* cos(PI/16)*2^15      */
#define FIX_COS_3_16_Q15 (27246)        /* cos(3*PI/16)*2^15    */
#define FIX_COS_5_16_Q15 (18205)        /* cos(5*PI/16)*2^15    */


#define FIX_COS_4_16_Q13 (5792)         /* cos{PI/4)*2^13       */
#define FIX_COS_1_16_Q13 (8034)         /* cos(PI/16)*2^13      */
#define FIX_COS_7_16_Q13 (1598)         /* cos(7*PI/16)*2^13    */
#define FIX_COS_2_16_Q13 (7568)         /* cos(2*PI/16)*2^13    */
#define FIX_COS_6_16_Q13 (3135)         /* cos(6*PI/16)*2^13    */
#define FIX_COS_3_16_Q13 (6811)         /* cos(3*PI/16)*2^13    */
#define FIX_COS_5_16_Q13 (4551)         /* cos(5*PI/16)*2^13    */

/****************************************************************/
/*Function      fDCT()                                          */
/*Description   Modify Loeffler's DCT to fit Hexagon programming*/
/*Arguments     input -- ptr to input data                      */
/*              output -- ptr to output data                    */
/*              num_fdcts -- number of 8x8 blocks to process    */
/*Return Value  None                                            */
/****************************************************************/ 

void fDCT (short int *input, int num_fdcts, short int *output )
{
    // input is 9 bits value (signed), output should be bounded by 12 bits (signed)

    Word16 x0,x1,x2,x3,x4,x5,x6,x7;              /* Flowgraph inputs/outputs */
    Word16 s0,s1,s2,s3,s4,s5,s6,s7;              /* Stage 1 outputs          */
    Word16 t0,t1,t2,t3,t4,t5,t6,t7;              /* Stage 2 outputs          */
    Word16 u0,u1,u2,u3,u4,u5,u6,u7;              /* Stage 3 outputs          */
    Word16 w0,w1,w2,w3,w4,w5,w6,w7;              /* Stage 4 outputs          */
    int i, ctr;

    if (!num_fdcts) return;

    for (i=0; i<num_fdcts; i++) {

       // horizontal process 
       for (ctr = 0; ctr < 8; ctr++) {

          x0 = input[i*64+ctr*8+0];
          x1 = input[i*64+ctr*8+1];
          x2 = input[i*64+ctr*8+2];
          x3 = input[i*64+ctr*8+3];
          x4 = input[i*64+ctr*8+4];
          x5 = input[i*64+ctr*8+5];
          x6 = input[i*64+ctr*8+6];
          x7 = input[i*64+ctr*8+7];

          // Stage 1

          s0 = x0 + x7;
          s1 = x1 + x6;
          s2 = x3 + x4;
          s3 = x2 + x5;
          s4 = x0 - x7;
          s5 = x1 - x6;
          s6 = x3 - x4;
          s7 = x2 - x5;

          // Stage 2

          t0 = s0;
          t1 = s1;
          t2 = s2;
          t3 = s3;
          t4 = s4;
          t5 = s6;
          t6 = mac_r(L_mult(FIX_COS_4_16_Q15, s5),  FIX_COS_4_16_Q15, s7);
          t7 = msu_r(L_mult(FIX_COS_4_16_Q15, s5),  FIX_COS_4_16_Q15, s7);

          //Stage 3

          u0 = t0 + t2;
          u1 = t1 + t3;
          u2 = t0 - t2;
          u3 = t1 - t3;
          u4 = t4 + t6;
          u5 = t5 + t7;
          u6 = t4 - t6;
          u7 = t5 - t7;

          //stage 4

          w0 = mac_r(L_mult(FIX_COS_4_16_Q15, u0),  FIX_COS_4_16_Q15, u1);
          w4 = msu_r(L_mult(FIX_COS_4_16_Q15, u0),  FIX_COS_4_16_Q15, u1);
          w2 = mac_r(L_mult(FIX_COS_2_16_Q15, u2),  FIX_COS_6_16_Q15, u3);
          w6 = msu_r(L_mult(FIX_COS_6_16_Q15, u2),  FIX_COS_2_16_Q15, u3);
          w1 = mac_r(L_mult(FIX_COS_1_16_Q15, u4),  FIX_COS_7_16_Q15, u5);
          w7 = msu_r(L_mult(FIX_COS_7_16_Q15, u4),  FIX_COS_1_16_Q15, u5);
          w5 = mac_r(L_mult(FIX_COS_5_16_Q15, u6),  FIX_COS_3_16_Q15, u7);
          w3 = msu_r(L_mult(FIX_COS_3_16_Q15, u6),  FIX_COS_5_16_Q15, u7);


          output[i*64+ctr*8+0] = w0;
          output[i*64+ctr*8+1] = w1;
          output[i*64+ctr*8+2] = w2;
          output[i*64+ctr*8+3] = w3;
          output[i*64+ctr*8+4] = w4;
          output[i*64+ctr*8+5] = w5;
          output[i*64+ctr*8+6] = w6;
          output[i*64+ctr*8+7] = w7;
       }       

       // column process
       for (ctr = 0; ctr < 8; ctr++) {

          x0 = output[i*64+0*8+ctr];
          x1 = output[i*64+1*8+ctr];
          x2 = output[i*64+2*8+ctr];
          x3 = output[i*64+3*8+ctr];
          x4 = output[i*64+4*8+ctr];
          x5 = output[i*64+5*8+ctr];
          x6 = output[i*64+6*8+ctr];
          x7 = output[i*64+7*8+ctr];

          // Stage 1

          s0 = x0 + x7;
          s1 = x1 + x6;
          s2 = x3 + x4;
          s3 = x2 + x5;
          s4 = x0 - x7;
          s5 = x1 - x6;
          s6 = x3 - x4;
          s7 = x2 - x5;

          // Stage 2

          t0 = s0;
          t1 = s1;
          t2 = s2;
          t3 = s3;
          t4 = s4;
          t5 = s6;
          t6 = mac_r(L_mult(FIX_COS_4_16_Q15, s5),  FIX_COS_4_16_Q15, s7);
          t7 = msu_r(L_mult(FIX_COS_4_16_Q15, s5),  FIX_COS_4_16_Q15, s7);

          //Stage 3

          u0 = t0 + t2;
          u1 = t1 + t3;
          u2 = t0 - t2;
          u3 = t1 - t3;
          u4 = t4 + t6;
          u5 = t5 + t7;
          u6 = t4 - t6;
          u7 = t5 - t7;

          //stage 4

          w0 = mac_r(L_mult(FIX_COS_4_16_Q13, u0),  FIX_COS_4_16_Q13, u1);
          w4 = msu_r(L_mult(FIX_COS_4_16_Q13, u0),  FIX_COS_4_16_Q13, u1);
          w2 = mac_r(L_mult(FIX_COS_2_16_Q13, u2),  FIX_COS_6_16_Q13, u3);
          w6 = msu_r(L_mult(FIX_COS_6_16_Q13, u2),  FIX_COS_2_16_Q13, u3);
          w1 = mac_r(L_mult(FIX_COS_1_16_Q13, u4),  FIX_COS_7_16_Q13, u5);
          w7 = msu_r(L_mult(FIX_COS_7_16_Q13, u4),  FIX_COS_1_16_Q13, u5);
          w5 = mac_r(L_mult(FIX_COS_5_16_Q13, u6),  FIX_COS_3_16_Q13, u7);
          w3 = msu_r(L_mult(FIX_COS_3_16_Q13, u6),  FIX_COS_5_16_Q13, u7);

          output[i*64+0*8+ctr] = w0;
          output[i*64+1*8+ctr] = w1;
          output[i*64+2*8+ctr] = w2;
          output[i*64+3*8+ctr] = w3;
          output[i*64+4*8+ctr] = w4;
          output[i*64+5*8+ctr] = w5;
          output[i*64+6*8+ctr] = w6;
          output[i*64+7*8+ctr] = w7;
       }       
    }
}

