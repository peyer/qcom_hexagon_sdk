/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/* ======================================================================== */
/*  FUNCTION:    void sobel                                                 */
/*  DESCRIPTION:                                                            */
/*                                                                          */
/*      Apply following horizontal and vertical masks to input              */
/*                                                                          */
/*          Horizontal:                                                     */
/*               1  2  1                                                    */
/*               0  0  0                                                    */
/*              -1 -2 -1                                                    */
/*                                                                          */
/*          Vertical:                                                       */
/*               1  0  -1                                                   */
/*               2  0  -2                                                   */
/*               1  0  -1                                                   */
/*                                                                          */
/* ======================================================================== */

#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"

void sobel( UWord8 *in, int width, int height, UWord8 *out)
{
    UWord8 p00, p01, p02,p10,p11,p12,p20,p21,p22;
    Word32 H, V, SUM;
    int i, j, idx;


    for (i = 1; i < height-1 ; i++) {

        for (j = 1; j < width-1; j++) { 

           idx = i*width + j;
           /* ------------------------------------------------------------- */
           /*  load 3x3 neighbour pixels from the input.                    */
           /* ------------------------------------------------------------- */
           p00 = in[idx - width -1];
           p01 = in[idx - width   ];
           p02 = in[idx - width +1];

           p10 = in[idx         -1];
           p11 = in[idx           ];
           p12 = in[idx         +1];

           p20 = in[idx + width -1];
           p21 = in[idx + width   ];
           p22 = in[idx + width +1];
           /* ------------------------------------------------------------- */
           /*  Apply horizontal and vertical filter masks.                  */
           /* ------------------------------------------------------------- */
           H =   p00 + 2*p01 + p02 
               - p20 - 2*p21 - p22;

           V =    p00 -   p02 + 
                2*p10 - 2*p12 +
                  p20 -   p22 ;

           SUM = L_abs(H) + L_abs(V);

           /* ------------------------------------------------------------- */
           /*  Saturate output.                                             */
           /* ------------------------------------------------------------- */
           out[idx] = (SUM > 255) ? 255 : SUM;

        }
    }
}

