/*****************************************************************************
*
*  Qualcomm Proprietary
*  Copyright (c) 2004  Qualcomm Incorporated.  All rights reserved.
*
*  All data and information contained in or disclosed by this document are
*  confidential and proprietary information of Qualcomm Incorporated, and
*  all rights therein are expressly reserved. By accepting this material,
*  the recipient agrees that this material and the information contained
*  therein are held in confidence and in trust and will not be used,
*  copied, reproduced in whole or in part, nor its contents revealed in
*  any manner to others without the express written permission of Qualcomm
*  param_incorporated.
*
*  This technology was exported from the United States in accordance with
*  the Export Administration Regulations. Diversion contrary to U.S. law
*  prohibited.
*
******************************************************************************
*  FILENAME:        color_correct.c
*  Authors:         Ananth, Bob Hung, KC Lai
*
******************************************************************************
*  FUNCTION:        color_correct()
*                   -This function implements the color correction module
*
******************************************************************************
*  SOURCE:            MSM7k VFE HW C simulation code
*****************************************************************************/
#include "stdio.h"
#include "typedef.h"
#include "color_correct.h"

void color_correct( UWord16 *R, UWord16 *G, UWord16 *B,
                    int width, int height, ColorCorrectParams *params)
{
    Word32 lG, lB, lR;
    Word16 C[9], K[3];
    UWord16 Sf;
    int i;

    //------------------------------------------------------------------------
    // Configuration
    //------------------------------------------------------------------------
    C[0]  = params->c0;
    C[1]  = params->c1;
    C[2]  = params->c2;
    C[3]  = params->c3;
    C[4]  = params->c4;
    C[5]  = params->c5;
    C[6]  = params->c6;
    C[7]  = params->c7;
    C[8]  = params->c8;
    K[0]  = params->k0;
    K[1]  = params->k1;
    K[2]  = params->k2;
    Sf = params->qfactor;

    //------------------------------------------------------------------------
    //
    // Filter implementation
    // - C[] are 12-bits, K[] are 11-bits
    // - RGB are 12-bits
    // 
    // Gout = [ ( G*C[0] + B*C[1] + R*C[2] ) >> Sf ] + K[0]
    // Bout = [ ( G*C[3] + B*C[4] + R*C[5] ) >> Sf ] + K[1]
    // Rout = [ ( G*C[6] + B*C[7] + R*C[8] ) >> Sf ] + K[2]
    //
    //------------------------------------------------------------------------
    for (i=0; i<height*width; i++)
    {
        lG = C[0]*G[i] + C[1]*B[i] + C[2]*R[i];
        lG = lG >> Sf;
        lG = lG + K[0];

        lB = C[3]*G[i] + C[4]*B[i] + C[5]*R[i];
        lB = lB >> Sf;
        lB = lB + K[1];

        lR = C[6]*G[i] + C[7]*B[i] + C[8]*R[i];
        lR = lR >> Sf;
        lR = lR + K[2];

        G[i] = lG < 0 ? 0:lG > 4095 ? 4095:lG;  
        B[i] = lB < 0 ? 0:lB > 4095 ? 4095:lB;
        R[i] = lR < 0 ? 0:lR > 4095 ? 4095:lR;
    }

}
