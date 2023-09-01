/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/**********************************************************************/ 
/*     Implements the viterbi algorithm to decode the convolutional   */
/*     codes used in the GSM full rate standard                       */
/**********************************************************************/ 

#include <stdio.h>
#include "typedef.h"

#include "vitgsm.h"

#define DEBUG         0


void vitgsm(signed char *INsoft, unsigned short int *Output_word)
{ 
// use 16-bits for PM and BM
  Word16 m0, m1, a, b;
  Word16 old[16],new[16];
  Word16 *tmp, *old_p, *new_p;
  UWord16 trans[N];

  int bit, state, bitpos, i;

// Initialize accumulated metric array 
        for (i=0; i<16; i++)
	        old[i] = 0;

// Add-compare-select 
        new_p=new;
        old_p=old;
        
	for (i=0; i<N; i++) 
	{
		m0 = INsoft[2*i] + INsoft[2*i + 1];
		m1 = INsoft[2*i] - INsoft[2*i + 1];
                trans[i]=0;

		a = old_p[7] + m1;	b = old_p[15] - m1;
		new_p[15] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[7] - m1;	b = old_p[15] + m1;
		new_p[14] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

		a = old_p[6] + m0;	b = old_p[14] - m0;
		new_p[13] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[6] - m0;	b = old_p[14] + m0;
		new_p[12] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

		a = old_p[5] + m1;	b = old_p[13] - m1;
		new_p[11] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[5] - m1;	b = old_p[13] + m1;
		new_p[10] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

		a = old_p[4] + m0;	b = old_p[12] - m0;
		new_p[9] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[4] - m0;	b = old_p[12] + m0;
		new_p[8] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

		a = old_p[3] - m1;	b = old_p[11] + m1;
		new_p[7] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[3] + m1;	b = old_p[11] - m1;
		new_p[6] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

		a = old_p[2] - m0;	b = old_p[10] + m0;
		new_p[5] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[2] + m0;	b = old_p[10] - m0;
		new_p[4] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

		a = old_p[1] - m1;	b = old_p[9] + m1;
		new_p[3] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[1] + m1;	b = old_p[9] - m1;
		new_p[2] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

                a = old_p[0] - m0;	b = old_p[8] + m0;
		new_p[1] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);
		a = old_p[0] + m0;	b = old_p[8] - m0;
		new_p[0] = (b>a)?b:a;  trans[i] = (trans[i]<<1)|(b>a);

	        tmp   = old_p;	
                old_p = new_p;
                new_p = tmp;

#if DEBUG
// print states of each stage    

                for (j=0; j<16; j++)
                  printf("%4x ", /* (unsigned short)*/ old_p[j]);
               printf("\n"); 
#endif

	}

#if DEBUG
// print 189 transition words 

        printf("Transition Words\n");

        for (i=0; i< 23; i++)
          { for (j=0; j< 8; j++)
               printf("%4x ", (unsigned short) trans[8*i+j]);
            printf("\n");
          }
        for (i=0; i<5; i++)
           printf("%4x ", (unsigned short) trans[184+i]);
        printf("\n");
        
#endif

//##############################################################
// Traceback 
// 189 information bits are stored into 12 words according to
//     output word 0:
//        bit15 = info bit 15 
//        bit14 = info bit 14
//        ......
//        bit0  = info bit 0
//     output word 1:
//        bit15 = info bit 31
//        .....
//        bit0 = info bit 16
//     .....
//##############################################################
 
	state = 0;
        Output_word[(N-1)>>4] = 0;

	for (i=N-1; i>=0; i--) {
		bitpos = state & 15;
		bit = (trans[i]>>bitpos) & 1;
                Output_word[i>>4] = (Output_word[i>>4] <<1) | (state & 1);
		state =  ( bit<<3) | (state >>1);
	}

       return;
}

