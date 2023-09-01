// this file contains a C implementation of the assembly function down2. This is for
// illustration/comparative profiling only, and should not be linked into the image
// unless the assembly version is removed



#include "downscaleBy2_asm.h"
#include <stdio.h>

void
down2( const unsigned  char *imgSrc, unsigned int width, unsigned int height, 
    unsigned int stride, unsigned  char *imgDst, unsigned int dstStride)
{
    unsigned int i,j;
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            *imgDst++ = (imgSrc[i*stride + j] + imgSrc[(i+1)*stride + j] + imgSrc[i*stride + (j+1)] + imgSrc[(i+1)*stride + (j+1)]) / 4;
        }
        imgDst += (dstStride - (width / 2));
    }
    return;
}

void
down2_hvx( const unsigned  char *imgSrc, unsigned int width, unsigned int height, 
    unsigned int stride, unsigned  char *imgDst, unsigned int dstStride, unsigned int VLEN)
{
    down2(imgSrc, width, height, stride, imgDst, dstStride);
}
