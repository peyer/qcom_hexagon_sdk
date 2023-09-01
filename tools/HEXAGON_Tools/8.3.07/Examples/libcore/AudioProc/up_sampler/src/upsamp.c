/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:02 CST 2008 
****************************************************************************/ 


#include <stdio.h>

#include "typedef.h"
#include "q6basic_op.h"

#define NUMTAPS 29
#define NUMFILT 8

/********************************************************
 *   Upsampling audio signal by 8                       * 
 ********************************************************/

// Coefficients for filter banks
const Word16 upsamp_filt_coefs[8*29] = 
{
1, -20, 9, 33, -42, 
96, -142, 208, -261, 313, -349, 344, -200, 
-304, 7386, 1667, -1078, 798, -573, 405, -277, 
178, -92, 42, 4, -4, 35, -26, 1, 

1, -24, 22, 16, -22, 
75, -127, 207, -287, 382, -487, 597, -651, 
599, 7552, 599, -651, 597, -487, 382, -287, 
207, -127, 75, -22, 16, 22, -24, 1, 

1, -26, 35, -4, 4, 
42, -92, 178, -277, 405, -573, 798, -1078, 
1667, 7386, -304, -200, 344, -349, 313, -261, 
208, -142, 96, -42, 33, 9, -20, 1, 

0, -27, 46, -24, 33, 
1, -42, 124, -228, 376, -588, 913, -1424, 
2837, 6904, -993, 220, 74, -179, 209, -204, 
184, -139, 105, -54, 47, -4, -15, 0, 

-2, -26, 54, -41, 61, 
-43, 18, 50, -145, 293, -526, 920, -1631, 
4029, 6141, -1442, 566, -178, -1, 88, -126, 
139, -119, 100, -58, 55, -14, -10, 0, 

-6, -21, 57, -53, 85, 
-85, 81, -37, -35, 163, -385, 804, -1649, 
5158, 5158, -1649, 804, -385, 163, -35, -37, 
81, -85, 85, -53, 57, -21, -6, 0, 

-10, -14, 55, -58, 100, 
-119, 139, -126, 88, -1, -178, 566, -1442, 
6141, 4029, -1631, 920, -526, 293, -145, 50, 
18, -43, 61, -41, 54, -26, -2, 0, 

-15, -4, 47, -54, 105, 
-139, 184, -204, 209, -179, 74, 220, -993, 
6904, 2837, -1424, 913, -588, 376, -228, 124, 
-42, 1, 33, -24, 46, -27, 0, 0 
};


void upsamp8( const Word16 *inputBuffer, 
              Word16 *filtMem,
              UWord32 numSamples, 
              int mode,
              Word16 *outputBuffer       )
{
    Word64 lTemp;                     
    Word16 cur;
    int i, j, k;
    int step;

    step = (mode)? 2 : 1;

    for (i=0; i<numSamples; i++) { /* Looping over each input sample */

        cur = *inputBuffer;
        inputBuffer += step;

        for (j=0; j<NUMFILT; j++)
        {  /* Looping over each polyphase filter */

            lTemp = upsamp_filt_coefs[j*NUMTAPS] * cur;

            for (k=0; k<NUMTAPS-1; k++) 
            {  /* Looping over each filter tap */
               lTemp += upsamp_filt_coefs[j*NUMTAPS+1+k] * filtMem[k];
            }

            /* Divide by 2^16 and multiplied by 8 */
            lTemp = lTemp >> 13;
            *outputBuffer = saturate((Word32)lTemp) ;
            outputBuffer += step;
        }

        /* Update of the filter memory */ 
        for (k=NUMTAPS-2; k>=1; k--) 
        {
            filtMem[k] = filtMem[k-1];
        }
        filtMem[0] = cur;
    }
}
