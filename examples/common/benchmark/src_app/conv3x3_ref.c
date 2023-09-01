/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

void conv3x3_ref (unsigned char *src, int srcStride, int width, int height, 
    const signed char *mask, int shift, unsigned char *dst, int dstStride)
{
  int x, y;
  int sum;
  
  for (y = 1; y < height - 1; y++)
  {
    for (x = 1; x < width - 1; x++)
    {
        sum = src[(y-1)*srcStride + (x-1)] * mask[0]
            + src[(y-1)*srcStride + (x)] * mask[1]
            + src[(y-1)*srcStride + (x+1)] * mask[2]
            + src[(y)*srcStride + (x-1)] * mask[4]
            + src[(y)*srcStride + (x)] * mask[5]
            + src[(y)*srcStride + (x+1)] * mask[6]
            + src[(y+1)*srcStride + (x-1)] * mask[8]
            + src[(y+1)*srcStride + (x)] * mask[9]
            + src[(y+1)*srcStride + (x+1)] * mask[10];
            
        sum = sum >> shift;     // shift
        dst[y*dstStride + x] = (sum > 255) ? 255 : (sum < 0 ? 0 : sum);   // sat
    } 
  }
}

