/**=============================================================================
Copyright (c) 2014-2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef MEDIAN_ASM_H
#define MEDIAN_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void median3x3PerRow(
    unsigned char   *src,
    int              stride_i,
    int              width,
    unsigned char   *dst
    );

#ifdef __cplusplus
}
#endif

#endif    // MEDIAN_ASM_H
