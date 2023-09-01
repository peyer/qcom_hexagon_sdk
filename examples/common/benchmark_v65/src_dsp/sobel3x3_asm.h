/**=============================================================================
Copyright (c) 2014-2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef SOBEL_ASM_H
#define SOBEL_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void sobelPer2Row(
    unsigned char   *src,
    int              stride_i,
    int              width,
    unsigned char   *dst
    );

#ifdef __cplusplus
}
#endif

#endif    // SOBEL_ASM_H
