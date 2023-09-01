/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "benchmark_ref.h"

#define MAX(a,b)       ((a) > (b) ? (a) : (b))

void dilate5x5_ref (unsigned char *src, int srcStride, int width, int height, unsigned char *dst, int dstStride)
{
  int x, y;
  unsigned char max;
  
  for (y = 2; y < height - 2; y++)
  {
    for (x = 2; x < width - 2; x++)
    {   
        max = src[y*srcStride + x];
        max = MAX(max, src[(y-2)*srcStride + (x-2)]);
        max = MAX(max, src[(y-2)*srcStride + (x-1)]);
        max = MAX(max, src[(y-2)*srcStride + (x+0)]);
        max = MAX(max, src[(y-2)*srcStride + (x+1)]);
        max = MAX(max, src[(y-2)*srcStride + (x+2)]);
        max = MAX(max, src[(y-1)*srcStride + (x-2)]);
        max = MAX(max, src[(y-1)*srcStride + (x-1)]);
        max = MAX(max, src[(y-1)*srcStride + (x+0)]);
        max = MAX(max, src[(y-1)*srcStride + (x+1)]);
        max = MAX(max, src[(y-1)*srcStride + (x+2)]);
        max = MAX(max, src[(y+0)*srcStride + (x-2)]);
        max = MAX(max, src[(y+0)*srcStride + (x-1)]);
        max = MAX(max, src[(y+0)*srcStride + (x+1)]);
        max = MAX(max, src[(y+0)*srcStride + (x+2)]);
        max = MAX(max, src[(y+1)*srcStride + (x-2)]);
        max = MAX(max, src[(y+1)*srcStride + (x-1)]);
        max = MAX(max, src[(y+1)*srcStride + (x+0)]);
        max = MAX(max, src[(y+1)*srcStride + (x+1)]);
        max = MAX(max, src[(y+1)*srcStride + (x+2)]);
        max = MAX(max, src[(y+2)*srcStride + (x-2)]);
        max = MAX(max, src[(y+2)*srcStride + (x-1)]);
        max = MAX(max, src[(y+2)*srcStride + (x+0)]);
        max = MAX(max, src[(y+2)*srcStride + (x+1)]);
        max = MAX(max, src[(y+2)*srcStride + (x+2)]);            
        dst[y*dstStride + x] = max;
    } 
  }
}


