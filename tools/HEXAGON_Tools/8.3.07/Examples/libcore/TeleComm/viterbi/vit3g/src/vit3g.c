/**********************************************************************************
 *  Qualcomm PROPRIETARY                                                          *
 *  Copyright 2006 Qualcomm Incorporated. All rights reserved                     *
 **********************************************************************************/

/**********************************************************************************
 *  Viterbi decoder                                                               *
 **********************************************************************************
 *  Convolutional Code:                                                           *
 *    - Constraint length K = 9                                                   *
 *    - Rate = 1/3                                                                *
 *    - Polynomial:                                                               *
 *       G0 = 557 (octal)                                                         *
 *       G1 = 663 (octal)                                                         *
 *       G2 = 711 (octal)                                                         *
 *			                                                          *
 *  Assumptions:                                                                  *
 *    - K-1 = 8 tail bits (zeroes) appended to 260 data bits (total = 268)        *
 *    - For each input soft decision:                                             *
 *        negative soft decision -> logic 1                                       *
 *        positive soft decision -> logic 0				          *
 *                                                                                *
 *    - Output format:                                                            *
 *        byte 0 = I0I1....I7                                                     *
 *        byte 1 = I8I9....I15                                                    *
 *        .......                                                                 *
 *        byte 33 = I264..I267,xx..x                                              * 
 *                                                                                *
 *    - Soft-Decision Quantization:                                               *
 *        IF the quantization level is too large, overflow might be a concern.    *
 *                                                                                *
 *                                                                                *
 **********************************************************************************
 *  I/O variables:                                                                *
 *    *insamp -> ptr to input soft decisions (3*268 = 804  signed 8-bit integers) *	
 *                                                                                *
 *    *output -> ptr to packed output decoded bits (34 bytes)                     *
 *                                                                                *
 *    *buffer -> ptr to a working buffer for storage of PM and transit words.     * 
 *               A total of (2x256+ 268x256/16) short integers are needed         *
 *                                                                                *
 *    numbits -> number of decoded bits                                           * 
 *                                                                                *
 *********************************************************************************/

#include "typedef.h"
#include "vit3g.h"

