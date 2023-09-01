/**=============================================================================
Copyright (c) 2016, 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef BENCHMARK_ASM_H
#define BENCHMARK_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

void dilate5x5_ref (unsigned char *src, int srcStride, int width, int height, unsigned char *dst, int dstStride);

void dilate3x3_ref (unsigned char *src, int srcStride, int width, int height, unsigned char *dst, int dstStride);

void integrate_ref(unsigned char *src, int srcStride, int width, int height, unsigned int *dst, int dstStride);

void epsilon_ref (unsigned char *src, int stride, int width, int height, int threshold, unsigned char *dst);

void bilateral_ref (unsigned char *input, int stride, int width, int height, unsigned char   *output);

void fast9_ref(const unsigned char *im, unsigned int stride, unsigned int xsize, unsigned int ysize,
    unsigned int barrier, unsigned int border, short int *xy, int maxnumcorners, int *numcorners);

void conv3x3_ref (unsigned char *src, int srcStride, int width, int height, const signed char *mask, 
    int shift, unsigned char *dst, int dstStride);
    
void gaussian7x7_ref(unsigned char *src, int width, int height, int stride, unsigned char *dst, int dstStride);

void sobel3x3_ref (unsigned char *src, int srcStride, int width, int height, unsigned char *dst, int dstStride);

int fft1024_ref (const uint8_t* src, int32_t* dst);

void gather_ref (const uint8_t* src, uint8_t* dst, int width, int height, int nPatches, int horizStep, int vertStep);
          
void scatter_ref (const uint8_t* src, uint8_t* dst, int width, int height, int nPatches, int horizStep, int vertStep);
          
void crash10_ref (unsigned char *input, int stride, int width, int height, unsigned char   *output);

void warp_ref(unsigned char *src, int srcStride, int srcWidth, int srcHeight, int32_t *gridarr,  unsigned char *dst, int dstStride);

int constructMeshFromHomography(float const hMat[9], float in_scale, int grid_pitch, int gridW, int gridH, int32_t *gridArr, int gridStride, int use_frame_origin); 


#ifdef __cplusplus
}
#endif

#endif
