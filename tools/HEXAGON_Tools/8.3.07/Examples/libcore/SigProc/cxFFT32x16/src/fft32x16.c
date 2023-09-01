/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/*****************************************************************/
/*     This program is a fixed-point implementation of           */
/*     radix-4 FFT. It is used as reference for ASM              */
/*     implementation.                                           */
/*****************************************************************/

#include <stdio.h>
#include "q6basic_op.h"

#include "fft_util.h"



void fft32x16(CWord2x32 *input, int N, CWord2x16 *w, CWord2x32 *output) {

  int i, j, k1, k2, n, m;
  int LOG2N;
  CWord2x16 Wa,Wb,Wc;
  CWord2x32 A[4];

 /*************************************/
 /*    Stage 1                        */
 /*  read input in bit-reversed order */
 /*************************************/
    LOG2N =  31 - cl0(N); 

    for(i=0, m=0; i<N; i+=4) 
    {
        A[0]= input[ bitrev(i  , LOG2N) ];
        A[1]= input[ bitrev(i+1, LOG2N) ];
        A[2]= input[ bitrev(i+2, LOG2N) ];
        A[3]= input[ bitrev(i+3, LOG2N) ];

        Radix4BTFLYqv3sc32( A );

        Wb = w[m++];      // Wb = w[j];
        Wa = w[m++];      // Wa = w[2*j+1];
        Wc = w[m++];      // Wc = cmult_r(Wa,Wb);

        output[i  ] = A[0];
        output[i+1] = L_cmult32x16( A[1], Wa );
        output[i+2] = L_cmult32x16( A[2], Wb );
        output[i+3] = L_cmult32x16( A[3], Wc );
    }

  /************************************/ 
  /*  Other Radix-4 stages            */
  /************************************/
    k1=  4;                           // # in each group
    k2 = N/16;                        // # of groups

    for(n = LOG2N-2; n > 2 ; n-=2) 
    {
       for (i=0, m=0; i< k2; i++) 
       {
          Wb = w[m++];      // Wb = w[i];
          Wa = w[m++];      // Wa = w[2*i+1];
          Wc = w[m++];      // Wc = cmult_r(Wa,Wb);

          for(j=0; j< k1; j++) 
          {
               A[0]= output[(4*i + 0)*k1 + j ];
               A[1]= output[(4*i + 1)*k1 + j ];
               A[2]= output[(4*i + 2)*k1 + j ];
               A[3]= output[(4*i + 3)*k1 + j ];  

               Radix4BTFLYqv3sc32( A );

               output[(4*i + 0)*k1 + j] = A[0];
               output[(4*i + 1)*k1 + j] = L_cmult32x16(A[1], Wa);
               output[(4*i + 2)*k1 + j] = L_cmult32x16(A[2], Wb);
               output[(4*i + 3)*k1 + j] = L_cmult32x16(A[3], Wc);
          }
       }
       k1 = k1 << 2;
       k2 = k2 >> 2;
    }

    if(n==2) 
    {
      /************************************/ 
      /*  last Radix-4 stage              */
      /************************************/
      for(j=0; j< N/4; j++) 
      {
           A[0]= output[ 0*(N/4) + j ];
           A[1]= output[ 1*(N/4) + j ];
           A[2]= output[ 2*(N/4) + j ];
           A[3]= output[ 3*(N/4) + j ];  

           Radix4BTFLYsc32( A );

           output[ 0*(N/4) + j] = A[0];
           output[ 1*(N/4) + j] = A[1];
           output[ 2*(N/4) + j] = A[2];
           output[ 3*(N/4) + j] = A[3];
      }
    }
    else if(n==1)
    {
      /************************************/ 
      /*  last Radix-2 stage              */
      /************************************/
      for(i=0; i<N/2; i++) 
      {
          A[0] = output[i    ];
          A[1] = output[i+N/2];

          Radix2BTFLYsc32( A );

          output[i    ] = A[0];	
          output[i+N/2] = A[1];	
      }
    }

}



