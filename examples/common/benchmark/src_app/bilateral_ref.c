/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#include <math.h>
#include <stdlib.h>

#define KERNEL_SIZE     9
#define Q               8
#define PRECISION       (1<<Q)
#define VLEN            128

static double getGauss(double sigma,double value)
{
    return exp( -( value/(2.0 * sigma * sigma) ) );
}

void bilateral_ref   (unsigned char   *input,
    int             stride,
    int             width,
    int             height,
    unsigned char   *output
    )
{
    //=============================================================================
    //  Generate coefficients table
    //=============================================================================
    unsigned char gaussLUT[KERNEL_SIZE*KERNEL_SIZE];
    unsigned char rangeLUT[256];
    double sigmaS = 0.6;
    double sigmaR = 0.2;

    // Space gaussian coefficients calculation
    int x, y;
    int center = KERNEL_SIZE/2;
    for(y=-center;y<center+1;y++)
    {
        for(x=-center;x<center+1;x++)
        {
            double y_r = y/(double)KERNEL_SIZE;
            double x_r = x/(double)KERNEL_SIZE;
            double gauss = getGauss(sigmaS,(x_r * x_r + y_r * y_r));
            unsigned char gauss_fixedpoint = gauss*PRECISION - 1;
            gaussLUT[(y+center)*KERNEL_SIZE + x+center] = gauss_fixedpoint;
        }
    }

    // range gaussian coefficients calculation
    for(y=0;y<PRECISION;y++)
    {
        double  y_r = y/(double)PRECISION;
        double     range = getGauss(sigmaR,y_r*y_r);
        unsigned char    range_fixedpoint = range*PRECISION -1;
        rangeLUT[y]=range_fixedpoint;
    }

    int bx, by;
    unsigned char centr, pixel, weight;
    unsigned int  filtered, weights;

    for(y=4; y<height-4; y++)
    {
        for(x=4; x<width-4; x++)
        {
            // block processing
            filtered = 0;
            weights = 0;

            centr = input[y*stride+x];

            for(by=-4; by<=4; by++)
            {
                for(bx=-4; bx<=4; bx++)
                {
                    pixel = input[(y+by)*stride + (x+bx)];
                    weight = (rangeLUT[abs(pixel-centr)]*gaussLUT[(by+4)*9+(bx+4)])>>8;
                    filtered += pixel*weight;
                    weights  += weight;

                }
            }

            output[y*stride+x] = weights ? filtered/weights : 0;
        }
    }

    return;
}
