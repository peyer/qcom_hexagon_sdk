/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     fcvNCCPatchOnSquare8x8u8                                           ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs ncc using 8x8 template and 18x18 ROI        ]*/
/*[     in an image. Total search points are 11x11.                        ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

//==============================================================================
// Include Files
//==============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "ncc.h"

//==============================================================================
// Macro
//==============================================================================
#define PATCH_SIZE 8
#define PATCH_CENTER (PATCH_SIZE/2)
#define PATCH_NUM_PIXELS 64
#define PATCH_NUM_PIXELS_SHIFT 6   // 2^6 = 64

#define MAX_SEARCH_RADIUS 5
#define SEARCH_SIZE (2*MAX_SEARCH_RADIUS+1)
#define AREA_SIZE (SEARCH_SIZE+PATCH_SIZE)
#define AREA_CENTER (AREA_SIZE/2)

#define DONT_USE_LOW_VARIANCE 0
#define FASTCV_MIN(a,b) ( a > b ? b : a)

//==============================================================================
// Functions
//==============================================================================

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
fcvDotProduct8x8u8C( const unsigned char* __restrict ptch,
                      const unsigned char* __restrict img,
                      unsigned short imgW, unsigned short imgH,
                      int nX, int nY, unsigned int nNum,
                      int* __restrict dotProducts )
{
    int y, chunk = FASTCV_MIN( nNum, 4 );
    unsigned n;

    assert(ptch && img && dotProducts && imgW && imgH);
    assert( PATCH_SIZE==8 );
    for (n=0; n<nNum; n+=chunk )
    {
        if( (nNum - n ) < 4 ) chunk = nNum - n;
        //printf("chunk=%d\n", chunk);
        switch(chunk)
        {
        case 4:
            {
                unsigned int dp0=0,dp1=0,dp2=0,dp3=0;

                const unsigned char* srcPatch = ptch;
                const unsigned char* srcImage = img + nX-PATCH_CENTER +
                    (nY-PATCH_CENTER)*imgW;


                //printf("%d %d %d %d\n",    srcImage[0], srcImage[1],srcImage[2],srcImage[3]);

                for (y=0; y<PATCH_SIZE; y++)
                {
                    unsigned int srcP  = srcPatch[0];
                    unsigned int srcI0 = srcImage[0];
                    unsigned int srcI1 = srcImage[1];
                    unsigned int srcI2 = srcImage[2];
                    unsigned int srcI3 = srcImage[3];

                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;
                    dp2 += srcP*srcI2;
                    dp3 += srcP*srcI3;

                    srcP  = srcPatch[1];
                    srcI0 = srcImage[4];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI2;
                    dp2 += srcP*srcI3;
                    dp3 += srcP*srcI0;

                    srcP  = srcPatch[2];
                    srcI1 = srcImage[5];
                    dp0 += srcP*srcI2;
                    dp1 += srcP*srcI3;
                    dp2 += srcP*srcI0;
                    dp3 += srcP*srcI1;

                    srcP  = srcPatch[3];
                    srcI2 = srcImage[6];
                    dp0 += srcP*srcI3;
                    dp1 += srcP*srcI0;
                    dp2 += srcP*srcI1;
                    dp3 += srcP*srcI2;

                    srcP  = srcPatch[4];
                    srcI3 = srcImage[7];
                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;
                    dp2 += srcP*srcI2;
                    dp3 += srcP*srcI3;

                    srcP  = srcPatch[5];
                    srcI0 = srcImage[8];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI2;
                    dp2 += srcP*srcI3;
                    dp3 += srcP*srcI0;

                    srcP  = srcPatch[6];
                    srcI1 = srcImage[9];
                    dp0 += srcP*srcI2;
                    dp1 += srcP*srcI3;
                    dp2 += srcP*srcI0;
                    dp3 += srcP*srcI1;

                    srcP  = srcPatch[7];
                    srcI2 = srcImage[10];
                    dp0 += srcP*srcI3;
                    dp1 += srcP*srcI0;
                    dp2 += srcP*srcI1;
                    dp3 += srcP*srcI2;

                    srcPatch += PATCH_SIZE;
                    srcImage += imgW;
                }

                dotProducts[0] = (int)dp0;
                dotProducts[1] = (int)dp1;
                dotProducts[2] = (int)dp2;
                dotProducts[3] = (int)dp3;
            }
            break;

        case 3:
            {
                unsigned int dp0=0,dp1=0,dp2=0;

                const unsigned char* srcPatch = ptch;
                const unsigned char* srcImage = img + nX-PATCH_CENTER +
                    (nY-PATCH_CENTER)*imgW;

                for (y=0; y<PATCH_SIZE; y++)
                {
                    unsigned int srcP  = srcPatch[0];
                    unsigned int srcI0 = srcImage[0];
                    unsigned int srcI1 = srcImage[1];
                    unsigned int srcI2 = srcImage[2];

                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;
                    dp2 += srcP*srcI2;

                    srcP  = srcPatch[1];
                    srcI0 = srcImage[3];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI2;
                    dp2 += srcP*srcI0;

                    srcP  = srcPatch[2];
                    srcI1 = srcImage[4];
                    dp0 += srcP*srcI2;
                    dp1 += srcP*srcI0;
                    dp2 += srcP*srcI1;

                    srcP  = srcPatch[3];
                    srcI2 = srcImage[5];
                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;
                    dp2 += srcP*srcI2;

                    srcP  = srcPatch[4];
                    srcI0 = srcImage[6];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI2;
                    dp2 += srcP*srcI0;

                    srcP  = srcPatch[5];
                    srcI1 = srcImage[7];
                    dp0 += srcP*srcI2;
                    dp1 += srcP*srcI0;
                    dp2 += srcP*srcI1;

                    srcP  = srcPatch[6];
                    srcI2 = srcImage[8];
                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;
                    dp2 += srcP*srcI2;

                    srcP  = srcPatch[7];
                    srcI0 = srcImage[9];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI2;
                    dp2 += srcP*srcI0;

                    srcPatch += PATCH_SIZE;
                    srcImage += imgW;
                }

                dotProducts[0] = (int)dp0;
                dotProducts[1] = (int)dp1;
                dotProducts[2] = (int)dp2;
            }
            break;

        case 2:
            {
                unsigned int dp0=0,dp1=0;

                const unsigned char* srcPatch = ptch;
                const unsigned char* srcImage = img + nX-PATCH_CENTER +
                    (nY-PATCH_CENTER)*imgW;

                for (y=0; y<PATCH_SIZE; y++)
                {
                    unsigned int srcP  = srcPatch[0];
                    unsigned int srcI0 = srcImage[0];
                    unsigned int srcI1 = srcImage[1];

                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;

                    srcP  = srcPatch[1];
                    srcI0 = srcImage[2];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI0;

                    srcP  = srcPatch[2];
                    srcI1 = srcImage[3];
                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;

                    srcP  = srcPatch[3];
                    srcI0 = srcImage[4];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI0;

                    srcP  = srcPatch[4];
                    srcI1 = srcImage[5];
                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;

                    srcP  = srcPatch[5];
                    srcI0 = srcImage[6];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI0;

                    srcP  = srcPatch[6];
                    srcI1 = srcImage[7];
                    dp0 += srcP*srcI0;
                    dp1 += srcP*srcI1;

                    srcP  = srcPatch[7];
                    srcI0 = srcImage[8];
                    dp0 += srcP*srcI1;
                    dp1 += srcP*srcI0;

                    srcPatch += PATCH_SIZE;
                    srcImage += imgW;
                }

                dotProducts[0] = (int)dp0;
                dotProducts[1] = (int)dp1;
            }
            break;

        case 1:
            {
                unsigned int dp = 0;

                const unsigned char* srcPatch = ptch;
                const unsigned char* srcImage = img + nX-PATCH_CENTER +
                    (nY-PATCH_CENTER)*imgW;

                assert(PATCH_SIZE==8);

                for (y=0; y<PATCH_SIZE; y++)
                {
                    dp += srcPatch[0]*srcImage[0] + srcPatch[1]*srcImage[1] +
                        srcPatch[2]*srcImage[2] + srcPatch[3]*srcImage[3] +
                        srcPatch[4]*srcImage[4] + srcPatch[5]*srcImage[5] +
                        srcPatch[6]*srcImage[6] + srcPatch[7]*srcImage[7];
                    srcPatch += PATCH_SIZE;
                    srcImage += imgW;
                }

                dotProducts[0] = (int)dp;
            }
            break;
        }

        nX += chunk;
        dotProducts += chunk;
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void
integratePatchu8C( const unsigned char* __restrict src,
                   unsigned int srcWidth,
                   unsigned int srcHeight,
                   unsigned int srcStride,
                   int patchX,
                   int patchY,
                   unsigned patchW,
                   unsigned patchH,
                   unsigned int* __restrict dst,
                   unsigned int* __restrict dst2 )
{
    unsigned i, j, iimgW;
    unsigned char val;
    unsigned int sum, sum2;
    unsigned int* iimg;
    unsigned int* iimgPrev;
    unsigned int* iimg2;
    unsigned int* iimg2Prev;
    const unsigned char* img;

    iimgW = patchW + 1;

    // first pxlC015
    img       = src + srcStride * patchY + patchX;
    iimgPrev  = dst;
    iimg2Prev = dst2;
    iimg      = dst  + iimgW;
    iimg2     = dst2 + iimgW;

    // zero first column
    *iimg++ = *iimg2++ = *iimgPrev++ = *iimg2Prev++ = sum = sum2 = 0;

    for ( j=0; j<patchW; ++j )
    {
        iimgPrev[j] = iimg2Prev[j] = 0;

        val      = img[j];
        sum     += val;
        sum2    += val * val;
        iimg[j]  = sum;
        iimg2[j] = sum2;
    }

    // pointer to previous pxlC015
    iimgPrev  = iimg;
    iimg2Prev = iimg2;
    img      += srcStride;
    iimg     += iimgW;
    iimg2    += iimgW;

    // remaining rows
    for ( i=1; i<patchH; ++i )
    {
        iimg[-1] = iimg2[-1] = sum = sum2 = 0;  // zero first column
        for ( j=0; j<patchW; ++j )
        {
            val      = img[j];
            sum     += val;
            sum2    += val * val;
            iimg[j]  = sum  + iimgPrev[j];
            iimg2[j] = sum2 + iimg2Prev[j];
        }
        iimgPrev  = iimg;
        iimg2Prev = iimg2;
        img      += srcStride;
        iimg     += iimgW;
        iimg2    += iimgW;
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
fcvIntegratePatchu8C( const unsigned char* __restrict src,
                      unsigned int srcWidth,
                      unsigned int srcHeight,
                      unsigned int srcStride,
                      int patchX,
                      int patchY,
                      unsigned patchW,
                      unsigned patchH,
                      unsigned int* __restrict dst,
                      unsigned int* __restrict dst2 )
{
    assert(src && dst && dst2 && srcWidth && srcHeight && (srcStride >= srcWidth));
    integratePatchu8C(src,srcWidth,srcHeight,srcStride,patchX,patchY,patchW,patchH,dst,dst2);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
fcvIntegrateImageLine64u8C( const unsigned char* __restrict pxls,
                             unsigned short* sum, unsigned int* sum2 )
{
    unsigned short i, s=0;
    unsigned int s2=0;
    assert(pxls && sum && sum2);

    for (i=0; i<64; ++i )
    {
        unsigned char p = pxls[i];
        s  += p;
        s2 += p*p;
    }

    *sum = s;
    *sum2 = s2;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void
calcSums( unsigned int* __restrict sumTbl,
          unsigned int* __restrict sum2Tbl, int sumW,
          int x, int y, unsigned int* sum, unsigned int* sum2 )
{
    int x0 = x,
        y0 = y,
        x1 = x + PATCH_SIZE,
        y1 = y + PATCH_SIZE;

    int idx_y0 = y0 * sumW;
    int idx_y1 = y1 * sumW;
    int idx11  = idx_y1 + x1;
    int idx10  = idx_y1 + x0;
    int idx01  = idx_y0 + x1;
    int idx00  = idx_y0 + x0;

    assert( x0 >= 0 );
    assert( y0 >= 0 );
    assert( x1 < sumW );
    assert( y1 < sumW );
    assert( x1 >= x0 );
    assert( y1 >= y0 );

    *sum  =  sumTbl[idx11] +  sumTbl[idx00] -  sumTbl[idx01] -  sumTbl[idx10];
    *sum2 = sum2Tbl[idx11] + sum2Tbl[idx00] - sum2Tbl[idx01] - sum2Tbl[idx10];

    assert( sumTbl[idx11] >= sumTbl[idx00] );
    assert( sumTbl[idx11] >= sumTbl[idx01] );
    assert( sumTbl[idx11] >= sumTbl[idx10] );
    assert( sum2Tbl[idx11] >= sum2Tbl[idx00] );
    assert( sum2Tbl[idx11] >= sum2Tbl[idx01] );
    assert( sum2Tbl[idx11] >= sum2Tbl[idx10] );
    assert( *sum>=0 && *sum<=AREA_SIZE*AREA_SIZE*255 );
    assert( *sum2>=0 && *sum2<=AREA_SIZE*AREA_SIZE*255*255 );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static short
calcNCC( int                  x,
         int                  y,
         int                  denomPatch,
         int                  noms[AREA_SIZE][AREA_SIZE],
         int                  filterLowVariance,
         unsigned int* __restrict sumTbl,
         unsigned int* __restrict sum2Tbl,
         int                  sumW )
{
    unsigned int sum, sum2;
    int denomImage, nom = noms[y][x];
    float ncc;
    short iNCC;

    assert( x < sumW );
    assert( y < sumW );

    // Requires that the noms have already been calculated
    calcSums( sumTbl, sum2Tbl, sumW, x, y, &sum, &sum2 );

    denomImage = sum2 - ((sum*sum) >> PATCH_NUM_PIXELS_SHIFT);

    // filterLowVariance == 0 means: don't use filterLowwVariance value
    // if filterLowVarince != 0, use the value to filter low variance values
    if ((filterLowVariance != 0)  && (denomImage <= filterLowVariance))
        return -127;

    ncc = 128.f * (float)nom /
        (float)sqrt( (float)denomImage * (float)denomPatch ) ;

    iNCC = ncc > 0 ? (short)(ncc+0.5f) : (short)(ncc-0.5f);
    assert( (iNCC >= -128) && (iNCC <= 128) );

    return iNCC;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static float
interpolateParabolicExtremum( short yL, short yCtr, short yR )
{
    short nom, denom;
    short tmp0, tmp1;
    float subX;

    assert( (yL <= yCtr && yCtr >= yR) ||
        (yL >= yCtr && yCtr <= yR) );

    nom = yR - yL;
    tmp0 = (yR + yL) << 1;
    tmp1 = yCtr << 2;
    denom = tmp1 - tmp0;

    subX = denom != 0 ? (float)nom/(float)denom : 0.f;
    assert( subX>-1 && subX<1 );

    return subX;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int
fcvNCCPatchOnSquare8x8u8(  const unsigned char* __restrict ptch8x8,
                           const unsigned char* __restrict img,
                           unsigned short imgW,
                           unsigned short imgH,
                           unsigned short srchX,
                           unsigned short srchY,
                           unsigned short srchW,
                           int filterLowVariance,
                           unsigned short* bestX,
                           unsigned short* bestY,
                           unsigned int* bestNCC,
                           int doSubPixel,
                           float* subX,
                           float* subY,
                           unsigned char* scratch
)
{
    int dots[SEARCH_SIZE+1]; // need to align to 128
    int (*noms)[AREA_SIZE];
    unsigned short patchSum, nSide, areaExt, patchW, sumW, xLT, yLT;
    unsigned int patchSum2, *sumTbl, *sum2Tbl;
    float bestDenomImage, bestNomnom;
    int bestFound, yN, xN, row, col, denomPatch, x, y, xBest, yBest;
    short bstNCC;

    assert( ptch8x8 && img && bestX && bestY && bestNCC &&
        ((doSubPixel == 0) || ((subX) && (subY))) &&
        imgW && imgH && srchX && srchY && srchW);

    *bestX = *bestY = 0;
    *bestNCC = 0;

    nSide = srchW >> 1;

    if( nSide > AREA_CENTER )
    {
        return 1;
    }

    areaExt = nSide + PATCH_CENTER;

    if( (srchX < areaExt) || (srchX > imgW-areaExt) ||
        (srchY < areaExt) || (srchY > imgH-areaExt) )
    {
        return 2;
    }

    sumTbl = (unsigned int *)malloc( AREA_SIZE * AREA_SIZE * sizeof( unsigned int ));
    //assert( sumTbl );
    assert(sumTbl);

    sum2Tbl = (unsigned int *)malloc( AREA_SIZE * AREA_SIZE * sizeof( unsigned int ));
    //assert( sum2Tbl );
    assert( sum2Tbl );

    noms = (int (*)[AREA_SIZE])malloc( AREA_SIZE * AREA_SIZE * sizeof( int ));
    //assert( noms );
    assert( noms );

    patchW  = (areaExt << 1);
    sumW    = patchW + 1;
    //assert( sumW <= AREA_SIZE);
    assert( sumW <= AREA_SIZE);

    // Build the summed area tables of pixel and squared pixel values
    xLT = (short)(srchX - areaExt);
    yLT = (short)(srchY - areaExt);

    // changed this to use C version only

    fcvIntegratePatchu8C      ( img, imgW, imgH, imgW, xLT, yLT, patchW, patchW, sumTbl, sum2Tbl );
    fcvIntegrateImageLine64u8C( ptch8x8, &patchSum, &patchSum2 );


    if (filterLowVariance != 0)
    {
        int denomPatch = patchSum2 - ((patchSum*patchSum)>>PATCH_NUM_PIXELS_SHIFT);

        if (denomPatch < filterLowVariance)
        {
            *bestNCC = (unsigned int)-127;
            free( sumTbl );
            free( sum2Tbl );
            free( noms );
            return 4;
        }
    }

    // Sweep over the search area and match using NCC
    bestDenomImage=1, bestNomnom=1;
    bestFound = 0;

    for (yN=-nSide, row = 0; yN<=nSide; ++yN, ++row)
    {
        int tstX = -nSide;

        int xFrom =  tstX,
            xTo = -tstX;
        int nNum = xTo - xFrom + 1;

        for (xN=xTo+1; xN<=nSide; ++xN )
        {
            noms[row][xN+nSide]  =
                noms[row][-xN+nSide] = -128;
        }

        // Use C version only
        fcvDotProduct8x8u8C( ptch8x8, img, imgW, imgH, srchX+xFrom, srchY+yN,
            nNum, dots );


        for (xN=xFrom, col=0; xN<=xTo; ++xN, ++col)
        {
            // Calculate the sums from summed-area-table and the dot product
            unsigned int sum, sumsum;
            int dp, nom, denomImage;
            float nomnom, refLeft, refRight;
            calcSums( sumTbl, sum2Tbl, sumW, xN+nSide, yN+nSide, &sum, &sumsum );

            dp = dots[col];
            nom = dp - ((patchSum*sum) >> PATCH_NUM_PIXELS_SHIFT);


            if( nom <= 0 )  // Negative NCC is definitely not good enough...
            {
                noms[row][xN+nSide] = 0;
                continue;
            }
            noms[row][xN+nSide] = nom;

            denomImage = sumsum - ((sum*sum) >> PATCH_NUM_PIXELS_SHIFT);

            //assert( denomImage > 0 );
            assert( denomImage > 0 );

            // Instead of dividing by denomImage we multiply it on the other
            // side of the equation
            nomnom = (float)nom;
            nomnom *= nomnom;

            // printf("nomnom %f denom %f \n", nomnom, (float)denomImage);

            refLeft = nomnom * bestDenomImage;
            refRight = bestNomnom * (float)denomImage;
            //assert( refLeft>=0 && refRight>=0 );   // try to catch overflows
            assert( refLeft>=0 && refRight>=0 );   // try to catch overflows
            //printf("refLeft %f refRight %f \n", refLeft, refRight);
            if( (refLeft > refRight) || (bestFound == 0) )
            {
                bestFound = 1;
                bestNomnom = nomnom;
                bestDenomImage = (float)denomImage;
                *bestX = (unsigned short)(xN + (int)srchX);
                *bestY = (unsigned short)(yN + (int)srchY);
            }
        }
    }



    if( bestNomnom == 1.f )
    {
        free( sumTbl );
        free( sum2Tbl );
        free( noms );
        return 3;
    }

    // Compute the NCC corresponding to the best location
    denomPatch = patchSum2 - ((patchSum*patchSum) >> PATCH_NUM_PIXELS_SHIFT);


    xBest = *bestX - srchX,
        yBest = *bestY - srchY;
    x = xBest + nSide,
        y = yBest + nSide;

    bstNCC = calcNCC( x, y, denomPatch, noms, filterLowVariance,
        sumTbl, sum2Tbl, sumW );

    *bestNCC = bstNCC;

    if ((filterLowVariance != 0) && (bstNCC == -127))
    {
        free( sumTbl );
        free( sum2Tbl );
        free( noms );
        return 5;
    }

    if( doSubPixel )
    {
        // We don't use filterLowVariance here, because we already decided that
        // the location is ok and we don't want error codes to mess up the
        // sub-pixel refinement!
        //
        *subX = *subY = 0.f;

        if( (xBest > -nSide) &&
            (xBest < nSide-1) &&
            (noms[y][x-1] > 0) &&
            (noms[y][x+1] > 0) )
        {
            int lNCC = calcNCC( x-1, y, denomPatch, noms, DONT_USE_LOW_VARIANCE,
                sumTbl, sum2Tbl, sumW );
            int rNCC = calcNCC( x+1, y, denomPatch, noms, DONT_USE_LOW_VARIANCE,
                sumTbl, sum2Tbl, sumW );
            *subX = interpolateParabolicExtremum( (short)lNCC, (short)*bestNCC, (short)rNCC );
        }

        if( (yBest > -nSide) &&
            (yBest < nSide-1) &&
            (noms[y-1][x] > 0) &&
            (noms[y+1][x] > 0) )
        {
            int tNCC = calcNCC( x, y-1, denomPatch, noms, DONT_USE_LOW_VARIANCE,
                sumTbl, sum2Tbl, sumW );
            int bNCC = calcNCC( x, y+1, denomPatch, noms, DONT_USE_LOW_VARIANCE,
                sumTbl, sum2Tbl, sumW );
            *subY = interpolateParabolicExtremum( (short)tNCC, (short)*bestNCC, (short)bNCC );
        }
    }

    free( sumTbl );
    free( sum2Tbl );
    free( noms );

    return 0;
}
