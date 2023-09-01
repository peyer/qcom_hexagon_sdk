#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "scale.h"


/*=============================================================================*/
/* Function    : scale_DownSampleToHalf                                        */
/*-----------------------------------------------------------------------------*/
/* Description : scale down image to half in both horizontal and vertial       */
/*=============================================================================*/
int scale_DownSampleToHalf ( const unsigned char * __restrict imgSrc,
                             int width,
                             int height,
                             unsigned char * __restrict imgDst    )
{
    // Checks if the source image is of even width and height
    if((width&1)!=0 || (height&1)!=0)
        return 0;


    int dstWidth  = width  >> 1;
    int dstHeight = height >> 1;
    const unsigned char *srcPixels = imgSrc;

    int sum;
    int i, j;

    for(i=0; i<dstHeight; i++)
    {

        srcPixels = imgSrc + 2*i*width;

        for(j=0; j<dstWidth; j++)
        {
            sum = (int)srcPixels[      0] + (int)srcPixels[      1] + \
                  (int)srcPixels[width+0] + (int)srcPixels[width+1];

            srcPixels+=2;

            *imgDst++ = (unsigned char)(sum>>2);
        }

    }

    return 1;
}




/*=============================================================================*/
/* Function    : scale_DownSampleToHalf                                        */
/*-----------------------------------------------------------------------------*/
/* Description : scale down image to quarter in both horizontal and vertial    */
/*=============================================================================*/
int scale_DownSampleToQuarter( const unsigned char * __restrict imgSrc,
                               int width,int height,
                               unsigned char * __restrict imgDst)
{
    // Checks if the source image has a size that is multiple of four
    if((width&3)!=0 || (height&3)!=0)
        return 0;

    int dstWidth  = width  >> 2;
    int dstHeight = height >> 2;
    const unsigned char *srcPixels = imgSrc;

    int sum;
    int i, j;

    for(i=0; i<dstHeight; i++)
    {
        srcPixels = imgSrc + 4*i*width;

        for(j=0; j<dstWidth; j++)
        {
            const unsigned char *p = srcPixels;

            sum  = (int)p[0] + (int)p[1] + (int)p[2] + (int)p[3];
            p += width;

            sum += (int)p[0] + (int)p[1] + (int)p[2] + (int)p[3];
            p += width;

            sum += (int)p[0] + (int)p[1] + (int)p[2] + (int)p[3];
            p += width;

            sum += (int)p[0] + (int)p[1] + (int)p[2] + (int)p[3];
            
            srcPixels += 4;

            *imgDst++ = (unsigned char)(sum>>4);
        }

    }

    return 1;
}

