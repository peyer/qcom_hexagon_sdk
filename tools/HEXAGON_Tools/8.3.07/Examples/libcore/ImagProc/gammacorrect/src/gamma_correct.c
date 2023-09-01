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
*  Incorporated.
*
*  This technology was exported from the United States in accordance with
*  the Export Administration Regulations. Diversion contrary to U.S. law
*  prohibited.
*
******************************************************************************
*  FILENAME:      gamma_correct.c
*  Authors:       
*
******************************************************************************
*  FUNCTION:      gamma_correct()
*                 - This function implements gamma correct by LUT
*
*****************************************************************************/
#include <stdio.h>
#include "typedef.h"

void gamma_correct( UWord16 *ch, int width, int height,
                    UWord16 *table, int mode) 
{
   int x;
   int i;

   for (i=0; i<height*width; i++) {

        if(mode) { 

            if (ch[i]<256) 
                 x = ch[i];
            
            else if (ch[i]>=256 && ch[i]<768) 
                    x = (ch[i] >> 1) + 128;

            else if (ch[i]>=768 && ch[i]<1536)
                    x = (ch[i] >> 2) + 320;

            else if (ch[i]>=1536 && ch[i]<2816)
                    x = (ch[i] >> 3) + 512;
            else
                    x = (ch[i] >> 4) + 688;
        }
        else
            x = ch[i]>>2;

        ch[i] = table[x];
    }
}

