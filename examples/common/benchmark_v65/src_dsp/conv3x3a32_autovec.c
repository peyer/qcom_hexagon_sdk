/**=============================================================================

@file
   conv3x3a32_autovec.c

@brief
   Modified from conv3x3_ref() within conv3x3_ref.c to illustrate how to use the automatic vectorization,
   which is introduced within Hexagon Tool 8.3.
   
Copyright (c) 2018 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/


void conv3x3Per2Row(unsigned char * restrict inp __attribute__((align_value(128))), int stride_i, int width, signed char * restrict mask, 
                    int shift, unsigned char * restrict outp, int stride_o)
{
     int x, y;
     int sum;     
    
     for (y = 0; y < 2; y++)
     {
       for (x = 1; x < width - 1; x++)
       {
           sum = inp[(y-1)*stride_i + (x-1)] * mask[0]
               + inp[(y-1)*stride_i + (x)] * mask[1]
               + inp[(y-1)*stride_i + (x+1)] * mask[2]
               + inp[(y)*stride_i + (x-1)] * mask[4]
               + inp[(y)*stride_i + (x)] * mask[5]
               + inp[(y)*stride_i + (x+1)] * mask[6]
               + inp[(y+1)*stride_i + (x-1)] * mask[8]
               + inp[(y+1)*stride_i + (x)] * mask[9]
               + inp[(y+1)*stride_i + (x+1)] * mask[10];
               
           sum = sum >> shift;     // shift
           outp[y*stride_o + x] = (sum > 255) ? 255 : (sum < 0 ? 0 : sum);   // sat
       } 
     }    
}