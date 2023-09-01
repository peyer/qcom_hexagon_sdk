/**=============================================================================
Copyright (c) 2014-2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef FAST9_ASM_H
#define FAST9_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void fast9_detect_coarse(
    unsigned char       *img,
    unsigned int         xsize,
    unsigned int         stride,
    unsigned int         barrier,
    unsigned int        *bitmask,
    unsigned int         boundary
    );

int fast9_detect_fine(
    unsigned char       *img,
    unsigned int         num_pixels32,
    unsigned int         stride,
    unsigned int         barrier,
    unsigned int        *bitmask,
    short int           *xpos,
    int                  xstart
    );

#ifdef __cplusplus
}
#endif

#endif    // FAST9_ASM_H
