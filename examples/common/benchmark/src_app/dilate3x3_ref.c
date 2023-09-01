/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "benchmark_ref.h"

#define MAX(a,b)       ((a) > (b) ? (a) : (b))

void dilate3x3_ref (unsigned char *src, int srcStride, int width, int height, unsigned char *dst, int dstStride)
{
  int x, y;
  unsigned char max;
  
  for (y = 1; y < height - 1; y++)
  {
    for (x = 1; x < width - 1; x++)
    {
        max = MAX(src[(y-1)*srcStride + (x-1)], src[(y-1)*srcStride + (x)]);
        max = MAX(max, src[(y-1)*srcStride + (x+1)]);
        max = MAX(max, src[(y)*srcStride + (x-1)]);
        max = MAX(max, src[(y)*srcStride + (x)]);
        max = MAX(max, src[(y)*srcStride + (x+1)]);
        max = MAX(max, src[(y+1)*srcStride + (x-1)]);
        max = MAX(max, src[(y+1)*srcStride + (x)]);
        max = MAX(max, src[(y+1)*srcStride + (x+1)]);
            
        dst[y*dstStride + x] = max;
    } 
  }
}

