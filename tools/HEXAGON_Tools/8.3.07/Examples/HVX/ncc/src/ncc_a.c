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
#if defined(__hexagon__)
#include "hexagon_types.h"
#endif

#include "hvx.cfg.h"
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

//==============================================================================
// Declarations
//==============================================================================
void
integrate64u8(
    const unsigned char* __restrict pxls,
    unsigned short* sum,
    unsigned int* sum2
);

void computecrossvar(
    unsigned char * ptch8x8,
    unsigned char * img,
    short stride,
    unsigned short patchSum,
    int *noms,
    int *denoms
);

int searchbestncc(
    int * nom,
    int * denom
);

/* ======================================================================== */
/*  Tables used in Assembly                                                 */
/* ======================================================================== */
int ncc_initloc[VLEN/sizeof(int)] __attribute__((aligned(VLEN))) =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
#if VLEN == 128
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
#endif
};

unsigned char ncc_ctrl07[2*VLEN] __attribute__((aligned(VLEN))) =
{
    0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x05, 0x0e, 0x0e, 0x0a, 0x0a, 0x09, 0x0b, 0x09, 0x0f,
    0x1c, 0x14, 0x14, 0x14, 0x17, 0x11, 0x13, 0x11, 0x12, 0x12, 0x16, 0x1e, 0x1d, 0x1f, 0x1d, 0x1b,
    0x28, 0x28, 0x28, 0x28, 0x2b, 0x25, 0x27, 0x25, 0x2e, 0x2e, 0x22, 0x22, 0x21, 0x23, 0x21, 0x2f,
    0x24, 0x24, 0x24, 0x34, 0x3f, 0x39, 0x3b, 0x39, 0x3a, 0x3a, 0x3e, 0x3e, 0x3d, 0x3f, 0x35, 0x33,
#if LOG2VLEN == 7
    0x40, 0x40, 0x40, 0x40, 0x43, 0x45, 0x47, 0x45, 0x4e, 0x4e, 0x4a, 0x4a, 0x49, 0x4b, 0x49, 0x4f,
    0x5c, 0x54, 0x54, 0x54, 0x57, 0x51, 0x53, 0x51, 0x52, 0x52, 0x56, 0x5e, 0x5d, 0x5f, 0x5d, 0x5b,
    0x68, 0x68, 0x68, 0x68, 0x6b, 0x65, 0x67, 0x65, 0x6e, 0x6e, 0x62, 0x62, 0x61, 0x63, 0x61, 0x6f,
    0x64, 0x64, 0x64, 0x74, 0x7f, 0x79, 0x7b, 0x79, 0x7a, 0x7a, 0x7e, 0x7e, 0x7d, 0x7f, 0x75, 0x73,
#endif
    0x0c, 0x04, 0x04, 0x04, 0x07, 0x01, 0x03, 0x01, 0x02, 0x02, 0x06, 0x0e, 0x0d, 0x0f, 0x0d, 0x0b,
    0x18, 0x18, 0x18, 0x18, 0x1b, 0x15, 0x17, 0x15, 0x1e, 0x1e, 0x12, 0x12, 0x11, 0x13, 0x11, 0x1f,
    0x34, 0x34, 0x34, 0x24, 0x2f, 0x29, 0x2b, 0x29, 0x2a, 0x2a, 0x2e, 0x2e, 0x2d, 0x2f, 0x25, 0x23,
    0x20, 0x20, 0x20, 0x20, 0x23, 0x25, 0x27, 0x35, 0x3e, 0x3e, 0x3a, 0x3a, 0x39, 0x3b, 0x39, 0x3f,
#if LOG2VLEN == 7
    0x4c, 0x44, 0x44, 0x44, 0x47, 0x41, 0x43, 0x41, 0x42, 0x42, 0x46, 0x4e, 0x4d, 0x4f, 0x4d, 0x4b,
    0x58, 0x58, 0x58, 0x58, 0x5b, 0x55, 0x57, 0x55, 0x5e, 0x5e, 0x52, 0x52, 0x51, 0x53, 0x51, 0x5f,
    0x74, 0x74, 0x74, 0x64, 0x6f, 0x69, 0x6b, 0x69, 0x6a, 0x6a, 0x6e, 0x6e, 0x6d, 0x6f, 0x65, 0x63,
    0x60, 0x60, 0x60, 0x60, 0x63, 0x65, 0x67, 0x75, 0x7e, 0x7e, 0x7a, 0x7a, 0x79, 0x7b, 0x79, 0x7f,
#endif
};