void vit3g_1r3k9 (Word8 *insamp, UWord8 *output, short *buffer, int numbits)
{
  UWord8 metric[(NUM_STATES/2)] = {0, 3, 5, 6, 6, 5, 3, 0, 2, 1, 7, 4, 4, 7, 1, 2,
				 5, 6, 0, 3, 3, 0, 6, 5, 7, 4, 2, 1, 1, 2, 4, 7,
				 4, 7, 1, 2, 2, 1, 7, 4, 6, 5, 3, 0, 0, 3, 5, 6,
				 1, 2, 4, 7, 7, 4, 2, 1, 3, 0, 6, 5, 5, 6, 0, 3,
				 6, 5, 3, 0, 0, 3, 5, 6, 4, 7, 1, 2, 2, 1, 7, 4,
				 3, 0, 6, 5, 5, 6, 0, 3, 1, 2, 4, 7, 7, 4, 2, 1,
				 2, 1, 7, 4, 4, 7, 1, 2, 0, 3, 5, 6, 6, 5, 3, 0,
				 7, 4, 2, 1, 1, 2, 4, 7, 5, 6, 0, 3, 3, 0, 6, 5}; 

       
  Word16 *state = buffer;                            // pres state and next state metrics  
  Word16 M[8], bm;
  Word16 s0,s1,s2,s3;
  Word16 *ps, *ns, *temp;
  UWord16 *trace = (UWord16 *)buffer + (2*NUM_STATES);  // trace buffer 
  int bit, wordpos,bitpos,trace_state;

  int i, j, k;

  for (i=0; i < (NUM_STATES); i++)		// Set all PM values to 0 
	state[i]=0 ;		
    
  ps = state;				        // Points to read buffer
  ns = state + NUM_STATES;                      // Points to write buffer

/******************************************************************************** 
 *										* 
 * main Loop:  perfom metric calculation and ACS for number of decoding bits	* 
 *										* 
 ********************************************************************************/

  for (i=0; i < numbits; i++) {

/******************************************************************************** 
 * Metric calculation								* 
 ********************************************************************************/
      M[0] =  insamp[3*i]+insamp[(3*i)+1]+insamp[(3*i)+2];
      M[1] =  insamp[3*i]+insamp[(3*i)+1]-insamp[(3*i)+2];
      M[2] =  insamp[3*i]-insamp[(3*i)+1]+insamp[(3*i)+2];
      M[3] =  insamp[3*i]-insamp[(3*i)+1]-insamp[(3*i)+2];

      M[4] = - M[3];
      M[5] = - M[2];
      M[6] = - M[1];
      M[7] = - M[0];

/******************************************************************************** 
 * Add-Compare-Select (ACS) calculation					        * 
 ********************************************************************************/
      for (j=0; j < (NUM_STATES/2); j++) {

	  k = (i*(NUM_STATES/16))+(j/8);	// Generate starting index for trace word	
	  
	  bm = M[metric[j]];			// Retrieve appropriate metric	

	  s0 = ps[j] + bm;			// First ACS
	  s1 = ps[j+(NUM_STATES/2)] - bm;
	  ns[2*j] = (s1 > s0) ? s1:s0;					
	  trace[k] = (trace[k]>>1)|((s1>s0)<<15);
	   
	  s2 = ps[j] - bm;			// Second ACS			
	  s3 = ps[j+(NUM_STATES/2)] + bm;	// (uses same metric as first)
	  ns[(2*j)+1]=(s3 > s2) ? s3:s2 ;	
	  trace[k] = (trace[k]>>1)|((s3>s2)<<15);
	  
      }

      temp = ps;				// Swap ps and ns pointers 	
      ps= ns;				
      ns = temp;
   }


/******************************************************************************** 
 * Trace back loop for Viterbi Decoding                                         * 
 ********************************************************************************/

   trace_state=0;				// known terminating state

   output[(numbits-1)>>3]=0;			// set un-used bits to 0

   for (i = numbits-1; i >= 0; i--) {
        wordpos = trace_state >>4; 
        bitpos = trace_state & 15;
        bit = (trace[i*(NUM_STATES/16)+wordpos]>>bitpos) & 1;		  
        output[i>>3] = ((unsigned short)output[i>>3] >> 1) | ((trace_state & 1) << 7);
        trace_state =  ( bit<<(K-2)) | (trace_state >>1);
   }
   return;
}



/**********************************************************************************
 *  Viterbi decoder                                                               *
 **********************************************************************************
 *  Convolutional Code:                                                           *
 *    - Constraint length K = 9                                                   *
 *    - Rate = 1/2                                                                *
 *    - Polynomial:                                                               *
 *       G0 = 561 (octal)                                                         *
 *       G1 = 753 (octal)                                                         *
 *                                                                                *
 *  Assumptions:                                                                  *
 *    - K-1 = 8 tail bits (zeroes) appended to data bits                          *
 *    - For each input soft decision:                                             *
 *        negative soft decision -> logic 1                                       *
 *        positive soft decision -> logic 0                                       *
 *                                                                                *
 *    - Output format:                                                            *
 *        byte 0 = I0I1....I7                                                     *
 *        byte 1 = I8I9....I15                                                    *
 *        .......                                                                 *
 *        byte 33 = I264..I267,....                                               * 
 *                                                                                *
 *    - Soft-Decision Quantization:                                               *
 *        IF the quantization level is too large, overflow might be a concern.    *
 *                                                                                *
 *                                                                                *
 **********************************************************************************
 *  I/O variables:                                                                *
 *    *insamp -> ptr to input soft decisions (2*268 = 804  signed 8-bit integers) *	
 *                                                                                *
 *    *output -> ptr to packed output decoded bits (34 bytes)                     *
 *                                                                                *
 *    *buffer -> ptr to a working buffer for storage of PM and transit words.     * 
 *               A total of (2x256+ 268x256/16) short integers are needed         *
 *                                                                                *
 *    numbits -> number of decoded bits                                           * 
 *                                                                                *
 *********************************************************************************/
