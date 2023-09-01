/**=============================================================================
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "benchmark_ref.h"

#define MAX(a,b)       ((a) > (b) ? (a) : (b))
#define MIN(a,b)       ((a) < (b) ? (a) : (b))

// 16x8 patches (assumed to be 2-byte aligned)
#define PATCH_WID    16
#define PATCH_HGT    8

void gather_ref (const uint8_t* src, uint8_t* dst, int width, int height, int nPatches, int horizStep, int vertStep)
{
    for (int i = 0; i < nPatches; i++)
    {
        int x = (i * horizStep) % width;
        int y = (i * vertStep) % height;
        x = MIN(x, width - PATCH_WID);
        y = MIN(y, height - PATCH_HGT);
        
        for (int yy = 0; yy < PATCH_HGT; yy++)
        {
            for (int xx = 0; xx < PATCH_WID; xx++)
            {
                *dst++ = src[(y+yy) * width + (x+xx)];
            }
        }
    }
    
}

void scatter_ref (const uint8_t* src, uint8_t* dst, int width, int height, int nPatches, int horizStep, int vertStep)
{
    for (int i = 0; i < nPatches; i++)
    {
        int x = (i * horizStep) % width;
        int y = (i * vertStep) % height;
        x = MIN(x, width - PATCH_WID);
        y = MIN(y, height - PATCH_HGT);
        
        for (int yy = 0; yy < PATCH_HGT; yy++)
        {
            for (int xx = 0; xx < PATCH_WID; xx++)
            {
                dst[(y+yy) * width + (x+xx)] = *src++;
            }
        }
    }
    
}

