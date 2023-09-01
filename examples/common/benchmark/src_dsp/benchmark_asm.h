/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef BENCHMARK_ASM_H
#define BENCHMARK_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void dilate5x5Per2Row(unsigned char *src, int stride_i, int width, unsigned char *dst, int stride_o);

void dilate3x3Per2Row(unsigned char *src, int stride_i, int width, unsigned char *dst, int stride_o);

void IntegrateRow(unsigned char *restrict src, int width, int stride_i, unsigned int *restrict dst, 
    int stride_o, unsigned int *restrict preint);

void IntegrateRowAcc(unsigned char *restrict src, int width, int stride_i,
    unsigned int *restrict dst, int stride_o, unsigned int *restrict preint);

void epsilonFiltPerRow(unsigned char  *src, int stride, int width, int threshold, 
    unsigned char  *dst);

void bilateral9x9PerRow(unsigned char *src, int stride, int width, unsigned char *dst);
void bilateral9x9PerRow_v65(unsigned char  *src, int stride, int width, 
    unsigned char  *gaussLUT, unsigned char  *rangeLUT, unsigned char  *dst, 
    unsigned char  *tmpbuf);
    
void fast9_detect_coarse(unsigned char *img, unsigned int xsize, unsigned int stride,
    unsigned int barrier, unsigned int *bitmask, unsigned int boundary);

int fast9_detect_fine(unsigned char *img, unsigned int num_pixels32, unsigned int stride,
    unsigned int barrier, unsigned int *bitmask, short int *xpos, int xstart);
    
void conv3x3Per2Row(unsigned char *inp, int stride_i, int width, signed char *mask,
    int shift, unsigned char *outp, int stride_o);

void Gaussian7x7u8PerRow(unsigned char **psrc, int width, unsigned char *dst, int VLEN);

void sobelPer2Row(unsigned char *src, int stride_i, int width, unsigned char *dst);

#ifdef __cplusplus
}
#endif

#endif    // BENCHMARK_ASM_H
