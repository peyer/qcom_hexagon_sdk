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
*  FILENAME:      luma_filt.c
*  Authors:       Bob Hung, KC Lai
*
******************************************************************************
*  FUNCTION:      luma_filt()
*                 - This function implements epsilon filter for luma component
*
*****************************************************************************/
#include <stdio.h>
#include "typedef.h"
#include "lumafilt.h"


void luma_filt( UWord8 *ch, int width, int height,  
                 LumaFiltParams *params, UWord8 *out)
{
    Word16 roi[9];
    Word16 tmp;
    Word16 *coef, bitshift;
    Word16 s1,s2,s3,s4, smax;
    Word16 s1_roi0, s1_roi1, s1_roi2;
    Word16 s2_roi0, s2_roi1, s2_roi2;
    Word16 s3_roi0, s3_roi1, s3_roi2;
    Word16 s4_roi0, s4_roi1, s4_roi2;
    Word32 sum;
    int lp_flag, hp_flag;
    int i,j,k,m,n, idx, idx2;

    //========================================================================
    // filtering (excluding the boundary pixels)
    //========================================================================

    for(i=1; i<height-1; i++) {
        for(j=1; j<width-1; j++) {

            // for each pixel location, select the 3x3 ROI
            idx= i*width + j;
            k=0;
            for (m=-1; m<=1; m++) { 
                for (n=-1; n<=1; n++) {
                    idx2= idx + m*width + n;
                    roi[k] = ch[idx2];
                    k++;
                }
            }

            // Use Sobel filter to detect the edge 
            s1_roi0 = (params->mask_sel==0) ? roi[3] : roi[6]; 
            s1_roi1 = (params->mask_sel==0) ? roi[4] : roi[7]; 
            s1_roi2 = (params->mask_sel==0) ? roi[5] : roi[8]; 

            s2_roi0 = (params->mask_sel==0) ? roi[1] : roi[2]; 
            s2_roi1 = (params->mask_sel==0) ? roi[4] : roi[5]; 
            s2_roi2 = (params->mask_sel==0) ? roi[7] : roi[8]; 

            s3_roi0 = (params->mask_sel==0) ? roi[6] : roi[7]; 
            s3_roi1 = (params->mask_sel==0) ? roi[4] : roi[8]; 
            s3_roi2 = (params->mask_sel==0) ? roi[2] : roi[5]; 

            s4_roi0 = (params->mask_sel==0) ? roi[0] : roi[3]; 
            s4_roi1 = (params->mask_sel==0) ? roi[4] : roi[6]; 
            s4_roi2 = (params->mask_sel==0) ? roi[8] : roi[7]; 

            // for horizontal edge
            s1  = roi[0] - s1_roi0;
            tmp = roi[1] - s1_roi1;
            s1  = s1 + tmp;
            tmp = roi[2] - s1_roi2;
            s1  = s1 + tmp;
            s1  = s1<0? -s1:s1;

            // for vertical edge
            s2  = roi[0] - s2_roi0;
            tmp = roi[3] - s2_roi1;
            s2  = s2 + tmp;
            tmp = roi[6] - s2_roi2;
            s2  = s2 + tmp;
            s2  = s2<0? -s2:s2;

            // for 45 degree angle
            s3  = roi[3] - s3_roi0;
            tmp = roi[0] - s3_roi1;
            s3  = s3 + tmp;
            tmp = roi[1] - s3_roi2;
            s3  = s3 + tmp;
            s3  = s3<0? -s3:s3 ;

            // for -45 degree angle
            s4  = roi[1] - s4_roi0;
            tmp = roi[2] - s4_roi1;
            s4  = s4 + tmp;
            tmp = roi[5] - s4_roi2;
            s4  = s4 + tmp;
            s4  = s4<0? -s4:s4;

            // find out the max of the edges
            smax = (s1>s2)   ? s1 : s2;
            smax = (s3>smax) ? s3 : smax;
            smax = (s4>smax) ? s4 : smax;

            lp_flag = (smax < params->lp_epsilon) ? 1: 0;
            hp_flag = (smax > params->hp_epsilon) ? 1: 0;

            // examine the flag values and determine what filter to apply
            if (hp_flag==1 && lp_flag==0) {
                 // do high pass filtering
                coef     = params->hp_coeff;
                bitshift = params->hp_shift;
            }
            else if (hp_flag==0 && lp_flag==1) {
                 // do low pass filtering
                coef     = params->lp_coeff;
                bitshift = params->lp_shift;
            }
            else {
                 // do nothing
                coef     = params->identity_coef;
                bitshift = 0;
            }

            // do filtering
            sum=0;
            for (k=0; k<9; k++) {
                sum += coef[k] * roi[k];
            }
            // After convolution, do shifting
            sum= sum >>bitshift;

            // Do clamping
            out[idx]= sum<0 ? 0:sum >255 ? 255:sum;
                                                        
        }
    }
    
   return;
}