void vit3g_1r2k9 (Word8 *insamp, UWord8 *output, short *buffer, int numbits)
{
  UWord8 metric[(NUM_STATES/2)] = { 0, 1, 3, 2, 3, 2, 0, 1, 2, 3, 1, 0, 1, 0, 2, 3, 
                                    1, 0, 2, 3, 2, 3, 1, 0, 3, 2, 0, 1, 0, 1, 3, 2, 
                                    0, 1, 3, 2, 3, 2, 0, 1, 2, 3, 1, 0, 1, 0, 2, 3, 
                                    1, 0, 2, 3, 2, 3, 1, 0, 3, 2, 0, 1, 0, 1, 3, 2, 
                                    1, 0, 2, 3, 2, 3, 1, 0, 3, 2, 0, 1, 0, 1, 3, 2, 
                                    0, 1, 3, 2, 3, 2, 0, 1, 2, 3, 1, 0, 1, 0, 2, 3, 
                                    1, 0, 2, 3, 2, 3, 1, 0, 3, 2, 0, 1, 0, 1, 3, 2, 
                                    0, 1, 3, 2, 3, 2, 0, 1, 2, 3, 1, 0, 1, 0, 2, 3 };

       
  Word16 *state = buffer;                               // pres state and next state metrics  
  Word16 M[4], bm;
  Word16 s0,s1,s2,s3;
  Word16 *ps, *ns, *temp;
  UWord16 *trace = (UWord16 *)buffer + (2*NUM_STATES);  // trace buffer 
  int bit, wordpos,bitpos,trace_state;

  int i, j, k;

  for (i=0; i < (NUM_STATES); i++)		// Set all PM values to 0 
	state[i]=0 ;		
    
  ps = state;				        // Points to read buffer
  ns = state + NUM_STATES;                      // Points to write buffer

/******************************************************************************** 
 *										* 
 * main Loop:  perfom metric calculation and ACS for number of decoding bits	* 
 *										* 
 ********************************************************************************/

  for (i=0; i < numbits; i++) {

/******************************************************************************** 
 * Metric calculation								* 
 ********************************************************************************/
      M[0] =  insamp[2*i]+insamp[(2*i)+1];
      M[1] =  insamp[2*i]-insamp[(2*i)+1];
      M[2] = - M[1];
      M[3] = - M[0];


/******************************************************************************** 
 * Add-Compare-Select (ACS) calculation					        * 
 ********************************************************************************/
      for (j=0; j < (NUM_STATES/2); j++) {

	  k = (i*(NUM_STATES/16))+(j/8);	// Generate starting index for trace word	
	  
	  bm = M[metric[j]];			// Retrieve appropriate metric	

	  s0 = ps[j] + bm;			// First ACS
	  s1 = ps[j+(NUM_STATES/2)] - bm;
	  ns[2*j] = (s1 > s0) ? s1:s0;					
	  trace[k] = (trace[k]>>1)|((s1>s0)<<15);
	   
	  s2 = ps[j] - bm;			// Second ACS			
	  s3 = ps[j+(NUM_STATES/2)] + bm;	// (uses same metric as first)
	  ns[(2*j)+1]=(s3 > s2) ? s3:s2 ;	
	  trace[k] = (trace[k]>>1)|((s3>s2)<<15);
	  
      }

      temp = ps;				// Swap ps and ns pointers 	
      ps= ns;				
      ns = temp;
   }


/******************************************************************************** 
 * Trace back loop for Viterbi Decoding                                         * 
 ********************************************************************************/

   trace_state=0;				// known terminating state

   output[(numbits-1)>>3]=0;			// set un-used bits to 0

   for (i = numbits-1; i >= 0; i--) {
        wordpos = trace_state >>4; 
        bitpos = trace_state & 15;
        bit = (trace[i*(NUM_STATES/16)+wordpos]>>bitpos) & 1;		  
        output[i>>3] = ((unsigned short)output[i>>3] >> 1) | ((trace_state & 1) << 7);
        trace_state =  ( bit<<(K-2)) | (trace_state >>1);
   }
   return;
}

