/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#include <math.h>
#include <stdlib.h>

#define VLEN            128

void sobel3x3_ref(const unsigned char* src,
                      unsigned int stride,
                      unsigned int width,
                      unsigned int height,
                      unsigned char* dst)

{
    unsigned char p00, p01, p02,p10,p12,p20,p21,p22;
    unsigned int SUM;
    unsigned int i, j, idx, H, V;

    for (i = 1; i < height-1 ; i++)
    {
        for (j = 1; j < width-1; j++)
        {
           idx = i*stride + j;
           p00 = src[idx - stride -1];
           p01 = src[idx - stride   ];
           p02 = src[idx - stride +1];
           p10 = src[idx             -1];
           p12 = src[idx             +1];
           p20 = src[idx + stride -1];
           p21 = src[idx + stride   ];
           p22 = src[idx + stride +1];
           H =   p00 + 2*p01 + p02
               - p20 - 2*p21 - p22;
           V =   p00 + 2*p10 + p20
               - p02 - 2*p12 - p22;
           SUM = abs((int)H) + abs((int)V);
           dst[i*stride + j] = (SUM > 255) ? 255 : (unsigned char)SUM;
        }
    }
}