//==============================================================================
// Functions
//==============================================================================

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
short
calcNCC(
    int                  denomPatch,
    int                  nom,
    int                  denom,
    int                  filterLowVariance,
    float                r
)
{
    float ncc;
    short iNCC;

    // filterLowVariance == 0 means: don't use filterLowwVariance value
    // if filterLowVarince != 0, use the value to filter low variance values
    if ((filterLowVariance != 0)  && (denom <= filterLowVariance))
        return -127;

    ncc = 128.f * (float)nom / (float)sqrt( (float)denom * (float)denomPatch ) ;

    iNCC = ncc > 0 ? (short)(ncc+r) : (short)(ncc-r);
    assert( (iNCC >= -128) && (iNCC <= 128) );

    return iNCC;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int
fcvNCCPatchOnSquare8x8u8(
    const unsigned char* __restrict ptch8x8,
    const unsigned char* __restrict img,
    unsigned short            imgW,
    unsigned short            imgH,
    unsigned short            srchX,
    unsigned short            srchY,
    unsigned short            srchW,
    int                       filterLowVariance,
    unsigned short*           bestX,
    unsigned short*           bestY,
    unsigned int*             bestNCC,
    int                       doSubPixel,
    float*                    subX,
    float*                    subY,
    unsigned char*            scratch
)
{
    unsigned int patchSum2;
    unsigned short patchSum, nSide = srchW >> 1;
    float r = .5;
    unsigned short areaExt = nSide + PATCH_CENTER;
    short xLT, yLT, bstNCC;
    int denomPatch, xBest, yBest, ind, best, bX, bY, bestFound;
    int *noms, *denoms;

    assert( ptch8x8 && img && bestX && bestY && bestNCC  &&
        imgW && imgH && srchX && srchY && srchW && scratch);
    *bestX = *bestY = 0;
    *bestNCC = 0;

    if( nSide > AREA_CENTER ) {
        return 1;
    }

    if( (srchX < areaExt) || (srchX > imgW-areaExt) || (srchY < areaExt) || (srchY > imgH-areaExt) ) {
        return 2;
    }

    noms = (int*)scratch;
    denoms = noms + (11*11+VLEN/4-1)/(VLEN/4)*(VLEN/4)+(VLEN/4);

    bX    = 0;
    bY    = 0;
    bstNCC= 0;

    xLT = (short)(srchX - areaExt);
    yLT = (short)(srchY - areaExt);

    integrate64u8( ptch8x8, &patchSum, &patchSum2);

    denomPatch = patchSum2 - ((patchSum* patchSum)>>PATCH_NUM_PIXELS_SHIFT);

    if (filterLowVariance != 0)
    {
        if (denomPatch < filterLowVariance)
        {
            *bestNCC = (unsigned int)-127;
            free(noms);
            return 4;
        }
    }

    computecrossvar((unsigned char*)ptch8x8, (unsigned char*)(img+xLT+(yLT)*imgW), imgW, patchSum, noms, denoms);

    bestFound = 0;
    best = searchbestncc(noms, denoms);
    bY = best >> 16;
    bX = best & 0xffff;

    if( best < 0 )  {
        *bestX = 0;
        *bestY = 0;
        return -best;

    } else {
        *bestX = (short)bX+srchX-5;
        *bestY = (short)bY+srchY-5;
    }

    xBest = *bestX - srchX;
    yBest = *bestY - srchY;


    ind = SEARCH_SIZE*(yBest + nSide)+xBest + nSide;
    bstNCC = calcNCC(denomPatch, noms[ind], denoms[ind], filterLowVariance, r);

    *bestNCC = bstNCC;
    if ((filterLowVariance != 0) && (bstNCC == -127))   {
        bestFound = 5;
    } else {
        bestFound = 0;
    }

    // Note: Removed sub-pixel
    assert(doSubPixel == 0);
    return bestFound;
}

